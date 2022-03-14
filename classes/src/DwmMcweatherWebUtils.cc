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
//!  \file DwmMcweatherWebUtils.cc
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::WebUtils class implementation
//---------------------------------------------------------------------------

#include <fstream>
#include <iostream>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/ConsoleCertificateHandler.h>
#include <Poco/URI.h>

#include "DwmSysLogger.hh"
#include "DwmMcweatherWebUtils.hh"

namespace Dwm {

  namespace Mcweather {

    using namespace std;
    using Poco::URI;
    using Poco::Net::HTTPClientSession;
    using Poco::Net::HTTPSClientSession;
    using Poco::Net::HTTPMessage;
    using Poco::Net::HTTPRequest;
    using Poco::Net::HTTPResponse;
    using Poco::Net::Context;

    static std::mutex    g_ctxmtx;
    static Context::Ptr  g_ctx = nullptr;

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int WebUtils::GetStatus(const string & host, const string & uristr)
    {
      int  rc = 0;

      {
        lock_guard<mutex>  lck(g_ctxmtx);
        if (nullptr == g_ctx) {
          try {
            g_ctx = new Context(Context::CLIENT_USE, "", "", "",
                                Context::VERIFY_NONE, 9, false,
                                "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
          }
          catch (Poco::Exception & ex) {
            Syslog(LOG_ERR, "Exception: %s", ex.displayText().c_str());
            return 0;
          }
          catch (std::exception & ex) {
            Syslog(LOG_ERR, "Exception: %s", ex.what());
            return 0;
          }
        }
      }
      
      Poco::URI    uri(uristr);
      if (uri.getPathEtc().empty()) {
        uri.setPathEtc("/");
      }
      HTTPClientSession    *sess;
      if (uri.getScheme() == "https") {
        sess = new HTTPSClientSession(host, uri.getPort(), g_ctx);
      }
      else {
        sess = new HTTPClientSession(host);
      }
      
      Poco::Timespan  timeout(5, 0);
      sess->setTimeout(timeout, timeout, timeout);
      
      HTTPRequest  req(HTTPRequest::HTTP_GET, uri.getPathAndQuery(),
                       HTTPMessage::HTTP_1_1);
      req.setHost(uri.getHost());
      req.set("User-Agent", "qmcrover/mcrover@caida.org");
      
      try {
        ostream & os = sess->sendRequest(req);
        if (os) {
          HTTPResponse  resp;
          istream  & is = sess->receiveResponse(resp);
          rc = resp.getStatus();
          sess->reset();
        }
      }
      catch (Poco::Exception & ex) {
        Syslog(LOG_ERR, "Exception, URI %s, %s, %s, %s",
               uristr.c_str(), ex.className(),
               ex.name(), ex.displayText().c_str());
      }
      catch (...) {
        Syslog(LOG_ERR, "Unknown exception");
      }
      delete sess;
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    int WebUtils::GetStatus(const string & uristr)
    {
      Poco::URI    uri(uristr);
      return GetStatus(uri.getHost(), uristr);
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool WebUtils::GetJson(const std::string & uristr, nlohmann::json & json)
    {
      bool  rc = false;
      json.clear();
      
      lock_guard<mutex>  lck(g_ctxmtx);
      if (nullptr == g_ctx) {
        try {
          g_ctx = new Context(Context::CLIENT_USE, "", "", "",
                              Context::VERIFY_NONE, 9, false,
                              "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
        }
        catch (Poco::Exception & ex) {
          Syslog(LOG_ERR, "Exception: %s", ex.displayText().c_str());
          return false;
        }
        catch (std::exception & ex) {
          Syslog(LOG_ERR, "Exception: %s", ex.what());
          return false;
        }
      }

      Poco::URI    uri(uristr);
      std::string  host(uri.getHost());
      std::string  path(uri.getPathAndQuery());
      HTTPClientSession    *sess;
      if (uri.getScheme() == "https") {
        sess = new HTTPSClientSession(host, uri.getPort(), g_ctx);
      }
      else {
        sess = new HTTPClientSession(host);
      }
      HTTPRequest  req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
      req.set("User-Agent", "qmcrover/mcrover@caida.org");
      
      try {
        ostream & os = sess->sendRequest(req);
        if (os) {
          HTTPResponse  resp;
          istream  & is = sess->receiveResponse(resp);
          if (resp.getStatus() == 200) {
            is >> json;
            if (! json.empty()) {
              rc = true;
            }
            else {
              Syslog(LOG_ERR, "Failed to parse JSON from %s", uristr.c_str());
            }
          }
          else {
            Syslog(LOG_ERR, "Got response code %d from %s",
                   resp.getStatus(), uristr.c_str());
          }
        }
        else {
          Syslog(LOG_ERR, "Failed to send GET to %s", uristr.c_str());
        }
      }
      catch (Poco::Exception & ex) {
        Syslog(LOG_ERR, "Exception, URI %s, %s, %s, %s",
               uristr.c_str(), ex.className(),
               ex.name(), ex.displayText().c_str());
        json.clear();
      }
      catch (...) {
        Syslog(LOG_ERR, "Unknown exception");
        json.clear();
      }
      
      delete sess;
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool WebUtils::SaveGIF(const string & uristr, const string & saveFile)
    {
      bool  rc = false;
      lock_guard<mutex>  lck(g_ctxmtx);
      if (nullptr == g_ctx) {
        try {
          g_ctx = new Context(Context::CLIENT_USE, "", "", "",
                              Context::VERIFY_NONE, 9, false,
                              "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
        }
        catch (Poco::Exception & ex) {
          Syslog(LOG_ERR, "Exception: %s", ex.displayText().c_str());
          return false;
        }
        catch (std::exception & ex) {
          Syslog(LOG_ERR, "Exception: %s", ex.what());
          return false;
        }
      }

      Poco::URI    uri(uristr);
      std::string  host(uri.getHost());
      std::string  path(uri.getPathAndQuery());
      HTTPClientSession    *sess;
      if (uri.getScheme() == "https") {
        sess = new HTTPSClientSession(host, uri.getPort(), g_ctx);
      }
      else {
        sess = new HTTPClientSession(host);
      }
      HTTPRequest  req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
      req.set("User-Agent", "qmcrover/mcrover@caida.org");
      req.set("Accept", "image/gif");
      
      try {
        ostream & os = sess->sendRequest(req);
        if (os) {
          HTTPResponse  resp;
          istream  & is = sess->receiveResponse(resp);
          if (resp.getStatus() == 200) {
            ofstream  saveos(saveFile);
            if (saveos) {
              saveos << is.rdbuf();
              rc = true;
              saveos.close();
            }
            else {
              Syslog(LOG_ERR, "Failed to open '%s': %m", saveFile.c_str());
            }
          }
          else {
            Syslog(LOG_ERR, "Got response code %d from %s",
                   resp.getStatus(), uristr.c_str());
          }
        }
        else {
          Syslog(LOG_ERR, "Failed to send GET to %s", uristr.c_str());
        }
      }
      catch (Poco::Exception & ex) {
        Syslog(LOG_ERR, "Exception, URI %s, %s, %s, %s",
               uristr.c_str(), ex.className(),
               ex.name(), ex.displayText().c_str());
      }
      catch (...) {
        Syslog(LOG_ERR, "Unknown exception");
      }
      
      delete sess;
      return rc;
    }
    

  }  // namespace Mcweather

}  // namespace Dwm
