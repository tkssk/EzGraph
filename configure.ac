#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(EzGraph, 2.0)
AM_INIT_AUTOMAKE
LT_INIT
AC_CONFIG_SRCDIR([EzGraph/EzGraph.c])
AC_CONFIG_HEADERS([EzGraph/config.h])

AC_ARG_ENABLE(Xdbe,
	AS_HELP_STRING([--enable-Xdbe],
		[Enable Xdbe (X Double Buffering Extension) @<:@default=no@:>@]),
		[ac_cv_Xdbe=$enableval], [ac_cv_Xdbe=no])

AC_ARG_ENABLE(iconv,
	AS_HELP_STRING([--enable-iconv=<EUC-JP/SHIFT-JIS/UTF-8>],
		[Set charactor code of source files @<:@default=UTF-8@:>@]),
		[ac_cv_EzCharCode=$enableval], [ac_cv_EzCharCode=UTF-8])

AC_ARG_ENABLE(ALSA,
	AS_HELP_STRING([--enable-ALSA],
		[Enable ALSA-MML (Music Macro Language) engine @<:@default=yes@:>@]),
		[ac_cv_ALSA=$enableval], [ac_cv_ALSA=yes])

AC_ARG_ENABLE(NETEXT,
	AS_HELP_STRING([--enable-NETEXT],
		[Enable network extention @<:@default=yes@:>@]),
		[ac_cv_NETEXT=$enableval], [ac_cv_NETEXT=yes])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AM_PROG_LIBTOOL

# Checks for header files.
AC_PATH_X
AC_PATH_XTRA

# Checks for typedefs, structures, and compiler characteristics.
## Save CFLAGS, CPPFLAGS, LDFLAGS
pre_CFLAGS=$CFLAGS
pre_CPPFLAGS=$CPPFLAGS
pre_LDFLAGS=$LDFLAGS
CFLAGS="${CFLAGS} ${X_CFLAGS}"
CPPFLAGS="${CPPFLAGS} ${X_CFLAGS}"
LDFLAGS="${LDFLAGS} ${X_LIBS}"

if test "X$ac_cv_Xdbe" == Xyes ; then
AC_CHECK_HEADERS([X11/extensions/Xdbe.h], [], [ac_cv_Xdbe=no],
	[[#include <X11/Xlib.h>]])
fi

AC_CHECK_LIB(Xft, XftDrawStringUtf8, [], [echo "Error: cannot find Xft library." && exit 1])

if test "X$ac_cv_ALSA" == Xyes ; then
AC_CHECK_HEADERS([alsa/asoundlib.h], [], [ac_cv_ALSA=no],
	[[#include <alsa/asoundlib.h>]])
AC_CHECK_LIB(asound, snd_pcm_open, [], [ac_vc_ALSA=no])
fi

if test "X$ac_cv_NETEXT" == Xyes ; then
AC_CHECK_HEADERS([sys/socket.h], [], [ac_cv_NETEXT=no],
	[[#include <sys/socket.h>]])
AC_CHECK_HEADERS([netinet/in.h], [], [ac_cv_NETEXT=no],
	[[#include <netinet/in.h>]])
AC_CHECK_HEADERS([arpa/inet.h], [], [ac_cv_NETEXT=no],
	[[#include <arpa/inet.h>]])
AC_CHECK_HEADERS([netdb.h], [], [ac_cv_NETEXT=no],
	[[#include <netdb.h>]])
fi

PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES(Freetype2,freetype2)

# Checks for library functions.

#Restore CFLAGS, CPPFLAGS, LDFLAGS
CFLAGS=$pre_CFLAGS
CPPFLAGS=$pre_CPPFLAGS
LDFLAGS=$pre_LDFLAGS

if test "X$ac_cv_Xdbe" == Xyes ; then
	defXdbe=-DHAVE_XDBE
	libXdbe=-lXext
fi
AC_SUBST(defXdbe)
AC_SUBST(libXdbe)

if test "X$ac_cv_ALSA" == Xyes ; then
	defALSA=-DHAVE_ALSA
	libALSA="-lasound -lpthread"
fi
AC_SUBST(defALSA)
AC_SUBST(libALSA)
AM_CONDITIONAL([ALSA], [test "X$ac_cv_ALSA" == Xyes])

if test "X$ac_cv_NETEXT" == Xyes ; then
	defNETEXT=-DHAVE_NETEXT
fi
AC_SUBST(defNETEXT)
AM_CONDITIONAL([NETEXT], [test "X$ac_cv_NETEXT" == Xyes])

if test "X$ac_cv_EzCharCode" != "XUTF-8" ; then
	AC_CHECK_HEADERS([iconv.h], [], [echo "Error: cannot find iconv.h" && exit 1])
	defEzCharCode="-DUseICONV -DEzCharCode=$ac_cv_EzCharCode"
fi

AC_SUBST(defEzCharCode)

AC_OUTPUT(Makefile EzGraph/Makefile sample/Makefile tools/Makefile html/Makefile)
