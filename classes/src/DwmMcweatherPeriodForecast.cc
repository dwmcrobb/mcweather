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
//!  \file DwmMcweatherPeriodForecast.cc
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::PeriodForecast class implementation
//---------------------------------------------------------------------------

#include <ctime>

#include "DwmStreamIO.hh"
#include "DwmMcweatherPeriodForecast.hh"

namespace Dwm {

  namespace Mcweather {

    using namespace std;
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    static time_t ParseTimeString(const string & timestr)
    {
      time_t  rc = 0;
      size_t  idx = timestr.find_last_of(':');
      if ((timestr.size() - 3) == idx) {
        string  s = timestr.substr(0, idx) + timestr.substr(idx+1);
        struct tm  tm;
        if (strptime(s.c_str(), "%Y-%m-%dT%H:%M:%S%z", &tm) != nullptr) {
          rc = mktime(&tm);
        }
      }
      return rc;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    PeriodForecast::PeriodForecast(const nlohmann::json & j)
    {
      using clk = std::chrono::system_clock;
      Clear();
      if (j.is_object()) {
        auto  it = j.find("name");
        if ((it != j.end()) && it->is_string()) {
          _name = it->get<string>();
        }
        it = j.find("startTime");
        if ((it != j.end()) && it->is_string()) {
          time_t  t = ParseTimeString(it->get<string>());
          _startTime = clk::from_time_t(t);
        }
        it = j.find("endTime");
        if ((it != j.end()) && it->is_string()) {
          time_t  t = ParseTimeString(it->get<string>());
          _endTime = clk::from_time_t(t);
        }
        it = j.find("isDaytime");
        if ((it != j.end()) && it->is_boolean()) {
          _isDayTime = it->get<bool>();
        }
        it = j.find("temperature");
        if ((it != j.end()) && it->is_number()) {
          _temperature = it->get<int>();
        }
        it = j.find("temperatureUnit");
        if ((it != j.end()) && it->is_string()) {
          string  tmpUnit = it->get<string>();
          if (! tmpUnit.empty()) {
            _temperatureUnit = tmpUnit[0];
          }
        }
        it = j.find("windSpeed");
        if ((it != j.end()) && it->is_string()) {
          _windSpeed = it->get<string>();
        }
        it = j.find("windDirection");
        if ((it != j.end()) && it->is_string()) {
          _windDirection = it->get<string>();
        }
        it = j.find("shortForecast");
        if ((it != j.end()) && it->is_string()) {
          _shortForecast = it->get<string>();
        }
        it = j.find("detailedForecast");
        if ((it != j.end()) && it->is_string()) {
          _detailedForecast = it->get<string>();
        }
      }
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    chrono::system_clock::time_point PeriodForecast::StartTime() const
    {
      return _startTime;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    chrono::system_clock::time_point PeriodForecast::EndTime() const
    {
      return _endTime;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    string PeriodForecast::Name() const
    {
      return _name;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool PeriodForecast::IsDayTime() const
    {
      return _isDayTime;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int PeriodForecast::Temperature() const
    {
      return _temperature;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    char PeriodForecast::TemperatureUnit() const
    {
      return _temperatureUnit;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    string PeriodForecast::WindSpeed() const
    {
      return _windSpeed;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    string PeriodForecast::WindDirection() const
    {
      return _windDirection;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    string PeriodForecast::ShortForecast() const
    {
      return _shortForecast;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    string PeriodForecast::DetailedForecast() const
    {
      return _detailedForecast;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::istream & PeriodForecast::Read(std::istream & is)
    {
      if (is) {
        int64_t  t;
        if (StreamIO::Read(is, t)) {
          _startTime = chrono::system_clock::from_time_t(t);
          if (StreamIO::Read(is, t)) {
            _endTime = chrono::system_clock::from_time_t(t);
            if (StreamIO::Read(is, _name)) {
              if (StreamIO::Read(is, _isDayTime)) {
                if (StreamIO::Read(is, _temperature)) {
                  if (StreamIO::Read(is, _temperatureUnit)) {
                    if (StreamIO::Read(is, _windSpeed)) {
                      if (StreamIO::Read(is, _windDirection)) {
                        if (StreamIO::Read(is, _shortForecast)) {
                          StreamIO::Read(is, _detailedForecast);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      return is;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::ostream & PeriodForecast::Write(std::ostream & os) const
    {
      if (os) {
        int64_t  t = chrono::system_clock::to_time_t(_startTime);
        if (StreamIO::Write(os, t)) {
          t = chrono::system_clock::to_time_t(_endTime);
          if (StreamIO::Write(os, t)) {
            if (StreamIO::Write(os, _name)) {
              if (StreamIO::Write(os, _isDayTime)) {
                if (StreamIO::Write(os, _temperature)) {
                  if (StreamIO::Write(os, _temperatureUnit)) {
                    if (StreamIO::Write(os, _windSpeed)) {
                      if (StreamIO::Write(os, _windDirection)) {
                        if (StreamIO::Write(os, _shortForecast)) {
                          StreamIO::Write(os, _detailedForecast);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      return os;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void PeriodForecast::Clear()
    {
      _startTime = chrono::system_clock::from_time_t(0);
      _endTime = chrono::system_clock::from_time_t(0);
      _name.clear();
      _isDayTime = false;
      _temperature = 255;
      _temperatureUnit = ' ';
      _windSpeed.clear();
      _windDirection.clear();
      _shortForecast.clear();
      _detailedForecast.clear();
    }
      
  }  // namespace Mcweather

}  // namespace Dwm
