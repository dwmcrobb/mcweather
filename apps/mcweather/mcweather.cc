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
//!  \brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <sstream>

#include "DwmSysLogger.hh"
#include "DwmTerminalTricks.hh"
#include "DwmCredencePeer.hh"
#include "DwmMcweatherCurrentConditions.hh"
#include "DwmMcweatherPeriodForecasts.hh"

using namespace std;
using namespace Dwm;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
string FormattedPeriodForecast(const Mcweather::PeriodForecast & forecast,
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
int main(int argc, char *argv[])
{
  //  Dwm::SysLogger::Open("mcweather", LOG_PID|LOG_PERROR, LOG_USER);
  TerminalTricks  termTricks;
  Credence::Peer  peer;

  if (peer.Connect("127.0.0.1", 2124)) {
    Credence::KeyStash   keyStash("/home/dwm/.credence");
    Credence::KnownKeys  knownKeys("/home/dwm/.credence");
    if (peer.Authenticate(keyStash, knownKeys)) {
      map<string,Mcweather::CurrentConditions>  cc;
      uint8_t  cmd = 1;
      if (peer.Send(cmd)) {
        if (peer.Receive(cc)) {
          for (const auto & c : cc) {
            cout << c.second.Station() << '\n'
                 << " temperature " << c.second.Temperature() << "F\n"
                 << " humidity    " << c.second.RelativeHumidity() << "%\n";
          }
          cmd = 2;
          if (peer.Send(cmd)) {
            Mcweather::PeriodForecasts  periodForecasts;
            if (peer.Receive(periodForecasts)) {
              for (const auto & f : periodForecasts.Forecasts()) {
                termTricks.Bold(cout, true);
                termTricks.Underscore(cout, true);
                cout << std::left << setw(15) << f.Name() << ' '
                     << std::right << setw(3) << f.Temperature()
                     << f.TemperatureUnit()
                     << ' ' << f.ShortForecast() << '\n';
                termTricks.Bold(cout, false);
                termTricks.Underscore(cout, false);
                cout << FormattedPeriodForecast(f, termTricks) << "\n\n";
              }
            }
          }
        }
      }
      
    }
  }
}
