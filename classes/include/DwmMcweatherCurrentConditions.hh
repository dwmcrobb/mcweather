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
//!  \file DwmMcweatherCurrentConditions.hh
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::CurrentCondition and
//!         Dwm::Mcweather::CurrentConditions class declarations
//---------------------------------------------------------------------------

#ifndef _DWMMCWEATHERCURRENTCONDITIONS_HH_
#define _DWMMCWEATHERCURRENTCONDITIONS_HH_

#include <string>
#include <nlohmann/json.hpp>

#include "DwmStreamIOCapable.hh"

namespace Dwm {

  namespace Mcweather {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class CurrentCondition
      : public StreamIOCapable
    {
    public:
      CurrentCondition();
      bool FromJson(const nlohmann::json & json);
      int Value() const;
      int Value(int value);
      const std::string & UnitCode() const;
      const std::string & UnitCode(const std::string & unitCode);
      const std::string & QualityControl() const;
      std::istream & Read(std::istream & is) override;
      std::ostream & Write(std::ostream & os) const override;
      
    private:
      int          _value;
      std::string  _unitCode;
      std::string  _qualityControl;
    };
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class CurrentConditions
      : public StreamIOCapable
    {
    public:
      CurrentConditions() = default;
      
      bool FromJson(const nlohmann::json & json);
      
      const std::string & Station() const;
      int Temperature() const;
      int RelativeHumidity() const;
      int BarometricPressure() const;
      int Dewpoint() const;
      int WindSpeed() const;
      int WindChill() const;
      int HeatIndex() const;

      std::istream & Read(std::istream & is) override;
      std::ostream & Write(std::ostream & os) const override;
      
    private:
      std::string  _station;
      int          _temperature;         // degrees F
      int          _relativeHumidity;    // percent
      int          _barometricPressure;  // in Pa
      int          _dewpoint;            // degrees F
      int          _windSpeed;
      int          _windChill;
      int          _heatIndex;
    };
      
  }  // namespace Mcweather

}  // namespace Dwm

#endif  // _DWMMCWEATHERCURRENTCONDITIONS_HH_
