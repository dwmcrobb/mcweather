include ../../../Makefile.vars

TARTARGETS  = ${TARDIR}/etc/rc.d/mcweatherd

all::
	echo "Nothing to be done for 'all'."

tarprep:: ${TARTARGETS}

${TARDIR}/etc/rc.d/mcweatherd: mcweatherd
	../../../install-sh -c -m 755 $< $@

clean:: clean-tarprep

clean-tarprep::
	rm -f ${TARTARGETS}

