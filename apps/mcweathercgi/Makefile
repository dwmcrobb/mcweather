include ../../Makefile.vars

LTLINK       = ${LIBTOOL} --mode=link --tag=CXX ${CXX}

OBJFILES = mcweathercgi.o
OBJDEPS	 = $(OBJFILES:%.o=deps/%_deps)
CXXFLAGS += -g
LDFLAGS  += -Wl,-rpath ${DWMDIR}/lib
ALLINC   = ${DWMINCS}
ALLINC   += ${NLOHMANNJSONINC} -I../../classes/include -I.
ALLLIBS  = ../../classes/lib/libDwmMcweather.la ${DWMLIBS}
TARTARGETS  = ${TARDIR}/bin/mcweathercgi

mcweathercgi: ${OBJFILES} ../../classes/lib/libDwmMcweather.la
	${LTLINK} ${LDFLAGS} -o $@ ${OBJFILES} ${ALLLIBS}

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

../../classes/lib/libDwmMcweather.la::
	${MAKE} -C ../../classes/src

tarprep: ${TARTARGETS}

${TARDIR}/bin/mcweathercgi: mcweathercgi
	../../install-sh -s -c -m 555 $< $@

clean:: clean-tarprep
	rm -f mcweathercgi ${OBJFILES}

clean-tarprep::
	rm -f ${TARTARGETS}

distclean:: clean
	rm -f deps/*_deps
