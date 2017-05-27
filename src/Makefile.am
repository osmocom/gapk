AM_CPPFLAGS = $(all_includes) -I$(top_srcdir)/include -I$(top_builddir)
AM_CFLAGS=-Wall $(LIBOSMOCODEC_CFLAGS) $(LIBOSMOCORE_CFLAGS) \
	  ${OPENCORE_AMRNB_CFLAGS} $(LIBALSA_CFLAGS)
AM_LDFLAGS=$(LIBOSMOCODEC_LIBS) $(LIBOSMOCORE_LIBS) \
	   ${OPENCORE_AMRNB_LIBS} ${LIBGSM_LIBS} $(LIBALSA_LIBS)

COM_SOURCES = procqueue.c pq_file.c pq_format.c pq_codec.c pq_rtp.c pq_alsa.c \
		formats.c fmt_amr.c fmt_gsm.c fmt_hr_ref.c fmt_racal.c \
			fmt_rawpcm.c fmt_ti.c benchmark.c \
		codecs.c codec_pcm.c codec_hr.c codec_fr.c codec_efr.c

bin_PROGRAMS = gapk

gapk_SOURCES =	main.c $(COM_SOURCES)

if ENABLE_GSMHR
gapk_LDADD = $(top_builddir)/libgsmhr/libgsmhr.la
endif
