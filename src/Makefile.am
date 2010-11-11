INCLUDES = $(all_includes) -I$(top_srcdir)/include -I$(top_builddir)
AM_CFLAGS=-Wall $(LIBOSMOCODEC_CFLAGS) ${OPENCORE_AMRNB_CFLAGS}
AM_LDFLAGS=$(LIBOSMOCODEC_LIBS) ${OPENCORE_AMRNB_LIBS} ${LIBGSM_LIBS}

bin_PROGRAMS = gapk
gapk_SOURCES =	main.c	\
		procqueue.c pq_file.c pq_format.c \
		formats.c fmt_amr.c fmt_gsm.c fmt_hr_ref.c fmt_racal.c \
		codecs.c codec_pcm.c codec_hr.c codec_fr.c codec_efr.c
