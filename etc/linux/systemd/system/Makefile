include ../../../../Makefile.vars

TARTARGETS  = ${STAGING_DIR}/etc/systemd/system/mcweatherd.service

all::
	echo "Nothing to be done for 'all'."

tarprep:: ${TARTARGETS}

${STAGING_DIR}/etc/systemd/system/mcweatherd.service: mcweatherd.service
	../../../../install-sh -c -m 644 $< $@

clean:: clean-tarprep

clean-tarprep::
	rm -f ${TARTARGETS}

