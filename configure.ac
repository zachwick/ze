#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([hello], [1], [/dev/null])
AC_CONFIG_SRCDIR([ze.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AM_INIT_AUTOMAKE
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([atexit ftruncate memmove memset strchr strdup strerror strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
