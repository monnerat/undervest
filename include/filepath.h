//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - File path class.                       *
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

#ifndef __INCLUDE_FILEPATH_H__
#define __INCLUDE_FILEPATH_H__

#include <unistd.h>

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>


class FilePath: public std::vector<std::string> {

public:
	typedef std::vector<std::string>	components_type;


	bool
	relative(bool rel)
	{
		bool old(_relative);
		_relative = rel;
		return old;
	}

	bool is_relative() const { return _relative; };


	void
	normalize()
	{
		for (auto p = begin(); p != end(); p++)
			if (*p == ".")
				erase(p--);
			else if (*p == "..") {
				if (p == begin()) {
					if (!is_relative())
						erase(p--);
					}
				else if (p[-1] != "..") {
					erase(p - 1, p + 1);
					p -= 2;
					}
				}
	}

	components_type::size_type
	parentCount() const
	{
		auto p(begin());

		for (; p < end(); p++)
			if (*p != "..")
				break;

		return p - begin();
	}

	components_type::size_type
	commonPrefixLength(const FilePath& path) const
	{
		auto p(begin());
		auto q(path.begin());
		size_type n = size() < path.size()? size(): path.size();

		if (is_relative() != path.is_relative())
			return 0;

		for (; n && *p == *q; n--) {
			p++;
			q++;
			}

		return p - begin();
	}

	components_type::size_type
	commonSuffixLength(const FilePath& path) const
	{
		auto p(rbegin());
		auto q(path.rbegin());
		size_type n = size() < path.size()? size(): path.size();

		for (; n && *p == *q; n--) {
			p++;
			q++;
			}

		return p - rbegin();
	}


	void
	clear()
	{
		relative(true);
		components_type::clear();
	}


	template<class InputIterator>
	void
	assign(bool rel, InputIterator first, InputIterator last)
	{
		relative(rel);
		components_type::assign(first, last);
	}

	void
	assign(const FilePath& path)
	{
		assign(path.is_relative(), path.begin(), path.end());
	}

	void
	assign(const std::string& path, const std::string& delims = "/")
	{
		std::string::size_type n, m;

		relative(!path.length() ||
		    delims.find(path[0]) == std::string::npos);
		n = path.find_first_not_of(delims);

		while (n < path.length()) {
			m = path.find_first_of(delims, n);

			if (m > path.length())
				m = path.length();

			push_back(path.substr(n, m - n));
			n = path.find_first_not_of(delims, m);
			}

		normalize();
	}

	void
	assign(const char * path,
			std::string::size_type len = std::string::npos,
			const std::string& delims = "/")
	{
		if (len == std::string::npos)
			assign(std::string(path), delims);
		else
			assign(std::string(path, len), delims);
	}

	template<class _Iterator>
	void
	assign(_Iterator b, _Iterator e, const std::string& delims = "/")
	{
		assign(std::string(b, e), delims);
	}

	FilePath() { clear(); };
	FilePath(const FilePath& path) { assign(path); };

	FilePath(const std::string& path, const std::string& delims = "/")
	{
		assign(path, delims);
	}

	FilePath(const char * path,
			std::string::size_type len = std::string::npos,
			const std::string& delims = "/")
	{
		assign(path, len, delims);
	}

	template<class InputIterator>
	FilePath(bool rel, InputIterator first, InputIterator last)
	{
		assign(rel, first, last);
	}


	FilePath&
	operator=(const FilePath& y)
	{
		assign(y);
		return *this;
	}

	FilePath&
	operator=(const std::string& y)
	{
		assign(y);
		return *this;
	}

	FilePath&
	operator=(const char * y)
	{
		assign(y);
		return *this;
	}


	FilePath&
	operator+=(const FilePath& y)
	{
		if (!y.is_relative())
			throw std::runtime_error("FilePath::+: "
			    "2nd operand not relative");

		for (auto p = y.begin(); p != y.end(); p++)
			push_back(*p);

		normalize();
		return *this;
	}

	FilePath&
	operator+=(const std::string& y)
	{
		return *this += FilePath(y);
	}

	FilePath&
	operator+=(const char * y)
	{
		return *this += FilePath(y);
	}


	FilePath
	operator+(const FilePath& y) const
	{
		FilePath t(*this);

		return t += y;
	}

	FilePath
	operator+(const std::string& y) const
	{
		FilePath t(*this);

		return t += y;
	}

	FilePath
	operator+(const char * y) const
	{
		FilePath t(*this);

		return t += y;
	}


	FilePath
	resolveRelative(const FilePath& base) const
	{
		FilePath path;

		if (!is_relative())
			return *this;

		path.assign(base);
		return path += *this;
	}

	FilePath
	resolveRelative(const std::string& base,
					const std::string& delims = "/") const
	{
		return resolveRelative(FilePath(base, delims));
	}

	FilePath
	resolveRelative(const char * base,
		std::string::size_type len = std::string::npos,
		const std::string& delims = "/") const
	{
		return resolveRelative(FilePath(base, len, delims));
	}


	static FilePath
	cwd(const std::string& delims = "/")
	{
		char buf[2048];

		::getcwd(buf, sizeof buf);
		return FilePath(buf, std::string::npos, delims);
	}


	FilePath
	absolute(const std::string& delims = "/") const
	{
		FilePath base;

		if (!is_relative())
			return *this;

		return resolveRelative(cwd(delims));
	}


	FilePath
	relativeFrom(const FilePath& base,
					const std::string& delims = "/") const
	{
		FilePath path;
		components_type& r(path);
		components_type::size_type n(parentCount());
		components_type::size_type m(base.parentCount());

		if (is_relative() != base.is_relative() || m > n)
			throw std::runtime_error("FilePath::relativeFrom: "
			    "paths are not related");

		n = commonPrefixLength(base);
		r.assign(base.size() - n, "..");
		path += FilePath(true, begin() + n, end());
		return path;
	}

	FilePath
	relativeFrom(const std::string& base,
					const std::string& delims = "/") const
	{
		return relativeFrom(FilePath(base, delims));
	}

	FilePath
	relativeFrom(const char * base,
		std::string::size_type len = std::string::npos,
		const std::string& delims = "/") const
	{
		return relativeFrom(FilePath(base, len, delims));
	}


	FilePath&
	operator-=(const FilePath& y)
	{
		return *this = relativeFrom(y);
	}

	FilePath&
	operator-=(const std::string& y)
	{
		return *this -= FilePath(y);
	}

	FilePath&
	operator-=(const char * y)
	{
		return *this -= FilePath(y);
	}


	FilePath
	operator-(const FilePath& y) const
	{
		return relativeFrom(y);
	}

	FilePath
	operator-(const std::string& y) const
	{
		return relativeFrom(y);
	}

	FilePath
	operator-(const char * y) const
	{
		return relativeFrom(y);
	}


	FilePath&
	operator/=(const FilePath& y)
	{
		components_type::size_type n;

		//	Prefix of this being the base of y.

		if (!y.is_relative()) {
			if (is_relative())
				throw std::runtime_error("FilePath::/: "
				   "unrelated paths");

			components_type::clear();
			return *this;
			}

		n = commonSuffixLength(y);

		if (n != y.size())
			throw std::runtime_error("FilePath::/: "
			   "unrelated paths");

		resize(size() - n);
	}

	FilePath&
	operator/=(const std::string& y)
	{
		return *this /= FilePath(y);
	}

	FilePath&
	operator/=(const char * y)
	{
		return *this /= FilePath(y);
	}


	FilePath
	baseOf(const FilePath& path) const
	{
		FilePath t(*this);

		return t /= path;
	}

	FilePath
	baseOf(const std::string& path, const std::string& delims = "/") const
	{
		return baseOf(FilePath(path, delims));
	}

	FilePath
	baseOf(const char * path,
		std::string::size_type len = std::string::npos,
		const std::string& delims = "/") const
	{
		return baseOf(FilePath(path, len, delims));
	}


	FilePath
	operator/(const FilePath& y) const
	{
		return baseOf(y);
	}

	FilePath
	operator/(const std::string& y) const
	{
		return baseOf(y);
	}

	FilePath
	operator/(const char * y) const
	{
		return baseOf(y);
	}


	FilePath
	commonPrefix(const FilePath& path) const
	{
		FilePath p;

		if (is_relative() != path.is_relative())
			return p;

		p = *this;
		p.resize(commonPrefixLength(path));
		return p;
	}

	FilePath
	commonPrefix(const std::string& suffix,
					const std::string& delims = "/") const
	{
		return commonPrefix(FilePath(suffix, delims));
	}

	FilePath
	commonPrefix(const char * suffix,
		std::string::size_type len = std::string::npos,
		const std::string& delims = "/") const
	{
		return commonPrefix(FilePath(suffix, len, delims));
	}


	FilePath
	commonSuffix(const FilePath& path) const
	{
		FilePath p;
		auto n(commonSuffixLength(path));

		p.assign(n != size() || n != path.size() ||
		    (is_relative() && path.is_relative()), end() - n, end());
		return p;
	}

	FilePath
	commonSuffix(const std::string& suffix,
					const std::string& delims = "/") const
	{
		return commonSuffix(FilePath(suffix, delims));
	}

	FilePath
	commonSuffix(const char * suffix,
		std::string::size_type len = std::string::npos,
		const std::string& delims = "/") const
	{
		return commonSuffix(FilePath(suffix, len, delims));
	}


	FilePath&
	operator<<=(long long n)
	{
		if (n) {
			if (n > 0) {
				if (size() <= (unsigned long long) n)
					components_type::clear();
				else {
					erase(begin(), begin() + n);
					relative(true);
					}
				}
			else if (size() <= (unsigned long long) -n)
				components_type::clear();
			else
				resize(size() + n);
			}

		return *this;
	}

	FilePath&
	operator>>=(long long n)
	{
		return *this <<= -n;
	}


	FilePath
	operator<<(long long n) const
	{
		FilePath t(*this);

		return t <<= n;
	}

	FilePath
	operator>>(long long n) const
	{
		FilePath t(*this);

		return t >>= n;
	}


	FilePath
	dirname() const
	{
		FilePath t(*this);

		t.push_back("..");
		t.normalize();
		return t;
	}

	std::string
	filename(char delim = '/') const
	{
		if (!size())
			return std::string(is_relative()? '.': delim, 1);

		if (back() == "..")
			return "";

		return back();
	}


	std::string
	get(bool as_dir = false, char delim = '/') const
	{
		std::string path;
		std::string d;

		if (as_dir)
			d = delim;

		for (auto i(rbegin()); i != rend(); d = delim)
			path = *i++ + d + path;

		if (!is_relative())
			path = delim + path;
		else if (!path.length())
			path = "." + d;

		return path;
	}

	operator std::string() const { return get(); };


private:
	bool				_relative;
};



inline FilePath
operator+(const std::string& x, const FilePath& y)
{
	FilePath t(x);

	return t += y;
}

inline FilePath
operator+(const char * x, const FilePath& y)
{
	FilePath t(x);

	return t += y;
}

inline FilePath
operator-(const std::string& x, const FilePath& y)
{
	FilePath t(x);

	return t -= y;
}

inline FilePath
operator-(const char * x, const FilePath& y)
{
	FilePath t(x);

	return t -= y;
}

inline FilePath
operator/(const std::string& x, const FilePath& y)
{
	FilePath t(x);

	return t /= y;
}

inline FilePath
operator/(const char * x, const FilePath& y)
{
	FilePath t(x);

	return t /= y;
}


template<typename CharT, class Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const FilePath& y)
{
	return os << y.get();
}


inline std::string
to_string(const FilePath& path, char delim = '/')
{
	return path.get(delim);
}

#endif
