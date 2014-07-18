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
//               *                                           *
//               * Copyright (c) 2014 Datasphere S.A.        *
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
