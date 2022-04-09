include ../Makefile.vars

TARTARGS = ${TARDIR}/etc/mcweatherd.cfg.example

all::
	echo "nothing to be done for 'all' target."

tarprep: ${TARTARGS}

${TARDIR}/etc/mcweatherd.cfg.example: mcweatherd.cfg.example
	../install-sh -c -m 444 $< $@

clean:: tarprep-clean

tarprep-clean::
	rm -f ${TARTARGS}

distclean::
	echo "Nothing to be done for distclean"
