AM_CPPFLAGS = $(all_includes) -I$(top_srcdir)/include -I$(top_builddir)
AM_CFLAGS = -fPIC -Wall ${SYMBOL_VISIBILITY}

EXTRA_DIST=fetch_sources.py

LIBVERSION=0:0:0

REFSRC_PATH=refsrc
REFSRC_SRC=refsrc/dtx.c refsrc/globdefs.c refsrc/host.c refsrc/sp_enc.c refsrc/sp_rom.c refsrc/vad.c refsrc/err_conc.c refsrc/homing.c refsrc/mathdp31.c refsrc/sp_dec.c refsrc/sp_frm.c refsrc/sp_sfrm.c

${REFSRC_PATH}/.downloaded:
	$(srcdir)/fetch_sources.py "${REFSRC_PATH}"
	for p in "$(srcdir)/patches"/*.patch; do	\
		cat "$$p" | patch -p0 ;			\
	done
	touch $@

$(REFSRC_SRC) libgsmhr.c: ${REFSRC_PATH}/.downloaded

lib_LTLIBRARIES = libgsmhr.la
libgsmhr_la_SOURCES = $(REFSRC_SRC) libgsmhr.c
libgsmhr_la_LIBADD = -ldl

clean-local:
	-rm -rf ${REFSRC_PATH}/*.{c,h} ${REFSRC_PATH}/.downloaded
