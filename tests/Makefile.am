AM_CPPFLAGS = \
	$(all_includes) \
	-I$(top_builddir) \
	-I$(top_srcdir)/include \
	$(NULL)

AM_CFLAGS = \
	-Wall \
	$(LIBOSMOCORE_CFLAGS) \
	$(LIBOSMOCODEC_CFLAGS) \
	$(NULL)

check_PROGRAMS = \
	procqueue/pq_test \
	io/pq_file_test \
	io/pq_rtp_test \
	ecu/ecu_fr_test \
	$(NULL)

procqueue_pq_test_SOURCES = procqueue/pq_test.c
procqueue_pq_test_LDADD = \
	$(top_builddir)/src/libosmogapk.la \
	$(LIBOSMOCORE_LIBS) \
	$(LIBOSMOCODEC_LIBS) \
	$(TALLOC_LIBS) \
	$(NULL)

io_pq_file_test_SOURCES = io/pq_file_test.c
io_pq_file_test_LDADD = \
	$(top_builddir)/src/libosmogapk.la \
	$(LIBOSMOCORE_LIBS) \
	$(LIBOSMOCODEC_LIBS) \
	$(TALLOC_LIBS) \
	$(NULL)

io_pq_rtp_test_SOURCES = io/pq_rtp_test.c
io_pq_rtp_test_LDADD = \
	$(top_builddir)/src/libosmogapk.la \
	$(LIBOSMOCORE_LIBS) \
	$(LIBOSMOCODEC_LIBS) \
	$(TALLOC_LIBS) \
	$(NULL)

ecu_ecu_fr_test_SOURCES = ecu/ecu_fr_test.c
ecu_ecu_fr_test_LDADD = \
	$(top_builddir)/src/libosmogapk.la \
	$(LIBOSMOCORE_LIBS) \
	$(LIBOSMOCODEC_LIBS) \
	$(TALLOC_LIBS) \
	$(NULL)

# The `:;' works around a Bash 3.2 bug when the output is not writeable.
$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	:;{ \
			echo '# Signature of the current package.' && \
			echo 'm4_define([AT_PACKAGE_NAME],' && \
			echo '  [$(PACKAGE_NAME)])' && \
			echo 'm4_define([AT_PACKAGE_TARNAME],' && \
			echo '  [$(PACKAGE_TARNAME)])' && \
			echo 'm4_define([AT_PACKAGE_VERSION],' && \
			echo '  [$(PACKAGE_VERSION)])' && \
			echo 'm4_define([AT_PACKAGE_STRING],' && \
			echo '  [$(PACKAGE_STRING)])' && \
			echo 'm4_define([AT_PACKAGE_BUGREPORT],' && \
			echo '  [$(PACKAGE_BUGREPORT)])'; \
			echo 'm4_define([AT_PACKAGE_URL],' && \
			echo '  [$(PACKAGE_URL)])'; \
			} >'$(srcdir)/package.m4'

EXTRA_DIST = \
	testsuite.at \
	$(TESTSUITE) \
	$(srcdir)/package.m4 \
	$(NULL)

EXTRA_DIST += \
	procqueue/pq_test.ok \
	ecu/ecu_fr_test.ok \
	io/pq_file_test.ok \
	io/pq_rtp_test.ok \
	io/io_sample.txt \
	ref-files \
	$(NULL)

DISTCLEANFILES = atconfig atlocal
TESTSUITE = $(srcdir)/testsuite

check-local: atconfig $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)

installcheck-local: atconfig $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(bindir)' \
		$(TESTSUITEFLAGS)

clean-local:
	test ! -f '$(TESTSUITE)' || \
		$(SHELL) '$(TESTSUITE)' --clean

AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te
AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): $(srcdir)/testsuite.at $(srcdir)/package.m4
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@
