//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Main program.                          *
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <unistd.h>
#include <fcntl.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <csignal>

#include <string>
#include <list>
#include <iostream>
#include <thread>
#include <chrono>

#include "filepath.h"
#include "daemon.h"
#include "milter.h"
#include "undervest.h"
#include "globals.h"
#include "source.h"
#include "domain.h"
#include "configuration.h"


Globals *	g = NULL;


//	Explicit instantiation for old C++ compiler compatibility.

template sfsistat milter::_envfrom<Undervest>(SMFICTX *, char * *);
template sfsistat milter::_envrcpt<Undervest>(SMFICTX *, char * *);


void
Globals::usage()

{
	std::cerr << "Usage: " << name() << " [-l] [-e] [-d]..." <<
	    " [-u <user>] [-g <group>] \\" << std::endl;
	std::cerr << "\t[-j <jail directory>] [-c <config>] [-n]" <<
	    " <milter socket>" << std::endl;
	exit(1);
}


void
Globals::log(const std::string& msg, int level)

{
	if (!stderrOnly)
		::syslog(level | LOG_MAIL, "%s", msg.data());
	else
		fprintf(stderr, "%s\n", msg.data());
}


void
Globals::reload()
{
	std::shared_ptr<Configuration> newconf;
	std::shared_ptr<Configuration> saveconf(conf);
	Logger * l(saveconf.get());

	//	Reload the configuration file and domain data.

	if (l == NULL)
		l = this;

	if (debuglvl)
		l->log("Reloading configuration and data", LOG_DEBUG);

	try {
		newconf = std::make_shared<Configuration>(this);
		newconf->get(config);

		//	Read initial data.

		for (auto it = newconf->domains.begin();
		    it != newconf->domains.end(); it++) {
			auto dom((*it).second);

			try {
				(*it).second = dom->refresh(dom, newconf);
				}
			catch (AlreadyLogged& e) {
				}
			catch (std::exception& e) {
				dom->log(e.what());
				}
			catch (...) {
				dom->log("An unknown exception occurred");
				}
			}

		conf = newconf;
		}
	catch (AlreadyLogged& e) {
		return;
		}
	catch (std::exception& e) {
		l->log(e.what());
		return;
		}
	catch (...) {
		l->log("An unknown exception occurred");
		return;
		}
}


void
Globals::reloader()
{
	::sigset_t sigs;
	int signo;

	//	Let some time for milter to start.
	//	This should be:
	//		std::this_thread::sleep_for(std::chrono::seconds(1));
	//	But the compiler does not accept it yet.

	{
		struct timeval tm;

		tm.tv_sec = 1;
		tm.tv_usec = 0;

		select(0, NULL, NULL, NULL, &tm);
	}

	//	Process our signal.

	::sigemptyset(&sigs);
        ::sigaddset(&sigs, SIGUSR1);

	for (;;) {
		::sigwait(&sigs, &signo);

		if (die)
			break;

		reload();
		}
}


int
Globals::run(int argc, char * * argv)

{
	Daemon daemon;
	char * cp;
	char * sockarg;

	name(FilePath(argv[0]).filename());

	//	Parse arguments.

	config = FilePath("/etc") + name();

	for (;;) {
		cp = *++argv;

		if (!--argc || *cp != '-' || !cp[1])
			break;

		while (*++cp)
			switch (*cp) {

			case 'c':	// Configuration file.
				if (!--argc)
					usage();

				config = *++argv;
				break;

			case 'd':	// Increase debug level.
				debuglvl++;
				break;

			case 'e':		// Log to stderr only.
				stderrOnly = true;
				break;

			case 'g':		// Group.
				if (!--argc)
					usage();

				group = *++argv;
				break;

			case 'j':		// Jail.
				if (!--argc)
					usage();

				jail = *++argv;
				break;

			case 'l':		// Log to stderr too.
				logstd = true;
				break;

			case 'n':		// Do not detach.
				nodaemon = true;
				break;


			case 'u':		// User.
				if (!--argc)
					usage();

				user = *++argv;
				break;

			default:
				usage();
				}
		}

	if (!cp)
		usage();
	else if (--argc)
		usage();

	sockarg = cp;

	//	Daemonize.
	//	Do not change user and/or group now, since we need full
	//		permissions for sockets.

	try {
		daemon.program(name());
		daemon.lock_file(FilePath(LOCKDIR) + daemon.program());
		daemon.pid_file(FilePath(RUNDIR) + (daemon.program() + ".pid"));
		daemon.user(user);
		daemon.group(group);
		daemon.log_to_stderr(logstd);
		daemon.jail(jail);
		daemon.block_signals();
		daemon.ignore_signals();

		if (!nodaemon)
			daemon.detach();

		daemon.close_all(!stderrOnly);
		daemon.open_log(LOG_PID | LOG_NDELAY, LOG_MAIL);
		::signal(SIGABRT, SIG_DFL);
		::signal(SIGTERM, SIG_DFL);
		::signal(SIGINT, SIG_DFL);
		::signal(SIGILL, SIG_DFL);
		::signal(SIGIOT, SIG_DFL);
		::signal(SIGBUS, SIG_DFL);
		::signal(SIGFPE, SIG_DFL);
		::signal(SIGSEGV, SIG_DFL);
		::signal(SIGSTKFLT, SIG_DFL);
		daemon.unblock_signals();

		if (!nodaemon) {
			if (!daemon.lock()) {
				log(name() + " already running: exit");
				closelog();
				return 1;
				}

			daemon.write_pid();
			}
		}
	catch (std::exception& e) {
		die = true;
		log(e.what(), LOG_CRIT);
		closelog();
		return 1;
		}

	try {
		daemon.go_home();

		//	Harden our umask so that the new socket gets created
		//		securely.

		umask(0077);

		//	Register to the milter interface.

		socket(sockarg);
		debug(debuglvl);

		if (initialize<Undervest>() != MI_SUCCESS) {
			log("Cannot initialize milter interface", LOG_CRIT);
			closelog();
			return 1;
			}

		//	If we need to set the uid/gid/jail, do it now.

		daemon.incarcerate();
		daemon.change_identity();

		//	Start the scheduler.

		schedThread = std::thread(&Scheduler::run, std::ref(sched));

		//	Read configuration file and domain data.

		reload();

		if (&(*conf) == NULL) {
			sched.stop();
			return 1;
			}

		//	Start a thread to monitor and process our signals.
		//	Note: Signals SIGTERM, SIGHUP and SIGABRT are processed
		//		by the milter library.

		sigThread = std::thread(&Globals::reloader, std::ref(*this));

		//	Enter the milter service loop.

		if (start(static_cast<void *>(this)) == MI_FAILURE)
			throw std::runtime_error("Cannot run the milter");
		}
	catch (std::exception& e) {
		die = true;
		sched.stop();
		log(e.what(), LOG_CRIT);

		if (!nodaemon) {
			daemon.delete_pid();
			daemon.delete_lock();
			}

		closelog();
		return 1;
		}

	die = true;
	sched.stop();

	if (!nodaemon) {
		daemon.delete_pid();
		daemon.delete_lock();
		}

	closelog();
	return 0;
}


int
main(int argc, char * * argv)

{
	int exitcode;

	g = new Globals();
	exitcode = g->run(argc, argv);
	delete g;
	exit(exitcode);
}
