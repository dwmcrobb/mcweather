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
//!  \file DwmMcweatherUtils.hh
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::Utils class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCWEATHERUTILS_HH_
#define _DWMMCWEATHERUTILS_HH_

#include <string>

#include "DwmMcweatherPointInfo.hh"
#include "DwmMcweatherCurrentConditions.hh"

namespace Dwm {

  namespace Mcweather {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class Utils
    {
    public:
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      static bool GetPointInfo(float latitude, float longitude,
                               PointInfo & pointInfo);

      //----------------------------------------------------------------------
      //!  Each pair in @ stations is filled with <stationID,stationName>.
      //!  stationID is needed for the weather.gov API, and stationName is
      //!  human readable.
      //----------------------------------------------------------------------
      static bool
      GetObservationStations(const PointInfo & pointInfo,
                             std::vector<std::pair<std::string,std::string>> & stations);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      static bool GetCurrentConditions(const std::string & station,
                                       CurrentConditions & wcc);

      static std::string GetNWSRadarURL(float latitude, float longitude);
      
    };
    

  }  // namespace Mcweather

}  // namespace Dwm

#endif  // _DWMMCWEATHERUTILS_HH_
