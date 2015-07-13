//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Daemon class.                          *
//               *                                           *
//               *-------------------------------------------*
//               *                                           *
//               * Copyright (c) 2014-2015 Datasphere S.A.   *
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

#ifndef __INCLUDE_DAEMON_H__
#define __INCLUDE_DAEMON_H__

#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <syslog.h>

#include <cerrno>
#include <cstdlib>
#include <csignal>
#include <cstdio>

#include <string>
#include <stdexcept>
#include <system_error>
#include <fstream>

#include "filepath.h"


class Daemon {

public:
	//	Field getters/setters.

	std::string program() const { return _program; }

	std::string
	program(const std::string& path)
	{
		std::string old(program());

		_program = FilePath(path).filename();
		return old;
	}

	bool log_to_stderr() const { return _log_to_stderr; };

	bool
	log_to_stderr(bool yesorno)
	{
		bool old(log_to_stderr());

		_log_to_stderr = yesorno;
		return old;
	}

	std::string group() const { return _group; };
	const ::gid_t& gid() const { return _gid; };

	std::string
	group(const std::string& gid_or_name)
	{
		std::string old(group());
		struct group grp;
		struct group * grpp;
		int err;
		char tmpbuf[1024];

		if (!gid_or_name.length())
			_group = "";
		else if (::geteuid())
			throw std::runtime_error(
			    "Daemon::group(): must be super-user");
		else if ((err = ::getgrnam_r(gid_or_name.data(),
		    &grp, tmpbuf, sizeof tmpbuf, &grpp)))
			throw std::system_error(err, std::system_category());
		else if (grpp) {
			_group = gid_or_name;
			_gid = grp.gr_gid;
			}
		else if (gid_or_name.find_first_not_of( "0123456789") !=
		    std::string::npos)
			throw std::runtime_error("Daemon::group(): group `" +
			    gid_or_name + "' not found");
		else if ((err = ::getgrgid_r(std::stoi(gid_or_name),
			    &grp, tmpbuf, sizeof tmpbuf, &grpp)))
			throw std::system_error(err, std::system_category());
		else if (grpp) {
			_group = grp.gr_name;
			_gid = grp.gr_gid;
			}
		else
			throw std::runtime_error("Daemon::group(): group `" +
			    gid_or_name + "' not found");

		return old;
	}

	::gid_t
	group(::gid_t groupid)
	{
		::gid_t old(gid());
		struct group grp;
		struct group * grpp;
		int err;
		char tmpbuf[1024];

		if (::geteuid())
			throw std::runtime_error(
			    "Daemon::group(): must be super-user");

		err = ::getgrgid_r(groupid, &grp, tmpbuf, sizeof tmpbuf, &grpp);

		if (err)
			throw std::system_error(err, std::system_category());

		if (!grpp)
			throw std::runtime_error("Daemon::group(): group " +
			    std::to_string(groupid) + " not found");

		_group = grp.gr_name;
		_gid = grp.gr_gid;
		return old;
	}

	std::string user() const { return _user; };
	const ::uid_t& uid() const { return _uid; };

	std::string
	user(const std::string& uid_or_name)
	{
		std::string old(user());
		struct passwd pwd;
		struct passwd * pwdp;
		int err;
		char tmpbuf[1024];

		if (!uid_or_name.length())
			_user = "";
		else if (::geteuid())
			throw std::runtime_error(
			    "Daemon::user(): must be super-user");
		else if ((err = ::getpwnam_r(uid_or_name.data(),
		    &pwd, tmpbuf, sizeof tmpbuf, &pwdp)))
			throw std::system_error(err, std::system_category());
		else if (pwdp) {
			group(pwd.pw_gid);
			_user = uid_or_name;
			_uid = pwd.pw_uid;
			}
		else if (uid_or_name.find_first_not_of( "0123456789") !=
		    std::string::npos)
			throw std::runtime_error("Daemon::user(): user `" +
			    uid_or_name + "' not found");
		else if ((err = ::getpwuid_r(std::stoi(uid_or_name),
		    &pwd, tmpbuf, sizeof tmpbuf, &pwdp)))
			throw std::system_error(err, std::system_category());
		else if (pwdp) {
			group(pwd.pw_gid);
			_user = pwd.pw_name;
			_uid = pwd.pw_uid;
			}
		else
			throw std::runtime_error("Daemon::user(): user `" +
			    uid_or_name + "' not found");

		return old;
	}

	::uid_t
	user(::uid_t userid)
	{
		::uid_t old(uid());
		struct passwd pwd;
		struct passwd * pwdp;
		int err;
		char tmpbuf[1024];

		if (::geteuid())
			throw std::runtime_error(
			    "Daemon::user(): must be super-user");

		err = ::getpwuid_r(userid, &pwd, tmpbuf, sizeof tmpbuf, &pwdp);

		if (err)
			throw std::system_error(err, std::system_category());

		if (!pwdp)
			throw std::runtime_error("Daemon::user(): user " +
			    std::to_string(userid) + " not found");

		group(pwd.pw_gid);
		_user = pwd.pw_name;
		_uid = userid;
		return old;
	}

	std::string home() const { return _home; };

	std::string
	home(const std::string& homedir)
	{
		std::string old(home());

		_home = homedir;
		return old;
	}

	std::string jail() const { return _jail; };

	std::string
	jail(const std::string& newfsroot)
	{
		std::string old(jail());

		if (newfsroot.length() && ::geteuid())
			throw std::runtime_error(
			    "Daemon::jail(): must be super-user");

		_jail = newfsroot;
		return old;
	}

	std::string lock_file() const { return _lock_file; };

	std::string
	lock_file(const std::string& lockfile)
	{
		std::string old(lock_file());

		if (!lockfile.length() ||
		    lockfile.find("/") != std::string::npos)
			_lock_file = lockfile;
		else
			_lock_file = "/var/lock/subsys/" + lockfile;

		return old;
	}

	std::string pid_file() const { return _pid_file; };

	std::string
	pid_file(const std::string& pidfile)
	{
		std::string old(pid_file());

		if (!pidfile.length() ||
		    pidfile.find("/") != std::string::npos)
			_pid_file = pidfile;
		else
			_pid_file = "/var/run/" + pidfile;

		return old;
	}


	//	Constructors.

	Daemon(): _log_to_stderr(false), _lockfd(-1) {};

	Daemon(const std::string& pgmpath):
	    _log_to_stderr(false), _lockfd(-1)
	{
		program(pgmpath);
	}

	//	Destructor.

	virtual ~Daemon()
	{
		unlock();
	}


	//	Actions.

	void
	detach() const
	{
		switch (::fork()) {

		case -1:					// Error.
			throw std::system_error(errno, std::system_category());

		case 0:						// Child.
			break;

		default:					// Father.
			std::exit(EXIT_SUCCESS);
			}

		setsid();				// Create new session.

		//	Let the session master be adopted by the init process.

		switch (::fork()) {

		case -1:					// Error.
			throw std::system_error(errno, std::system_category());

		case 0:						// Child.
			break;

		default:					// Father.
			exit(EXIT_SUCCESS);
			}
	}

	void
	close_all(bool close_stderr = true) const
	{
		int fd;

		for (fd = ::getdtablesize(); --fd > ::fileno(stderr);)
			::close(fd);

		if (close_stderr && !log_to_stderr())
			::close(fd);

		while (fd--)
			::close(fd);

		//	Open bit buckets on standard file descriptors.

		for (fd = ::open("/dev/null", O_RDWR); fd >= 0;
		    fd = ::dup(fd))
			if (fd >= ::fileno(stderr)) {
				if (fd > ::fileno(stderr))
					::close(fd);

				break;
				}
	}

	bool
	lock()
	{
		if (_lockfd >= 0)
			throw std::runtime_error("Daemon:lock(): "
			    "already locked");

		if (!lock_file().length()) {
			if (program().length())
				lock_file(program());
			else
				throw std::runtime_error("Daemon:lock(): "
				    "no lock file name");
			}

		_lockfd = ::open(lock_file().data(), O_RDWR | O_CREAT,
		    S_IRUSR | S_IWUSR | S_IRGRP | S_IRUSR | S_IWUSR);

		if (_lockfd < 0)
			throw std::system_error(errno, std::system_category());

		if (::lockf(_lockfd, F_TLOCK, 0) < 0) {
			::close(_lockfd);

			if (errno == EACCES || errno == EAGAIN)
				return false;		// Already locked.

			throw std::system_error(errno, std::system_category());
			}

		//	Do not close the file to keep lock on it.

		return true;				// We just locked it.
	}

	void
	unlock()
	{
		if (_lockfd >= 0) {
			::close(_lockfd);
			_lockfd = -1;
			}
	}

	void
	delete_lock()
	{
		unlock();

		if (!_lock_file.empty())
			::unlink(_lock_file.c_str());
	}

	void
	write_pid()
	{
		std::ofstream os;

		if (!pid_file().length()) {
			if (program().length())
				pid_file(program() + ".pid");
			else
				throw std::runtime_error("Daemon:write_pid(): "
				    "no pid file name");
			}

		os.exceptions(std::ios_base::failbit | std::ios_base::badbit);
		os.open(pid_file().data());
		os << ::getpid();
		os.close();
	}

	void
	delete_pid()
	{
		if (!_pid_file.empty())
			::unlink(_pid_file.c_str());
	}

	void
	go_home()
	{
		if (home().length() && ::chdir(home().data()))
			throw std::system_error(errno, std::system_category());
	}

	void
	open_log(int option = LOG_PID, int facility = LOG_USER)
	{
		::closelog();

		if (jail().length())
			option = (option & LOG_ODELAY) | LOG_NDELAY;

		if (log_to_stderr())
			option |= LOG_PERROR;

		::openlog(program().data(), option, facility);
	}

	void
	incarcerate()
	{
		if (jail().length() && ::chroot(jail().data()))
			throw std::system_error(errno, std::system_category());
	}

	void
	change_identity()
	{
		if (user().length() || group().length()) {
			if (::setgid(gid()))
				throw std::system_error(errno,
				    std::system_category());

			::setgroups(1, &gid());

			if (user().length() && ::setuid(uid()))
				throw std::system_error(errno,
				    std::system_category());
			}
	}

	void
	block_signals()
	{
		::sigset_t all;

		::sigfillset(&all);
		::sigprocmask(SIG_BLOCK, &all, &_signals);
	}

	void
	unblock_signals() const
	{
		::sigprocmask(SIG_SETMASK, &_signals, NULL);
	}

	void
	ignore_signals()
	{
		for (int signo(1); signo < _NSIG; signo++)
			::signal(signo, SIG_IGN);
	}

private:
	std::string		_program;	// Program name (for log).
	bool			_log_to_stderr;	// Log to stderr also.
	std::string		_user;		// Running user.
	std::string		_group;		// Running group.
	::uid_t			_uid;		// Running user id.
	::gid_t			_gid;		// Running group id.
	std::string		_home;		// Current directory.
	std::string		_jail;		// Jail directory.
	std::string		_lock_file;	// Lock file path.
	std::string		_pid_file;	// Process id file path.
	::sigset_t		_signals;	// Saved signal mask.
	int			_lockfd;	// Lock file descriptor.
};

#endif
