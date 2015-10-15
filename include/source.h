//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Mail addresses source class.           *
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

#ifndef __INCLUDE_SOURCE_H__
#define __INCLUDE_SOURCE_H__

#include <string>
#include <cdlfcn>

#include "logger.h"


//	E-mail addresses source.

class Domain;

class Source: public Logger {

public:
	//	Loader function types.

	typedef void *		(* open_source_t)(Source * source);
	typedef std::string	(* get_recipient_t)(void * handle,
							Source * source);
	typedef void		(* close_source_t) (void * handle,
							Source * source);


	std::string	uri;		// Source URI.
	std::string	loader;		// Loader shared library id.
	std::dl		dll;		// Loader access.
	open_source_t	openSource;	// Loader openSource function.
	get_recipient_t	getRecipient;	// Loader getRecipient function.
	close_source_t	closeSource;	// Loader closeSource function.
	Domain *	domain;		// Domain for this source.


	Source(Domain * dp);
	Source(const Source& s);
	Source(Source&& s);
	virtual		~Source();

	Source& operator=(const Source& s);
	Source& operator=(Source&& s);

	virtual		void log(const std::string& msg, int level = LOG_ERR);

	void		setParent(Domain * d);
	void		load_loader();
	void		load_data();

private:
	void		open_loader(const std::string& path);
	void		search_loader(const std::string& prefix,
						const std::string& suffix);
};


#endif
