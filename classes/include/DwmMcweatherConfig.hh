//===========================================================================
// @(#) $DwmPath$
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2020, 2022, 2024
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
//!  \file DwmMcweatherConfig.hh
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::Config class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCWEATHERCONFIG_HH_
#define _DWMMCWEATHERCONFIG_HH_

#include <string>

#include "DwmMcweatherServiceConfig.hh"
#include "DwmMcweatherWeatherConfig.hh"

namespace Dwm {

  namespace Mcweather {

    //------------------------------------------------------------------------
    //!  Encapsulates mcweather configuration.
    //------------------------------------------------------------------------
    class Config
    {
    public:
      //----------------------------------------------------------------------
      //!  Default constructor
      //----------------------------------------------------------------------
      Config();
      
      //----------------------------------------------------------------------
      //!  Parse the configuration from the file at the given @c path.
      //!  Returns true on success, false on failure.
      //----------------------------------------------------------------------
      bool Parse(const std::string & path);

      //----------------------------------------------------------------------
      //!  Returns the path to the directory where weather conditions will
      //!  be cached.
      //----------------------------------------------------------------------
      const std::string & CacheDirectory() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the path to the directory where weather
      //!  conditions will be cached.
      //----------------------------------------------------------------------
      const std::string & CacheDirectory(const std::string & cacheDir);

      //----------------------------------------------------------------------
      //!  Returns the syslog facility that will be used for logging.
      //----------------------------------------------------------------------
      const std::string & SyslogFacility() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the syslog facility that will be used for
      //!  logging.
      //----------------------------------------------------------------------
      const std::string & SyslogFacility(const std::string & facility);

      //----------------------------------------------------------------------
      //!  Returns the minimum level of syslog message that will be logged.
      //----------------------------------------------------------------------
      const std::string & SyslogLevel() const;

      //----------------------------------------------------------------------
      //!  Sets and returns minimum the level of syslog message that will be
      //!  logged.
      //----------------------------------------------------------------------
      const std::string & SyslogLevel(const std::string & level);

      //----------------------------------------------------------------------
      //!  Returns true if we will log file name and line number in syslog
      //!  messages.
      //----------------------------------------------------------------------
      bool SyslogLocations() const;

      //----------------------------------------------------------------------
      //!  if @c logLocations is @c true, we will log file name and line
      //!  number in syslog messages.
      //----------------------------------------------------------------------
      bool SyslogLocations(bool logLocations);

      //----------------------------------------------------------------------
      //!  Returns true if mcweatherd should run a server.
      //----------------------------------------------------------------------
      bool RunService() const;
      
      //----------------------------------------------------------------------
      //!  If @c runService is true, mcweatherd should run a server.
      //----------------------------------------------------------------------
      bool RunService(bool runService);

      //----------------------------------------------------------------------
      //!  Returns the network service configuration.
      //----------------------------------------------------------------------
      const ServiceConfig & Service() const;

      //----------------------------------------------------------------------
      //!  Sets and returns the network service configuration.
      //----------------------------------------------------------------------
      const ServiceConfig Service(const ServiceConfig & service);
      
      //----------------------------------------------------------------------
      //!  Returns the weather configuration.
      //----------------------------------------------------------------------
      const WeatherConfig & Weather() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the weather configuration.
      //----------------------------------------------------------------------
      const WeatherConfig & Weather(const WeatherConfig & weather);

      //----------------------------------------------------------------------
      //!  ostream output operator
      //----------------------------------------------------------------------
      friend std::ostream &
      operator << (std::ostream & os, const Config & cfg);
      
    private:
      std::string                       _cacheDirectory;
      bool                              _runService;
      ServiceConfig                     _service;
      std::string                       _syslogFacility;
      std::string                       _syslogLevel;
      bool                              _logLocations;
      WeatherConfig                     _weather;
      
      void Clear();
    };
    
  }  // namespace Mcweather

}  // namespace Dwm

#endif  // _DWMMCWEATHERCONFIG_HH_
