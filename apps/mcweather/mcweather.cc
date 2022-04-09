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

#include "DwmArguments.hh"
#include "DwmSysLogger.hh"
#include "DwmTerminalTricks.hh"
#include "DwmCredencePeer.hh"
#include "DwmMcweatherCache.hh"
// #include "DwmMcweatherCurrentConditions.hh"
// #include "DwmMcweatherPeriodForecasts.hh"

using namespace std;
using namespace Dwm;

typedef   Dwm::Arguments<Dwm::Argument<'s',string,false>,
                         Dwm::Argument<'o',bool>,
                         Dwm::Argument<'c',bool>,
                         Dwm::Argument<'d',bool>,
                         Dwm::Argument<'h',bool>>  MyArgType;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void InitArgs(MyArgType & args)
{
  args.SetHelp<'s'>("mcweatherd host");
  args.SetValueName<'s'>("host");
  args.SetHelp<'c'>("get current conditions");
  args.SetHelp<'d'>("get daily forecasts");
  args.SetHelp<'h'>("get hourly forecasts");
  args.SetHelp<'o'>("get observation stations");
  args.LoadFromEnvironment("MCWEATHER");
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void
PrintCurrentConditions(ostream & os, const Mcweather::CurrentConditions & cc)
{
  os << std::left << setw(7) << cc.Station() << "  "
     << std::right << setw(3) << cc.Temperature() << "F  "
     << std::right << setw(4) << cc.RelativeHumidity() << "%  "
     << std::right << setw(4) << cc.Dewpoint() << "F  "
     << std::right << std::setprecision(2) << std::fixed << std::setw(5)
     << cc.BarometricPressure() * 0.0002952998751 << " in. Hg  ";
  if (cc.WindSpeed() != INT_MAX) {
    os << std::right << setw(3) << cc.WindSpeed() << " mph  ";
  }
  else {
    os << std::right << "    ---" << "  ";
  }
  if (cc.WindChill() != INT_MAX) {
    os << std::right << setw(4) << cc.WindChill() << "F  ";
  }
  else {
    os << std::right << setw(5) << "  ---" << "  ";
  }
  if (cc.HeatIndex() != INT_MAX) {
    os << std::right << setw(6) << cc.HeatIndex() << "F  ";
  }
  else {
    os << std::right << setw(7) << "    ---  ";
  }
  time_t     t = cc.Timestamp();
  struct tm  tm;
  localtime_r(&t, &tm);
  char  timestr[8] = {0};
  strftime(timestr, 8, "%H:%M", &tm);
  os << std::right << setw(5) << timestr;
  
  os << '\n';
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void
PrintCurrentConditions(ostream & os, TerminalTricks & termTricks,
                       const map<string,Mcweather::CurrentConditions> & cc)
{
  string  secTitle("Current Conditions");
  int  leadingCols = (termTricks.Columns() - secTitle.size()) / 2;
  
  os << setw(leadingCols) << ' '
     << termTricks.Underscore(termTricks.Bold("Current Conditions"))
     << "\n\n";
  
  os << std::left << setw(7) << termTricks.Underscore("Station") << "  "
     << std::right << setw(4) << termTricks.Underscore("Temp") << "  "
     << std::right << setw(5) << termTricks.Underscore("Humid") << "  "
     << std::right << setw(5) << termTricks.Underscore("DewPt") << "  "
     << std::right << setw(12) << termTricks.Underscore("Bar Pressure") << "  "
     << std::right << setw(7) << termTricks.Underscore("WindSpd") << "  "
     << std::right << setw(5) << termTricks.Underscore("Chill") << "  "
     << std::right << setw(7) << termTricks.Underscore("HeatIdx") << "  "
     << std::right << setw(5) << termTricks.Underscore(" Time") << "  "
     << '\n';

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
  os << '\n';
  
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
static void PrintDailyForecasts(ostream & os, TerminalTricks & termTricks,
                                const Mcweather::PeriodForecasts & forecasts)
{
  string  secTitle("Daily Forecasts");
  int  leadingCols = (termTricks.Columns() - secTitle.size()) / 2;
  os << setw(leadingCols) << ' '
     << termTricks.Underscore(termTricks.Bold(secTitle)) << "\n\n";

  for (const auto & f : forecasts.Forecasts()) {
    termTricks.Bold(os, true);
    termTricks.Underscore(os, true);
    os << std::left << setw(15) << f.Name() << ' '
       << std::right << setw(3) << f.Temperature()
       << f.TemperatureUnit()
       << ' ' << f.ShortForecast() << '\n';
    termTricks.Bold(os, false);
    termTricks.Underscore(os, false);
    os << FormattedPeriodForecast(f, termTricks) << "\n\n";
  }
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
  uint8_t  cmd = 4;
  if (peer.Send(cmd)) {
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
  
  uint8_t  cmd = 3;
  if (peer.Send(cmd)) {
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
static bool ShowCurrentConditions(Credence::Peer & peer,
                                  TerminalTricks & termTricks)
{
  bool  rc = false;
  uint8_t  cmd = 1;
  if (peer.Send(cmd)) {
    map<string,Mcweather::CurrentConditions>  cc;
    if (peer.Receive(cc)) {
      PrintCurrentConditions(cout, termTricks, cc);
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
static bool ShowDailyForecasts(Credence::Peer & peer,
                               TerminalTricks & termTricks)
{
  bool  rc = false;
  uint8_t  cmd = 2;
  if (peer.Send(cmd)) {
    Mcweather::PeriodForecasts  dailyForecasts;
    if (peer.Receive(dailyForecasts)) {
      PrintDailyForecasts(cout, termTricks, dailyForecasts);
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
int main(int argc, char *argv[])
{
  MyArgType  args;
  InitArgs(args);
  
  int  argind = args.Parse(argc, argv);
  if (argind < 0) {
    cerr << args.Usage(argv[0], "");
    exit(1);
  }

  // Dwm::SysLogger::Open("mcweather", LOG_PID|LOG_PERROR, LOG_USER);
  TerminalTricks  termTricks;
  Credence::Peer  peer;

  if (peer.Connect(args.Get<'s'>(), 2124)) {
    Credence::KeyStash   keyStash;
    Credence::KnownKeys  knownKeys;
    if (peer.Authenticate(keyStash, knownKeys)) {
      if (args.Get<'o'>()) {
        ShowObservationStations(peer);
      }
      if (args.Get<'c'>()) {
        ShowCurrentConditions(peer, termTricks);
      }
      if (args.Get<'d'>()) {
        ShowDailyForecasts(peer, termTricks);
      }
      if (args.Get<'h'>()) {
        ShowHourlyForecasts(peer, termTricks);
      }
      peer.Send((uint8_t)255);
    }
  }
  else {
    cerr << "Failed to connect to " << args.Get<'s'>() << '\n';
    return 1;
  }
}
