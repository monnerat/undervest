//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - String handling procedures.            *
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

#ifndef __INCLUDE_STRINGLIB_H__
#define __INCLUDE_STRINGLIB_H__

#include <cctype>
#include <string>
#include <vector>
#include <list>
#include <algorithm>


//	Trim from start.

template<typename _T>
static inline _T
ltrim(const _T& s, const _T& trimc = _T(" \t\r\n"))
{
	auto pos = s.find_first_not_of(trimc);

	return pos >= s.size()? "": s.substr(pos);
}


//	Trim from end.

template<typename _T>
static inline _T
rtrim(const _T& s, const _T& trimc = _T(" \t\r\n"))
{
	auto pos = s.find_last_not_of(trimc);

	return pos >= s.size()? "": s.substr(0, pos + 1);
}


//	Trim from both ends.

template<typename _T>
static inline _T
trim(const _T& s, const _T& trimc = _T(" \t\r\n"))
{
	return ltrim(rtrim(s, trimc), trimc);
}


//	PHP-like.

template<class _Str, class _It>
static inline _Str
implode(const _Str& separator, _It first, _It last)
{
	_Str s;

	if (first == last)
		return _Str("");

	for (s = *first; ++first != last;)
		s += separator + *first;

	return s;
}

template<class _CharT, class _It>
static inline std::basic_string<_CharT>
implode(const _CharT * separator, _It first, _It last)
{
	return implode(std::basic_string<_CharT>(separator), first, last);
}

template<class _Str, typename _CharT>
static inline _Str
implode(const _Str& separator, const _CharT * const * v,
					typename _Str::size_type n = _Str::npos)
{
	const _CharT * const * e;

	if (n != _Str::npos)
		e = v + n;
	else
		for (e = v; *e; e++)
			;

	return implode(separator, v, e);
}


template<typename _CharT>
static inline std::basic_string<_CharT>
implode(const _CharT * separator, const _CharT * const * v,
				size_t n = std::basic_string<_CharT>::npos)
{
	return implode(std::basic_string<_CharT>(separator), v, n);
}


template<class _Str>
static inline _Str
implode(const _Str& separator, const _Str * v, size_t n)
{
	return implode(separator, v, v + n);
}


template<class _Str>
static inline _Str
implode(const _Str& separator, const std::vector<_Str> v)
{
	return implode(separator, v.begin(), v.end());
}


template<class _Str>
static inline _Str
implode(const _Str& separator, const std::list<_Str> v)
{
	return implode(separator, v.begin(), v.end());
}


template<typename _T>
static inline std::vector<_T>
explode(const _T& separator, const _T& s, long n)
{
	std::vector<_T> v;
	typename _T::size_type b, e;

	v.reserve(s.size());

	if (!separator.size())
		for (auto i = s.begin(); i != s.end(); i++)
			v.push_back(_T(*i));
	else {
		b = 0;

		do {
			e = s.find(separator, b);
			v.push_back(s.substr(b, e - b));
			b = e + separator.size();
		} while (e < s.size());
		}

	if (n < 0) {
		n += v.size();

		if (n < 0)
			n = 0;
		}
	else {
		if (!n)
			n++;

		if (v.size() > n)
			v[n - 1] = implode(separator,
			    v.begin() + n - 1, v.end());
		}

	v.resize(n);
	v.shrink_to_fit();
	return v;
}


template<typename _T>
static inline std::vector<_T>
explode(const _T& separator, const _T& s)
{
	return explode(separator, s, s.size());
}


template<class _Str, class _Iterator>
static inline _Str
strtolower(_Iterator b, _Iterator e)
{
	_Str t(b, e);

	std::transform(t.begin(), t.end(), t.begin(),
	    [](typename _Str::value_type c)
	    {
		return c & ~0xFF? c: ::tolower(c);
	    });
	return t;
}


template<class _Str>
static inline _Str
strtolower(const _Str& s)
{
	return strtolower<_Str>(s.begin(), s.end());
}


template<class _Str, class _Iterator>
static inline _Str
strtoupper(_Iterator b, _Iterator e)
{
	_Str t(b, e);

	std::transform(t.begin(), t.end(), t.begin(),
	    [](typename _Str::value_type c)
	    {
		return c & ~0xFF? c: ::toupper(c);
	    });
	return t;
}


template<class _Str>
static inline _Str
strtoupper(const _Str& s)
{
	return strtoupper<_Str>(s.begin(), s.end());
}

#endif
