AC_INIT([mjpeg-cgi], m4_esyscmd([./version | tr -d '\n']),[http://www.xxx.com/support],[mjpeg-cgi.tar.gz],[http://www.xxx.com/])
AC_SUBST([RELEASE],[main])
AC_SUBST([GIT_BRANCH],[dev])

AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_INSTALL

AC_LANG_CPLUSPLUS
AX_CXX_COMPILE_STDCXX_11

AC_SUBST([DOLLAR_SIGN],[$])

AC_ARG_ENABLE(debug,
  AS_HELP_STRING([--enable-debug],[enable debugging, default: false]),
    [debug=true ],
    [debug=false])

AM_CONDITIONAL([DEBUG], test x"$debug" = x"true")

if test x$debug == xtrue; then
AC_DEFINE(DEBUG, 1, [debug enable])

AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])

CFLAGS=""
CXXFLAGS=""
fi

AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([fcgi], [main])
AC_CHECK_LIB([tinyxml], [main])
AC_CHECK_LIB([curl], [main])
AC_CHECK_LIB([upnp], [main])
AC_CHECK_LIB([ixml], [main])

dnl Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([bzero memset regcomp strerror])
AC_CHECK_FUNCS([gethostbyname])
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([inet_ntoa])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([mkfifo])
AC_CHECK_FUNCS([select])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strtol])
AC_CHECK_FUNCS([strtoul])
dnl Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h syslog.h unistd.h mqueue.h])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/time.h])
dnl Checks for typedefs, structures, and compiler characteristics.
dnl AC_FUNC_STRNLEN
AC_PROG_RANLIB
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
