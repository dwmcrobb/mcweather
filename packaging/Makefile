include ../Makefile.vars

ifeq ("${OSNAME}", "freebsd")
PKGPCDIR = libdata/pkgconfig
else
PKGPCDIR = lib/pkgconfig
endif

TARTARGS = ${TARDIR}/${PKGPCDIR}/Mcweather.pc

all::
	echo "nothing to be done for 'all' target."

tarprep: ${TARTARGS}

${TARDIR}/${PKGPCDIR}/Mcweather.pc: Mcweather.pc
	../install-sh -c -m 444 $< $@

tarprep-clean::
	rm -f ${TARTARGS}

clean:: tarprep-clean

distclean:: clean
	rm -f debcontrol fbsd_manifest Mcweather.pc
