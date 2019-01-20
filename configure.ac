dnl                                  \|||/
dnl                                 { o o }
dnl              *------------.oOO-----U-----OOo.------------*
dnl              *                                           *
dnl              *                 UNDERVEST                 *
dnl              *                                           *
dnl              *  - Project configuration directives.      *
dnl              *                                           *
dnl              *-------------------------------------------*
dnl              *                                           *
dnl              * Copyright (c) 2014-2015 Datasphere S.A.   *
dnl              * Copyright (c) 2015 D+H                    *
dnl              *                                           *
dnl              *   This software is licensed as described  *
dnl              * in the file LICENSE, which you should     *
dnl              * have received as part of this             *
dnl              * distribution.                             *
dnl              *   You may opt to use, copy, modify,       *
dnl              * merge, publish, distribute and/or sell    *
dnl              * copies of this software, and permit       *
dnl              * persons to whom this software is          *
dnl              * furnished to do so, under the terms of    *
dnl              * the LICENSE file.                         *
dnl              *   This software is distributed on an      *
dnl              * "AS IS" basis, WITHOUT WARRANTY OF ANY    *
dnl              * KIND, either express or implied.          *
dnl              *                                           *
dnl              *-------------------------------------------*
dnl              * CREATION                                  *
dnl              *   P. MONNERAT                  26/02/2014 *
dnl              *--------------.oooO-----Oooo.--------------*
dnl
dnl*****************************************************************************

dnl	Process this file with autoconf to produce a configure script.

AC_INIT([undervest], [0.1.7])
AC_CONFIG_SRCDIR(src/milter/undervest.cpp)
AM_INIT_AUTOMAKE

dnl	Disable building of static libraries.

AC_DISABLE_STATIC

dnl	Checks for programs.

AC_LANG([C++])
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11
AC_PROG_INSTALL
AC_PROG_LN_S
PKG_PROG_PKG_CONFIG


dnl	Check for dynamic loading library.

LT_INIT([dlopen disable-static])
LT_LIB_DLLOAD


dnl	Checks for header files.

AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h sys/file.h syslog.h unistd.h ldap.h)


dnl	Checks for typedefs, structures, and compiler characteristics.

AC_TYPE_UID_T
AC_TYPE_PID_T


dnl	Checks for library functions.

AC_CHECK_FUNCS(strerror strtoul)

AC_CHECK_FUNC(setgroups,
	[AC_DEFINE([HAVE_SETGROUPS], 1, [setgroups() function])]
)


dnl	Check for confuse library.

PKG_CHECK_MODULES(CONFUSE, libconfuse)

if test "${CONFUSE_PKG_ERRORS}"
then	AC_MSG_ERROR([${CONFUSE_PKG_ERRORS}])
fi

CFLAGS="$CFLAGS $CONFUSE_CFLAGS"
LIBS="$LIBS $CONFUSE_LIBS"


dnl Search for pthreads, and make them the default (have to do it here else
dnl	the -lmilter check will fail to link).

AX_PTHREAD(,AC_MSG_ERROR([Cannot determine how to use pthreads]))
CFLAGS="${CFLAGS} ${PTHREAD_CFLAGS}"
LIBS="${PTHREAD_CFLAGS} ${PTHREAD_LIBS} ${LIBS}"
CC="${PTHREAD_CC}"


dnl Check for libmilter and its header files in the usual locations.

LDFLAGS="$LDFLAGS -L/usr/lib/libmilter"

AC_CHECK_LIB(milter, mi_stop,,[
	# Older sendmails require libsm for support functions.
	AC_SEARCH_LIBS(strlcpy, sm smutil)
	unset ac_cv_lib_milter_mi_stop
	AC_CHECK_LIB(milter, mi_stop,,[
		AC_MSG_ERROR([Cannot find libmilter])
	])
])

AC_CHECK_HEADERS(libmilter/mfapi.h,,[
        AC_MSG_ERROR([Please install mfapi.h from the sendmail distribution])
])


LDFLAGS="$LDFLAGS -lmilter"
AC_CHECK_FUNCS(smfi_opensocket)


dnl	Check for openldap library.

AC_CHECK_LIB(ldap, ldap_initialize)


dnl	Package library subdirectory for loaders.

CFLAGS="${CFLAGS} -DHANDLERDIR=\\\"\${pkglibdir}\\\""


dnl	Use our own include directory.

CFLAGS="\"-I\${top_srcdir}/include\" ${CFLAGS}"


dnl	Define system directories.

AC_ARG_WITH([rundir],
	AS_HELP_STRING([--with-rundir=DIR],
		[directory for pid file [LOCALSTATEDIR/run]]),
	[RUNDIR=$withval],
	[RUNDIR='${localstatedir}/run'])
AC_SUBST([RUNDIR])

AC_ARG_WITH([lockdir],
	AS_HELP_STRING([--with-lockdir=DIR],
		[directory for lock file [LOCALSTATEDIR/lock/subsys]]),
	[LOCKDIR=$withval],
	[LOCKDIR='${localstatedir}/lock/subsys'])
AC_SUBST([LOCKDIR])

CFLAGS="${CFLAGS} -DRUNDIR=\\\"${RUNDIR}\\\" -DLOCKDIR=\\\"${LOCKDIR}\\\""

CPPFLAGS="${CPPFLAGS} ${CFLAGS}"


dnl	Output target files.

AC_CONFIG_FILES([
	Makefile
	undervest.spec
	docs/Makefile
	include/Makefile
	src/Makefile
	src/milter/Makefile
	src/loaders/Makefile
])
AC_OUTPUT
