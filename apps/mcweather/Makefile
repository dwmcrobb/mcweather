include ../../Makefile.vars

OBJFILES = mcweather.o
OBJDEPS	 = $(OBJFILES:%.o=deps/%_deps)
CXXFLAGS += -g
LDFLAGS  += -g
ALLINC   = ${CREDENCEINCS} ${DWMINCS}
ALLINC   += ${NLOHMANNJSONINC} -I../../classes/include -I.
POCOLIBS = -lPocoNet -lPocoNetSSL -lPocoCrypto -lPocoUtil -lPocoXML -lPocoJSON -lPocoFoundation
ALLLIBS  = ../../classes/lib/libDwmMcweather.a ${CREDENCELIBS} ${POCOLIBS} -lsodium

mcweather: ${OBJFILES} ../../classes/lib/libDwmMcweather.a
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
	${LIBTOOL} --mode=clean rm -f mcweather ${OBJFILES}
