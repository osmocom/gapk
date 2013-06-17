AUTOMAKE_OPTIONS = foreign dist-bzip2 1.6
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = $(all_includes) -I$(top_srcdir)/include

SUBDIRS = include

if ENABLE_GSMHR
SUBDIRS += libgsmhr
endif

SUBDIRS += src
