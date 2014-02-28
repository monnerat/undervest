//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - URI parser class.                      *
//               *    See RFC 3986.                          *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#ifndef __INCLUDE_URI_H__
#define __INCLUDE_URI_H__

#include <string>

#include "filepath.h"

class URI {
	std::string	_scheme;
	std::string	_userinfo;
	std::string	_user;
	std::string	_password;
	std::string	_host;
	unsigned short	_port;
	FilePath	_path;
	std::string	_query;
	std::string	_fragment;

public:
	URI(): _port(0) {};
	URI(const std::string& uri): _port(0) { parse(uri); };

	std::string		unescape(const std::string& s) const;

	const std::string&	scheme() const { return _scheme; };

	bool
	scheme(const std::string& schm)
	{
		if (!valid_scheme(schm))
			return false;

		_scheme = schm;
		return true;
	}

	const std::string&	userinfo() const { return _userinfo; };

	bool
	userinfo(const std::string& info)
	{
		std::string::size_type i;

		if (!valid_userinfo(info))
			return false;

		_userinfo = info;
		_user = _password = "";
		i = info.find(":");

		if (i < info.length()) {
			_user = unescape(info.substr(0, i));
			_password = unescape(info.substr(i + 1));
			}

		return true;
	}

	const std::string&	user() { return _user; };
	const std::string&	password() { return _password; };

	const std::string&	host() const { return _host; };

	bool
	host(const std::string& hst)
	{
		if (!valid_host(hst))
			return false;

		_host = hst;
		return true;
	}

	unsigned short		port() const { return _port; };

	void
	port(unsigned short prt)
	{
		_port = prt;
	}

	const FilePath& path() const { return _path; }
	bool path(const FilePath& pth) { _path = pth; return true; };

	bool path(const std::string& pth)
	{
		if (!valid_path(pth))
			return false;

		_path = pth;
		return true;
	}

	const std::string& segment(FilePath::size_type n) const
	{
		return _path[n];
	}

	bool
	segment(FilePath::size_type n, const std::string& segmt)
	{
		if (!valid_segment(segmt))
			return false;

		_path[n] = segmt;
		return true;
	}

	const std::string&
	query() const
	{
		return _query;
	}

	bool
	query(const std::string& qry)
	{
		if (!valid_query(qry))
			return false;

		_query = qry;
		return true;
	}

	const std::string& fragment() const { return _fragment; }

	bool
	fragment(const std::string& frgmt)
	{
		if (!valid_fragment(frgmt))
			return false;

		_fragment = frgmt;
		return true;
	}

	void	clear();
	bool	parse(const std::string& uri);

	URI&	operator=(const std::string& uri) { parse(uri); return *this; };

	URI&
	operator=(const URI& uri)
	{
		_scheme = uri._scheme;
		_userinfo = uri._userinfo;
		_user = uri._user;
		_password = uri._password;
		_host = uri._host;
		_port = uri._port;
		_path = uri._path;
		_query = uri._query;
		_fragment = uri._fragment;
		return *this;
	}

	operator std::string() const;

private:
	bool	valid_scheme(const std::string& schm) const;
	bool	valid_userinfo(const std::string& info) const;
	bool	valid_host(const std::string& host) const;
	bool	valid_path(const std::string& pth) const;
	bool	valid_segment(const std::string& segmt) const;
	bool	valid_query(const std::string& qry) const;
	bool	valid_fragment(const std::string& frgmt) const;
};

#endif
