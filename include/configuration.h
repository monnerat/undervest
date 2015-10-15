//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Configuration class.                   *
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

#ifndef __INCLUDE_CONFIGURATION_H__
#define __INCLUDE_CONFIGURATION_H__

#include <confuse.h>

#include <cstdarg>
#include <string>
#include <list>
#include <unordered_map>
#include <memory>

#include "logger.h"



//	Configuration.

class Domain;
class Globals;

class Configuration: public Logger {

public:
	std::unordered_map<std::string, std::shared_ptr<Domain> > domains;
	std::list<std::string>	loader_path;	// Loader directory paths.
	std::string		smtp_message;	// SMTP status message.
	std::string		smtp_extended;	// SMTP extended status.
	Globals *		glob;		// Pointer to global params. */
	unsigned short		smtp_status;	// SMTP status code.
	unsigned short		minloglevel;	// Minimum log level.
	unsigned short		maxloglevel;	// Maximum log level.
	bool			luserWarned;	// LUSER_RELAY warning issued.

	Configuration(Globals * gp);

	void		get(const std::string& configfile);
	virtual		void log(const std::string& msg, int level = LOG_ERR);
	std::string	recipient(const std::string& emailaddr,
			    bool * fixit, bool inError = false);

private:
	static int	logLevel(cfg_t * cfg, cfg_opt_t * opt,
					const char * value, void * result);
	static void	config_error(cfg_t * cfg,
					const char * fmt, va_list args);
};

#endif
