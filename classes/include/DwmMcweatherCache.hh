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
//!  \file DwmMcweatherCache.hh
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::Cache class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCWEATHERCACHE_HH_
#define _DWMMCWEATHERCACHE_HH_

#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>

#include "DwmStreamIO.hh"
#include "DwmSysLogger.hh"
#include "DwmMcweatherCurrentConditions.hh"
#include "DwmMcweatherPeriodForecasts.hh"
#include "DwmMcweatherPointInfo.hh"
#include "DwmMcweatherWeatherConfig.hh"

namespace Dwm {

  namespace Mcweather {

    //------------------------------------------------------------------------
    //!  This needs refactoring.  It's coupled too tightly to WeatherFetcher
    //!  and Utils and does more than it should.
    //------------------------------------------------------------------------
    class Cache
    {
    public:
      using ObservationStations =
        std::vector<std::pair<std::string,std::string>>;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      Cache(const std::string & cacheDir, const WeatherConfig & config);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool GetPointInfo(PointInfo & info) const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      void SetPointInfo(const PointInfo & info);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      time_t AgeOfPointInfo() const;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool
      GetObservationStations(ObservationStations & stations) const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      void
      SetObservationStations(const ObservationStations & stations);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      time_t AgeOfObservationStations() const;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool GetCurrentConditions(std::map<std::string, CurrentConditions> & currentConditions) const;

      bool SetConditions(const CurrentConditions & conditions);

      bool GetPeriodForecasts(PeriodForecasts & forecasts) const;

      bool SetPeriodForecasts(const PeriodForecasts & forecasts);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      std::string PointCacheDir() const;

    private:
      std::string                                      _cacheDir;
      WeatherConfig                                    _config;
      static std::mutex                                _pointInfoMtx;
      PointInfo                                        _pointInfo;
      static std::mutex                                _obsStationMtx;
      std::vector<std::pair<std::string,std::string>>  _observationStations;
      static std::mutex                                _currentCondMtx;
      std::map<std::string, CurrentConditions>         _currentConditions;
      static std::mutex                                _periodForecastsMtx;
      PeriodForecasts                                  _periodForecasts;
      
      static bool EnsureDirExists(const std::filesystem::path & s);
      static std::time_t LastWriteTime(const std::filesystem::path & fsPath);
      static std::time_t AgeOfFile(const std::filesystem::path & fsPath);
      bool SavePointInfo() const;
      bool SaveObservationStations() const;
      bool SaveCurrentConditions() const;
      bool SavePeriodForecasts() const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      template <typename T>
      bool SaveCacheFile(const T & t, std::mutex & mtx,
                         const std::filesystem::path & p) const
      {
        namespace fs = std::filesystem;
        
        bool  rc = false;
        if (EnsureDirExists(p.parent_path())) {
          if ((fs::exists(p) && fs::is_regular_file(p))
              || (! fs::exists(p))) {
            std::ofstream  os(p.string());
            if (os) {
              std::lock_guard<std::mutex>  lock(mtx);
              if (StreamIO::Write(os, t)) {
                rc = true;
              }
              else {
                Syslog(LOG_ERR, "Failed to write to '%s'", p.string().c_str());
              }
            }
            else {
              Syslog(LOG_ERR, "Failed to open '%s'", p.string().c_str());
            }
          }
          else {
            Syslog(LOG_ERR, "'%s' is not a regular file", p.string().c_str());
          }
        }
        return rc;
      }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      template <typename T>
      bool LoadCacheFile(T & t, std::mutex & mtx,
                         const std::filesystem::path & p) const
      {
        namespace fs = std::filesystem;
        bool  rc = false;
        if (fs::exists(p) && fs::is_regular_file(p)) {
          std::ifstream  is(p);
          if (StreamIO::Read(is, t)) {
            rc = true;
          }
          else {
            Syslog(LOG_ERR, "Failed to read from '%s'", p.string().c_str());
          }
        }
        else {
          Syslog(LOG_WARNING, "Invalid cache file '%s'", p.string().c_str());
        }
        return rc;
      }
          
    };

  }  // namespace Mcweather

}  // namespace Dwm

#endif  // _DWMMCWEATHERWEATHERCACHE_HH_
