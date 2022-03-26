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
//!  \file TestConfig.cc
//!  \author Daniel W. McRobb
//!  \brief Unit tests for Dwm::Mcweather::Config class
//---------------------------------------------------------------------------

#include <fstream>

#include "DwmSysLogger.hh"
#include "DwmUnitAssert.hh"
#include "DwmMcweatherConfig.hh"

using namespace std;
using Dwm::Assertions;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  Dwm::SysLogger::Open("TestConfig", LOG_PID|LOG_PERROR, LOG_USER);
  
  int  rc = 1;

  string  cfgFile("./test.cfg");
  if (argc > 1) {
    cfgFile = argv[1];
  }

  using batcp = boost::asio::ip::tcp;
  
  Dwm::Mcweather::Config  config;
  if (UnitAssert(config.Parse(cfgFile))) {
    UnitAssert(config.Service().Addresses().size() == 2);
    UnitAssert(config.Service().Addresses().find(batcp::endpoint(batcp::v4(), 2124))
               != config.Service().Addresses().end());
    UnitAssert(config.Service().Addresses().find(batcp::endpoint(batcp::v6(), 2124))
               != config.Service().Addresses().end());

    ofstream  tmpofs("./tmp_config.cfg");
    if (UnitAssert(tmpofs)) {
      tmpofs << config;
      tmpofs.close();

      if (UnitAssert(config.Parse("./tmp_config.cfg"))) {
        UnitAssert(config.Service().Addresses().size() == 2);
        UnitAssert(config.Service().Addresses().find(batcp::endpoint(batcp::v4(), 2124))
                   != config.Service().Addresses().end());
        UnitAssert(config.Service().Addresses().find(batcp::endpoint(batcp::v6(), 2124))
                   != config.Service().Addresses().end());
      }
      std::remove("./tmp_config.cfg");
    }
  }

  if (Assertions::Total().Failed()) {
    Assertions::Print(cerr, true);
  }
  else {
    cout << Assertions::Total() << " passed" << endl;
    rc = 0;
  }
  return rc;
}
