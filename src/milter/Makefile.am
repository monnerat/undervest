#                                    \|||/
#                                   { o o }
#                *------------.oOO-----U-----OOo.------------*
#                *                                           *
#                *                 UNDERVEST                 *
#                *                                           *
#                *  - Makefile for milter.                   *
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

sbin_PROGRAMS =		undervest

undervest_SOURCES =	source.cpp					\
			domain.cpp					\
			configuration.cpp				\
			matcher.cpp					\
			milter.cpp					\
			undervest.cpp					\
			uri.cpp						\
			crc32.cpp					\
			main.cpp

undervest_LDFLAGS =	-export-dynamic
undervest_LDADD  =	$(LIBADD_DLOPEN)
