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
//!  \file DwmMcweatherWeatherFetcher.hh
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::WeatherFetcher class declaration
//---------------------------------------------------------------------------

#include <mutex>
#include <condition_variable>
#include <thread>

#include <nlohmann/json.hpp>

#include "DwmMcweatherConfig.hh"
#include "DwmMcweatherCache.hh"
#include "DwmMcweatherCurrentConditions.hh"


namespace Dwm {

  namespace Mcweather {

    //------------------------------------------------------------------------
    //!  Encapsulates a thread that periodically fetches and caches weather
    //!  data according to a given configuration.
    //------------------------------------------------------------------------
    class WeatherFetcher
    {
    public:
      //----------------------------------------------------------------------
      //!  Construct from the given configuration @c config.
      //----------------------------------------------------------------------
      WeatherFetcher(const Config & config);
      
      //----------------------------------------------------------------------
      //!  Destructor
      //----------------------------------------------------------------------
      ~WeatherFetcher();
      
      //----------------------------------------------------------------------
      //!  Start the fetcher.  Returns true on success, false on failure.
      //----------------------------------------------------------------------
      bool Start();
      
      //----------------------------------------------------------------------
      //!  Stop the fetcher.  Returns true on success, false on failure.
      //----------------------------------------------------------------------
      bool Stop();

    private:
      Config                   _config;
      bool                     _run;
      std::mutex               _runmtx;
      std::condition_variable  _runcv;
      std::thread              _thread;
      Cache                    _cache;

      bool UpdatePointInfo();
      bool UpdateObservationStations();
      bool UpdatePeriodForecasts();
      bool UpdateHourlyForecasts();
      bool UpdateCurrentConditions();
      bool GetHourlyForecasts();
      void Run();
    };
    
  }  // namespace Mcweather

}  // namespace Dwm
