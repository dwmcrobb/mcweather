include ../Makefile.vars

ifeq ("${OSNAME}", "freebsd")
PKGPCDIR = libdata/pkgconfig
else
PKGPCDIR = lib/pkgconfig
endif

TARTARGS = ${TARDIR}/${PKGPCDIR}/Mcweather.pc
ifeq ("${OSNAME}","linux")
TARTARGS += ${STAGING_DIR}/DEBIAN/postinst
TARTARGS += ${STAGING_DIR}/DEBIAN/prerm
TARTARGS += ${STAGING_DIR}/DEBIAN/postrm
endif

all::
	echo "nothing to be done for 'all' target."

tarprep: ${TARTARGS}

${TARDIR}/${PKGPCDIR}/Mcweather.pc: Mcweather.pc
	../install-sh -c -m 444 $< $@

${STAGING_DIR}/DEBIAN/postinst: debpostinst
	../install-sh -c -m 555 $< $@

${STAGING_DIR}/DEBIAN/prerm: debprerm
	../install-sh -c -m 555 $< $@

${STAGING_DIR}/DEBIAN/postrm: debpostrm
	../install-sh -c -m 555 $< $@

tarprep-clean::
	rm -f ${TARTARGS}

clean:: tarprep-clean

distclean:: clean
	rm -f debcontrol fbsd_manifest Mcweather.pc
