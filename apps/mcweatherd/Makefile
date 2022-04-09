include ../../Makefile.vars

OBJFILES = mcweatherd.o \
           DwmMcweatherResponder.o \
           DwmMcweatherServer.o
OBJDEPS	 = $(OBJFILES:%.o=deps/%_deps)
CXXFLAGS += -g
LDFLAGS  += -g
ALLINC   = ${CREDENCEINCS} ${DWMWEBUTILSINCS}
ALLINC   += ${NLOHMANNJSONINC} -I../../classes/include -I.
ALLLIBS  = ../../classes/lib/libDwmMcweather.a ${CREDENCELIBS} ${DWMWEBUTILSLIBS}

mcweatherd: ${OBJFILES} ../../classes/lib/libDwmMcweather.a
	${CXX} -o $@ ${OBJFILES} ${ALLLIBS}

#  dependency rule
deps/%_deps: %.cc 
	@echo "making dependencies for $<"
	@set -e; \
	${CXX} -MM ${CXXFLAGS} ${ALLINC} -c $< | \
	 sed 's/\($*\)\.o[ :]*/\1.o $(@D)\/$(@F) : /g' > $@ ; [ -s $@ ] || \
	 rm -f $@

#  only include dependency makefiles if target is not 'clean'
ifneq ($(MAKECMDGOALS),clean)
-include ${OBJDEPS}
endif

%.o: %.cc deps/%_deps
	${CXX} ${CXXFLAGS} ${PTHREADCXXFLAGS} ${ALLINC} -c $<

../../classes/lib/libMcweather.a::
	${MAKE} -C ../../classes/src

clean:
	${LIBTOOL} --mode=clean rm -f mcweatherd ${OBJFILES}
