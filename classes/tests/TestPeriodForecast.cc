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
//!  \file TestWeatherPeriodForecast.cc
//!  \author Daniel W. McRobb
//!  \brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include <fstream>

#include "DwmUnitAssert.hh"
#include "DwmMcweatherPeriodForecast.hh"

using namespace std;
using namespace Dwm;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestForecast()
{
  nlohmann::json  j;
  ifstream  is("./forecast.json");
  if (UnitAssert(is)) {
    if (UnitAssert(is >> j)) {
      UnitAssert(j.is_object());
      auto  props = j.find("properties");
      if (UnitAssert((props != j.end()) && props->is_object())) {
        auto  periods = props->find("periods");
        if (UnitAssert((periods != props->end()) && periods->is_array())) {
          for (const auto & period : *periods) {
            Mcweather::PeriodForecast  periodForecast(period);
            UnitAssert(! periodForecast.Name().empty());
            UnitAssert(! periodForecast.ShortForecast().empty());
            UnitAssert(! periodForecast.DetailedForecast().empty());
            UnitAssert(periodForecast.StartTime()
                       < periodForecast.EndTime());
            UnitAssert((periodForecast.TemperatureUnit() == 'F')
                       || (periodForecast.TemperatureUnit() == 'C'));
          }
        }
      }
    }
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestHourlyForecast()
{
  using clk = std::chrono::system_clock;
  
  nlohmann::json  j;
  ifstream  is("./hourlyForecast.json");
  if (UnitAssert(is)) {
    if (UnitAssert(is >> j)) {
      UnitAssert(j.is_object());
      auto  props = j.find("properties");
      if (UnitAssert((props != j.end()) && props->is_object())) {
        auto  periods = props->find("periods");
        if (UnitAssert((periods != props->end()) && periods->is_array())) {
          clk::time_point  stp, etp;
          for (const auto & period : *periods) {
            Mcweather::PeriodForecast  periodForecast(period);
            UnitAssert(! periodForecast.ShortForecast().empty());
            UnitAssert(periodForecast.StartTime()
                       < periodForecast.EndTime());
            UnitAssert(periodForecast.StartTime() > stp);
            stp = periodForecast.StartTime();
            UnitAssert(periodForecast.EndTime() > etp);
            etp = periodForecast.EndTime();
            UnitAssert((periodForecast.TemperatureUnit() == 'F')
                       || (periodForecast.TemperatureUnit() == 'C'));
#if 0            
            auto  now = clk::now();
            time_t  t = clk::to_time_t(now + (stp - now));
            cout << ctime(&t) << "  "
                 << periodForecast.ShortForecast() << '\n';
#endif
          }
        }
      }
    }
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  TestForecast();
  TestHourlyForecast();
  
  if (Assertions::Total().Failed()) {
    Assertions::Print(cerr, true);
  }
  else {
    cout << Assertions::Total() << " passed" << endl;
  }
}
