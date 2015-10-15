//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Globals class.                         *
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

#ifndef __INCLUDE_GLOBALS_H__
#define __INCLUDE_GLOBALS_H__

#include "signal.h"

#include <string>
#include <memory>
#include <thread>

#include "logger.h"
#include "milter.h"
#include "configuration.h"
#include "scheduler.h"


class Globals: public Logger, public milter {

public:
	std::string		user;		// Running user.
	std::string		group;		// Running group.
	std::string		config;		// Configuration file name.
	std::string		jail;		// Jail directory.
	bool			nodaemon; 	// Do not detach.
	bool			logstd;		// Log to stderr too.
	bool			stderrOnly;	// Log to stderr only. */
	unsigned int		debuglvl;	// Debug level.
	std::shared_ptr<Configuration> conf;	// Current configuration.

	Scheduler		sched;		// Timed event scheduler.
	std::thread		sigThread;	// Reload signal monitor.
	std::thread		schedThread;	// Scheduler thread.
	bool			die;		// True if stop requested.

	Globals(): nodaemon(false), logstd(false), stderrOnly(false),
	    debuglvl(0), die(false) {};

	virtual ~Globals()
	{
		die = true;

		if (sigThread.joinable()) {
			::pthread_kill(sigThread.native_handle(), SIGUSR1);
			sigThread.join();
			}

		if (schedThread.joinable()) {
			sched.stop();
			schedThread.join();
			}
	}

	void		usage();
	virtual		void log(const std::string& msg, int level = LOG_ERR);
	int		run(int argc, char * * argv);
	void		reload();
	void		reloader();
};

#endif
