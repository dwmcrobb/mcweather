//===========================================================================
// @(#) $DwmPath$
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2020
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
//!  \file DwmMcweatherPeriodForecast.hh
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::PeriodForecast class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCWEATHERPERIODFORECAST_HH_
#define _DWMMCWEATHERPERIODFORECAST_HH_

#include <chrono>
#include <string>

#include <nlohmann/json.hpp>

namespace Dwm {

  namespace Mcweather {
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class PeriodForecast
    {
    public:
      PeriodForecast(const nlohmann::json & j);
      std::chrono::system_clock::time_point StartTime() const;
      std::chrono::system_clock::time_point EndTime() const;
      std::string Name() const;
      bool IsDayTime() const;
      int Temperature() const;
      char TemperatureUnit() const;
      std::string WindSpeed() const;
      std::string WindDirection() const;
      std::string ShortForecast() const;
      std::string DetailedForecast() const;
        
    private:
      std::chrono::system_clock::time_point  _startTime;
      std::chrono::system_clock::time_point  _endTime;
      std::string                            _name;
      bool                                   _isDayTime;
      int                                    _temperature;
      char                                   _temperatureUnit;
      std::string                            _windSpeed;
      std::string                            _windDirection;
      std::string                            _shortForecast;
      std::string                            _detailedForecast;
        
      void Clear();
    };
      
  }  // namespace Mcweather

}  // namespace Dwm

#endif  // _DWMMCWEATHERPERIODFORECAST_HH_
