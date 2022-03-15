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
//!  \file DwmMcweatherTCPAddress.cc
//!  \author Daniel W. McRobb
//!  \brief Dwm::Mcweather::TCPAddress class implementation
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
}

#include "DwmMcweatherTCPAddress.hh"
#include "DwmStreamIO.hh"

namespace Dwm {

  namespace Mcweather {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    TCPAddress::TCPAddress(const Ipv6Address & addr, uint16_t port)
        : _addr(addr), _port(port)
    {}
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    TCPAddress::TCPAddress(const Ipv4Address & addr, uint16_t port)
        : _addr(addr), _port(port)
    {}

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    TCPAddress::TCPAddress(const std::string & addr, uint16_t port)
    {
      if (addr == "inaddr_any") {
        _addr = Ipv4Address(INADDR_ANY);
      }
      else if (addr == "in6addr_any") {
        _addr = Ipv6Address(in6addr_any);
      }
      else {
        struct in6_addr  in6Addr;
        if (inet_pton(AF_INET6, addr.c_str(), &in6Addr) == 1) {
          _addr = Ipv6Address(in6Addr);
        }
        else {
          struct in_addr  inAddr;
          if (inet_pton(AF_INET, addr.c_str(), &inAddr) == 1) {
            _addr = Ipv4Address(inAddr.s_addr);
          }
          else {
            _addr = Ipv4Address(INADDR_NONE);
          }
        }
      }
      _port = port;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    TCPAddress::operator std::string () const
    {
      std::string   rc = (std::string)_addr + ':';
      rc += std::to_string(_port);
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool TCPAddress::operator < (const TCPAddress & addr) const
    {
      bool  rc = false;
      if (_addr < addr._addr) {
        rc = true;
      }
      else if (_addr == addr._addr) {
        rc = (_port < addr._port);
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool TCPAddress::operator == (const TCPAddress & addr) const
    {
      return ((_addr == addr._addr) && (_port == addr._port));
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::istream & TCPAddress::Read(std::istream & is)
    {
      if (is) {
        if (StreamIO::Read(is, _addr)) {
          StreamIO::Read(is, _port);
        }
      }
      return is;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::ostream & TCPAddress::Write(std::ostream & os) const
    {
      if (os) {
        if (StreamIO::Write(os, _addr)) {
          StreamIO::Write(os, _port);
        }
      }
      return os;
    }
    
    
  }  // namespace Mcweather

}  // namespace Dwm
