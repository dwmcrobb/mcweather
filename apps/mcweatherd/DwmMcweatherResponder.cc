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
//!  \file DwmMcweatherResponder.cc
//!  \author Daniel W. McRobb
//!  \brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
}

#include <thread>

#include <boost/asio.hpp>

#include "DwmSysLogger.hh"
#include "DwmCredencePeer.hh"
#include "DwmMcweatherCache.hh"
#include "DwmMcweatherServer.hh"

namespace Dwm {

  namespace Mcweather {

    using namespace std;

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    Responder::Responder(boost::asio::ip::tcp::socket && s, Server & server)
        :  _peer(), _server(server), _running(false)
    {
      boost::system::error_code  ec;
      s.native_non_blocking(false, ec);
      if (_peer.Accept(std::move(s))) {
        _thread = thread(&Responder::Run, this);
        _running.store(true);
      }
      else {
        _peer.Disconnect();
        Syslog(LOG_ERR, "Failed to accept peer");
      }
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    Responder::~Responder()
    {
      Join();
      Syslog(LOG_INFO, "Responder destroyed for %s", _peer.Id().c_str());
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Responder::Join()
    {
      bool  rc = false;
      bool  isRunning = _running.load();
      if (! isRunning) {
        if (_thread.joinable()) {
          _thread.join();
          rc = true;
          Syslog(LOG_DEBUG, "Joined thread for responder %s",
                 _peer.Id().c_str());
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Responder::SendCurrentConditions()
    {
      bool  rc = false;
      Cache  cache(_server.GetConfig().CacheDirectory(),
                   _server.GetConfig().Weather());
      std::map<std::string, CurrentConditions>  cc;
      cache.GetCurrentConditions(cc);
      if (_peer.Send(cc)) {
        rc = true;
      }
      else {
        Syslog(LOG_ERR, "Failed to write current conditions to client %s",
               _peer.Id().c_str());
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Responder::SendPeriodForecasts()
    {
      bool  rc = false;
      Cache  cache(_server.GetConfig().CacheDirectory(),
                   _server.GetConfig().Weather());
      PeriodForecasts  forecasts;
      cache.GetPeriodForecasts(forecasts);
      if (_peer.Send(forecasts)) {
        rc = true;
      }
      else {
        Syslog(LOG_ERR, "Failed to write current conditions to client %s",
               _peer.Id().c_str());
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Responder::HandleCommand(uint8_t cmd)
    {
      bool  rc = false;

      Syslog(LOG_INFO, "Got cmd %hhu from %s", cmd, _peer.Id().c_str());
      switch (cmd) {
        case 1:          rc = SendCurrentConditions();   break;
        case 2:          rc = SendPeriodForecasts();     break;
#if 0
        case 3:          SendAllAlerts(os);         break;
#endif
        default:         rc = SendCurrentConditions();   break;
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void Responder::Run()
    {
      Syslog(LOG_INFO, "Responder started");
      if (_peer.Authenticate(_server.GetKeyStash(), _server.GetKnownKeys())) {
        Syslog(LOG_INFO, "Authenticated client %s", _peer.Id().c_str());
        uint8_t  cmd;
        while (_peer.Receive(cmd)) {
          HandleCommand(cmd);
        }
      }
      else {
        Syslog(LOG_ERR, "Authentication failed for client");
      }
      _peer.Disconnect();
      Syslog(LOG_DEBUG, "Responder done");
      
      _running.store(false);
      return;
    }
    
    
  }  // namespace Mcweather

}  // namespace Dwm
