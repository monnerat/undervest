//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - URI parser methods.                    *
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

#include <bitset>

#include "uri.h"


typedef std::bitset<1 << (sizeof(char) << 3)>	charSet;


inline charSet
make_charSet(const std::string& s)
{
	charSet t;

	for (auto p(s.begin()); p != s.end(); p++)
		t.set((unsigned char) *p);

	return t;
}


static const charSet	upalpha(make_charSet("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
static const charSet	alpha(upalpha |
			    make_charSet("abcdefghijklmnopqrstuvwxyz"));
static const charSet	digit(make_charSet("0123456789"));
static const charSet	alphanum(alpha | digit);
static const charSet	hex(digit | make_charSet("ABCDEFabcdef"));
static const charSet	unreserved(alpha | digit | make_charSet("-._~"));
static const charSet	sub_delims(make_charSet("!$&'()*+,;="));
static const charSet	pchar(unreserved | sub_delims | make_charSet("%:@"));
static const charSet	pcharSlash(pchar | make_charSet("/"));
static const charSet	queryChar(pchar | make_charSet("/?"));
static const charSet	schemeChar(alpha | digit | make_charSet("+-."));
static const charSet	userinfoChar(unreserved | sub_delims |
							make_charSet("%:"));
static const charSet	regnameChar(unreserved | sub_delims |
							make_charSet("%"));
static const charSet	futureChar(unreserved | sub_delims | make_charSet(":"));


static inline bool
is_in_set(const charSet& cs, char c)

{
	return cs.test((size_t) c);
}


static inline bool
is_escaped(std::string::const_iterator p)

{
	return p[0] == '%' && is_in_set(hex, p[1]) && is_in_set(hex, p[2]);
}


static inline std::string::const_iterator
skip_charSet(std::string::const_iterator p, const charSet& cs)

{
	while (is_in_set(cs, *p))
		if (*p != '%')
			p++;
		else if (is_escaped(p))
			p += 3;
		else
			break;

	return p;
}


static inline bool
check_charSet(const std::string& s, const charSet& cs)

{
	return skip_charSet(s.begin(), cs) == s.end();
}


static inline int
hexdigit(char c)

{
	if (is_in_set(digit, c))
		return c - '0';

	if (is_in_set(upalpha, c))
		return c - 'A' + 10;

	return c - 'a' + 10;
}


static bool
valid_ipv4(const std::string& s)

{
	unsigned short v(0);
	unsigned short n(0);
	bool first(true);

	for (auto p(s.begin()); p != s.end(); p++)
		if (is_in_set(digit, *p)) {
			v = (v * 10) + *p - '0';

			if (v >= 256 || (first & !v))
				return false;

			first = false;
			}
		else if (*p == '.') {
			if (first || ++n >= 4)
				return false;

			v = 0;
			first = true;
			}
		else
			return false;

	return !first && ++n == 4;
}


static bool
valid_registered_name(const std::string& s)

{
	return check_charSet(s, regnameChar);
}


static bool
valid_ipv6(const std::string& s)

{
	std::string::const_iterator p(s.begin());
	std::string::const_iterator e(s.end());
	std::string::const_iterator q;
	unsigned short v(0);
	unsigned short n(0);
	bool skip(s.length() >= 2 && p[0] == ':' && p[1] == ':');

	if (skip)
		p += 2;

	if (e - p >= 2 && e[-1] == ':' && e[-2] == ':') {
		if (skip)
			return false;

		skip = true;
		e -= 2;
		}

	if (skip)
		n++;

	for (q = p; p != e; p++)
		if (is_in_set(hex, *p)) {
			if (v & 0xF000)
				return false;

			v = (v << 4) | hexdigit(*p);
			}
		else if (*p == ':') {
			if (p > q) {
				if (++n >= 8)
					return false;
				}
			else if (skip)
				return false;
			else
				skip = true;

			v = 0;
			q = p + 1;
			}
		else if (*p != '.')
			return false;
		else if (!valid_ipv4(s.substr(q - s.begin())))
			return false;
		else {
			n++;
			break;
			}

	if (++n > 8)
		return false;

	if (!skip && n != 8)
		return false;

	return p != q;
}


static bool
valid_future(const std::string& s)

{
	std::string::const_iterator p(s.begin());
	std::string::const_iterator e(s.end());

	if (p[0] != 'v' || !is_in_set(hex, p[1]))
		return false;

	p = skip_charSet(p + 2, hex);

	if (*p != '.')
		return false;

	return ++p != s.end() && skip_charSet(p, futureChar) == s.end();
}


std::string
URI::unescape(const std::string& s) const

{
	std::string res;

	for (auto p(s.begin()); p != s.end();)
		if (!is_escaped(p))
			res += *p++;
		else {
			res += (char) ((hexdigit(p[1]) << 4) | hexdigit(p[2]));
			p += 3;
			}

	return res;
}


bool
URI::valid_scheme(const std::string& schm) const

{
	return schm.empty() ||
	    (is_in_set(alpha, schm[0]) && check_charSet(schm, schemeChar));
}


bool
URI::valid_userinfo(const std::string& info) const

{
	return check_charSet(info, userinfoChar);
}


bool
URI::valid_host(const std::string& hst) const

{
	return !hst.length() || valid_ipv4(hst) || valid_ipv6(hst) ||
	    valid_future(hst) || valid_registered_name(hst);
}


bool
URI::valid_path(const std::string& pth) const

{
	return check_charSet(pth, pcharSlash);
}


bool
URI::valid_segment(const std::string& segmt) const

{
	return check_charSet(segmt, pchar);
}


bool
URI::valid_query(const std::string& qry) const

{
	return check_charSet(qry, queryChar);
}


bool
URI::valid_fragment(const std::string& frgmt) const

{
	return check_charSet(frgmt, queryChar);
}


void
URI::clear()
{
	_scheme = _userinfo = "";
	_user = _password = _host = _query = _fragment = "";
	_port = 0;
	_path.clear();
}


bool
URI::parse(const std::string& uri)
{
	std::string::const_iterator p(uri.begin());
	std::string::const_iterator q;
	std::string::const_iterator r;
	std::string::size_type i;
	unsigned int prt;

	clear();

	//	Parse scheme.

	q = skip_charSet(p, schemeChar);

	if (is_in_set(alpha, *p) && q[0] == ':') {
		_scheme.assign(p, q);
		p = q + 1;

		if (p[0] == '/' && p[1] == '/') {
			//	Check for userinfo.

			q = skip_charSet(p + 2, userinfoChar);

			if (*q == '@') {
				_userinfo.assign(p + 2, q);
				p = q + 1;
				}

			//	Parse host.

			if (*p == '[') {
				//	Should be an IPv6 or literal address.

				i = uri.find(']', ++p - uri.begin());

				if (i >= uri.length()) {
					_scheme = _userinfo = "";
					return false;	// Invalid host syntax.
					}

				_host.assign(p, q);
				q++;
				}
			else {
				q = skip_charSet(p, regnameChar);
				_host.assign(p, q);
				}

			if (!valid_host(_host)) {
				_scheme = _userinfo = _host = "";
				return false;
				}

			p = q;

			//	Check for port.

			if (*p == ':') {
				p++;

				for (prt = 0; is_in_set(digit, *p); p++)
					if (!(prt & ~0xFFFF))
						prt = (prt * 10) + *q - '0';

				if (!(prt & ~0xFFFF))
					_port = prt;
				else {
					_scheme = _userinfo = _host = "";
					return false;
					}
				}

			//	Split userinfo into user and password for
			//		easy access.

			i = _userinfo.find(':');

			if (i < _userinfo.length()) {
				_user = unescape(_userinfo.substr(0, i));
				_password = unescape(_userinfo.substr(i + 1));
				}
			}
		}

	//	Parse path.

	q = skip_charSet(p, pcharSlash);

	if (q != p) {
		_path.assign(p, q);
		p = q;
		}

	//	Parse query.

	if (*p == '?') {
		q = skip_charSet(++p, queryChar);
		_query.assign(p, q);
		p = q;
		}

	//	Parse fragment.

	if (*p == '#') {
		q = skip_charSet(++p, queryChar);
		_fragment.assign(p, q);
		p = q;
		}

	return p == uri.end();
}


URI::operator std::string() const

{
	std::string uri;

	if (!_scheme.empty())
		uri = _scheme + ':';

	if (!_userinfo.empty() || !_host.empty() || _port ||
	    !_path.is_relative()) {
		uri += "//";

		if (!_userinfo.empty())
			uri += _userinfo + "@";

		if (!_host.empty()) {
			if (valid_ipv6(_host) || valid_future(_host))
				uri += "[" + _host + "]";
			else
				uri += _host;
			}

		if (_port)
			uri += ":" + std::to_string(_port);
		}

	if (!_path.empty() || !_path.is_relative())
		uri += _path.get();

	if (!_query.empty())
		uri += "?" + _query;

	if (!_fragment.empty())
		uri += "#" + _fragment;

	return uri;
}
