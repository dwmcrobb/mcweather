include ../../Makefile.vars

LTLINK       = ${LIBTOOL} --mode=link --tag=CXX ${CXX}

OBJFILES = mcweatherd.o \
           DwmMcweatherResponder.o \
           DwmMcweatherServer.o
OBJDEPS	 = $(OBJFILES:%.o=deps/%_deps)
CXXFLAGS += -g
LDFLAGS  += -g
ALLINC   = ${DWMINCS}
ALLINC   += ${NLOHMANNJSONINC} -I../../classes/include -I.
ALLLIBS  = ${DWMLIBS}
TARTARGETS  = ${TARDIR}/sbin/mcweatherd

mcweatherd: ${OBJFILES} ../../classes/lib/libDwmMcweather.la
	${LTLINK} -o $@ $^ -rpath ${DWMDIR}/lib ${ALLLIBS}

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

../../classes/lib/libMcweather.la::
	${MAKE} -C ../../classes/src

tarprep: ${TARTARGETS}

${TARDIR}/sbin/mcweatherd: mcweatherd
	${LIBTOOL} --mode=install ../../install-sh -s -c -m 555 $< $@

clean:: clean-tarprep
	${LIBTOOL} --mode=clean rm -f mcweatherd ${OBJFILES}

clean-tarprep::
	${LIBTOOL} --mode=uninstall rm -f ${TARTARGETS}
