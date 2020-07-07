AC_REVISION([m4_esyscmd_s([build-aux/git-version])])

AC_INIT([roq-bitmex],
        m4_esyscmd_s([build-aux/git-version]),
        [thraneh@gmail.com],
        [roq-bitmex])

AC_PREREQ([2.59])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AC_CONFIG_FILES([
 Makefile
 src/Makefile
 src/roq/Makefile
 src/roq/bitmex/Makefile
 src/roq/bitmex/json/Makefile
 test/Makefile
])
AC_CONFIG_FILES([roq-bitmex.pc])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AM_PROG_AR
LT_INIT([disable-shared])
AC_PROG_CXX
AC_LANG([C++])

AX_PTHREAD

ROQ_CHECK_STDCXX

ROQ_CHECK_RT

ROQ_CHECK_ABSEIL
ROQ_CHECK_BROTLI
ROQ_CHECK_CPPTOML
ROQ_CHECK_CROSSGUID
ROQ_CHECK_CRYPTO
ROQ_CHECK_DATE
ROQ_CHECK_FMT
ROQ_CHECK_GFLAGS
ROQ_CHECK_HTTP_PARSER
ROQ_CHECK_LIBEVENT
ROQ_CHECK_OPENSSL

PKG_CHECK_MODULES([ROQ_LOGGING], [roq-logging])
PKG_CHECK_MODULES([ROQ_API], [roq-api])
PKG_CHECK_MODULES([ROQ_CORE], [roq-core])
PKG_CHECK_MODULES([ROQ_SERVER], [roq-server])

AC_ARG_ENABLE([benchmark],
	      AS_HELP_STRING([--enable-benchmark], [Enable benchmark]),
	      [],
	      [enable_urls=no])

AS_IF([test "x$enable_benchmark" = "xyes"],
      [AC_CONFIG_FILES([benchmark/Makefile])])

AS_IF(
  [test "x$enable_benchmark" = "xyes"],
  [ROQ_CHECK_BENCHMARK])

AM_CONDITIONAL([ENABLE_BENCHMARK],
	       [test "$enable_benchmark" = "yes"])

AC_OUTPUT
