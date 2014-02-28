//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Compute CRC-32.                        *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#ifndef __INCLUDE_CRC32_H__
#define __INCLUDE_CRC32_H__

#include <cstdlib>


// 	The initial computation algorithm is:
//
// 	#define POLY	0xEDB88320
//
// 	_	Initialize CRC to 0xFFFFFFFF
// 	_	For each byte to include in CRC, do:
// 		_	XOR byte to low CRC byte,
// 		_	Do 8 times:
// 			_	Shift CRC right, putting leftover
// 					rightmost bit in carry,
// 			_	If carry is set, XOR POLY to CRC
// 	_	Return the 1's complement of CRC.
//
//
// 	The algorithm below is faster: it is based on a table computed
// 		with the previous algorithm.


class CRC32 {

private:
	unsigned long	accu;

	static unsigned long	xortable[256];

public:
	CRC32(): accu(0) {};
	CRC32(const CRC32& other): accu(other.accu) {};
	virtual ~CRC32() {};

	CRC32&
	add(unsigned char c)
	{
		accu = xortable[(accu ^ c) & 0xFF] ^ (accu >> 8);
		return *this;
	}

	CRC32&
	add(char c)
	{
		return add((unsigned char) c);
	}

	CRC32&
	add(const unsigned char * p, size_t len = (size_t) ~0)
	{
		if (len == (size_t) ~0)
			while (*p)
				add(*p++);
		else
			while (len--)
				add(*p++);

		return *this;
	}

	CRC32&
	add(const char * p, size_t len = (size_t) ~0)
	{
		return add((const unsigned char *) p, len);
	}

	template<class _Iterator>
	CRC32&
	add(_Iterator b, _Iterator e)
	{
		while (b != e)
			add(*b++);

		return *this;
	}

	template<class _Container>
	CRC32&
	add(const _Container& c)
	{
		return add(c.begin(), c.end());
	}

	CRC32& operator+(unsigned char c) { return add(c); };
	CRC32& operator+(char c) { return add(c); };
	CRC32& operator+(const unsigned char * p) { return add(p); };
	CRC32& operator+(const char * p) { return add(p); };

	template<class _Container>
	CRC32&
	operator+(const _Container& c)
	{
		return add(c);
	}

	unsigned long
	value() const
	{
		return accu;
	}
};


inline unsigned long
CRC_32(const unsigned char * p, size_t len = (size_t) ~0)
{
	CRC32 crc;

	crc.add(p, len);
	return crc.value();
}


inline unsigned long
CRC_32(const char * p, size_t len = (size_t) ~0)
{
	return CRC_32((const unsigned char *) p, len);
}


template<class _Iterator>
inline unsigned long
CRC_32(_Iterator b, _Iterator e)
{
	CRC32 crc;

	crc.add(b, e);
	return crc.value();
}


template<class _Container>
inline unsigned long
CRC_32(const _Container& c)
{
	CRC32 crc;

	crc.add(c);
	return crc.value();
}

#endif
