AUTOMAKE_OPTIONS = foreign dist-bzip2 1.6
ACLOCAL_AMFLAGS = -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libosmogapk.pc

SUBDIRS = include

if ENABLE_GSMHR
SUBDIRS += libgsmhr
endif

SUBDIRS += src tests
