AC_PREREQ([2.65])
AC_INIT([gapk], [0.1], [main@lists.airprobe.org])

AM_INIT_AUTOMAKE([dist-bzip2])

# kernel style compile messages
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	include/Makefile
	include/gapk/Makefile
])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
 # libosmocore (codec module)
PKG_CHECK_MODULES(LIBOSMOCODEC, libosmocodec >= 0.1.25)

 # libgsm for FR decoding
found_libgsm=yes
AC_CHECK_HEADERS(gsm/gsm.h, ,
	[AC_CHECK_HEADERS(gsm.h, ,found_libgsm=no)])
	AC_CHECK_LIB(gsm, gsm_create, LIBGSM_LIBS="-lgsm", found_libgsm=no)
if test "$found_libgsm" = yes; then
	AC_DEFINE(HAVE_LIBGSM, 1, [Define to 1 if libgsm is available])
fi
AC_SUBST(LIBGSM_LIBS)

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

AC_OUTPUT
