%{
  #include <map>
  #include <regex>

  #include "DwmIpv4Address.hh"
  #include "DwmIpv6Address.hh"
  #include "DwmMcweatherConfig.hh"
  #include "DwmMcweatherConfigParse.hh"

  extern std::string  g_configPath;
  
  extern "C" {
    #include <stdarg.h>

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void mcweathercfgerror(const char *arg, ...)
    {
      va_list  ap;
      va_start(ap, arg);
      vfprintf(stderr, arg, ap);
      fprintf(stderr, ": '%s' at line %d of %s\n",
              yytext, yylineno, g_configPath.c_str());
      return;
    }
  }

  //--------------------------------------------------------------------------
  //!  
  //--------------------------------------------------------------------------
  static const std::map<std::string,int>  g_configKeywords = {
    { "addresses",          ADDRESSES      },
    { "address",            ADDRESS        },
    { "cache",              CACHE          },
    { "allowedClients",     ALLOWEDCLIENTS },
    { "facility",           FACILITY       },
    { "keyDirectory",       KEYDIRECTORY   },
    { "latitude",           LATITUDE       },
    { "level",              LEVEL          },
    { "location",           LOCATION       },
    { "logLocations",       LOGLOCATIONS   },
    { "longitude",          LONGITUDE      },
    { "port",               PORT           },
    { "service",            SERVICE        },
    { "stations",           STATIONS       },
    { "store",              STORE          },
    { "syslog",             SYSLOG         },
    { "weather",            WEATHER        }
  };

  //--------------------------------------------------------------------------
  //!  
  //--------------------------------------------------------------------------
  static bool IsKeyword(const std::string & s, int & token)
  {
    bool  rc = false;
    auto  it = g_configKeywords.find(s);
    if (g_configKeywords.end() != it) {
      token = it->second;
      rc = true;
    }
    return rc;
  }

  //--------------------------------------------------------------------------
  //!  
  //--------------------------------------------------------------------------
  static bool IsNumber(const std::string & s)
  {
    using std::regex, std::smatch;
    static regex  rgx("[0-9]+", regex::ECMAScript|regex::optimize);
    smatch        sm;
    return std::regex_match(s, sm, rgx);
  }

%}

%option noyywrap
%option prefix="mcweathercfg"
%option yylineno

%x x_quoted
        
%%

<INITIAL>#.*\n
<INITIAL>[^ \t\n\[\]{}=,;"]+       { if (IsNumber(yytext)) {
                                       mcweathercfglval.intVal = atoi(yytext);
                                       return INTEGER;
                                     }
                                     else {
                                       int  token;
                                       if (IsKeyword(yytext, token)) {
                                         return token;
                                       }
                                       else {
                                         mcweathercfglval.stringVal =
                                           new std::string(yytext);
                                         return STRING;
                                       }
                                     }
                                   }
<INITIAL>["]                       { BEGIN(x_quoted); }
<x_quoted>[^"]+                    { mcweathercfglval.stringVal =
                                       new std::string(yytext);
                                     return STRING; }
<x_quoted>["]                      { BEGIN(INITIAL); }
<INITIAL>[=,;\[\]\{\}]             { return yytext[0]; }
<INITIAL>[ \t\n]

%%
