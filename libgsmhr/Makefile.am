# This is _NOT_ the library release version, it's an API version.
# Please read Chapter 6 "Library interface versions" of the libtool
# documentation before making any modification
LIBVERSION = 0:0:0

AM_CPPFLAGS = $(all_includes) -I$(top_srcdir)/include -I$(top_builddir)
AM_CFLAGS = -fPIC -Wall ${SYMBOL_VISIBILITY}

EXTRA_DIST = \
	fetch_sources.py \
	patches \
	$(NULL)

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
libgsmhr_la_LDFLAGS = -version-info $(LIBVERSION)
libgsmhr_la_LIBADD = -ldl

clean-local:
	-rm -rf ${REFSRC_PATH}/*.lo

distclean-local:
	-rm -rf ${REFSRC_PATH}/
