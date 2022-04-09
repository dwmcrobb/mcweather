//===========================================================================
// @(#) $DwmPath$
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2022
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
//!  \file DwmMcweatherWeatherdRequest.hh
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::WeatherdRequest declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCWEATHERWEATHERDREQUEST_HH_
#define _DWMMCWEATHERWEATHERDREQUEST_HH_

#include <cstdint>

namespace Dwm {

  namespace Mcweather {

    //------------------------------------------------------------------------
    //!  Requests that can be set to @c mcweatherd.
    //!
    //!  @c e_currentConditions requests a @c map<string,CurrentConditions> of
    //!  current weather conditions, where the map keys are observation
    //!  stations.
    //!  @c e_dailyforecasts requests a @c PeriodForecasts containing daily
    //!  forecasts.
    //!  @c e_observationStations requests a @c Cache::ObservationStations.
    //!  @c e_hourlyForecasts requests a @c PeriodForecasts containing hourly
    //!  forecasts.
    //------------------------------------------------------------------------
    enum WeatherdRequest : std::uint8_t {
      e_currentConditions   = 1,
      e_dailyForecasts      = 2,
      e_observationStations = 3,
      e_hourlyForecasts     = 4,
      e_buhBye              = 255
    };

  }  // namespace Mcweather

}  // namespace Dwm

#endif  // _DWMMCWEATHERWEATHERDREQUEST_HH_
