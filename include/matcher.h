//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Fuzzy matcher class.                   *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#ifndef __INCLUDE__MATCHER_H__
#define __INCLUDE__MATCHER_H__

#include <cstring>
#include <unordered_map>
#include <vector>

#include "scaled.h"
#include "substring.h"



class Matcher {

public:
	typedef std::vector<substring>	slices;

	class target {

	public:
		target(const std::string * txt = NULL, unsigned short cnt = 0):
		    text(txt), count(cnt) {};

		const std::string *	text;	// The string to match.
		unsigned short	count;		// n-gram count in string.
	};

	typedef scaled<unsigned short, 65535>	score_type;

	class result {

	public:
		template<typename T, T S>
		result(const std::string * txt = NULL, scaled<T, S> sc = 0):
		    text(txt), score(sc) {};

		bool operator<(const result& op2) const
		{
			return score < op2.score;
		}

		const std::string *	text;	// The matched string.
		score_type		score;	// The match score.
	};

	class target_slice {

	public:
		target_slice(unsigned int n = 0, unsigned int cnt = 0):
			index(n), count(cnt) {};

		unsigned int	index;		// Target index.
		unsigned int	count;		// # of this slice in target.
	};


	Matcher() {};

	virtual void
	slice(slices& s, const std::string& str, bool indexing) const
	{
		s.clear();
		s.emplace_back(str.data(), str.length());
	}

	void	add(const std::string& s);
	void	match(std::vector<result>& results, const std::string& s) const;


	std::unordered_map<substring, std::vector<target_slice> >
	    			index;		// The slice->target map.
	std::vector<target>	targets;	// The match targets.
};

#endif
