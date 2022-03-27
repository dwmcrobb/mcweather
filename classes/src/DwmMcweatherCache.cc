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
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
}

#include <chrono>
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

    mutex  Cache::_pointInfoMtx;
    mutex  Cache::_obsStationMtx;
    mutex  Cache::_currentCondMtx;
    mutex  Cache::_periodForecastsMtx;
    mutex  Cache::_hourlyForecastsMtx;
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    Cache::Cache(const std::string & cacheDir, const WeatherConfig & config)
        : _cacheDir(cacheDir), _config(config)
    {
      string    pointCacheDir(PointCacheDir());
      
      fs::path  pointPath(pointCacheDir + "/point");
      LoadCacheFile(_pointInfo, _pointInfoMtx, pointPath);

      fs::path  stationsPath(pointCacheDir + "/observationStations");
      LoadCacheFile(_observationStations, _obsStationMtx, stationsPath);

      fs::path  conditionsPath(pointCacheDir + "/currentConditions");
      LoadCacheFile(_currentConditions, _currentCondMtx, conditionsPath);

      fs::path  forecastsPath(pointCacheDir + "/periodForecasts");
      LoadCacheFile(_periodForecasts, _periodForecastsMtx, forecastsPath);

      fs::path  hourlyPath(pointCacheDir + "/hourlyForecasts");
      LoadCacheFile(_hourlyForecasts, _hourlyForecastsMtx, hourlyPath);
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::GetCurrentConditions(std::map<std::string, CurrentConditions> & currentConditions) const
    {
      bool      rc = false;
      fs::path  p(PointCacheDir() + "/currentConditions");
      lock_guard<mutex>  lock(_currentCondMtx);
      if (LastWriteTime(p) > 0) {
        currentConditions = _currentConditions;
        rc = true;
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::SaveCurrentConditions() const
    {
      fs::path  p(PointCacheDir() + "/currentConditions");
      return SaveCacheFile(_currentConditions, _currentCondMtx, p);
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::SetConditions(const CurrentConditions & conditions)
    {
      bool  rc = true;
      bool  updated = false;
      {
        lock_guard<mutex>  lock(_currentCondMtx);
        auto  it = _currentConditions.find(conditions.Station());
        if (it != _currentConditions.end()) {
          if (it->second != conditions) {
            it->second = conditions;
            updated = true;
          }
        }
        else {
          _currentConditions[conditions.Station()] = conditions;
          updated = true;
        }
      }
      
      if (updated) {
        ClearOldConditions();
        rc = SaveCurrentConditions();
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void Cache::ClearOldConditions()
    {
      std::time_t  now = time((time_t *)0);
      for (auto it = _currentConditions.begin();
           it != _currentConditions.end(); ) {
        if (now > (it->second.Timestamp() + 3600)) {
          it = _currentConditions.erase(it);
        }
        else {
          ++it;
        }
      }
      return;
    }
            
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    time_t Cache::AgeOfCurrentConditions() const
    {
      fs::path  p(PointCacheDir() + "/currentConditions");
      return AgeOfFile(p);
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::GetPeriodForecasts(PeriodForecasts & forecasts) const
    {
      bool      rc = false;
      fs::path  p(PointCacheDir() + "/periodForecasts");
      lock_guard<mutex>  lock(_periodForecastsMtx);
      if (LastWriteTime(p) > 0) {
        forecasts = _periodForecasts;
        rc = true;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::SetPeriodForecasts(const PeriodForecasts & forecasts)
    {
      {
        lock_guard<mutex>  lock(_periodForecastsMtx);
        _periodForecasts = forecasts;
      }
      return SavePeriodForecasts();
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::SavePeriodForecasts() const
    {
      fs::path  p(PointCacheDir() + "/periodForecasts");
      return SaveCacheFile(_periodForecasts, _periodForecastsMtx, p);
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    time_t Cache::AgeOfPeriodForecasts() const
    {
      fs::path  p(PointCacheDir() + "/periodForecasts");
      return AgeOfFile(p);
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::GetHourlyForecasts(PeriodForecasts & forecasts) const
    {
      bool      rc = false;
      fs::path  p(PointCacheDir() + "/hourlyForecasts");
      lock_guard<mutex>  lock(_hourlyForecastsMtx);
      if (LastWriteTime(p) > 0) {
        forecasts = _hourlyForecasts;
        rc = true;
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::SetHourlyForecasts(const PeriodForecasts & forecasts)
    {
      {
        lock_guard<mutex>  lock(_hourlyForecastsMtx);
        _hourlyForecasts = forecasts;
      }
      return SaveHourlyForecasts();
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::SaveHourlyForecasts() const
    {
      fs::path  p(PointCacheDir() + "/hourlyForecasts");
      return SaveCacheFile(_hourlyForecasts, _hourlyForecastsMtx, p);
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    time_t Cache::AgeOfHourlyForecasts() const
    {
      fs::path  p(PointCacheDir() + "/hourlyForecasts");
      return AgeOfFile(p);
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::GetPointInfo(PointInfo & info) const
    {
      bool      rc = false;
      fs::path  p(PointCacheDir() + "/point");
      
      lock_guard<mutex>  lock(_pointInfoMtx);
      if (LastWriteTime(p) > 0) {
        info = _pointInfo;
        rc = true;
      }
      return rc;

#if 0
      bool  rc = false;
      std::string  pointCacheDir = PointCacheDir();
      if (EnsureDirExists(pointCacheDir)) {
        fs::path  pointCache(pointCacheDir + "/point");
        if (fs::exists(pointCache) && fs::is_regular_file(pointCache)) {
          time_t  lastModified = LastWriteTime(pointCache.string());
          time_t  now = time((time_t *)0);
          if ((lastModified + 604800) > now) {
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
              Syslog(LOG_ERR, "Failed to open points cache for reading: %m");
            }
          }
          else {
            //  cache is older than 1 week, should refresh it.
            if (Utils::GetPointInfo(_config.Latitude(), _config.Longitude(),
                                    info)) {
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
          //  points cache doesn't exist.
          if (Utils::GetPointInfo(_config.Latitude(), _config.Longitude(),
                                  info)) {
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
        rc = Utils::GetPointInfo(_config.Latitude(), _config.Longitude(),
                                 info);
      }
      return rc;
#endif
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    time_t Cache::AgeOfPointInfo() const
    {
      fs::path  p(PointCacheDir() + "/point");
      return AgeOfFile(p);
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::SetPointInfo(const PointInfo & info)
    {
      {
        lock_guard<mutex>  lock(_pointInfoMtx);
        _pointInfo = info;
      }
      return SavePointInfo();
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::SavePointInfo() const
    {
      fs::path  p(PointCacheDir() + "/point");
      return SaveCacheFile(_pointInfo, _pointInfoMtx, p);
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool
    Cache::SetObservationStations(const vector<pair<string,string>> & stations)
    {
      {
        lock_guard<mutex>  lock(_obsStationMtx);
        _observationStations = stations;
      }
      return SaveObservationStations();
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::SaveObservationStations() const
    {
      fs::path  p(PointCacheDir() + "/observationStations");
      return SaveCacheFile(_observationStations, _obsStationMtx, p);
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool
    Cache::GetObservationStations(vector<pair<string,string>> & stations) const
    {
      {
        lock_guard<mutex>  lock(_obsStationMtx);
        stations = _observationStations;
      }
      return true;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    time_t Cache::AgeOfObservationStations() const
    {
      fs::path  p(PointCacheDir() + "/observationStations");
      return AgeOfFile(p);
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Cache::EnsureDirExists(const fs::path & p)
    {
      bool             rc = false;
      std::error_code  ec;
      if (fs::exists(p, ec)) {
        if (fs::is_directory(p, ec)) {
          rc = true;
        }
      }
      else {
        rc = fs::create_directories(p, ec);
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    string Cache::PointCacheDir() const
    {
      string  rc(_cacheDir);
      ostringstream  os;
      os << '/' << setprecision(6) << _config.Latitude() << ','
         << _config.Longitude() << '/';
      rc += os.str();
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::time_t Cache::LastWriteTime(const fs::path & fsPath)
    {
      std::time_t  rc = 0;
      struct stat  statbuf;
      if (stat(fsPath.string().c_str(), &statbuf) == 0) {
        rc = statbuf.st_mtim.tv_sec;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::time_t Cache::AgeOfFile(const fs::path & fsPath)
    {
      return (time((time_t *)0) - LastWriteTime(fsPath));
    }
    
  }  // namespace Mcweather

}  // namespace Dwm
