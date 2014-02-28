#                                    \|||/
#                                   { o o }
#                *------------.oOO-----U-----OOo.------------*
#                *       D A T A S P H E R E   S . A .       *
#                *-------------------------------------------*
#                *                                           *
#                *                 UNDERVEST                 *
#                *                                           *
#                *  - Makefile for the whole project.        *
#                *                                           *
#                *-------------------------------------------*
#                * CREATION                                  *
#                *   P. MONNERAT                  26/02/2014 *
#                *--------------.oooO-----Oooo.--------------*
#
#*******************************************************************************

SUFFIXES	=	.conf .conf.in .service .service.in .init .init.in

SUBDIRS		=	src include

EXTRA_DIST	=	undervest.spec					\
			undervest.conf.in				\
			undervest.sysconfig				\
			undervest.init.in				\
			undervest.service.in

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
