//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Mail domain class.                     *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#ifndef __INCLUDE_DOMAIN_H__
#define __INCLUDE_DOMAIN_H__

#include <string>
#include <list>
#include <unordered_set>
#include <memory>

#include "matcher.h"
#include "logger.h"
#include "scheduler.h"


class Source;
class Configuration;


//	Mail domain.

class Domain: public Logger {

	//	Matching classes for domain recipient.

	class NgramMatcher: public Matcher {

	public:
		NgramMatcher(unsigned short siz = 3): ngram_size(siz) {};

		virtual void	slice(slices& s,
				const std::string& str, bool indexing) const;

		unsigned short	ngram_size;
	};

	class WordMatcher: public Matcher {

	public:
		virtual void	slice(slices& s,
				const std::string& str, bool indexing) const;
	};


public:
	std::string		name;		// Domain name.
	std::list<Source>	sources;	// Data sources.
	Configuration *		configuration;	// Configuration applying.
	std::unordered_set<std::string> recipients; // Valid domain recipients.
	NgramMatcher		ngram_match;	// n-gram matcher for domain.
	WordMatcher		word_match;	// Word matcher for domain.
	double			ngramThreshold;	// Positive lower probability.
	double			wordThreshold;	// Positive lower probability.
	unsigned int		refresh_sec;	// Refresh period (# sec).
	unsigned short		disabled;	// 1->temporary, 2->permanent.
	unsigned short		ngram_size;	// n-gram size for domain.
	bool			fixRecipient;	// Fix non-ambiguous recipient.

	Domain(Configuration * cp, const std::string& name);
	Domain(const Domain& s);
	Domain(Domain&& s);

	virtual ~Domain() {};

	Domain& operator=(const Domain& s);
	Domain& operator=(Domain&& s);

	virtual void	log(const std::string& msg, int level = LOG_ERR);

	void			load_loaders();
	void			load_data();
	const std::string *	recipient(const std::string& s) const;
	std::shared_ptr<Domain>	refresh(std::shared_ptr<Configuration> c);
};

#endif
