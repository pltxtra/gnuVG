#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([gnuVG], [0.1.0], [pltxtra@vuknob.com])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
LT_INIT
AC_CONFIG_MACRO_DIR([m4])
PKG_INSTALLDIR
PKG_PROG_PKG_CONFIG

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([freetype2], [freetype2])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 gnuVG.pc
                 src/Makefile
                 src/libtess2/Makefile
                 src/libtess2/src/Makefile
                 src/skyline/Makefile
		 ])
AC_OUTPUT
