//===========================================================================
// @(#) $DwmPath$
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2022, 2024
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
//!  \file DwmMcweatherPeriodForecasts.hh
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::PeriodForecasts class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCWEATHERPERIODFORECASTS_HH_
#define _DWMMCWEATHERPERIODFORECASTS_HH_

#include <vector>
#include <nlohmann/json.hpp>

#include "DwmMcweatherPeriodForecast.hh"

namespace Dwm {

  namespace Mcweather {

    //------------------------------------------------------------------------
    //!  Encapsulates a vector of PeriodForecast objects.
    //------------------------------------------------------------------------
    class PeriodForecasts
    {
    public:
      //----------------------------------------------------------------------
      //!  Default constructor
      //----------------------------------------------------------------------
      PeriodForecasts() = default;
      
      //----------------------------------------------------------------------
      //!  Copy constructor
      //----------------------------------------------------------------------
      PeriodForecasts(const PeriodForecasts &) = default;
      
      //----------------------------------------------------------------------
      //!  Construct from the given JSON @c forecasts.  The JSON must be in
      //!  the form used by api.weather.gov
      //----------------------------------------------------------------------
      PeriodForecasts(nlohmann::json forecasts);

      //----------------------------------------------------------------------
      //!  Populate from the given JSON @c forecasts.  The JSON must be in
      //!  the form used by api.weather.gov
      //----------------------------------------------------------------------
      bool FromJson(nlohmann::json forecasts);
      
      //----------------------------------------------------------------------
      //!  Assignment operator
      //----------------------------------------------------------------------
      PeriodForecasts & operator = (const PeriodForecasts &) = default;
      
      //----------------------------------------------------------------------
      //!  Returns a reference to the encapsulated vector of PeriodForecast
      //!  objects.
      //----------------------------------------------------------------------
      const std::vector<PeriodForecast> & Forecasts() const
      { return _forecasts; }
      
      //----------------------------------------------------------------------
      //!  Reads the PeriodForecasts from the given istream @c is.  Returns
      //!  @c is.
      //----------------------------------------------------------------------
      std::istream & Read(std::istream & is);

      //----------------------------------------------------------------------
      //!  Writes the PeriodForecasts to the given ostream @c os.  Returns
      //!  @c os.
      //----------------------------------------------------------------------
      std::ostream & Write(std::ostream & os) const;

    private:
      std::vector<PeriodForecast>  _forecasts;
    };
    
  }  // namespace Mcweather

}  // namespace Dwm

#endif  // _DWMMCWEATHERPERIODFORECASTS_HH_
