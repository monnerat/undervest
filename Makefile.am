#                                    \|||/
#                                   { o o }
#                *------------.oOO-----U-----OOo.------------*
#                *                                           *
#                *                 UNDERVEST                 *
#                *                                           *
#                *  - Makefile for the whole project.        *
#                *                                           *
#                *-------------------------------------------*
#                *                                           *
#                * Copyright (c) 2014-2015 Datasphere S.A.   *
#                * Copyright (c) 2015 D+H                    *
#                *                                           *
#                *   This software is licensed as described  *
#                * in the file LICENSE, which you should     *
#                * have received as part of this             *
#                * distribution.                             *
#                *   You may opt to use, copy, modify,       *
#                * merge, publish, distribute and/or sell    *
#                * copies of this software, and permit       *
#                * persons to whom this software is          *
#                * furnished to do so, under the terms of    *
#                * the LICENSE file.                         *
#                *   This software is distributed on an      *
#                * "AS IS" basis, WITHOUT WARRANTY OF ANY    *
#                * KIND, either express or implied.          *
#                *                                           *
#                *-------------------------------------------*
#                * CREATION                                  *
#                *   P. MONNERAT                  26/02/2014 *
#                *--------------.oooO-----Oooo.--------------*
#
#*******************************************************************************

AUTOMAKE_OPTIONS =	foreign

SUFFIXES	=	.conf .conf.in .service .service.in .init .init.in

SUBDIRS		=	src include docs

EXTRA_DIST	=	undervest.spec					\
			undervest.conf.in				\
			undervest.sysconfig				\
			undervest.init.in				\
			undervest.service.in				\
			LICENSE

sysconf_DATA	=	undervest.conf
noinst_DATA	=	undervest.service				\
			undervest.init


SED_CMD		=	sed -e "s,[@]PKGLIBDIR[@],${pkglibdir},g"	\
			    -e "s,[@]SYSCONFDIR[@],${sysconfdir},g"	\
			    -e "s,[@]RUNDIR[@],${RUNDIR},g"		\
			    -e "s,[@]LOCKDIR[@],${LOCKDIR},g"		\
			    -e "s,[@]BINDIR[@],${bindir},g"		\
			    -e "s,[@]SBINDIR[@],${sbindir},g"

.conf.in.conf:
	${SED_CMD} < $< > $@ && touch -r $< $@

.service.in.service:
	${SED_CMD} < $< > $@ && touch -r $< $@

.init.in.init:
	${SED_CMD} < $< > $@ && touch -r $< $@
