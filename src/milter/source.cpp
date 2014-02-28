//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - E-mail addresses source methods.       *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#include <string.h>

#include <exception>
#include <utility>

#include "cdlfcn"

#include "source.h"
#include "domain.h"
#include "configuration.h"
#include "stringlib.h"
#include "support11.h"


Source::~Source()

{
}


void
Source::setParent(Domain * d)
{
	Logger::setParent(d);
	domain = d;
}


Source::Source(Domain * d)

{
	setParent(d);
}


Source::Source(const Source& s)

{
	uri = s.uri;
	loader = s.loader;
	dll = s.dll;
	openSource = s.openSource;
	getRecipient = s.getRecipient;
	closeSource = s.closeSource;
	setParent(s.domain);
}


Source::Source(Source&& s)

{
	std::swap(*this, s);
}


Source&
Source::operator=(const Source& s)

{
	if (&s != this) {
		uri = s.uri;
		loader = s.loader;
		dll = s.dll;
		openSource = s.openSource;
		getRecipient = s.getRecipient;
		closeSource = s.closeSource;
		setParent(s.domain);
		}

	return *this;
}


Source&
Source::operator=(Source&& s)

{
	if (&s != this)
		std::swap(*this, s);

	return *this;
}


void
Source::log(const std::string& msg, int level)

{
	if (!uri.length())
		up->log(msg, level);

	up->log("URI: " + uri + ", " + msg, level);
}


void
Source::open_loader(const std::string& path)

{
	try {
		dll.open(path);
		}
	catch (...) {
		return;
		}

	try {
		dll.symbol(openSource, "openSource");
		dll.symbol(getRecipient, "getRecipient");
		dll.symbol(closeSource, "closeSource");
		}
	catch (...) {
		dll.close();
		}
}


void
Source::search_loader(const std::string& prefix, const std::string& suffix)

{

	if (loader[0] == '/') {				// Absolute path.
		open_loader(loader);
		return;
		}

	for (auto lp = domain->configuration->loader_path.begin();
	    lp != domain->configuration->loader_path.end(); lp++) {
		std::string s(*lp);

		if (s.length() && s[s.length() - 1] != '/')
			s += '/';

		open_loader(s + prefix + loader + suffix);

		if (dll.is_open())
			return;
		}

	open_loader(prefix + loader + suffix);
}


void
Source::load_loader()

{
	//	Find the loader for the given source.

	search_loader("", "");

	if (!dll.is_open()) {
		//	Check if loader may be a name.

		if (loader.find('/') == std::string::npos) {
			search_loader("", ".so");

			if (!dll.is_open())
				search_loader("lib", "");

			if (!dll.is_open())
				search_loader("lib", ".so");
			}
		}

	if (!dll.is_open())
		log_throw(std::string("No suitable loader found: ") + loader);
}


void
Source::load_data()

{
	const char * domname = domain->name.c_str();
	void * h;
	std::string recipient;
	size_t i;

	try {
		h = (*openSource)(this);

		try {
			while ((recipient =
			    (*getRecipient)(h, this)).length()) {
				i = recipient.find('@');

				if (i != std::string::npos) {
					if (strcasecmp(domname,
					    recipient.substr(i + 1).c_str()))
						continue;

					recipient = recipient.substr(0, i);
					}

				us_emplace(domain->recipients,
				    strtolower(recipient));
				}
			}
		catch(...) {
			(*closeSource)(h, this);
			throw;
			}

		(*closeSource)(h, this);
		}
	catch (AlreadyLogged& e) {
		throw e;
		}
	catch (std::exception& e) {
		log_throw(e.what());
		}
}
