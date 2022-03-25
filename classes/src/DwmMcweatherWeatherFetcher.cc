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
//!  \file DwmMcweatherWeatherFetcher.cc
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::WeatherFetcher class implementation
//---------------------------------------------------------------------------

#include <fstream>
#include <sstream>

#include "DwmSysLogger.hh"
#include "DwmMcweatherCache.hh"
#include "DwmMcweatherCurrentConditions.hh"
#include "DwmMcweatherUtils.hh"
#include "DwmMcweatherWebUtils.hh"
#include "DwmMcweatherWeatherFetcher.hh"

namespace Dwm {

  namespace Mcweather {

    using namespace std;
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    WeatherFetcher::WeatherFetcher(const Config & config)
        : _config(config), _run(false), _runmtx(),
          _runcv(), _thread(), _lastFetchedForecast(0),
          _cache(_config.CacheDirectory(), _config.Weather())
    {
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    WeatherFetcher::~WeatherFetcher()
    {
      Stop();
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool WeatherFetcher::Start()
    {
      bool  rc = false;
      _run = true;
      try { 
        _thread = std::thread(&WeatherFetcher::Run, this);
        rc = true;
      }
      catch (...) {
        Syslog(LOG_ERR, "Exception starting WeatherFetcher thread");
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool WeatherFetcher::Stop()
    {
      lock_guard<mutex>  lk(_runmtx);
      _run = false;
      _runcv.notify_one();
      if (_thread.joinable()) {
        _thread.join();
        Syslog(LOG_INFO, "WeatherFetcher thread joined.");
      }
      return true;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool WeatherFetcher::GetHourlyForecasts()
    {
      bool  rc = false;
      PointInfo  pointInfo;
      if (_cache.GetPointInfo(pointInfo)) {
        nlohmann::json  json;
        if (WebUtils::GetJson(pointInfo.HourlyForecastURI(), json)) {
          ofstream  os(_cache.PointCacheDir() + '/' + "hourlyForecasts");
          if (os) {
            os << json.dump(3) << '\n';
            rc = true;
          }
        }
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool WeatherFetcher::GetForecasts()
    {
      bool  rc = false;
      PointInfo  pointInfo;
      if (_cache.GetPointInfo(pointInfo)) {
        nlohmann::json  json;
        if (WebUtils::GetJson(pointInfo.ForecastURI(), json)) {
          if (json.is_object()) {
            auto  props = json.find("properties");
            if ((props != json.end()) && props->is_object()) {
              auto  periods = props->find("periods");
              if ((periods != props->end()) && periods->is_array()) {
                PeriodForecasts  periodForecasts(json);
                _cache.SavePeriodForecasts(periodForecasts);
                SaveForecast(json);
                rc = true;
              }
              else {
                Syslog(LOG_ERR, "Failed to find periods in forecast");
              }
            }
            else {
              Syslog(LOG_ERR, "Failed to find properties in forecast");
            }
          }
          else {
            Syslog(LOG_ERR, "forecast not a JSON object");
          }
        }
      }
      if (! rc) {
        Syslog(LOG_WARNING, "Unable to fetch forecast");
        nlohmann::json  json;
        if (LoadSavedForecast(json)) {
          Syslog(LOG_INFO, "Using cached forecast");
          auto  props = json.find("properties");
          if ((props != json.end()) && props->is_object()) {
            auto  periods = props->find("periods");
            if ((periods != props->end()) && periods->is_array()) {
#if 0
              emit newForecasts(*periods);
              Syslog(LOG_DEBUG, "Emitted period forecasts from cache");
#endif
            }
          }
        }
      }
      
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool WeatherFetcher::SaveForecast(const nlohmann::json & forecast) const
    {
      bool  rc = false;
      std::string  forecastPath(_config.CacheDirectory());
      if (! forecastPath.empty()) {
        forecastPath += "/forecast.json";
        ofstream  os(forecastPath);
        if (os) {
          os << forecast.dump(3) << '\n';
          os.close();
          rc = true;
        }
        else {
          Syslog(LOG_ERR, "Failed to open '%s': %m", forecastPath.c_str());
        }
      }
      else {
        Syslog(LOG_ERR, "No weather cache store configured");
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool WeatherFetcher::LoadSavedForecast(nlohmann::json & forecast) const
    {
      bool  rc = false;
      std::string  forecastPath(getenv("HOME"));
      if (! forecastPath.empty()) {
        forecastPath += "/.forecast.json";
        ifstream  is(forecastPath.c_str());
        if (is) {
          is >> forecast;
          rc = forecast.is_object();
          is.close();
        }
        else {
          Syslog(LOG_ERR, "Failed to open '%s': %m", forecastPath.c_str());
        }
      }
      else {
        Syslog(LOG_ERR, "HOME environment variable not set");
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void WeatherFetcher::Run()
    {
      bool    keepRunning = true;
      string  curHost;
      uint32_t  cycles = 0;
      
      Syslog(LOG_INFO, "WeatherFetcher thread started");
      while (keepRunning) {
        if (cycles % 4 == 0) {
          if (GetForecasts()) {
            _lastFetchedForecast = time((time_t *)0);
            GetHourlyForecasts();
          }
        }
        vector<pair<string,string>>  obsStations;
        if (_cache.GetObservationStations(obsStations)) {
          int  station = 0;
          // set<string>  stations;
          vector<string>  stations;
          for (const auto & cfgstation : _config.Weather().Stations()) {
            stations.push_back(cfgstation);
          }
          for (const auto & obsStation : obsStations) {
            if (find(stations.begin(), stations.end(), obsStation.first)
                == stations.end()) {
              stations.push_back(obsStation.first);
            }
          }
          if (! stations.empty()) {
            bool  cacheUpdated = false;
            CurrentConditions  wcc;
            auto  stit = stations.begin();
            while (stit != stations.end()) {
              if (Utils::GetCurrentConditions(*stit, wcc)) {
                cacheUpdated |= _cache.SetConditions(wcc);
                Syslog(LOG_INFO, "Got current conditions for %s",
                       wcc.Station().c_str());
                break;
              }
              ++stit;
            }
            if (stit != stations.end()) {
              ++stit;
              while (stit != stations.end()) {
                if (Utils::GetCurrentConditions(*stit, wcc)) {
                  cacheUpdated |= _cache.SetConditions(wcc);
                  Syslog(LOG_INFO, "Got current conditions for %s",
                         wcc.Station().c_str());
                  break;
                }
                ++stit;
              }
            }
            if (cacheUpdated) {
              _cache.SaveCurrentConditions();
            }
          }
        }
        else {
          Syslog(LOG_ERR, "Failed to get observation stations");
        }

        {  //  scoped locking
          unique_lock<mutex>  lk(_runmtx);
          if (_runcv.wait_for(lk, 900s) != std::cv_status::timeout) {
            keepRunning = _run;
          }
        }
        ++cycles;
      }
      Syslog(LOG_INFO, "WeatherFetcher thread done");
      
      return;
    }
    

  }  // namespace Mcweather

}  // namespace Dwm
