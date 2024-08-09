AUTOMAKE_OPTIONS = foreign dist-bzip2 1.6
ACLOCAL_AMFLAGS = -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libosmogapk.pc

SUBDIRS = include

if ENABLE_GSMHR
SUBDIRS += libgsmhr
endif

SUBDIRS += src tests

BUILT_SOURCES = \
	$(top_srcdir)/.version \
	$(NULL)
EXTRA_DIST = \
	git-version-gen \
	.version \
	$(NULL)

@RELMAKE@

# versioning magic
$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@
dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version
