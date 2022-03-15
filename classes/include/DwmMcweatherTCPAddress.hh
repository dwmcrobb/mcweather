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
//!  \file DwmMcweatherTCPAddress.hh
//!  \author Daniel W. McRobb
//!  \brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCWEATHERTCPADDRESS_HH_
#define _DWMMCWEATHERTCPADDRESS_HH_


extern "C" {
  #include <sys/types.h>
  #include <sys/socket.h>
}

#include <variant>

#include "DwmIpAddress.hh"
#include "DwmStreamIOCapable.hh"

namespace Dwm {

  namespace Mcweather {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class TCPAddress
      : public StreamIOCapable
    {
    public:
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      TCPAddress()
          : _addr(Ipv4Address(INADDR_NONE)), _port(0)
      {}

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      TCPAddress(const TCPAddress &) = default;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      TCPAddress(const Ipv6Address & addr, uint16_t port);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      TCPAddress(const Ipv4Address & addr, uint16_t port);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      TCPAddress(const std::string & addr, uint16_t port);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const IpAddress & Addr() const  { return _addr; }
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      uint16_t Port() const   { return _port; }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      operator std::string () const;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool operator < (const TCPAddress & addr) const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool operator == (const TCPAddress & addr) const;

      std::istream & Read(std::istream & is) override;
      std::ostream & Write(std::ostream & os) const override;
      
    private:
      IpAddress  _addr;
      uint16_t   _port;
    };
    
  }  // namespace Mcweather

}  // namespace Dwm

#endif  // _DWMMCWEATHERTCPADDRESS_HH_
