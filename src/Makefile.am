# This is _NOT_ the library release version, it's an API version.
# Please read Chapter 6 "Library interface versions" of the libtool
# documentation before making any modification
LIBVERSION = 0:0:0

AM_CPPFLAGS = \
	$(all_includes) \
	-I$(top_builddir) \
	-I$(top_srcdir)/include \
	$(NULL)

AM_CFLAGS = \
	-Wall \
	$(LIBOSMOCORE_CFLAGS) \
	$(LIBOSMOCODEC_CFLAGS) \
	${OPENCORE_AMRNB_CFLAGS} \
	$(LIBALSA_CFLAGS) \
	$(NULL)

lib_LTLIBRARIES = libosmogapk.la

libosmogapk_la_LDFLAGS = \
	$(LIBOSMOCORE_LIBS) \
	$(LIBOSMOCODEC_LIBS) \
	${OPENCORE_AMRNB_LIBS} \
	${LIBGSM_LIBS} \
	$(LIBALSA_LIBS) \
	-version-info $(LIBVERSION) \
	-no-undefined \
	$(NULL)

if ENABLE_GSMHR
libosmogapk_la_LIBADD = $(top_builddir)/libgsmhr/libgsmhr.la
endif

# Processing queue implementation
libosmogapk_la_SOURCES = \
	procqueue.c \
	pq_format.c \
	pq_codec.c \
	pq_file.c \
	pq_alsa.c \
	pq_rtp.c \
	$(NULL)

# Formats implementation
libosmogapk_la_SOURCES += \
	formats.c \
	fmt_ti.c \
	fmt_amr.c \
	fmt_gsm.c \
	fmt_hr_ref.c \
	fmt_racal.c \
	fmt_rawpcm.c \
	fmt_rtp_amr.c \
	fmt_rtp_efr.c \
	fmt_rtp_hr_etsi.c \
	fmt_rtp_hr_ietf.c \
	fmt_amr_opencore.c \
	$(NULL)

# Codecs implementation
libosmogapk_la_SOURCES += \
	codecs.c \
	codec_pcm.c \
	codec_hr.c \
	codec_fr.c \
	codec_efr.c \
	codec_amr.c \
	$(NULL)

# Codec benchmarking
libosmogapk_la_SOURCES += \
	benchmark.c \
	$(NULL)

# libosmogapk representative application
bin_PROGRAMS = gapk

gapk_SOURCES = \
	main.c \
	$(NULL)

gapk_LDFLAGS = \
	$(LIBOSMOCORE_LIBS) \
	$(NULL)

gapk_LDADD = \
	$(top_builddir)/src/libosmogapk.la \
	$(NULL)
