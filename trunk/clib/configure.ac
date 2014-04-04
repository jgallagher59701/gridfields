#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([gridfields], [1.0.3], [Bill Howe <billhowe@cs.washington.edu>])
AC_CONFIG_SRCDIR([src/abstractcellarray.h])
# This has to be named specially since it's included in headers that
# are installed. The config header defines compile-time switches that 
# select between older (deprecated) versions of hash map and C++'s new
# unordered_map. jhrg 4/4/14
AM_CONFIG_HEADER([config_gridfields.h])

# This provides a place to keep all the autotools stuff (m4 files, etc.)
AC_CONFIG_AUX_DIR(conf)
AC_CONFIG_MACRO_DIR([conf])

AM_INIT_AUTOMAKE

dnl library version: Update these when the interface changes. Generally,
dnl assume that the interface tracks the major and minor release numbers.
GFLIB_CURRENT=1
GFLIB_AGE=0
GFLIB_REVISION=0
AC_SUBST(GFLIB_CURRENT)
AC_SUBST(GFLIB_AGE)
AC_SUBST(GFLIB_REVISION)

LIBGF_VERSION="$GFLIB_CURRENT:$GFLIB_REVISION:$GFLIB_AGE"
AC_SUBST(LIBGF_VERSION)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

dnl echo "CC = $CC"
if test "$CC" = "g++" -o "$CC" == "gcc"
then
    AM_CONDITIONAL([COMPILER_IS_GCC],[true])
else
    AM_CONDITIONAL([COMPILER_IS_GCC],[false])
fi

AC_ARG_ENABLE([netcdf], [AS_HELP_STRING([--enable/disable-netcdf],
					[use netcdf (default is yes)])], [], [])
# $enable_netcdf will be 'no' if --disable-netcdf is used, 'yes' if
# --enable-netcdf is used and '' if neither is used

netcdf_found_ac=false

if test -z "$enable_netcdf" -o "$enable_netcdf" = "yes"
then
AC_CHECK_NETCDF(
   [
   LIBS="$LIBS $NC_LIBS"
   LDFLAGS="$LDFLAGS $NC_LDFLAGS"
   CPPFLAGS="$CPPFLAGS $NC_CPPFLAGS"
   netcdf_found_ac=true
   ],
   [
   echo "*** Use --with-netcdf for the root netcdf directory."
   echo "*** Otherwise use --with-netcdf-include switch for includes directory"
   echo "*** and --with-netcdf-libdir switch for libraries directory."
   AC_MSG_WARN([netcdf library and netcdf headers not found.])
   ],[3]
)
fi

AM_CONDITIONAL([NETCDF_FOUND], [$netcdf_found_ac])

if test "$netcdf_found_ac" = "true"
then
    AC_MSG_CHECKING([Checking the netCDF library version number])

    AC_EGREP_HEADER(nc_inq_user_type, netcdf.h, NETCDF_MAJOR_VERSION=4, NETCDF_MAJOR_VERSION=3)
    AC_DEFINE_UNQUOTED(NETCDF_VERSION, $NETCDF_MAJOR_VERSION, [What version of netcdf were we built with?])

    AC_MSG_RESULT($NETCDF_MAJOR_VERSION)
fi

# FIXME
AM_CONDITIONAL([VTK_FOUND], [false])

# This can be really useful, but we're not exactly there yet.
# 
# AM_CONDITIONAL([NETCDF4_TESTS], [test "$NETCDF_MAJOR_VERSION" = "4"])

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([fcntl.h float.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
# <functional> is not needed...
# AC_HEADER_TR1_FUNCTIONAL
# AC_HEADER_UNORDERED_MAP
AC_CHECK_HEADER(tr1/unordered_map,[AC_DEFINE([HAVE_TR1_UNORDERED_MAP],[],["Have tr1"])],[])
AC_CHECK_HEADER(unordered_map,[AC_DEFINE([HAVE_UNORDERED_MAP],[],["Have C++0x"])],[])

AC_C_INLINE
AC_TYPE_OFF_T
AC_C_RESTRICT
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FSEEKO
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_STRTOD
AC_CHECK_FUNCS([floor gettimeofday pow putenv sqrt strtol])

AC_CONFIG_FILES([Makefile tests/Makefile])
AC_OUTPUT
