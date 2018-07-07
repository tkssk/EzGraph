bin_SCRIPTS = ezcc

CLEANFILES = $(bin_SCRIPTS)

do_subst = sed -e 's,[@]PREFIX[@],$(prefix),g'

ezcc: ezcc.in Makefile
	$(do_subst) < $(srcdir)/ezcc.in > ezcc
	chmod +x ezcc
