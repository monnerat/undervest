//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Local file recipient data importer.    *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#include <cstdlib>
#include <cstring>

#include <fstream>
#include <string>
#include <stdexcept>

#include "source.h"
#include "uri.h"
#include "stringlib.h"


class sm_handle {

public:
	std::ifstream	in;

	sm_handle(const std::string& name): in(name) {};
};


extern "C" void *
openSource(Source * source)

{
	URI u(source->uri);
	std::string s(u.path().get());

	if (!s.length() ||
	    (!u.scheme().empty() && strtolower(u.scheme()) != "file"))
		throw std::runtime_error(
		    "Sendmail handler requires a file URI");

	return (void *) new sm_handle(s);
}


extern "C" void
closeSource(void * vh, Source * source)

{
	delete (sm_handle *) vh;
}


extern "C" std::string
getRecipient(void * vh, Source * source)

{
	sm_handle * h = (sm_handle *) vh;
	std::string line;
	std::string::size_type n;

	while (getline(h->in, line)) {
		n = line.find_first_of(":# \t\r\n");

		if (n != std::string::npos && n && line[n] == ':')
			return line.substr(0, n);
		}

	return "";
}
