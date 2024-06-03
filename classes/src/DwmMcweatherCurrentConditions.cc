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
//!  \file DwmMcweatherCurrentConditions.cc
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::CurrentCondition and
//!         Dwm::Mcweather::CurrentConditions class implementation
//---------------------------------------------------------------------------

#include <climits>
#include <fstream>
#include <type_traits>
#include <experimental/type_traits>

#include "DwmStreamIO.hh"
#include "DwmSysLogger.hh"
#include "DwmMcweatherCurrentConditions.hh"

namespace Dwm {

  namespace Mcweather {

    using std::experimental::is_detected;
      
    //------------------------------------------------------------------------
    //!  Detection idiom stuff to be used to determine if a given type
    //!  has a FromJson() member.  Note that this technique requires
    //!  expression SFINAE.  It also requires is_detected from Library
    //!  Fundamentals TS v2.
    //------------------------------------------------------------------------
    template <class T>
    using has_from_json_t =
      decltype(std::declval<T>().FromJson(nlohmann::json()));
  
    template <class T>
    using HasFromJson = is_detected<has_from_json_t, T>;
  
    //------------------------------------------------------------------------
    //!  Template function to assign @c value from the JSON found with name
    //!  @c name inside @c j.
    //!  If the JSON node is found and @c predf returns true for the node
    //!  found, we assign @c value from the JSON node and return true.
    //!  If the JSON node is found but @c predf returns false, we return
    //!  false without assigning @c value.
    //!  If the JSON node is not found, we return @c noNodeOK without
    //!  assigning to @c value.  This allows the caller to specify whether
    //!  or not it is OK to not find the node in the JSON.
    //!
    //!  @c predf is expected to be one of json::is_string(),
    //!  json::is_number(), etc.
    //------------------------------------------------------------------------
    template <typename T>
    bool AssignFromJson(const nlohmann::json & j, const std::string & name,
                        bool (nlohmann::json::*predf)() const, T & value,
                        bool noNodeOK = true)
    {
      bool  rc = noNodeOK;
      auto  it = j.find(name);
      if (it != j.end()) {
        if (((*it).*predf)() && (! it->is_null())) {
          // Syslog(LOG_DEBUG, "Predicate passed for '%s'", name.c_str());
          if constexpr (HasFromJson<T>::value) {
              //  T has a FromJson() member; use it
              rc = value.FromJson(*it);
            }
          else {
            //  Assume T has assignment operator
            value = it->get<T>();
            rc = true;
          }
        }
        else {
          rc = false;
          Syslog(LOG_ERR, "Predicate failed for '%s'", name.c_str());
        }
      }
      return rc;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    static bool AssignFromJson(const nlohmann::json & json,
                               const std::string & name,
                               int & value, bool noNodeOK)
    {
      bool  rc = false;
      if (json.is_object()) {
        rc = AssignFromJson(json, name, &nlohmann::json::is_number_integer,
                            value, noNodeOK);
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    static bool AssignFromJson(const nlohmann::json & json,
                               const std::string & name,
                               std::string & value, bool noNodeOK)
    {
      bool  rc = false;
      if (json.is_object()) {
        rc = AssignFromJson(json, name, &nlohmann::json::is_string,
                            value, noNodeOK);
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    static bool CelsiusToFahrenheit(CurrentCondition & cond)
    {
      bool  rc = false;
      if ("unit:degC" == cond.UnitCode()
          || "wmoUnit:degC" == cond.UnitCode()) {
        float  f = cond.Value() * 9;
        f /= 5;
        f += 32.5;
        cond.Value(f);
        cond.UnitCode("unit:degF");
        rc = true;
      }
      else if ("unit:degF" == cond.UnitCode()
               || "wmoUnit:degF" == cond.UnitCode()) {
        rc = true;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    static bool MetersPerSecondToMph(CurrentCondition & cond)
    {
      bool  rc = false;
      if (cond.Value() == INT_MAX) {
        Syslog(LOG_ERR, "Bad value %d", cond.Value());
        return false;
      }
      if ("unit:m_s-1" == cond.UnitCode()) {
        cond.Value((cond.Value() * 2.23694) + 0.5);
        cond.UnitCode("unit:mph");
        rc = true;
      }
      else if ("unit:km_h-1" == cond.UnitCode()
               || "wmoUnit:km_h-1" == cond.UnitCode()) {
        cond.Value((cond.Value() * 0.621371) + 0.5);
        cond.UnitCode("unit:mph");
        rc = true;
      }
      else {
        Syslog(LOG_ERR, "Unknown unit code '%s'", cond.UnitCode().c_str());
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    CurrentCondition::CurrentCondition()
        : _value(INT_MAX), _unitCode(), _qualityControl()
    {}
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool CurrentCondition::FromJson(const nlohmann::json & json)
    {
      bool  rc = false;
      _value = INT_MAX;
      _unitCode.clear();
      _qualityControl.clear();
        
      if (json.is_object()) {
        if (! AssignFromJson(json, "value",
                             &nlohmann::json::is_number,
                             _value, false)) {
          _value = INT_MAX;
        }
        rc = AssignFromJson(json, "unitCode", _unitCode, false);
        rc &= AssignFromJson(json, "qualityControl", _qualityControl, false);
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int CurrentCondition::Value() const
    {
      return _value;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int CurrentCondition::Value(int value)
    {
      _value = value;
      return _value;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & CurrentCondition::UnitCode() const
    {
      return _unitCode;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string &
    CurrentCondition::UnitCode(const std::string & unitCode)
    {
      _unitCode = unitCode;
      return _unitCode;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & CurrentCondition::QualityControl() const
    {
      return _qualityControl;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::istream & CurrentCondition::Read(std::istream & is)
    {
      if (is) {
        StreamIO::ReadV(is, _value, _unitCode, _qualityControl);
      }
      return is;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::ostream & CurrentCondition::Write(std::ostream & os) const
    {
      if (os) {
        StreamIO::WriteV(os, _value, _unitCode, _qualityControl);
      }
      return os;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool CurrentCondition::operator == (const CurrentCondition & cc) const
    {
      return ((_value == cc._value)
              && (_unitCode == cc._unitCode)
              && (_qualityControl == cc._qualityControl));
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool CurrentCondition::operator != (const CurrentCondition & cc) const
    {
      return (! (*this == cc));
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool CurrentConditions::FromJson(const nlohmann::json & js)
    {
      using nlohmann::json;
        
      bool  rc = false;

      _temperature = INT_MAX;
      _windSpeed = INT_MAX;
      _windChill = INT_MAX;
      _heatIndex = INT_MAX;
      auto  props = js.find("properties");
      if ((props != js.end()) && props->is_object()) {
        if (AssignFromJson(*props, "station", _station, false)) {
          std::ofstream  dbgos("/tmp/currentWeather_" + _station + ".json");
          dbgos << js.dump(3) << '\n';
          dbgos.close();
        
          size_t  idx = _station.find_last_of('/');
          if (idx < (_station.size() - 1)) {
            _station = _station.substr(idx + 1);
          }
          CurrentCondition  cc;
          if (AssignFromJson(*props, "temperature", &json::is_object,
                             cc, false)) {
            if (cc.Value() != INT_MAX) {
              CelsiusToFahrenheit(cc);
              _temperature = cc.Value();
            }
            if (AssignFromJson(*props, "relativeHumidity",
                               &json::is_object, cc, false)) {
              _relativeHumidity = cc.Value();
              if (AssignFromJson(*props, "barometricPressure",
                                 &json::is_object, cc, false)) {
                _barometricPressure = cc.Value();
                if (AssignFromJson(*props, "dewpoint", &json::is_object,
                                   cc, false)) {
                  CelsiusToFahrenheit(cc);
                  _dewpoint = cc.Value();
                  if (AssignFromJson(*props, "windSpeed", &json::is_object,
                                     cc, false)) {
                    if (MetersPerSecondToMph(cc)) {
                      _windSpeed = cc.Value();
                    }
                    if (AssignFromJson(*props, "windChill",
                                       &json::is_object, cc, false)) {
                      if (cc.Value() != INT_MAX) {
                        CelsiusToFahrenheit(cc);
                        _windChill = cc.Value();
                      }
                      if (AssignFromJson(*props, "heatIndex",
                                         &json::is_object, cc, false)) {
                        if (cc.Value() != INT_MAX) {
                          CelsiusToFahrenheit(cc);
                          _heatIndex = cc.Value();
                        }
                        rc = true;
                      }
                    }
                  }
                  else {
                    Syslog(LOG_ERR, "Failed to assign windSpeed");
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
    const std::string & CurrentConditions::Station() const
    {
      return _station;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int64_t CurrentConditions::Timestamp() const
    {
      return _timestamp;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int64_t CurrentConditions::Timestamp(int64_t ts)
    {
      _timestamp = ts;
      return _timestamp;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int CurrentConditions::Temperature() const
    {
      return _temperature;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int CurrentConditions::RelativeHumidity() const
    {
      return _relativeHumidity;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int CurrentConditions::BarometricPressure() const
    {
      return _barometricPressure;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int CurrentConditions::Dewpoint() const
    {
      return _dewpoint;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int CurrentConditions::WindSpeed() const
    {
      return _windSpeed;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int CurrentConditions::WindChill() const
    {
      return _windChill;
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int CurrentConditions::HeatIndex() const
    {
      return _heatIndex;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::istream & CurrentConditions::Read(std::istream & is)
    {
      if (is) {
        StreamIO::ReadV(is, _station, _timestamp, _temperature,
                        _relativeHumidity, _barometricPressure, _dewpoint,
                        _windSpeed, _windChill, _heatIndex);
      }
      return is;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::ostream & CurrentConditions::Write(std::ostream & os) const
    {
      if (os) {
        if (StreamIO::Write(os, _station)) {
          if (StreamIO::Write(os, _timestamp)) {
            if (StreamIO::Write(os, _temperature)) {
              if (StreamIO::Write(os, _relativeHumidity)) {
                if (StreamIO::Write(os, _barometricPressure)) {
                  if (StreamIO::Write(os, _dewpoint)) {
                    if (StreamIO::Write(os, _windSpeed)) {
                      if (StreamIO::Write(os, _windChill)) {
                        StreamIO::Write(os, _heatIndex);
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
    bool CurrentConditions::operator == (const CurrentConditions & ccs) const
    {
      return ((_station               == ccs._station)
              && (_timestamp          == ccs._timestamp)
              && (_temperature        == ccs._temperature)
              && (_relativeHumidity   == ccs._relativeHumidity)
              && (_barometricPressure == ccs._barometricPressure)
              && (_dewpoint           == ccs._dewpoint)
              && (_windSpeed          == ccs._windSpeed)
              && (_windChill          == ccs._windChill)
              && (_heatIndex          == ccs._heatIndex));
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool CurrentConditions::operator != (const CurrentConditions & ccs) const
    {
      return (! (*this == ccs));
    }
      
    
  }  // namespace Mceather

}  // namespace Dwm
