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
//!  \file DwmMcweatherPeriodForecasts.cc
//!  \author Daniel W. McRobb
//!  \brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include "DwmStreamIO.hh"
#include "DwmSysLogger.hh"
#include "DwmMcweatherPeriodForecasts.hh"

namespace Dwm {

  namespace Mcweather {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    PeriodForecasts::PeriodForecasts(nlohmann::json json)
    {
      FromJson(json);
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool PeriodForecasts::FromJson(nlohmann::json json)
    {
      _forecasts.clear();
      if (json.is_object()) {
        nlohmann::json::json_pointer  p("/properties/periods");
        try {
          auto  periods = json.at(p);
          if (periods.is_array()) {
            for (const auto & period : periods) {
              _forecasts.push_back(PeriodForecast(period));
            }
          }
        }
        catch (...) {
          Syslog(LOG_ERR, "Failed to find properties/periods");
        }
      }
      return (! _forecasts.empty());
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::istream & PeriodForecasts::Read(std::istream & is)
    {
      _forecasts.clear();
      return StreamIO::Read(is, _forecasts);
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::ostream & PeriodForecasts::Write(std::ostream & os) const
    {
      return StreamIO::Write(os, _forecasts);
    }
    
  }  // namespace Mcweather

}  // namespace Dwm
