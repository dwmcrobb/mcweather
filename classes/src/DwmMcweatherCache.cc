//===========================================================================
// @(#) $DwmPath$
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2020, 2022
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//  3. The names of the authors and copyright holders may not be used to
//     endorse or promote products derived from this software without
//     specific prior written permission.
//
//  IN NO EVENT SHALL DANIEL W. MCROBB BE LIABLE TO ANY PARTY FOR
//  DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
//  INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE,
//  EVEN IF DANIEL W. MCROBB HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
//
//  THE SOFTWARE PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND
//  DANIEL W. MCROBB HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
//  UPDATES, ENHANCEMENTS, OR MODIFICATIONS. DANIEL W. MCROBB MAKES NO
//  REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER
//  IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE,
//  OR THAT THE USE OF THIS SOFTWARE WILL NOT INFRINGE ANY PATENT,
//  TRADEMARK OR OTHER RIGHTS.
//===========================================================================

//---------------------------------------------------------------------------
//!  \file DwmMcweatherCache.cc
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::Cache class implementation
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/stat.h>
}

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "DwmStreamIO.hh"
#include "DwmSysLogger.hh"
#include "DwmMcweatherCache.hh"
#include "DwmMcweatherUtils.hh"
#include "DwmMcweatherWebUtils.hh"

namespace Dwm {

  namespace Mcweather {

    using namespace std;

    namespace fs = std::filesystem;
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::GetPointInfo(float latitude, float longitude,
                             PointInfo & info)
    {
      bool  rc = false;
      std::string  pointCacheDir = CacheDir(latitude, longitude);
      if (EnsureDirExists(pointCacheDir)) {
        fs::path  pointCache(pointCacheDir + "/point");
        if (fs::exists(pointCache) && fs::is_regular_file(pointCache)) {
          struct stat  statbuf;
          if (0 == stat(pointCache.string().c_str(), &statbuf)) {
            time_t  now = time((time_t *)0);
            if ((statbuf.st_mtime + 604800) > now) {
              //  cache is recent (1 week old or less).
              ifstream  is(pointCache.string());
              if (is) {
                if (info.Read(is)) {
                  rc = true;
                }
                else {
                  Syslog(LOG_ERR, "Failed to read point cache");
                }
                is.close();
              }
              else {
                Syslog(LOG_ERR,
                       "Failed to open points cache for reading: %m");
              }
            }
            else {
              //  cache is older than 1 week, should refresh it.
              if (Utils::GetPointInfo(latitude, longitude, info)) {
                rc = true;
                ofstream  os(pointCache.string());
                if (os) {
                  if (! info.Write(os)) {
                    Syslog(LOG_ERR, "Failed to save point cache");
                  }
                  os.close();
                }
              }
              else {
                Syslog(LOG_ERR, "Failed to get point info");
              }
            }
          }
          else {
            // stat() failed despite file existing.
            Syslog(LOG_ERR, "stat('%s',%p) failed: %m",
                   pointCache.string().c_str());
          }
        }
        else {
          //  points cache doesn't exist.
          if (Utils::GetPointInfo(latitude, longitude, info)) {
            rc = true;
            ofstream  os(pointCache.string());
            if (os) {
              if (! info.Write(os)) {
                Syslog(LOG_ERR, "Failed to save point cache");
              }
              os.close();
            }
          }
          else {
            Syslog(LOG_ERR, "Failed to get point info");
          }
        }
      }
      else {
        //  Error: cache dir doesn't exist and we couldn't create it.
        Syslog(LOG_INFO, "'%s' doesn't exist and can't be created",
               pointCacheDir.c_str());
        rc = Utils::GetPointInfo(latitude, longitude, info);
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool
    Cache::GetObservationStations(float latitude, float longitude,
                                  vector<pair<string,string>> & stations)
    {
      stations.clear();
      bool  rc = false;
      std::string  pointCacheDir = CacheDir(latitude, longitude);
      if (EnsureDirExists(pointCacheDir)) {
        fs::path  stationCache(pointCacheDir + "/observationStations");
        if (fs::exists(stationCache) && fs::is_regular_file(stationCache)) {
          struct stat  statbuf;
          if (0 == stat(stationCache.string().c_str(), &statbuf)) {
            time_t  now = time((time_t *)0);
            if ((statbuf.st_mtime + 604800) > now) {
              //  cache is recent (1 week old or less).
              ifstream  is(stationCache.string());
              if (is) {
                if (StreamIO::Read(is, stations)) {
                  rc = true;
                }
                else {
                  Syslog(LOG_ERR,
                         "Failed to read observation station cache");
                }
                is.close();
              }
              else {
                Syslog(LOG_ERR,
                       "Failed to open observation station cache for "
                       " reading: %m");
              }
            }
            else {
              //  cache is old.  Refresh it.
              PointInfo  pointInfo;
              if (GetPointInfo(latitude, longitude, pointInfo)) {
                if (Utils::GetObservationStations(pointInfo,
                                                         stations)) {
                  rc = true;
                  ofstream  os(stationCache.string());
                  if (os) {
                    if (! StreamIO::Write(os, stations)) {
                      Syslog(LOG_ERR,
                             "Failed to save observation station cache");
                    }
                    os.close();
                  }
                  else {
                    Syslog(LOG_ERR, "Failed to open '%s': %m",
                           stationCache.string().c_str());
                  }
                }
                else {
                  Syslog(LOG_ERR, "Failed to fetch observation stations");
                }
              }
              else {
                Syslog(LOG_ERR, "Failed to get point info for %f,%f",
                       latitude, longitude);
              }
            }
          }
          else {
            // stat() failed despite file existing.
            Syslog(LOG_ERR, "stat('%s',%p) failed: %m",
                   stationCache.string().c_str());
          }
        }
        else {
          //  Observation station cache doesn't exist.
          PointInfo  pointInfo;
          if (GetPointInfo(latitude, longitude, pointInfo)) {
            if (Utils::GetObservationStations(pointInfo, stations)) {
              rc = true;
              ofstream  os(stationCache.string());
              if (os) {
                if (! StreamIO::Write(os, stations)) {
                  Syslog(LOG_ERR,
                         "Failed to save observation station cache");
                }
                os.close();
              }
              else {
                Syslog(LOG_ERR, "Failed to open '%s': %m",
                       stationCache.string().c_str());
              }
            }
            else {
              Syslog(LOG_ERR, "Failed to fetch observation stations");
            }
          }
          else {
            Syslog(LOG_ERR, "Failed to get point info for %f,%f",
                   latitude, longitude);
          }
        }
      }
      else {
        //  Error: cache dir doesn't exist and we couldn't create it.
        Syslog(LOG_INFO, "'%s' doesn't exist and can't be created",
               pointCacheDir.c_str());
        PointInfo  pointInfo;
        if (GetPointInfo(latitude, longitude, pointInfo)) {
          rc = Utils::GetObservationStations(pointInfo, stations);
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::EnsureDirExists(const string & s)
    {
      bool             rc = false;
      fs::path         dirPath(s);
      std::error_code  ec;
      if (fs::exists(dirPath, ec)) {
        if (fs::is_directory(dirPath, ec)) {
          rc = true;
        }
      }
      else {
        rc = fs::create_directories(dirPath, ec);
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    string Cache::CacheDir(float latitude, float longitude)
    {
      string  rc;
      char  *home = getenv("HOME");
      if (home) {
        rc = home;
        rc += "/.mcweather/weatherCache/";
        ostringstream  os;
        os << setprecision(6) << latitude << ',' << longitude << '/';
        rc += os.str();
      }
      return rc;
    }
      
  }  // namespace Mcweather

}  // namespace Dwm
