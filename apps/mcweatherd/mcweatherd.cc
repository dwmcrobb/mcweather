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
//!  \brief weather cache server daemon
//---------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>

#include "DwmDaemonUtils.hh"
#include "DwmSysLogger.hh"
#include "DwmMcweatherCurrentConditions.hh"
#include "DwmMcweatherServer.hh"
#include "DwmMcweatherWeatherFetcher.hh"

using namespace std;
using namespace Dwm;

static string  g_pidFile;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void SavePID(const string & pidFile)
{
#ifndef O_EXLOCK
  #define O_EXLOCK 0
#endif
  g_pidFile = pidFile;
  pid_t  pid = getpid();
  string  &&pidstr = to_string(pid) + "\n";
  int    fd = open(g_pidFile.c_str(), O_WRONLY|O_CREAT|O_TRUNC|O_EXLOCK, 0644);
  if (fd >= 0) {
    write(fd, pidstr.c_str(), pidstr.size());
    close(fd);
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void RemovePID()
{
  if (! g_pidFile.empty()) {
    std::remove(g_pidFile.c_str());
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void Usage(const char *argv0)
{
  cerr << "Usage: " << argv0 << " [-c configFile] [-p pidFile]\n"
       << "  Default configFile: /usr/local/etc/mcweatherd.cfg\n"
       << "  Default pidFile: /var/run/mcweatherd.pid\n";
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  string  configFile("/usr/local/etc/mcweatherd.cfg");
  string  pidFile("/var/run/mcweatherd.pid");
  
  int  optChar;
  while ((optChar = getopt(argc, argv, "c:p:")) != -1) {
    switch (optChar) {
      case 'c':
        configFile = optarg;
        break;
      case 'p':
        pidFile = optarg;
        break;
      default:
        Usage(argv[0]);
        return 1;
        break;
    }
  }

  Mcweather::Config  config;
  if (config.Parse(configFile)) {
    DaemonUtils::Daemonize();
    Dwm::SysLogger::Open("mcweatherd", LOG_PID, config.SyslogFacility());
    Dwm::SysLogger::MinimumPriority(config.SyslogLevel());
    Dwm::SysLogger::ShowFileLocation(config.SyslogLocations());

    Mcweather::WeatherFetcher  fetcher(config);
    if (fetcher.Start()) {
      SavePID(pidFile);
      atexit(RemovePID);
      boost::asio::io_context context;
      Mcweather::Server  server(context.get_executor(), config);
      context.run();
      for (;;) {
        sleep(1);
      }
    }
    else {
      Syslog(LOG_ERR, "Failed to start fetcher!!!");
      return 1;
    }
  }
  else {
    cerr << "failed to parse configuration in '" << configFile << "'\n";
    return 1;
  }
  
  return 0;
}
