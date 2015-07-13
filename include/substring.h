//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Static (i.e.: no allocator)            *
//               *    (sub-)strings.                         *
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

#ifndef __INCLUDE_SUBSTRING_H__
#define __INCLUDE_SUBSTRING_H__

#include <string>
#include <sstream>
#include <stdexcept>
#include <functional>

#include "crc32.h"


template<typename _CharT, typename _Traits = std::char_traits<_CharT> >
class basic_substring {

//	Types:

public:
	typedef _Traits					traits_type;
	typedef typename _Traits::char_type		value_type;
	typedef size_t					size_type;
	typedef std::ptrdiff_t				difference_type;
	typedef _CharT&					reference;
	typedef const _CharT&				const_reference;
	typedef _CharT *				pointer;
	typedef const _CharT *				const_pointer;
	typedef __gnu_cxx::__normal_iterator<pointer, basic_substring>
							iterator;
	typedef __gnu_cxx::__normal_iterator<const_pointer, basic_substring>
							const_iterator;
	typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;
	typedef std::reverse_iterator<iterator>		reverse_iterator;


	static const size_type	npos = static_cast<size_type>(-1);


	basic_substring&
	assign(const value_type * s, size_type len = npos)
	{
		return __assign_helper(s, 0, len, len);
	}

	basic_substring&
	assign(const value_type * s, size_type pos, size_type len)
	{
		return __assign_helper(s, pos, len,
		    len == npos? npos: pos + len);
	}

	basic_substring&
	assign(const basic_substring& s,
				size_type pos = 0, size_type len = npos)
	{
		return __assign_helper(s.data(), pos, len, s.length());
	}

	template<typename _Alloc>
	basic_substring&
	assign(const std::basic_string<value_type, traits_type, _Alloc>& s,
					size_type pos = 0, size_type len = npos)
	{
		return __assign_helper(s.data(), pos, len, s.length());
	}


	basic_substring(): _M_data(0), _M_length(0) {};

	basic_substring(const value_type * s, size_type len = npos)
	{
		__assign_helper(s, 0, len, len);
	}

	basic_substring(const value_type * s, size_type pos, size_type len)
	{
		__assign_helper(s, pos, len, len == npos? npos: pos + len);
	}

	basic_substring(const basic_substring& s,
				size_type pos = 0, size_type len = npos)
	{
		__assign_helper(s.data(), pos, len, s.length());
	}

	template<typename _Alloc>
	basic_substring(const std::basic_string<value_type, traits_type,
			_Alloc>& s, size_type pos = 0, size_type len = npos)
	{
		__assign_helper(s.data(), pos, len, s.length());
	}


	basic_substring& operator=(const basic_substring& s)
	{
		return assign(s);
	}

	template<typename _Alloc>
	basic_substring& operator=(const std::basic_string<value_type,
							traits_type, _Alloc>& s)
	{
		return assign(s);
	}

	basic_substring& operator=(const value_type * s)
	{
		return assign(s);
	}

	template<typename _Alloc>
	operator std::basic_string<value_type, traits_type, _Alloc>() const
	{
		return std::basic_string<value_type,
		    traits_type, _Alloc>(data(), length());
	}

	int
	compare(const basic_substring& str) const
	{
		const size_type size = length();
		const size_type osize = str.length();
		const size_type len = std::min(size, osize);
		int r = traits_type::compare(data(), str.data(), len);

		if (!r)
			r = __compare_size(size, osize);

		return r;
	}

	template<typename _Alloc>
	int
	compare(const std::basic_string<value_type, traits_type, _Alloc>& str)
									const
	{
		return compare(static_cast<basic_substring>(str));
	}

	int
	compare(size_type pos, size_type len, const basic_substring& str) const
	{
		basic_substring t(*this, pos, len);
		return t.compare(str);
	}

	template<typename _Alloc>
	int
	compare(size_type pos, size_type len,
		const std::basic_string<value_type, traits_type, _Alloc>& str)
									const
	{
		basic_substring t(*this, pos, len);
		return t.compare(str);
	}

	int
	compare(size_type pos1, size_type len1, const basic_substring& str,
				size_type pos2, size_type len2) const
	{
		basic_substring t1(*this, pos1, len1);
		basic_substring t2(str, pos2, len2);
		return t1.compare(t2);
	}

	template<typename _Alloc>
	int
	compare(size_type pos1, size_type len1,
		const std::basic_string<value_type, traits_type, _Alloc>& str,
		size_type pos2, size_type len2) const
	{
		basic_substring t1(*this, pos1, len1);
		basic_substring t2(str, pos2, len2);
		return t1.compare(t2);
	}

	int
	compare(const_pointer s) const
	{
		basic_substring t(s);
		return compare(t);
	}

	int
	compare(size_type pos, size_type len, const_pointer s) const
	{
		basic_substring t1(*this, pos, len);
		return t1.compare(s);
	}

	int
	compare(size_type pos, size_type len1,
				const_pointer s, size_type len2) const
	{
		basic_substring t1(*this, pos, len1);
		basic_substring t2(s, 0, -1, len2);
		return t1.compare(t2);
	}

	size_type length() const { return _M_length; };
	size_type size() const { return length(); };
	void clear() { _M_data = 0; _M_length = 0; };
	bool empty() const { return length() == 0; };

	const_reference
	operator[](size_type pos) const
	{
		static const value_type znull(0);

		return pos >= length()? znull: data()[pos];
	}

	const_reference
	at(size_type pos) const
	{
		if (pos >= length())
			throw std::out_of_range("basic_substring::at");

		return data()[pos];
	}

	const_reference front() const { return *begin(); }
	const_reference back() const { return *(end() - 1); }

	void
	swap(basic_substring& s)
	{
		basic_substring t(*this);

		*this = s;
		s = t;
	}

	void
	pop_back()
	{
		if (length())
			if (!--_M_length)
				clear();
	}


	const_pointer c_str() const { return _M_data; }
	const_pointer data() const { return _M_data; }


	size_type
	find(const basic_substring& s) const
	{
		const_pointer p;
		const_pointer e;

		if (!s.length() || s.length() > length())
			return npos;

		p = data();

		for (e = p + length() - s.length() + 1;
		    (p = traits_type::find(p, e - p, *s.data())); p++)
			if (!traits_type::compare(p, s.data(), s.length()))
				return p - data();

		return npos;
	}

	size_type
	find(const basic_substring& s, size_type pos) const
	{
		basic_substring x(*this, pos);
		size_type p = x.find(s);

		return p == npos? p: p + pos;
	}

	template<typename _Alloc>
	size_type
	find(const std::basic_string<value_type, traits_type, _Alloc>& s,
							size_type pos = 0) const
	{
		basic_substring x(*this, pos);
		basic_substring y(s);
		size_type p = x.find(y);

		return p == npos? p: p + pos;
	}

	size_type
	find(const_pointer s, size_type pos = 0, size_type len = npos)
	{
		basic_substring x(*this, pos);
		basic_substring y(s, len);
		size_type p = x.find(y);

		return p == npos? p: p + pos;
	}

	size_type
	find(value_type c, size_type pos = 0) const
	{
		basic_substring x(*this, pos);
		const_pointer p = traits_type::find(x.data(), x.length(), c);

		return p? p - x.data(): npos;
	}


	size_type
	rfind(const basic_substring& s) const
	{
		const_pointer p;
		const_pointer e;

		if (!s.length() || s.length() > length())
			return npos;

		p = data() + length() - s.length();

		do {
			if (!traits_type::compare(p, s.data(), s.length()))
				return p - data();
		} while (p-- != data());

		return npos;
	}

	size_type
	rfind(const basic_substring& s, size_type pos) const
	{
		basic_substring x(*this, pos);
		size_type p = x.rfind(s);

		return p == npos? p: p + pos;
	}

	template<typename _Alloc>
	size_type
	rfind(const std::basic_string<value_type, traits_type, _Alloc>& s,
							size_type pos = 0) const
	{
		basic_substring x(*this, pos);
		basic_substring y(s);
		size_type p = x.rfind(y);

		return p == npos? p: p + pos;
	}

	size_type
	rfind(const_pointer s, size_type pos = 0, size_type len = npos)
	{
		basic_substring x(*this, pos);
		basic_substring y(s, len);
		size_type p = x.rfind(y);

		return p == npos? p: p + pos;
	}

	size_type
	rfind(value_type c, size_type pos = 0) const
	{
		basic_substring x(*this, pos);
		basic_substring y(&c, 1);
		size_type p = x.rfind(y);

		return p == npos? p: p + pos;
	}


	size_type
	find_first_of(const basic_substring& s, size_type pos = 0) const
	{
		while (pos < length())
			if (s.find(at(pos)) != npos)
				return pos;

		return npos;
	}

	template<typename _Alloc>
	size_type
	find_first_of(const std::basic_string<value_type,
			traits_type, _Alloc>& s, size_type pos = 0) const
	{
		return find_first_of(basic_substring(s), pos);
	}

	size_type
	find_first_of(const_pointer s,
				size_type pos = 0, size_type len = npos)
	{
		return find_first_of(basic_substring(s, len), pos);
	}

	size_type
	find_first_of(value_type c, size_type pos = 0)
	{
		return find(c, pos);
	}


	size_type
	find_first_not_of(const basic_substring& s, size_type pos = 0) const
	{
		while (pos < length())
			if (s.find(at(pos)) == npos)
				return pos;

		return npos;
	}

	template<typename _Alloc>
	size_type
	find_first_not_of(const std::basic_string<value_type,
			traits_type, _Alloc>& s, size_type pos = 0) const
	{
		return find_first_not_of(basic_substring(s), pos);
	}

	size_type
	find_first_not_of(const_pointer s,
				size_type pos = 0, size_type len = npos)
	{
		return find_first_not_of(basic_substring(s, len), pos);
	}

	size_type
	find_first_not_of(value_type c, size_type pos = 0)
	{
		return find_first_not_of(basic_substring(&c, 1), pos);
	}


	size_type
	find_last_of(const basic_substring& s, size_type pos = 0) const
	{
		size_type p = pos > length()? length(): pos;

		while (p > pos)
			if (s.find(at(--p)) != npos)
				return p;

		return npos;
	}

	template<typename _Alloc>
	size_type
	find_last_of(const std::basic_string<value_type,
			traits_type, _Alloc>& s, size_type pos = 0) const
	{
		return find_last_of(basic_substring(s), pos);
	}

	size_type
	find_last_of(const_pointer s,
				size_type pos = 0, size_type len = npos)
	{
		return find_last_of(basic_substring(s, len), pos);
	}

	size_type
	find_last_of(value_type c, size_type pos = 0)
	{
		return rfind(c, pos);
	}


	size_type
	find_last_not_of(const basic_substring& s, size_type pos = 0) const
	{
		size_type p = pos > length()? length(): pos;

		while (p > pos)
			if (s.find(at(--p)) == npos)
				return p;

		return npos;
	}

	template<typename _Alloc>
	size_type
	find_last_not_of(const std::basic_string<value_type,
			traits_type, _Alloc>& s, size_type pos = 0) const
	{
		return find_last_not_of(basic_substring(s), pos);
	}

	size_type
	find_last_not_of(const_pointer s,
				size_type pos = 0, size_type len = npos)
	{
		return find_last_not_of(basic_substring(s, len), pos);
	}

	size_type
	find_last_not_of(value_type c, size_type pos = 0)
	{
		return find_last_not_of(basic_substring(&c, 1), pos);
	}


	basic_substring
	substr(size_type pos = 0, size_type len = npos) const
	{
		return basic_substring(*this, pos, len);
	}


	iterator begin() { return iterator(data()); };
	iterator end() { return iterator(data() + length()); };
	const_iterator begin() const { return const_iterator(data()); };

	const_iterator
	end() const
	{
		return const_iterator(data() + length());
	}

	reverse_iterator rbegin() { return reverse_iterator(end()); };
	reverse_iterator rend() { return reverse_iterator(begin()); };

	const_reverse_iterator
	rbegin() const
	{
		return const_reverse_iterator(end());
	}

	const_reverse_iterator
	rend() const
	{
		return const_reverse_iterator(begin());
	}

	const_iterator cbegin() const { return const_iterator(data()); };

	const_iterator
	cend() const
	{
		return const_iterator(data() + length());
	}

	const_reverse_iterator
	crbegin()
	{
		return const_reverse_iterator(end());
	}

	const_reverse_iterator
	crend()
	{
		return const_reverse_iterator(begin());
	}


	template<typename _Alloc>
	std::basic_string<value_type, traits_type, _Alloc>
	operator+(const std::basic_string<value_type, traits_type, _Alloc>& y)
									const
	{
		return std::basic_string<value_type,
		    traits_type, _Alloc>(*this) + y;
	}

	template<typename _Alloc>
	std::basic_string<value_type, traits_type, _Alloc>
	operator+(const basic_substring& y) const
	{
		return *this + std::basic_string<value_type,
		    traits_type, _Alloc>(y);
	}

	template<typename _Alloc>
	std::basic_string<value_type, traits_type, _Alloc>
	operator+(const_pointer y) const
	{
		return *this +
		    std::basic_string<value_type, traits_type, _Alloc>(y);
	}


	bool operator<(const basic_substring& y) const { return compare(y) < 0; }
	bool operator<(const_pointer y) const { return compare(y) < 0; }

	template<typename _Alloc>
	bool
	operator<(const std::basic_string<value_type, traits_type, _Alloc>& y)
									const

	{
		return compare(y) < 0;
	}

	bool operator<=(const basic_substring& y) const
	{
		return compare(y) <= 0;
	}

	bool operator<=(const_pointer y) const { return compare(y) <= 0; }

	template<typename _Alloc>
	bool
	operator<=(const std::basic_string<value_type, traits_type, _Alloc>& y)
									const
	{
		return compare(y) <= 0;
	}

	bool operator==(const basic_substring& y) const
	{
		return compare(y) == 0;
	}

	bool operator==(const_pointer y) const { return compare(y) == 0; }

	template<typename _Alloc>
	bool
	operator==(const std::basic_string<value_type, traits_type, _Alloc>& y)
									const
	{
		return compare(y) == 0;
	}

	bool operator!=(const basic_substring& y) const
	{
		return compare(y) != 0;
	}

	bool operator!=(const_pointer y) const { return compare(y) != 0; }

	template<typename _Alloc>
	bool
	operator!=(const std::basic_string<value_type, traits_type, _Alloc>& y)
									const
	{
		return compare(y) != 0;
	}

	bool operator>=(const basic_substring& y) const
	{
		return compare(y) >= 0;
	}

	bool operator>=(const_pointer y) const { return compare(y) >= 0; }

	template<typename _Alloc>
	bool
	operator>=(const std::basic_string<value_type, traits_type, _Alloc>& y)
									const
	{
		return compare(y) >= 0;
	}

	bool operator>(const basic_substring& y) const
	{
		return compare(y) > 0;
	}

	bool operator>(const_pointer y) const { return compare(y) > 0; }

	template<typename _Alloc>
	bool
	operator>(const std::basic_string<value_type, traits_type, _Alloc>& y)
									const
	{
		return compare(y) > 0;
	}



private:
	basic_substring&
	__assign_helper(const value_type * s,
				size_type pos, size_type len, size_type ssize)
	{
		if (!s) {
			_M_data = 0;
			_M_length = 0;
			}
		else {
			if (ssize == npos)
				ssize = traits_type::length(s);

			if (pos > ssize)
				pos = ssize;

			if (len == npos || len > ssize - pos);
				len = ssize - pos;

			_M_length = len;
			_M_data = len? s + pos: 0;
			}

		return *this;
	}

	static int
	__compare_size(size_type n1, size_type n2)
	{
		const difference_type d = difference_type(n1 - n2);

		if (d > __gnu_cxx::__numeric_traits<int>::__max)
			return __gnu_cxx::__numeric_traits<int>::__max;
		else if (d < __gnu_cxx::__numeric_traits<int>::__min)
			return __gnu_cxx::__numeric_traits<int>::__min;
		else
			return int(d);
	}


private:
	const_pointer	_M_data;
	size_type	_M_length;
};


template<typename _CharT, typename _Traits, typename _Alloc>
inline std::basic_string<_CharT, _Traits, _Alloc>
operator+(const std::basic_string<_CharT, _Traits, _Alloc>& x,
				const basic_substring<_CharT, _Traits>& y)
{
	std::basic_string<_CharT, _Traits, _Alloc> t(y.data(), y.length());

	return x + t;
}


template<typename _CharT, typename _Traits, typename _Alloc>
inline std::basic_string<_CharT, _Traits, _Alloc>
operator+(const _CharT * x, const basic_substring<_CharT, _Traits>& y)
{
	std::basic_string<_CharT, _Traits, _Alloc> t(y.data(), y.length());

	return x + t;
}


#define __SUBSTRING_REL_OP_HELPER(tmplate, type, op, rop)		\
	template<tmplate>						\
	inline bool							\
	operator op(const type x, const basic_substring<_CharT, _Traits>& y) \
	{								\
		return y.compare(x) > 0;				\
	}
#define __SUBSTRING_SINGLE_ARG(...)	__VA_ARGS__
#define __SUBSTRING_REL_OP(tmplate, type)				\
	__SUBSTRING_REL_OP_HELPER(__SUBSTRING_SINGLE_ARG(tmplate),	\
	    __SUBSTRING_SINGLE_ARG(type), <, >)				\
	__SUBSTRING_REL_OP_HELPER(__SUBSTRING_SINGLE_ARG(tmplate),	\
	    __SUBSTRING_SINGLE_ARG(type), <=, >=)			\
	__SUBSTRING_REL_OP_HELPER(__SUBSTRING_SINGLE_ARG(tmplate),	\
	    __SUBSTRING_SINGLE_ARG(type), ==, ==)			\
	__SUBSTRING_REL_OP_HELPER(__SUBSTRING_SINGLE_ARG(tmplate),	\
	    __SUBSTRING_SINGLE_ARG(type), !=, !=)			\
	__SUBSTRING_REL_OP_HELPER(__SUBSTRING_SINGLE_ARG(tmplate),	\
	    __SUBSTRING_SINGLE_ARG(type), >=, <=)			\
	__SUBSTRING_REL_OP_HELPER(__SUBSTRING_SINGLE_ARG(tmplate),	\
	    __SUBSTRING_SINGLE_ARG(type), >, <)

__SUBSTRING_REL_OP(__SUBSTRING_SINGLE_ARG(typename _CharT, typename _Traits),
    _CharT *)

__SUBSTRING_REL_OP(
    __SUBSTRING_SINGLE_ARG(typename _CharT, typename _Traits, typename _Alloc),
    __SUBSTRING_SINGLE_ARG(std::basic_string<_CharT, _Traits, _Alloc>))



template<typename CharT, class Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os,
				const basic_substring<CharT, Traits>& y)
{
	std::basic_ostringstream<CharT, Traits> s;
	typename basic_substring<CharT, Traits>::size_type i;

	s.flags(os.flags());
	s.imbue(os.getloc());
	s.precision(os.precision());

	for (i = 0; i < y.length(); i++)
		s << y[i];

	return os << s.str();
}


template<typename CharT, class Traits, typename _Alloc>
std::basic_string<CharT, Traits, _Alloc>
to_string(const basic_substring<CharT, Traits>& y)
{
	return y;
}


#define __SUBSTRING_INSTANCE(name, type)				\
	typedef basic_substring<type>	name;				\
	namespace std {							\
	template<> struct hash<name> {					\
		size_t operator()(const name& s) const			\
		{							\
			return CRC_32((const char *) s.c_str(),		\
			    s.length() * sizeof(type));			\
		}							\
	};								\
	}

__SUBSTRING_INSTANCE(substring, char)

#ifdef _GLIBCXX_USE_WCHAR_T
__SUBSTRING_INSTANCE(wsubstring, wchar_t)
#endif

#if defined(__GXX_EXPERIMENTAL_CXX0X__) && defined(_GLIBCXX_USE_C99_STDINT_TR1)
__SUBSTRING_INSTANCE(u16substring, char16_t)
__SUBSTRING_INSTANCE(u32substring, char32_t)
#endif

#endif
