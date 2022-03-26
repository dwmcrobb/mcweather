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
          _runcv(), _thread(),
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
    bool WeatherFetcher::UpdatePointInfo()
    {
      bool  rc = false;
      if (_cache.AgeOfPointInfo() > (7 * 24 * 60 * 60)) {
        //  cache is old.
        PointInfo  pointInfo;
        if (Utils::GetPointInfo(_config.Weather().Latitude(),
                                _config.Weather().Longitude(),
                                pointInfo)) {
          rc = _cache.SetPointInfo(pointInfo);
        }
        else {
          Syslog(LOG_ERR, "Failed to get point info");
        }
      }
      else {
        //  cache is current.
        rc = true;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool WeatherFetcher::UpdatePeriodForecasts()
    {
      bool  rc = false;
      if (_cache.AgeOfPeriodForecasts() > (15 * 60)) {
        //  cache is old.
        PointInfo  pointInfo;
        if (_cache.GetPointInfo(pointInfo)) {
          nlohmann::json  json;
          if (WebUtils::GetJson(pointInfo.ForecastURI(), json)) {
            if (json.is_object()) {
              auto  props = json.find("properties");
              if ((props != json.end()) && props->is_object()) {
                auto  periods = props->find("periods");
                if ((periods != props->end()) && periods->is_array()) {
                  PeriodForecasts  forecasts;
                  if (forecasts.FromJson(json)) {
                    rc = _cache.SetPeriodForecasts(forecasts);
                  }
                }
              }
            }
          }
        }
      }
      else {
        //  cache is current.
        rc = true;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool WeatherFetcher::UpdateHourlyForecasts()
    {
      bool  rc = false;
      if (_cache.AgeOfHourlyForecasts() > (15 * 60)) {
        //  cache is old.
        PointInfo  pointInfo;
        if (_cache.GetPointInfo(pointInfo)) {
          nlohmann::json  json;
          if (WebUtils::GetJson(pointInfo.HourlyForecastURI(), json)) {
            if (json.is_object()) {
              auto  props = json.find("properties");
              if ((props != json.end()) && props->is_object()) {
                auto  periods = props->find("periods");
                if ((periods != props->end()) && periods->is_array()) {
                  PeriodForecasts  forecasts;
                  if (forecasts.FromJson(json)) {
                    rc = _cache.SetHourlyForecasts(forecasts);
                  }
                }
              }
            }
          }
        }
      }
      else {
        //  cache is current.
        rc = true;
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool WeatherFetcher::UpdateObservationStations()
    {
      bool  rc = false;
      if (_cache.AgeOfObservationStations() >  (7 * 24 * 60 * 60)) {
        //  cache is old.
        PointInfo  pointInfo;
        if (_cache.GetPointInfo(pointInfo)) {
          vector<pair<string,string>>  stations;
          if (Utils::GetObservationStations(pointInfo, stations)) {
            rc = _cache.SetObservationStations(stations);
          }
          else {
            Syslog(LOG_ERR, "Failed to get observation stations");
          }
        }
        else {
          Syslog(LOG_ERR, "Failed to get point information");
        }
      }
      else {
        //  cache is current.
        rc = true;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool WeatherFetcher::UpdateCurrentConditions()
    {
      bool  rc = false;
      vector<pair<string,string>>  obsStations;
      if (_cache.GetObservationStations(obsStations)) {
        vector<string>  stationNames;
        for (const auto & cfgstation : _config.Weather().Stations()) {
          stationNames.push_back(cfgstation);
        }
        for (const auto & obsStation : obsStations) {
          if (find(stationNames.begin(), stationNames.end(), obsStation.first)
              == stationNames.end()) {
              stationNames.push_back(obsStation.first);
          }
        }
        if (! stationNames.empty()) {
          CurrentConditions  wcc;
          auto  stit = stationNames.begin();
          while (stit != stationNames.end()) {
            if (Utils::GetCurrentConditions(*stit, wcc)) {
              rc = _cache.SetConditions(wcc);
              Syslog(LOG_INFO, "Got current conditions for %s",
                     wcc.Station().c_str());
              break;
            }
            ++stit;
          }
          if (stit != stationNames.end()) {
            ++stit;
            while (stit != stationNames.end()) {
              if (Utils::GetCurrentConditions(*stit, wcc)) {
                rc = _cache.SetConditions(wcc);
                Syslog(LOG_INFO, "Got current conditions for %s",
                       wcc.Station().c_str());
                break;
              }
              ++stit;
            }
          }
        }
      }
      return rc;
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
    void WeatherFetcher::Run()
    {
      bool    keepRunning = true;
      string  curHost;
      
      Syslog(LOG_INFO, "WeatherFetcher thread started");
      while (keepRunning) {
        if (UpdatePointInfo()) {
          if (! UpdatePeriodForecasts()) {
            Syslog(LOG_ERR, "Failed to get period forecasts");
          }
          if (! UpdateHourlyForecasts()) {
            Syslog(LOG_ERR, "Failed to get hourly forecasts");
          }
          if (UpdateObservationStations()) {
            if (! UpdateCurrentConditions()) {
              Syslog(LOG_ERR, "Failed to get current conditions");
            }
          }
          else {
            Syslog(LOG_ERR, "Failed to get observation stations");
          }
        }
        
        {  //  scoped locking
          unique_lock<mutex>  lk(_runmtx);
          if (_runcv.wait_for(lk, 900s) != std::cv_status::timeout) {
            keepRunning = _run;
          }
        }
      }
      Syslog(LOG_INFO, "WeatherFetcher thread done");
      
      return;
    }
    

  }  // namespace Mcweather

}  // namespace Dwm
