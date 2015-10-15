//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Milter context methods.                *
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

#include <string>
#include <iostream>

#include "undervest.h"
#include "globals.h"
#include "configuration.h"
#include "stringlib.h"
#include "support11.h"


void
Undervest::clear()
{
	rejected.clear();
	maybe.clear();
	added.clear();
}


sfsistat
Undervest::connect(const char * hostname,
                                const _SOCK_ADDR * hostaddr, void * arg)
{
	glob = static_cast<Globals *>(arg);
	conf = glob->conf;

	if (glob->debug() >= 1)
		std::cerr << "connect: " << hostname << std::endl;

	return SMFIS_CONTINUE;
}


sfsistat
Undervest::envelopeFrom(const char * address, const char * const * argv)
{
	if (glob->debug() >= 1)
		std::cerr << "envelopeFrom: " << address << std::endl;

	clear();
	return SMFIS_CONTINUE;
}


sfsistat
Undervest::envelopeRecipient(const char * address, const char * const * argv)

{
	std::string s(strtolower(std::string(address)));
	bool fixit = false;
	std::string mapped;
	std::string mailer(symbolValue("{rcpt_mailer}"));
	bool inError(mailer == "error");

	if (glob->debug() >= 1)
		std::cerr << "envelopeRecipient: " << address <<
		    ' ' << mailer << std::endl;

	mapped = conf->recipient(s, &fixit, inError);

	if (mapped.empty()) {
		//	Unknown recipient.

		reply(conf->smtp_status, conf->smtp_extended,
		    conf->smtp_message);
		return SMFIS_REJECT;
		}

	if (glob->debug() >= 2)
		conf->log("orig=\"" + s + "\", fixed=\"" + mapped +
		    "\", fix=" + std::to_string(fixit));

	if (mapped == s || !fixit) {
		//	Comes here if:
		//	- perfect match or
		//	- more than a single fuzzy match or
		//	- single address fuzzy match without fix.

		if (inError)				// Re-add later.
			um_emplace(added, s, implode(" ", argv));

		if (mapped != s)
			us_emplace(maybe, mapped);

		return SMFIS_CONTINUE;
		}

	//	Single fuzzy match with fix.
	//	Replace by the new address later.

	um_emplace(added, mapped, implode(" ", argv));
	us_emplace(rejected, s);
	return SMFIS_CONTINUE;
}


sfsistat
Undervest::endOfMessage()
{
	if (glob->debug() >= 1)
		std::cerr << "endOfMessage" << std::endl;

	for (auto i = rejected.begin(); i != rejected.end(); i++) {
		addHeader("X-undervest-original-recipient", *i);
		deleteRecipient(*i);
		}

	for (auto i = maybe.begin(); i != maybe.end(); i++)
		addHeader("X-undervest-maybe-to", *i);

	for (auto i = added.begin(); i != added.end(); i++)
		addRecipient((*i).first, (*i).second);

	return SMFIS_CONTINUE;
}


void
Undervest::uncatched_exception(const std::string& msg, void * arg)
{
	static_cast<Globals *>(arg)->log(msg);
}
