#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(cleancode, 0.28, pmandin@caramail.com)
AC_CONFIG_SRCDIR([video/xbios.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_AUX_DIR([autotools])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_AS
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for system services
AC_OBJEXT

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_VOLATILE

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_CHECK_FUNCS([memset strcasecmp])

# Checks for libraries
AC_CHECK_LIB(gem, appl_init)

AC_CONFIG_FILES([
Makefile
audio/Makefile
bus/Makefile
cdrom/Makefile
common/Makefile
disk/Makefile
hardware/Makefile
joystick/Makefile
keyboard/Makefile
mouse/Makefile
video/Makefile
])

AC_OUTPUT
