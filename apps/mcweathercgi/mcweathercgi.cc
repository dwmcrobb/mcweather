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
//!  \file mcweather.cc
//!  \author Daniel W. McRobb
//!  \brief simple client for mcweatherd
//---------------------------------------------------------------------------

#include <ctime>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "DwmSysLogger.hh"
#include "DwmTerminalTricks.hh"
#include "DwmCredencePeer.hh"
#include "DwmMcweatherCache.hh"
#include "DwmMcweatherWeatherdRequest.hh"

using namespace std;
using namespace Dwm;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void
PrintCurrentConditions(ostream & os, const Mcweather::CurrentConditions & cc)
{
  os << "<tr>"
     << "  <td align=left>" << cc.Station() << "</td>"
     << "  <td align=right>" << cc.Temperature() << "F</td>"
     << "  <td align=right>" << cc.RelativeHumidity() << "%</td>"
     << "  <td align=right>" << cc.Dewpoint() << "F</td>"
     << "  <td align=right>"
     << std::setprecision(2) << std::fixed << std::setw(5)
     << cc.BarometricPressure() * 0.0002952998751 << "</td>"
     << "  <td align=right>";
  
  if (cc.WindSpeed() != INT_MAX) {
    os << cc.WindSpeed() << " mph";
  }
  else {
    os << "---";
  }
  os << "</td>"
     << "  <td align=right>";
  if (cc.WindChill() != INT_MAX) {
    os << cc.WindChill() << "F";
  }
  else {
    os <<  "---";
  }
  os << "</td>"
     << "  <td align=right>";
  if (cc.HeatIndex() != INT_MAX) {
    os << cc.HeatIndex() << "F";
  }
  else {
    os << "---";
  }
  os << "</td>"
     << "  <td align=right>";
  time_t     t = cc.Timestamp();
  struct tm  tm;
  localtime_r(&t, &tm);
  char  timestr[8] = {0};
  strftime(timestr, 8, "%H:%M", &tm);
  os << timestr << "</td></tr>";
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void
PrintCurrentConditions(ostream & os,
                       const map<string,Mcweather::CurrentConditions> & cc)
{
  string  secTitle("Current Conditions");
  os << "<table width=90%>"
     << "  <thead>"
     << "    <tr>"
     << "      <th colspan=9 align=center><h2>" << secTitle << "</h2></th>"
     << "    </tr>"
     << "    <tr>"
     << "      <th align=left><b>Station</th>"
     << "      <th align=right><b>Temperature</b></th>"
     << "      <th align=right><b>Humidity</b></th>"
     << "      <th align=right><b>Dew Point</b></th>"
     << "      <th align=right><b>Barometric Pressure (in. Hg)</b></th>"
     << "      <th align=right><b>Wind Speed</b></th>"
     << "      <th align=right><b>Wind Chill</b></th>"
     << "      <th align=right><b>Heat Index</b></th>"
     << "      <th align=right><b>Time</b></th>"
     << "    </tr>"
     << "  </thead>"
     << "  <tbody>";

  vector<Mcweather::CurrentConditions>  ccv;
  for (auto & c : cc) {
    ccv.push_back(c.second);
  }
  std::sort(ccv.begin(), ccv.end(),
            [&] (const Mcweather::CurrentConditions & a,
                 const Mcweather::CurrentConditions & b)
            { return (a.Timestamp() > b.Timestamp()); });
  for (auto & c : ccv) {
    PrintCurrentConditions(os, c);
  }
  os << "  </tbody></table><br/><hr>\n";
  
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static string
FormattedPeriodForecast(const Mcweather::PeriodForecast & forecast,
                        TerminalTricks & termTricks)
{
  string         rc;
  istringstream  is(forecast.DetailedForecast());
  ostringstream  os;
  os << "  ";

  while (is) {
    string  s;
    is >> s;
    if (s.size() + os.tellp() > (termTricks.Columns() - 4)) {
      rc += os.str();
      os.str("");
      os << "\n  " << s << ' ';
    }
    else {
      os << s << ' ';
    }
  }
  rc += os.str();
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void PrintDailyForecasts(ostream & os,
                                const Mcweather::PeriodForecasts & forecasts)
{
  string  secTitle("Daily Forecasts");
  os << "<br/>"
     << "<table width=90% cellpadding=5 cellspacing=0>"
     << "  <thead>"
     << "    <tr><th colspan=9 align=center><h2>" << secTitle
     << "</h2></th></tr>"
     << "  </thead>"
     << "  <tbody>";
  
  for (const auto & f : forecasts.Forecasts()) {
    if (! f.IsDayTime()) {
      os << "<tr style=\"background-color: black; color: white;\">";
    }
    else {
      os << "<tr>";
    }
    os << "  <td align=left><b>" << f.Name() << "</b></td>"
       << "  <td align=right><b>" << f.Temperature() << f.TemperatureUnit()
       << "</b></td>"
       << "  <td align=right><b>" << f.ShortForecast() << "</b></td>"
       << "</tr>";
    if (! f.IsDayTime()) {
      os << "<tr style=\"background-color: black; color: white;\">";
    }
    else {
      os << "<tr>";
    }
    os << "  <td align=left colspan=3>" << f.DetailedForecast() << "</td>"
       << "</tr>";
  }
  os << "  </tbody>"
     << "</table>\n"
     << "<br/>\n<hr>\n";
  
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void PrintHourlyForecasts(ostream & os, TerminalTricks & termTricks,
                                 const Mcweather::PeriodForecasts & forecasts)
{
  string  secTitle("Hourly Forecasts");
  int  leadingCols = (termTricks.Columns() - secTitle.size()) / 2;
  os << setw(leadingCols) << ' '
     << termTricks.Underscore(termTricks.Bold(secTitle)) << "\n\n";

  for (const auto & f : forecasts.Forecasts()) {
    // termTricks.Bold(os, true);
    // termTricks.Underscore(os, true);
    time_t  startTime =  std::chrono::system_clock::to_time_t(f.StartTime());
    struct tm  tm;
    localtime_r(&startTime, &tm);
    char  tmbuf[24];
    strftime(tmbuf, 24, "%a %b %e %H:%M", &tm);
    os << std::left << setw(15) << tmbuf << "  "
       << std::right << setw(3) << f.Temperature()
       << f.TemperatureUnit()
       << "  " << f.ShortForecast() << '\n';
    //    termTricks.Bold(os, false);
    //    termTricks.Underscore(os, false);
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static bool ShowHourlyForecasts(Credence::Peer & peer,
                                TerminalTricks & termTricks)
{
  bool  rc = false;
  uint8_t  req = Mcweather::e_hourlyForecasts;
  if (peer.Send(req)) {
    Mcweather::PeriodForecasts  hourlyForecasts;
    if (peer.Receive(hourlyForecasts)) {
      PrintHourlyForecasts(cout, termTricks, hourlyForecasts);
      rc = true;
    }
    else {
      cerr << "Failed to receive hourly forecasts from " << peer.Id() << '\n';
    }
  }
  else {
    cerr << "Failed to send hourly forecast request to " << peer.Id() << '\n';
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static bool ShowObservationStations(Credence::Peer & peer)
{
  bool  rc = false;
  
  uint8_t  req = Mcweather::e_observationStations;
  if (peer.Send(req)) {
    Mcweather::Cache::ObservationStations  stations;
    if (peer.Receive(stations)) {
      for (const auto & station : stations) {
        cout << station.first << ' ' << station.second << '\n';
      }
      rc = true;
    }
    else {
      cerr << "Failed to receive observation stations from "
           << peer.Id() << '\n';
    }
  }
  else {
    cerr << "Failed to send observation station request to "
         << peer.Id() << '\n';
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static bool EmbedCurrentConditions(Credence::Peer & peer)
{
  bool  rc = false;
  uint8_t  req = Mcweather::e_currentConditions;
  if (peer.Send(req)) {
    map<string,Mcweather::CurrentConditions>  cc;
    if (peer.Receive(cc)) {
      PrintCurrentConditions(cout, cc);
      rc = true;
    }
    else {
      cerr << "Failed to receive current conditions from "
           << peer.Id() << '\n';
    }
  }
  else {
    cerr << "Failed to send current conditions request to "
         << peer.Id() << '\n';
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static bool EmbedDailyForecasts(Credence::Peer & peer)
{
  bool  rc = false;
  uint8_t  req = Mcweather::e_dailyForecasts;
  if (peer.Send(req)) {
    Mcweather::PeriodForecasts  dailyForecasts;
    if (peer.Receive(dailyForecasts)) {
      PrintDailyForecasts(cout, dailyForecasts);
      rc = true;
    }
    else {
      cerr << "Failed to receive daily forecasts from " << peer.Id() << '\n';
    }
  }
  else {
    cerr << "Failed to send daily forecast request to " << peer.Id() << '\n';
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static bool EmbedNWSRadar(Credence::Peer & peer)
{
  bool  rc = false;
  uint8_t  req = Mcweather::e_nwsRadarUrl;
  if (peer.Send(req)) {
    string  radarUrl;
    if (peer.Receive(radarUrl)) {
      cout << "<iframe src=\"" << radarUrl << "\" width=90% height=90% style=\"border:none;\">"
           << "</iframe>";
      rc = true;
    }
    else {
      Syslog(LOG_ERR, "Failed to receive NWS radar URL from %s",
             peer.Id().c_str());
    }
  }
  else {
    Syslog(LOG_ERR, "Failed to send NWS radar URL request to %s",
           peer.Id().c_str());
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  Dwm::SysLogger::Open("mcweathercgi", LOG_PID, LOG_USER);
  Credence::Peer  peer;

  if (peer.Connect("thrip", 2124)) {
    Credence::KeyStash   keyStash;
    Credence::KnownKeys  knownKeys;
    if (peer.Authenticate(keyStash, knownKeys)) {
      cout << "<html><body>";
      EmbedCurrentConditions(peer);
      EmbedDailyForecasts(peer);
      EmbedNWSRadar(peer);
      cout << "</body></html>\n";
      peer.Send((uint8_t)255);
    }
  }
  else {
    cerr << "Failed to connect to thrip\n";
    return 1;
  }
}
