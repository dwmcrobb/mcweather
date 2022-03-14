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
//!  \file DwmMcweatherPointInfo.cc
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::PointInfo class implementation
//---------------------------------------------------------------------------

#include <climits>

#include "DwmStreamIO.hh"
#include "DwmMcweatherPointInfo.hh"

namespace Dwm {

  namespace Mcweather {

    using namespace std;
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    PointInfo::PointInfo()
    {
      Clear();
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool PointInfo::FromJson(const nlohmann::json & j)
    {
      bool  rc = false;
      Clear();
      if (j.is_object()) {
        auto  props = j.find("properties");
        if ((props != j.end()) && props->is_object()) {
          auto  it = props->find("forecast");
          if ((it != props->end()) && it->is_string()) {
            _forecastURI = it->get<string>();
            it = props->find("forecastHourly");
            if ((it != props->end()) && it->is_string()) {
              _hourlyForecastURI = it->get<string>();
              it = props->find("observationStations");
              if ((it != props->end()) && it->is_string()) {
                _observationStationsURI = it->get<string>();
                it = props->find("gridX");
                if ((it != props->end()) && it->is_number_integer()) {
                  _gridX = it->get<int>();
                  it = props->find("gridY");
                  if ((it != props->end()) && it->is_number_integer()) {
                    _gridY = it->get<int>();
                    it = props->find("forecastZone");
                    if ((it != props->end()) && it->is_string()) {
                      _forecastZoneURI = it->get<string>();
                      rc = true;
                    }
                  }
                }
              }
            }
          }
        }
      }
      return rc;
    }
        
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & PointInfo::ForecastURI() const
    {
      return _forecastURI;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & PointInfo::HourlyForecastURI() const
    {
      return _hourlyForecastURI;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & PointInfo::ObservationStationsURI() const
    {
      return _observationStationsURI;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int PointInfo::GridX() const
    {
      return _gridX;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int PointInfo::GridY() const
    {
      return _gridY;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & PointInfo::ForecastZoneURI() const
    {
      return _forecastZoneURI;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::istream & PointInfo::Read(std::istream & is)
    {
      Clear();
      if (is) {
        tuple<string,string,string,int,int,string>  t;
        if (StreamIO::Read(is, t)) {
          std::tie(_forecastURI, _hourlyForecastURI,
                   _observationStationsURI, _gridX, _gridY,
                   _forecastZoneURI) = t;
        }
      }
      return is;
    }
             
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::ostream & PointInfo::Write(std::ostream & os) const
    {
      if (os) {
        tuple<string,string,string,int,int,string>
          t(_forecastURI, _hourlyForecastURI, _observationStationsURI,
            _gridX, _gridY, _forecastZoneURI);
        StreamIO::Write(os, t);
      }
      return os;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void PointInfo::Clear()
    {
      _forecastURI.clear();
      _hourlyForecastURI.clear();
      _observationStationsURI.clear();
      _gridX = INT_MAX;
      _gridY = INT_MAX;
      _forecastZoneURI.clear();
      return;
    }
      
      
  }  // namespace Mcweather

}  // namespace Dwm
