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
      _peer.SetKeyExchangeTimeout(chrono::milliseconds(2000));
      _peer.SetIdExchangeTimeout(chrono::milliseconds(2000));
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
    bool Responder::SendObservationStations()
    {
      bool  rc = false;
      Cache  cache(_server.GetConfig().CacheDirectory(),
                   _server.GetConfig().Weather());
      Cache::ObservationStations  stations;
      cache.GetObservationStations(stations);
      if (_peer.Send(stations)) {
        rc = true;
      }
      else {
        Syslog(LOG_ERR, "Failed to send observation stations to client %s",
               _peer.Id().c_str());
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Responder::SendHourlyForecasts()
    {
      bool  rc = false;
      Cache  cache(_server.GetConfig().CacheDirectory(),
                   _server.GetConfig().Weather());
      PeriodForecasts  forecasts;
      cache.GetHourlyForecasts(forecasts);
      if (_peer.Send(forecasts)) {
        rc = true;
      }
      else {
        Syslog(LOG_ERR, "Failed to send hourly forecasts to client %s",
               _peer.Id().c_str());
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Responder::HandleRequest(WeatherdRequest cmd)
    {
      bool  rc = false;

      switch (cmd) {
        case e_currentConditions:    rc = SendCurrentConditions();    break;
        case e_dailyForecasts:       rc = SendPeriodForecasts();      break;
        case e_observationStations:  rc = SendObservationStations();  break;
        case e_hourlyForecasts:      rc = SendHourlyForecasts();      break;
        case e_buhBye:                                                break;
        default:
          Syslog(LOG_ERR, "Invalid command %hhu from %s", cmd,
                 _peer.Id().c_str());
          break;
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
        WeatherdRequest  cmd;
        while (_peer.Receive((uint8_t &)cmd)) {
          if (! HandleRequest(cmd)) {
            break;
          }
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
