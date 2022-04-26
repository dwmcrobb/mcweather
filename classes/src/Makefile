include ../../Makefile.vars

LTCOMPILE    = ${LIBTOOL} --mode=compile --tag=CXX ${CXX}
LTLINK       = ${LIBTOOL} --mode=link --tag=CXX ${CXX}

ALLINC       = -I../include ${DWMINCS}
OBJFILESNP   = DwmMcweatherCache.o \
               DwmMcweatherConfigLex.o \
               DwmMcweatherConfigParse.o \
               DwmMcweatherCurrentConditions.o \
               DwmMcweatherPeriodForecast.o \
               DwmMcweatherPeriodForecasts.o \
               DwmMcweatherPointInfo.o \
               DwmMcweatherServiceConfig.o \
               DwmMcweatherUtils.o \
               DwmMcweatherVersion.o \
               DwmMcweatherWeatherConfig.o \
               DwmMcweatherWeatherFetcher.o
OBJFILES     = $(OBJFILESNP:%=../obj/%)
OBJDEPS      = $(OBJFILESNP:%.o=deps/%_deps)
SHARED_OBJFILES = $(OBJFILES:%.o=%.lo)
shlib_version = $(shell ../../getvers.sh -s)

all: ../lib/libDwmMcweather.la

../lib/libDwmMcweather.la: ${SHARED_OBJFILES}
	${LTLINK} -o $@ $^ -rpath ${INSTALLPREFIX}/lib -version-info ${shlib_version} ${DWMLIBS}

#  dependency rule
deps/%_deps: %.cc
	@echo "making dependencies for $<"
	@set -e; \
	${CXX} -MM ${CXXFLAGS} ${PTHREADCXXFLAGS} ${ALLINC} -c $< | \
	 sed 's/\($*\)\.o[ :]*/\1.lo $(@D)\/$(@F) : /g' > $@ ; [ -s $@ ] || \
	 rm -f $@

#  only include dependency makefiles if target is not 'clean' or 'distclean'
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include ${OBJDEPS}
endif
endif

../obj/%.lo: %.cc deps/%_deps
	${LTCOMPILE} ${CXXFLAGS} ${PTHREADCXXFLAGS} ${ALLINC} -c $< -o $@

DwmMcweatherConfigParse.hh: DwmMcweatherConfigParse.cc

DwmMcweatherConfigParse.cc: DwmMcweatherConfigParse.y
	bison -d -o$@ $<

DwmMcweatherConfigLex.cc: DwmMcweatherConfigLex.lex DwmMcweatherConfigParse.hh
	flex -o$@ $<

tarprep: ${TARDIR}/lib/libDwmMcweather.la

${TARDIR}/lib/libDwmMcweather.la: ../lib/libDwmMcweather.la
	${LIBTOOL} --mode=install ../../install-sh -c $< $@

clean:: clean-tarprep
	${LIBTOOL} --mode=clean rm -f ../lib/libDwmMcweather.la ${OBJFILES}
	${LIBTOOL} --mode=clean rm -f  DwmMcweatherConfigParse.cc DwmMcweatherConfigParse.hh DwmMcweatherConfigLex.cc

distclean:: clean
	rm -f ${OBJDEPS}

clean-tarprep::
	${LIBTOOL} --mode=uninstall rm -f ${TARDIR}/lib/libDwmMcweather.la
