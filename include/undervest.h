//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Milter context data class.             *
//               *                                           *
//               *-------------------------------------------*
//               *                                           *
//               * Copyright (c) 2014-2015 Datasphere S.A.   *
//               * Copyright (c) 2015 D+H                    *
//               *                                           *
//               *   This software is licensed as described  *
//               * in the file LICENSE, which you should     *
//               * have received as part of this             *
//               * distribution.                             *
//               *   You may opt to use, copy, modify,       *
//               * merge, publish, distribute and/or sell    *
//               * copies of this software, and permit       *
//               * persons to whom this software is          *
//               * furnished to do so, under the terms of    *
//               * the LICENSE file.                         *
//               *   This software is distributed on an      *
//               * "AS IS" basis, WITHOUT WARRANTY OF ANY    *
//               * KIND, either express or implied.          *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#ifndef __INCLUDE_UNDERVEST_H__
#define __INCLUDE_UNDERVEST_H__

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <memory>

#include "milter.h"


class Globals;
class Configuration;

class Undervest: public milter::context {

	std::unordered_set<std::string>	rejected;	// Rejected recipients.
	std::unordered_map<std::string, std::string> added;	// Added rcpts.
	std::unordered_set<std::string>	maybe;		// Possible matches.

public:
	Globals *		glob;		// Global storage.
	std::shared_ptr<Configuration> conf;	// Configuration in use.


	static const unsigned long	callbackFlags = SMFIP_NOHELO |
	    SMFIP_NOBODY | SMFIP_NODATA | SMFIP_NOHDRS | SMFIP_NOEOH |
	    SMFIP_NOUNKNOWN | SMFIP_RCPT_REJ;
	static const unsigned long	flags =
	     SMFIF_ADDRCPT | SMFIF_ADDRCPT_PAR | SMFIF_DELRCPT | SMFIF_ADDHDRS;

	Undervest(): glob(0), conf() {};

	virtual ~Undervest() {};

	sfsistat
	connect(const char * hostname, const _SOCK_ADDR * hostaddr, void * arg);
	sfsistat	envelopeFrom(const char * address,
						const char * const * argv);
	sfsistat	envelopeRecipient(const char * address,
                                                const char * const * argv);
	sfsistat	endOfMessage();

	static void	uncatched_exception(const std::string& msg, void * arg);

private:
	void		clear();
};

#endif
