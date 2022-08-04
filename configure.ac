AC_PREREQ([2.65])
AC_INIT([gapk],
	m4_esyscmd([./git-version-gen .tarball-version]),
	[openbsc@lists.osmocom.org])

AM_INIT_AUTOMAKE([dist-bzip2 subdir-objects])
LT_INIT([disable-static])

CFLAGS="$CFLAGS -std=gnu11"

# kernel style compile messages
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([src/app_osmo_gapk.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_TESTDIR(tests)

# Options
AC_ARG_ENABLE(gsmhr,
	[AS_HELP_STRING(
		[--enable-gsmhr],
		[Enable support for GSM HR codec using GSM 06.06 reference code]
	)],
	[enable_gsmhr="yes"], [enable_gsmhr=$enableval])
AM_CONDITIONAL(ENABLE_GSMHR, test "x$enable_gsmhr" = "xyes")
if test "x$enable_gsmhr" = "xyes"; then
	AM_PATH_PYTHON([2.4])
	AC_DEFINE(HAVE_LIBGSMHR, 1, [Define to 1 if libgsmhr is available])
fi

# Check for -fvisibility support
#  (The following test is taken from WebKit's webkit.m4)
saved_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -fvisibility=hidden "
AC_MSG_CHECKING([if ${CC} supports -fvisibility=hidden])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([char foo;])],
	[ AC_MSG_RESULT([yes])
	  SYMBOL_VISIBILITY="-fvisibility=hidden"],
	AC_MSG_RESULT([no]))
CFLAGS="$saved_CFLAGS"
AC_SUBST(SYMBOL_VISIBILITY)

dnl check os: some linker flags not available on osx
case $host in
*-darwin*)
	;;
*)
	LTLDFLAGS_OSMOGAPK='-Wl,--version-script=$(srcdir)/libosmogapk.map'
	;;
esac
AC_SUBST(LTLDFLAGS_OSMOGAPK)

# Checks for programs.
AC_PROG_CC

AC_ARG_ENABLE(sanitize,
               [AS_HELP_STRING([--enable-sanitize], [Compile with address sanitizer enabled], )],
               [sanitize=$enableval], [sanitize="no"])
if test x"$sanitize" = x"yes"
then
       CFLAGS="$CFLAGS -fsanitize=address -fsanitize=undefined"
       CPPFLAGS="$CPPFLAGS -fsanitize=address -fsanitize=undefined"
fi

# Checks for libraries.
 # libosmocore (codec module)
PKG_CHECK_MODULES(LIBOSMOCORE, libosmocore >= 0.3.2)
PKG_CHECK_MODULES(LIBOSMOCODEC, libosmocodec >= 0.1.25)
PKG_CHECK_MODULES(LIBALSA, alsa, [AC_DEFINE([HAVE_ALSA], [1], [We have ALSA])])

 # opencore-amrnb for AMR and EFR decoding
found_opencore_amrnb=yes
PKG_CHECK_MODULES(OPENCORE_AMRNB, opencore-amrnb >= 0.1.0, , found_opencore_amrnb=no)
if test "$found_opencore_amrnb" = yes; then
	AC_DEFINE(HAVE_OPENCORE_AMRNB, 1, [Define to 1 if OpenCore AMR-NB library is available])
fi

 # libgsm for FR decoding
found_libgsm=yes
AC_CHECK_HEADERS(gsm/gsm.h, ,
	[AC_CHECK_HEADERS(gsm.h, ,found_libgsm=no)])
	AC_CHECK_LIB(gsm, gsm_create, LIBGSM_LIBS="-lgsm", found_libgsm=no)
if test "$found_libgsm" = yes; then
	AC_DEFINE(HAVE_LIBGSM, 1, [Define to 1 if libgsm is available])
fi
AC_SUBST(LIBGSM_LIBS)

# Conditional codec tests
AC_SUBST(ENABLE_AMR_EFR_TESTS, [$found_opencore_amrnb])
AC_SUBST(ENABLE_HR_TESTS, [$enable_gsmhr])
AC_SUBST(ENABLE_FR_TESTS, [$found_libgsm])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([contrib/gapk.spec
		 Makefile
		 src/Makefile
		 libgsmhr/Makefile
		 include/Makefile
		 include/gsmhr/Makefile
		 libosmogapk.pc
		 tests/atlocal
		 tests/Makefile])
AC_OUTPUT
