#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([gridfields], [1.0.0], [Bill Howe <billhowe@cs.washington.edu>])
AC_CONFIG_SRCDIR([src/abstractcellarray.h])
AM_CONFIG_HEADER([config.h])

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
if test "$CC" = "g++"
then
    AM_CONDITIONAL([COMPILER_IS_GCC],[true])
else
    AM_CONDITIONAL([COMPILER_IS_GCC],[false])
fi

# Checks for libraries.
# These appear to be used by test code
# FIXME: Replace `main' with a function in `-lefence':
# AC_CHECK_LIB([efence], [main])
# FIXME: Replace `main' with a function in `-lnetcdf':
# AC_CHECK_LIB([netcdf], [main])
# FIXME: Replace `main' with a function in `-lvtkCommon':
# AC_CHECK_LIB([vtkCommon], [main])
# FIXME: Replace `main' with a function in `-lvtkFiltering':
# AC_CHECK_LIB([vtkFiltering], [main])
# FIXME: Replace `main' with a function in `-lvtkIO':
# AC_CHECK_LIB([vtkIO], [main])
# FIXME: Replace `main' with a function in `-lvtkRendering':
# AC_CHECK_LIB([vtkRendering], [main])
# FIXME: Replace `main' with a function in `-lwayne':
# AC_CHECK_LIB([wayne], [main])

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
   netcdf_found_ac=false
   AC_MSG_WARN([netcdf library and netcdf headers not found.])
   ],[3]
)

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
