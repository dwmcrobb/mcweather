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
//!  \file DwmMcweatherUtils.cc
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::Utils class implementation
//---------------------------------------------------------------------------

#include <climits>
#include <iomanip>
#include <regex>
#include <sstream>

#include "DwmSysLogger.hh"
#include "DwmCredenceUtils.hh"
#include "DwmWebUtils.hh"
#include "DwmMcweatherUtils.hh"

namespace Dwm {

  namespace Mcweather {

    using namespace std;

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Utils::GetPointInfo(float latitude, float longitude,
                             PointInfo & pointInfo)
    {
      bool  rc = false;
      ostringstream  oss;
      oss << "https://api.weather.gov/points/" << setprecision(6)
          << latitude << ',' << longitude;
      string  infostr = oss.str();
      nlohmann::json  j;
      if (WebUtils::GetJson(infostr, j)) {
        rc = pointInfo.FromJson(j);
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool
    Utils::GetObservationStations(const PointInfo & pointInfo,
                                  vector<pair<string,string>> & stations)
    {
      stations.clear();
      nlohmann::json  j;
      if (WebUtils::GetJson(pointInfo.ObservationStationsURI(), j)) {
        auto  features = j.find("features");
        if ((features != j.end()) && features->is_array()) {
          for (const auto & feat : *features) {
            if (feat.is_object()) {
              auto  props = feat.find("properties");
              if ((props != feat.end()) && props->is_object()) {
                auto  it = props->find("stationIdentifier");
                if ((it != props->end()) && it->is_string()) {
                  pair<string,string>  station(it->get<string>(), "");
                  it = props->find("name");
                  if ((it != props->end()) && it->is_string()) {
                    station.second = it->get<string>();
                  }
                  stations.push_back(station);
                  Syslog(LOG_INFO, "station %s %s", station.first.c_str(),
                         station.second.c_str());
                }
                else {
                  Syslog(LOG_ERR, "Failed to get stationIdentifier");
                }
              }
              else {
                Syslog(LOG_ERR, "Failed to get properties");
              }
            }
            else {
              Syslog(LOG_ERR, "feature is not a JSON object");
            }
          }
        }
        else {
          Syslog(LOG_ERR, "Failed to find features");
        }
      }
      else {
        Syslog(LOG_ERR, "Failed to get observation stations");
      }
      return (! stations.empty());
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Utils::GetCurrentConditions(const string & station,
                                     CurrentConditions & wcc)
    {
      bool  rc = false;
      string  uristr = "https://api.weather.gov/stations/" + station
        + "/observations/latest";
      nlohmann::json  js;
      if (WebUtils::GetJson(uristr, js)) {
        rc = wcc.FromJson(js);
        wcc.Timestamp(time((time_t *)0));
        if (rc) {
          if (wcc.Temperature() == INT_MAX) {
            Syslog(LOG_ERR, "Invalid temperature from %s",
                   wcc.Station().c_str());
            rc = false;
          }
          else {
            Syslog(LOG_INFO,
                   "Got current weather conditions from %s: temp %d"
                   " humidity %d%%",
                   wcc.Station().c_str(), wcc.Temperature(),
                   wcc.RelativeHumidity());
          }
        }
        else {
          Syslog(LOG_ERR, "Failed to get current weather conditions from %s",
                 station.c_str());
        }
        
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::string Utils::GetNWSRadarURL(float latitude, float longitude)
    {
      nlohmann::json  j;
      j["agenda"]["id"] = "weather";
      j["agenda"]["center"] = nlohmann::json::array({longitude,latitude});
      j["agenda"]["zoom"] = 6;
      j["agenda"]["location"] = nlohmann::json::array({longitude,latitude});
      j["base"] = "topographic";
      j["county"] = false;
      j["cwa"] = false;
      j["state"] = true;
      j["menu"] = false;
      j["shortFusedOnly"] = false;

      std::string  jstr = j.dump();
      std::string  jstrb64 = Credence::Utils::Bin2Base64(jstr);
      std::regex  rgx("[=]");
      jstrb64 = regex_replace(jstrb64, rgx, "%3D");
      return (string("https://radar.weather.gov/?settings=v1_")
              + jstrb64 + "#/");
    }
    
  }  // namespace Mcweather

}  // namespace Dwm
