#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([exgcoup], [0.1.0], [exgcoup@googlegroups.com])
AM_INIT_AUTOMAKE(exgcoup, 0.1.0)
AC_CONFIG_SRCDIR([include/libcfr.h])
AC_CONFIG_HEADERS([config.h])
#AM_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AM_PROG_CC_C_O

# Checks for libraries.
# FIXME: Replace `main' with a function in `-levent':
AC_CHECK_LIB([event], [event_init])
# FIXME: Replace `main' with a function in `-lrt':
AC_CHECK_LIB([rt], [aio_read])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getcwd gethostbyname gethostname gettimeofday memset modf setenv socket strncasecmp strstr])

# Define Values
libegc_version=0:0:0
ACP_DATE='April 6, 2016'
AC_SUBST(libegc_version)

top_egc_builddir="`pwd`"
AC_SUBST(top_egc_builddir)



AC_CONFIG_FILES([Makefile
                 src/Makefile
                 sample/Makefile])
AC_OUTPUT
