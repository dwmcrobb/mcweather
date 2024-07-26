//===========================================================================
// @(#) $DwmPath$
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2020, 2024
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
//!  \file DwmMcweatherWeatherConfig.hh
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::WeatherConfig class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCWEATHERWEATHERCONFIG_HH_
#define _DWMMCWEATHERWEATHERCONFIG_HH_

#include <string>
#include <vector>

namespace Dwm {

  namespace Mcweather {

    //------------------------------------------------------------------------
    //!  Encapsulates the 'weather' stanza of the mcweather configuration.
    //!  Latitude and longitude are required.  Stations may be used to
    //!  override the desired weather stations.
    //------------------------------------------------------------------------
    class WeatherConfig
    {
    public:
      //----------------------------------------------------------------------
      //!  Returns the latitude.
      //----------------------------------------------------------------------
      float Latitude() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the latitude.
      //----------------------------------------------------------------------
      float Latitude(float latitude);
      
      //----------------------------------------------------------------------
      //!  Returns the longitude.
      //----------------------------------------------------------------------
      float Longitude() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the longitude.
      //----------------------------------------------------------------------
      float Longitude(float longitude);
      
      //----------------------------------------------------------------------
      //!  Returns a const reference to the stations.
      //----------------------------------------------------------------------
      const std::vector<std::string> & Stations() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the stations.
      //----------------------------------------------------------------------
      const std::vector<std::string> &
      Stations(const std::vector<std::string> & stations);
      
    private:
      float                     _latitude;
      float                     _longitude;
      std::vector<std::string>  _stations;
    };
    
  }  // namespace Mcweather

}  // namespace Dwm

#endif  // _DWMMCWEATHERWEATHERCONFIG_HH_
