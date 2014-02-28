//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Scaled numbers (i.e.: fixed point)     *
//               *    class.                                 *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#ifndef __INCLUDE_SCALED_H_
#define __INCLUDE_SCALED_H_

#include <string>
#include <sstream>


template<typename T, T S>
class scaled {

	template<typename To, To So>
	friend class scaled;

public:
	scaled(): value(T()) {};

	template<typename Ts, Ts Ss>
	scaled(const scaled<Ts, Ss>& val):
		value(T((long long) val.value * S / Ss)) {};

	template<typename Ts>
	scaled(const Ts& val): value(val * S) {};

	template<typename Ts, Ts Ss>
	scaled& operator=(const scaled<Ts, Ss>& y)
	{
		value = T((long long) y.value * S / Ss);
		return *this;
	}

	template<class Ts>
	scaled& operator=(const Ts& y)
	{
		value = T(y * S);
		return *this;
	}

	template<typename Td, Td Sd>
	operator scaled<Td, Sd>() const
	{
		scaled<Td, Sd> t(*this);
		return t;
	}

	operator bool() const { return value != T(); };

	template<class Td> operator Td() const
	{
		return Td(value) / S;
	}

	scaled operator-() const { scaled t; t.value = -value; return t; };

	template<typename Ty, Ty Sy>
	scaled& operator+=(const scaled<Ty, Sy>& y)
	{
		value += ((long long) y.value * S) / Sy;
		return *this;
	}

	template<class Ty>
	scaled& operator+=(const Ty& y)
	{
		value += scaled(y).value;
		return *this;
	}

	template<typename Ty, Ty Sy>
	scaled& operator-=(const scaled<Ty, Sy>& y)
	{
		value -= ((long long) y.value * S) / Sy;
		return *this;
	}

	template<class Ty>
	scaled& operator-=(const Ty& y)
	{
		value -= scaled(y).value;
		return *this;
	}

	template<typename Ty, Ty Sy>
	scaled& operator*=(const scaled<Ty, Sy>& y)
	{
		value = (long long) value * y.value / Sy;
		return *this;
	}

	template<class Ty>
	scaled& operator*=(const Ty& y)
	{
		value *= T(y);
		return *this;
	}

	template<typename Ty, Ty Sy>
	scaled& operator/=(const scaled<Ty, Sy>& y)
	{
		value = (long long) value * Sy / y.value;
		return *this;
	}

	template<class Ty>
	scaled& operator/=(const Ty& y)
	{
		value /= T(y);
		return *this;
	}

	template<typename Ty, Ty Sy>
	scaled& operator%=(const scaled<Ty, Sy>& y)
	{
		value -= (*this / y).value;
		return *this;
	}

	template<class Ty>
	scaled& operator%=(const Ty& y)
	{
		value %= T(y) * S;
		return *this;
	}

	template<class Ty>
	const scaled operator+(const Ty& y) const
	{
		scaled t = *this;
		t += y;
		return t;
	}

	template<class Ty>
	const scaled operator-(const Ty& y) const
	{
		scaled t = *this;
		t -= y;
		return t;
	}

	template<class Ty>
	const scaled operator*(const Ty& y) const
	{
		scaled t = *this;
		t *= y;
		return t;
	}

	template<class Ty>
	const scaled operator/(const Ty& y) const
	{
		scaled t = *this;
		t /= y;
		return t;
	}

	template<class Ty>
	const scaled operator%(const Ty& y) const
	{
		scaled t = *this;
		t %= y;
		return t;
	}

	template<typename Ty, Ty Sy>
	int compare(const scaled<Ty, Sy>& y) const
	{
		scaled t;

		if (S < Sy)
			return -y.compare(*this);

		if (value < T()) {
			if (y.value >= Ty())
				return -1;
			}
		else if (y.value < Ty())
			return 1;

		if (value < T())
			return -scaled<unsigned long long, S>(-*this).compare(
			    scaled<unsigned long long, Sy>(-y));

		t = scaled(y);
		return value < t.value? -1: value == t.value? 0: 1;
	}

	template<typename Ty, Ty Sy>
	bool operator<(const scaled<Ty, Sy>& y) const
	{
		return compare(y) < 0;
	}

	template<class Ty>
	bool operator<(const Ty& y) const
	{
		return *this < scaled(y);
	}

	template<typename Ty, Ty Sy>
	bool operator<=(const scaled<Ty, Sy>& y) const
	{
		return compare(y) <= 0;
	}

	template<class Ty>
	bool operator<=(const Ty& y) const
	{
		return *this <= scaled(y);
	}

	template<typename Ty, Ty Sy>
	bool operator==(const scaled<Ty, Sy>& y) const
	{
		return compare(y) == 0;
	}

	template<class Ty>
	bool operator==(const Ty& y) const
	{
		return *this == scaled(y);
	}

	template<typename Ty, Ty Sy>
	bool operator!=(const scaled<Ty, Sy>& y) const
	{
		return compare(y) != 0;
	}

	template<class Ty>
	bool operator!=(const Ty& y) const
	{
		return *this != scaled(y);
	}

	template<typename Ty, Ty Sy>
	bool operator>=(const scaled<Ty, Sy>& y) const
	{
		return compare(y) >= 0;
	}

	template<class Ty>
	bool operator>=(const Ty& y) const
	{
		return *this >= scaled(y);
	}

	template<typename Ty, Ty Sy>
	bool operator>(const scaled<Ty, Sy>& y) const
	{
		return compare(y) > 0;
	}

	template<class Ty>
	bool operator>(const Ty& y) const
	{
		return *this > scaled(y);
	}

private:
	T	value;
};


#define __SCALED_IMPLEMENT_ARITH_HELPER(type, op)			\
template<typename Ty, Ty Sy>						\
inline scaled<Ty, Sy>							\
operator op(type x, const scaled<Ty, Sy>& y)				\
{									\
	scaled<Ty, Sy> t(x);						\
	t op##= y;							\
	return t;							\
}

#define __SCALED_IMPLEMENT_ARITH(type)					\
	__SCALED_IMPLEMENT_ARITH_HELPER(type, +)			\
	__SCALED_IMPLEMENT_ARITH_HELPER(type, -)			\
	__SCALED_IMPLEMENT_ARITH_HELPER(type, *)			\
	__SCALED_IMPLEMENT_ARITH_HELPER(type, /)			\
	__SCALED_IMPLEMENT_ARITH_HELPER(type, %)

__SCALED_IMPLEMENT_ARITH(char)
__SCALED_IMPLEMENT_ARITH(unsigned char)
__SCALED_IMPLEMENT_ARITH(short)
__SCALED_IMPLEMENT_ARITH(unsigned short)
__SCALED_IMPLEMENT_ARITH(int)
__SCALED_IMPLEMENT_ARITH(unsigned int)
__SCALED_IMPLEMENT_ARITH(long)
__SCALED_IMPLEMENT_ARITH(unsigned long)
__SCALED_IMPLEMENT_ARITH(long long)
__SCALED_IMPLEMENT_ARITH(unsigned long long)
__SCALED_IMPLEMENT_ARITH(float)
__SCALED_IMPLEMENT_ARITH(double)
__SCALED_IMPLEMENT_ARITH(long double)


#define __SCALED_IMPLEMENT_REL_HELPER(type, op, rop)			\
template<typename Ty, Ty Sy>						\
inline bool								\
operator op(type x, const scaled<Ty, Sy>& y)				\
{									\
	return y rop x;							\
}

#define __SCALED_IMPLEMENT_REL(type)					\
	__SCALED_IMPLEMENT_REL_HELPER(type, <, >)			\
	__SCALED_IMPLEMENT_REL_HELPER(type, <=, >=)			\
	__SCALED_IMPLEMENT_REL_HELPER(type, ==, ==)			\
	__SCALED_IMPLEMENT_REL_HELPER(type, !=, !=)			\
	__SCALED_IMPLEMENT_REL_HELPER(type, >=, <=)			\
	__SCALED_IMPLEMENT_REL_HELPER(type, >, <)

__SCALED_IMPLEMENT_REL(char)
__SCALED_IMPLEMENT_REL(unsigned char)
__SCALED_IMPLEMENT_REL(short)
__SCALED_IMPLEMENT_REL(unsigned short)
__SCALED_IMPLEMENT_REL(int)
__SCALED_IMPLEMENT_REL(unsigned int)
__SCALED_IMPLEMENT_REL(long)
__SCALED_IMPLEMENT_REL(unsigned long)
__SCALED_IMPLEMENT_REL(long long)
__SCALED_IMPLEMENT_REL(unsigned long long)
__SCALED_IMPLEMENT_REL(float)
__SCALED_IMPLEMENT_REL(double)
__SCALED_IMPLEMENT_REL(long double)



template<typename T, T S, typename CharT, class Traits>
std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits>& is, scaled<T, S>& y)

{
	CharT ch;
	bool sign;
	scaled<T, S> acc;
	std::basic_string<CharT, Traits> digits("0123456789");
	int i;
	bool hasdigits = false;
	T div;

	if (is.bad() || is.eof()) {
		is.setstate(std::ios_base::failbit);
		return is;
		}

	is >> ch;

	if (is.bad() || is.eof()) {
		is.setstate(std::ios_base::failbit);
		return is;
		}

	sign = ch == '-' && T(~0) < 0;

	if (sign) {
		is.get(ch);

		if (is.bad() || is.eof()) {
			is.setstate(std::ios_base::failbit);
			return is;
			}
		}

	i = digits.find_first_of(std::basic_string<CharT, Traits>(&ch, 1));

	while (i != std::string::npos) {
		hasdigits = true;
		acc *= 10;
		acc += i;
		is.get(ch);

		if (is.bad() || is.eof())
			break;

		i = digits.find_first_of(std::basic_string<CharT,
                    Traits>(&ch, 1));
		}

	if (ch == '.' && !is.bad() && !is.eof()) {
		is.get(ch);

		if (!is.bad() && !is.eof()) {
			div = 1;
			i = digits.find_first_of(std::basic_string<CharT,
			    Traits>(&ch, 1));

			while (i != std::string::npos && div < S) {
				hasdigits = true;
				div *= 10;
				acc *= 10;
				acc += i;
				is.get(ch);

				if (is.bad() || is.eof())
					break;

				i = digits.find_first_of(
				    std::basic_string<CharT, Traits>(&ch, 1));
				}

			acc /= div;
			}
		}

	if (!hasdigits || is.bad())
		is.setstate(std::ios_base::failbit);
	else {
		if (sign)
			acc = -acc;

		if (!is.eof())
			is.putback(ch);

		y = acc;
		}

	return is;
}


template<typename T, T S, typename CharT, class Traits>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const scaled<T, S>& y)
{
	bool sign(y < scaled<T, S>());
	scaled<unsigned long long, S> t(sign? -y: y);
	std::basic_ostringstream<CharT, Traits> s;
	int n;

	s.flags(os.flags());
	s.imbue(os.getloc());
	s.precision(os.precision());

	if (sign)
		s << '-';

	s << T(t);
	t -= T(t);

	if (t) {
		s << '.';

		for (auto i = s.precision(); i > 0 && t; i--) {
			t *= 10;
			n = t;
			s << CharT('0' + n);
			t -= n;
			}
		}

	return os << s.str();
}


template<typename T, T S>
scaled<T, S>
from_string(const std::string s)
{
	scaled<T, S> t;
	std::istringstream iss(s);

	if ((iss >> t).fail())
		throw std::ios_base::failure("Illegal input data");

	return t;
}


template<typename T, T S> std::string
to_string(const scaled<T, S> y)
{
	std::ostringstream s;

	s << y;
	return s.str();
}

#endif
