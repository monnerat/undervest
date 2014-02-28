//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Mail domain specific methods.          *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#include <string.h>
#include <cctype>
#include <memory>
#include <utility>

#include "logger.h"
#include "source.h"
#include "domain.h"
#include "configuration.h"
#include "globals.h"
#include "scheduler.h"


void
Domain::NgramMatcher::slice(slices& s,
				const std::string& str, bool indexing) const
{
	int sz = str.length();
	int n = sz > ngram_size? ngram_size: sz;
	int j = sz - n + 1;

	s.clear();

	for (auto i = 0; i < j; i++)
		s.emplace_back(str.data() + i, n);
}


void
Domain::WordMatcher::slice(slices& s,
				const std::string& str, bool indexing) const
{
	std::string::size_type i;
	std::string::size_type j;
	static const char zero('0');
	static const char one('1');

	s.clear();

	for (i = 0; i < str.length();) {
		j = i;

		if (isalpha(str[i])) {
			do {
				i++;
			} while (i < str.length() && isalpha(str[i]));

			s.emplace_back(str.data() + j, i - j);
			}
		else if (isdigit(str[i++]))
			s.emplace_back(indexing? &one: &zero, 1);
		}
}


Domain::Domain(Configuration * cp, const std::string& domname): Logger(cp)

{
	configuration = cp;
	name = domname;
	ngramThreshold = 0.8;
	wordThreshold = 0.5;
	disabled = 1;
}


Domain::Domain(const Domain& s): Logger(s.configuration)

{
	name = s.name;
	sources = s.sources;
	configuration = s.configuration;
	ngramThreshold = s.ngramThreshold;
	wordThreshold = s.wordThreshold;
	refresh_sec = s.refresh_sec;
	disabled = s.disabled;
	ngram_size = s.ngram_size;
	fixRecipient = s.fixRecipient;
	ngram_match = NgramMatcher(ngram_size);
	word_match = WordMatcher();

	if (disabled != 2) {
		disabled = 1;
		recipients = s.recipients;

		for (auto i = recipients.begin(); i != recipients.end(); i++) {
			ngram_match.add(*i);
			word_match.add(*i);
			}

		disabled = 0;
		}
}


Domain::Domain(Domain&& s): Logger(s.configuration)

{
	if (&s != this)
		std::swap(*this, s);
}


Domain&
Domain::operator=(const Domain& s)

{
	if (&s != this) {
		Logger::operator=(s);
		name = s.name;
		sources = s.sources;
		configuration = s.configuration;
		ngramThreshold = s.ngramThreshold;
		wordThreshold = s.wordThreshold;
		refresh_sec = s.refresh_sec;
		disabled = s.disabled;
		ngram_size = s.ngram_size;
		fixRecipient = s.fixRecipient;
		ngram_match = NgramMatcher(ngram_size);
		word_match = WordMatcher();

		if (disabled != 2) {
			disabled = 1;
			recipients = s.recipients;

			for (auto i = recipients.begin();
			    i != recipients.end(); i++) {
				ngram_match.add(*i);
				word_match.add(*i);
				}

			disabled = 0;
			}
		}

	return *this;
}


Domain&
Domain::operator=(Domain&& s)

{
	if (&s != this)
		std::swap(*this, s);

	return *this;
}


void
Domain::log(const std::string& msg, int level)

{
	if (!name.length())
		up->log(msg, level);

	up->log("domain: " + name + ", " + msg, level);
}


void
Domain::load_loaders()

{
	for (auto i = sources.begin(); i != sources.end(); i++)
		try {
			(*i).load_loader();
			}
		catch (...) {
			disabled = 2;
			throw;
			}

	for (auto i = recipients.begin(); i!= recipients.end(); i++)
		puts((*i).c_str());
}


void
Domain::load_data()

{
	if (disabled != 2) {
		for (auto i = sources.begin(); i != sources.end(); i++)
			try {
				(*i).load_data();
				}
			catch (...) {
				disabled = 1;
				throw;
				}

		ngram_match = NgramMatcher(ngram_size);
		word_match = WordMatcher();

		for (auto i = recipients.begin(); i != recipients.end(); i++) {
			ngram_match.add(*i);
			word_match.add(*i);
			}

		disabled = 0;
		}
}


const std::string *
Domain::recipient(const std::string& s) const

{
	std::vector<Matcher::result> results;

	if (disabled)
		return &s;

	ngram_match.match(results, s);

	if (results.size() && results[0].score >= ngramThreshold) {
		if (results.size() > 1 && results[1].score >= ngramThreshold)
			return &s;

		return results[0].text;
		}

	word_match.match(results, s);

	if (results.size() && results[0].score >= wordThreshold) {
		if (results.size() > 1 &&
		    results[1].score >= wordThreshold)
			return &s;

		return results[0].text;
		}

	return NULL;
}


static void
domainRefresher(Globals * g, std::shared_ptr<Configuration> c,
	std::shared_ptr<Domain> d, Scheduler& s, Scheduler::TimedEvent t)

{
	if (c != g->conf)
		return;

	auto domit = c->domains.find(d->name);

	if (domit == c->domains.end() || (*domit).second != d)
		return;			// Do not refresh if outdated.

	try {
		(*domit).second = d->refresh(c);
		}
	catch (AlreadyLogged& e) {
		;
		}
	catch (std::exception& e) {
		d->log(e.what());
		}
	catch (...) {
		d->log("An unknown exception occurred");
		}
}


std::shared_ptr<Domain>
Domain::refresh(std::shared_ptr<Configuration> c)

{
	std::shared_ptr<Domain> d;

	if (disabled == 2)
		throw AlreadyLogged();

	if (c->glob->debuglvl > 1)
		log("reloading");

	//	Clone domain's configuration.

	d = std::make_shared<Domain>(&*c, name);
	d->sources = sources;

	for (auto i = d->sources.begin(); i != d->sources.end(); i++)
		(*i).setParent(&*d);

	d->ngramThreshold = ngramThreshold;
	d->wordThreshold = wordThreshold;
	d->refresh_sec = refresh_sec;
	d->ngram_size = ngram_size;
	d->fixRecipient = fixRecipient;

	//	Load address data.

	d->load_data();

	if (d->refresh_sec)
		c->glob->sched.schedule(
		    Scheduler::TimedEvent(std::chrono::seconds(d->refresh_sec),
		    domainRefresher, c->glob, c, d));

	return d;
}
