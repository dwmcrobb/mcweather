%code requires
{
  #include <string>
  #include <vector>
  #include <boost/asio.hpp>

  #include "DwmIpPrefix.hh"

  using std::vector, std::set, std::string, std::pair,
        Dwm::Ipv4Address, Dwm::Ipv4Prefix, Dwm::Ipv6Address;
}

%{
  #include <cstdio>

  extern "C" {
    #include <netdb.h>

    extern void mcweathercfgerror(const char *arg, ...);
    extern FILE *mcweathercfgin;
  }
        
  #include <string>
  #include <vector>

  #include "DwmSysLogger.hh"
  #include "DwmMcweatherConfig.hh"
  #include "DwmMcweatherUtils.hh"

  using namespace std;
  
  string                   g_configPath;
  Dwm::Mcweather::Config  *g_config = nullptr;

%}

%define api.prefix {mcweathercfg}

%union {
  int                                            intVal;
  uint32_t                                       uint32Val;
  float                                          floatVal;
  string                                        *stringVal;
  vector<string>                                *stringVecVal;
  vector<uint16_t>                              *uint16VecVal;
  Ipv4Address                                   *ipv4AddrVal;
  Ipv6Address                                   *ipv6AddrVal;
  pair<string,uint8_t>                          *stringUint8PairVal;
  vector<pair<string,uint8_t>>                  *stringUint8PairVecVal;
  vector<Dwm::Ipv4Prefix>                       *ipv4PrefixVecVal;
  pair<Ipv4Prefix,vector<Dwm::Ipv4Prefix>>      *ipv4PrefixVecPairVal;
  vector<pair<Ipv4Prefix,vector<Ipv4Prefix>>>   *ipv4PrefixVecVecVal;
  pair<uint16_t,uint16_t>                       *uint16PairVal;
  pair<float,float>                             *floatPairVal;
  Dwm::Mcweather::WeatherConfig                 *weatherConfigVal;
  set<Dwm::IpPrefix>                            *ipPrefixSetVal;
  Dwm::Mcweather::ServiceConfig                 *serviceConfigVal;
  boost::asio::ip::tcp::endpoint                *serviceAddrVal;
  std::set<boost::asio::ip::tcp::endpoint>      *serviceAddrSetVal;
  pair<string,string>                           *stringStringPairVal;
  std::map<string,string>                       *stringStringMapVal;
}

%code provides
{
  // Tell Flex the expected prototype of yylex.
  #define YY_DECL                             \
    int mcweathercfglex ()

  // Declare the scanner.
  YY_DECL;
}

%token ADDRESS ADDRESSES ALLOWEDCLIENTS CACHE FACILITY IPV4 IPV6
%token KEYDIRECTORY LATITUDE LEVEL LOCATION LOGLOCATIONS LONGITUDE PORT
%token SERVICE STATIONS STATUS STORE STRIPDOMAINS SYSLOG URI WEATHER

%token<stringVal>  STRING
%token<intVal>     INTEGER

%type<intVal>                  TCP4Port
%type<floatVal>                Latitude Longitude
%type<stringVal>               CacheDirectory
%type<stringVal>               KeyDirectory
%type<stringVal>               Uri
%type<stringVecVal>            VectorOfString Stations
%type<floatPairVal>            Location LocationData
%type<weatherConfigVal>        WeatherData
%type<serviceConfigVal>        ServiceSettings
%type<serviceAddrSetVal>       ServiceAddresses ServiceAddressSet
%type<ipPrefixSetVal>          AllowedClients
%type<serviceAddrVal>          ServiceAddress

%%

Config: TopStanza | Config TopStanza;

TopStanza: Cache | Service | Syslog | Weather;

Cache: CACHE '{' CacheDirectory '}' ';'
{
  if (g_config) {
     g_config->CacheDirectory(*$3);
  }
  delete $3;
};

CacheDirectory: STORE '=' STRING ';'
{
  $$ = $3;
};

Syslog: SYSLOG '{' SyslogSettings '}' ';';

SyslogSettings: SyslogSetting | SyslogSettings SyslogSetting;

SyslogSetting: SyslogFacility | SyslogLevel | SyslogLocations
{};

SyslogLevel: LEVEL '=' STRING ';'
{
  if (g_config) {
    g_config->SyslogLevel(*$3);
  }
  delete $3;
};

SyslogFacility: FACILITY '=' STRING ';'
{
  if (g_config) {
    g_config->SyslogFacility(*$3);
  }
  delete $3;
};

SyslogLocations: LOGLOCATIONS '=' STRING ';'
{
  if (g_config) {
    if (strncasecmp($3->c_str(), "yes", 3) == 0) {
      g_config->SyslogLocations(true);
    }
  }
  delete $3;
};

Service: SERVICE '{' ServiceSettings '}' ';'
{
  if (g_config) {
    g_config->RunService(true);
    g_config->Service(*$3);
  }
  delete $3;
};

ServiceSettings: ServiceAddresses
{
  $$ = new Dwm::Mcweather::ServiceConfig();
  $$->Addresses(*$1);
  delete $1;
}
| KeyDirectory
{
  $$ = new Dwm::Mcweather::ServiceConfig();
  $$->KeyDirectory(*$1);
  delete $1;
}
| AllowedClients
{
  $$ = new Dwm::Mcweather::ServiceConfig();
  $$->AllowedClients() = *$1;
  delete $1;
}
| ServiceSettings ServiceAddresses
{
  $$->Addresses(*$2);
  delete $2;
}
| ServiceSettings KeyDirectory
{
  $$->KeyDirectory(*$2);
  delete $2;
}
| ServiceSettings AllowedClients
{
  $$->AllowedClients() = *$2;
  delete $2;
};

ServiceAddresses: ADDRESSES '=' '[' ServiceAddressSet ']' ';'
{
    $$ = $4;
};

ServiceAddressSet: ServiceAddress
{
  $$ = new std::set<boost::asio::ip::tcp::endpoint>();
  $$->insert(*$1);
  delete $1;
}
| ServiceAddressSet ',' ServiceAddress
{
  $$->insert(*$3);
  delete $3;
};

ServiceAddress: '{' ADDRESS '=' STRING ';' '}'
{
  using batcp = boost::asio::ip::tcp;
    
  if (*$4 == "in6addr_any") {
      $$ = new batcp::endpoint(batcp::v6(), 2124);
  }
  else if (*$4 == "inaddr_any") {
      $$ = new batcp::endpoint(batcp::v4(), 2124);
  }
  else {
    boost::system::error_code  ec;
    boost::asio::ip::address  addr =
      boost::asio::ip::address::from_string(*$4, ec);
    if (ec) {
      mcweathercfgerror("invalid IP address");
      delete $4;
      return 1;
    }
    $$ = new boost::asio::ip::tcp::endpoint(addr, 2124);
  }
  delete $4;
}
| '{' ADDRESS '=' STRING ';' PORT '=' TCP4Port ';' '}'
{
  namespace baip = boost::asio::ip;
  using batcp =	boost::asio::ip::tcp;
  
  if (($8 <= 0) || ($8 > 65535)) {
    mcweathercfgerror("invalid port");
    delete $4;
    return 1;
  }

  if (*$4 == "in6addr_any") {
      $$ = new batcp::endpoint(batcp::v6(), $8);
  }
  else if (*$4 == "inaddr_any") {
      $$ = new batcp::endpoint(batcp::v4(), $8);
  }
  else {
    boost::system::error_code  ec;
    baip::address  addr = baip::address::from_string(*$4, ec);
    if (ec) {
      mcweathercfgerror("invalid IP address");
      delete $4;
      return 1;
    }
    $$ = new batcp::endpoint(addr, $8);
  }
  delete $4;
}
| '{' PORT '=' TCP4Port ';' ADDRESS '=' STRING ';' '}'
{
  namespace baip = boost::asio::ip;
  using batcp = boost::asio::ip::tcp;

  if (($4 <= 0) || ($4 > 65535)) {
    mcweathercfgerror("invalid port");
    delete $8;
    return 1;
  }
  baip::address  addr;
  if (*$8 == "in6addr_any") {
    $$ = new batcp::endpoint(batcp::v6(), $4);
  }
  else if (*$8 == "inaddr_any") {
    $$ = new batcp::endpoint(batcp::v4(), $4);
  }
  else {
    boost::system::error_code  ec;
    baip::address addr = baip::address::from_string(*$8, ec);
    if (ec) {
      mcweathercfgerror("invalid IP address");
      delete $8;
      return 1;
    }
    $$ = new batcp::endpoint(addr, $4);
  }
  delete $8;
};

KeyDirectory : KEYDIRECTORY '=' STRING ';'
{
  $$ = $3;
};

AllowedClients: ALLOWEDCLIENTS '=' '[' VectorOfString ']' ';'
{
  $$ = new std::set<Dwm::IpPrefix>();
  for (const auto & pfxstr : *$4) {
    Dwm::IpPrefix   pfx(pfxstr);
    if (pfx.Family() != AF_INET) {
      $$->insert(pfx);
    }
    else {
      if (pfx.Prefix<Ipv4Prefix>()->Network().Raw() != INADDR_NONE) {
        $$->insert(pfx);
      }
      else {
        mcweathercfgerror("invalid IP prefix");
        return 1;
      }
    }
  }
  delete $4;
}
|
{};

TCP4Port: INTEGER
{
  if (($1 > 0) && ($1 < 65536)) {
    $$ = $1;
  }
  else {
    mcweathercfgerror("invalid TCP port number");
    return 1;
  }
}
| STRING
{
  auto  servEntry = getservbyname($1->c_str(), "tcp");
  if (servEntry) {
    $$ = ntohs(servEntry->s_port);
  }
  else {
      mcweathercfgerror("unknown TCP service");
      delete $1;
      return 1;
  }
  delete $1;
};

Uri: URI '=' STRING ';'
{
  $$ = $3;
};

Weather: WEATHER '{' WeatherData '}' ';'
{
  g_config->Weather(*$3);
  delete $3;
};

WeatherData: Location
{
  $$ = new Dwm::Mcweather::WeatherConfig();
  $$->Latitude($1->first);
  $$->Longitude($1->second);
  delete $1;
}
| Stations
{
  $$ = new Dwm::Mcweather::WeatherConfig();
  $$->Stations(*$1);
  delete $1;
}
| WeatherData Location
{
  $$->Latitude($2->first);
  $$->Longitude($2->second);
  delete $2;
}
| WeatherData Stations
{
  $$->Stations(*$2);
  delete $2;
}
;

Location: LOCATION '=' '{' LocationData '}' ';'
{
  $$ = $4;
};

LocationData: Latitude ',' Longitude
{
  $$ = new pair<float,float>($1, $3);
}
| Longitude ',' Latitude
{
  $$ = new pair<float,float>($3, $1);
};

Latitude: LATITUDE '=' STRING
{
  $$ = std::strtof($3->c_str(), nullptr);
  delete $3;
};

Longitude: LONGITUDE '=' STRING
{
  $$ = strtof($3->c_str(), nullptr);
  delete $3;
};

Stations: STATIONS '=' '[' VectorOfString ']' ';'
{
  $$ = $4;
};

VectorOfString: STRING
{
  $$ = new vector<string>();
  $$->push_back(*$1);
  delete $1;
}
| VectorOfString ',' STRING    { $$->push_back(*$3); delete $3; }
;

%%

namespace Dwm {

  namespace Mcweather {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    Config::Config()
        : _cacheDirectory("/tmp/weatherCache"), _runService(false),
          _service(), _syslogFacility("daemon"), _syslogLevel("info"),
          _logLocations(false)
    {
    }
      
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Config::Parse(const string & path)
    {
      using  batcp = boost::asio::ip::tcp;
        
      bool  rc = false;
      Clear();
      
      mcweathercfgin = fopen(path.c_str(), "r");
      if (mcweathercfgin) {
        g_configPath = path;
        g_config = this;
        rc = (0 == mcweathercfgparse());
        fclose(mcweathercfgin);
      }
      else {
        Syslog(LOG_ERR, "Failed to open config file '%s'", path.c_str());
      }
      if (rc) {
        if (_service.Addresses().empty()) {
          _service.AddAddress(batcp::endpoint(batcp::v6(), 2124));
          _service.AddAddress(batcp::endpoint(batcp::v4(), 2124));
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Config::CacheDirectory() const
    {
      return _cacheDirectory;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Config::CacheDirectory(const string & cacheDir)
    {
      _cacheDirectory = cacheDir;
      return _cacheDirectory;
    }

    //-----------------------------------------------------------------------
    //!  
    //-----------------------------------------------------------------------
    const string & Config::SyslogFacility() const
    {
      return _syslogFacility;
    }
    
    //-----------------------------------------------------------------------
    //!  
    //-----------------------------------------------------------------------
    const string & Config::SyslogFacility(const string & facility)
    {
      _syslogFacility = facility;
      return _syslogFacility;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Config::SyslogLevel() const
    {
      return _syslogLevel;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const string & Config::SyslogLevel(const string & level)
    {
      _syslogLevel = level;
      return _syslogLevel;
    }

    //-----------------------------------------------------------------------
    //!  
    //-----------------------------------------------------------------------
    bool Config::SyslogLocations() const
    {
      return _logLocations;
    }
    

    //-----------------------------------------------------------------------
    //!  
    //-----------------------------------------------------------------------
    bool Config::SyslogLocations(bool logLocations)
    {
      _logLocations = logLocations;
      return _logLocations;
    }

    //-----------------------------------------------------------------------
    //!  
    //-----------------------------------------------------------------------
    bool Config::RunService() const
    {
      return _runService;
    }
    
    //-----------------------------------------------------------------------
    //!  
    //-----------------------------------------------------------------------
    bool Config::RunService(bool runService)
    {
      _runService = runService;
      return _runService;
    }

    //-----------------------------------------------------------------------
    //!  
    //-----------------------------------------------------------------------
    const ServiceConfig & Config::Service() const
    {
      return _service;
    }
    
    //-----------------------------------------------------------------------
    //!  
    //-----------------------------------------------------------------------
    const ServiceConfig Config::Service(const ServiceConfig & service)
    {
      _service = service;
      return _service;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const WeatherConfig & Config::Weather() const
    {
      return _weather;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const WeatherConfig & Config::Weather(const WeatherConfig & weather)
    {
      _weather = weather;
      return _weather;
    }
    
    //-----------------------------------------------------------------------
    //!  
    //-----------------------------------------------------------------------
    void Config::Clear()
    {
      _cacheDirectory = "/tmp/weatherCache";
      _runService = false;
      _service.Clear();
      _syslogFacility = "daemon";
      _syslogLevel = "info";
      _logLocations = false;
      _weather = WeatherConfig();
      return;
    }

    //-----------------------------------------------------------------------
    //!  
    //-----------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os, const Config & cfg)
    {
      os << "#=============================================================="
        "==============\n"
         << "#  Cache configuration.\n"
         << "#=============================================================="
        "==============\n"
         << "cache {\n"
         << "    #----------------------------------------------------------"
        "--------------\n"
         << "    #  Directroy in which to store cached forecast.\n"
         << "    #----------------------------------------------------------"
        "--------------\n"
         << "    store = \"" << cfg._cacheDirectory << "\";\n"
         << "};\n\n";
      
      os << cfg._service << '\n';

      os << "#=============================================================="
          "==============\n"
         << "#  syslog configuration.\n"
         << "#=============================================================="
          "==============\n"
         << "syslog {\n"
         << "    #----------------------------------------------------------"
          "--------------\n"
         << "    #  Syslog facility.  Defaults to \"daemon\" if not set.\n"
         << "    #----------------------------------------------------------"
          "--------------\n"
         << "    facility = \"" << cfg._syslogFacility << "\";\n\n"
         << "    #----------------------------------------------------------"
          "--------------\n"
         << "    #  Minimum syslog priority to log.  Defaults to \"info\" if"
          " not set.\n"
         << "    #----------------------------------------------------------"
          "--------------\n"
         << "    level = \"" << cfg._syslogLevel << "\";\n\n"
         << "    #----------------------------------------------------------"
          "--------------\n"
         << "    #  Set to \"yes\" to get {filename:line} in syslog.\n"
         << "    #----------------------------------------------------------"
          "--------------\n"
         << "    logLocations = \"" << (cfg._logLocations ? "yes" : "no")
         << "\";\n"
         << "};\n\n";

      
      return os;
    }
    

  }  // namespace Mcweather

}  // namespace Dwm
