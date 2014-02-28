//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Logger class.                          *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#ifndef __INCLUDE_LOGGER_H__
#define __INCLUDE_LOGGER_H__

#include <syslog.h>


class AlreadyLogged: public std::exception {

public:
	virtual const char *	what() const throw()
	{
		return "This exception has already been logged";
	}
};


class Logger {

public:
	Logger(Logger * parent = NULL): up(parent) {};
	Logger(const Logger& s): up(s.up) {};

	Logger& operator=(const Logger& s) { up = s.up; return *this; };

	void setParent(Logger * parent) { up = parent; };

	virtual void log(const std::string& msg, int level = LOG_ERR) = 0;

	void
	log_throw(const std::string& msg, int level = LOG_ERR)
	{
		log(msg, level);
		throw AlreadyLogged();
	}

protected:
	Logger *	up;
};

#endif
