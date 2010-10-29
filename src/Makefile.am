INCLUDES = $(all_includes) -I$(top_srcdir)/include -I$(top_builddir)
AM_CFLAGS=-Wall $(LIBOSMOCODEC_CFLAGS)
AM_LDFLAGS=$(LIBOSMOCODEC_LIBS)

bin_PROGRAMS = gapk
gapk_SOURCES =	main.c	\
		formats.c fmt_gsm.c fmt_hr_ref.c fmt_racal.c \
		codecs.c codec_pcm.c codec_hr.c codec_fr.c codec_efr.c
