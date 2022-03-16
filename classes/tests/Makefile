include ../../Makefile.vars

OBJFILES = TestConfig.o \
	   TestPeriodForecast.o
OBJDEPS	 = $(OBJFILES:%.o=deps/%_deps)
TESTS	 = $(OBJFILES:%.o=%)
CXXFLAGS += -g
LDFLAGS  += -g
ALLINC   = ${CREDENCEINCS} ${DWMINCS}
ALLINC   += ${NLOHMANNJSONINC} -I../include -I.
ALLLIBS  = -L../../classes/lib -lDwmMcweather ${CREDENCELIBS}
ALLLIBS   += ${CREDENCELIBS}

all: ${TESTS}

runtests: all
	@ for tp in ${TESTS} ; do \
		printf "%-36s " $$tp ; \
		out=`./$$tp` ; \
		if [ $$? -eq 0 ]; then \
		  printf "%25s\n" "$$out" ; \
		else \
		  printf "\n%s" "$$out" ; \
		fi ; \
	done

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

%: %.o ../lib/libDwmMcweather.a
	${CXX} ${LDFLAGS} -o $@ $< ${ALLLIBS}

../lib/libMcweather.a::
	${MAKE} -C ../src

clean:
	${LIBTOOL} --mode=clean rm -f ${TESTS} ${OBJFILES}
