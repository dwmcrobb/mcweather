include ../Makefile.vars

TARTARGS = ${TARDIR}/etc/mcweatherd.cfg.example

all::
	echo "nothing to be done for 'all' target."

tarprep: ${TARTARGS} tarprep-${OSNAME}

tarprep-freebsd::
	${MAKE} -C freebsd/rc.d tarprep

tarprep-linux::
	${MAKE} -C linux/systemd/system tarprep

tarprep-darwin::
	echo "No startup scripts for darwin"

${TARDIR}/etc/mcweatherd.cfg.example: mcweatherd.cfg.example
	../install-sh -c -m 444 $< $@

clean:: tarprep-clean

tarprep-clean:: clean-tarprep-${OSNAME}
	rm -f ${TARTARGS}

clean-tarprep-freebsd::
	${MAKE} -C freebsd/rc.d clean-tarprep

clean-tarprep-linux::
	${MAKE} -C linux/systemd/system clean-tarprep

clean-tarprep-darwin::
	echo "No startup scripts for darwin"

distclean::
	echo "Nothing to be done for distclean"
