include ../../Makefile.vars

ALLINC       = -I../include ${CREDENCEINCS}
OBJFILESNP   = DwmMcweatherCache.o \
               DwmMcweatherConfigLex.o \
               DwmMcweatherConfigParse.o \
               DwmMcweatherCurrentConditions.o \
               DwmMcweatherPeriodForecast.o \
               DwmMcweatherPeriodForecasts.o \
               DwmMcweatherPointInfo.o \
               DwmMcweatherServiceConfig.o \
               DwmMcweatherUtils.o \
               DwmMcweatherWeatherConfig.o \
               DwmMcweatherWeatherFetcher.o \
               DwmMcweatherWebUtils.o
OBJFILES     = $(OBJFILESNP:%=../obj/%)
OBJDEPS      = $(OBJFILESNP:%.o=deps/%_deps)

all: ../lib/libDwmMcweather.a

../lib/libDwmMcweather.a: ${OBJFILES}
	ar rv $@ $^
	-ranlib $@

#  dependency rule
deps/%_deps: %.cc
	@echo "making dependencies for $<"
	@set -e; \
	${CXX} -MM ${CXXFLAGS} ${PTHREADCXXFLAGS} ${ALLINC} -c $< | \
	 sed 's/\($*\)\.o[ :]*/\1.o $(@D)\/$(@F) : /g' > $@ ; [ -s $@ ] || \
	 rm -f $@

#  only include dependency makefiles if target is not 'clean' or 'distclean'
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include ${OBJDEPS}
endif
endif

../obj/%.o: %.cc deps/%_deps
	${CXX} ${CXXFLAGS} ${PTHREADCXXFLAGS} ${ALLINC} -c $< -o $@

DwmMcweatherConfigParse.hh: DwmMcweatherConfigParse.cc

DwmMcweatherConfigParse.cc: DwmMcweatherConfigParse.y
	bison -d -o$@ $<

DwmMcweatherConfigLex.cc: DwmMcweatherConfigLex.lex DwmMcweatherConfigParse.hh
	flex -o$@ $<

clean::
	rm -f ${OBJFILES} DwmMcweatherConfigParse.cc DwmMcweatherConfigParse.hh
	rm -f DwmMcweatherConfigLex.cc
