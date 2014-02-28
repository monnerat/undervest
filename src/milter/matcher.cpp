//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Fuzzy matcher algorithm.               *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#include <algorithm>

#include "matcher.h"
#include "support11.h"


void
Matcher::add(const std::string& str)

{
	slices s;

	slice(s, str, true);

	if (!s.size())
		s.emplace_back(str.data(), 0);

	for (auto i = s.begin(); i != s.end(); i++) {
		auto p = &(*um_emplace(index, *i,
		    std::vector<Matcher::target_slice>()).first).second;
		auto sz = p->size();

		if (!sz || (*p)[sz - 1].index != targets.size())
			p->emplace_back(targets.size(), 1);
		else
			(*p)[sz - 1].count++;
		}

	targets.emplace_back(&str, s.size());
}


void
Matcher::match(std::vector<Matcher::result>& results,
						const std::string& str) const

{
	slices s;
	slices::iterator i, j;
	std::unordered_map<unsigned int, unsigned int> r;

	results.clear();
	slice(s, str, false);

	if (!s.size())
		s.emplace_back(str.data(), 0);

	std::sort(s.begin(), s.end());
	j = s.begin();

	for (i = j + 1; i != s.end(); i++)
		if (*j < *i) {
			auto q = index.find(*j);

			if (q != index.end()) {
				unsigned int n = i - j;

				for (auto k = (*q).second.begin();
				    k != (*q).second.end(); k++) {
					auto l = &(*um_emplace(r, (*k).index,
					    0).first).second;
					*l += n < (*k).count? n: (*k).count;
					}
				}

			j = i;
			}

	auto q = index.find(*j);

	if (q != index.end()) {
		unsigned int n = i - j;

		for (auto k = (*q).second.begin();
		    k != (*q).second.end(); k++) {
			auto l = &(*um_emplace(r, (*k).index, 0).first).second;
			*l += n < (*k).count? n: (*k).count;
			}
		}

	for (auto k = r.begin(); k != r.end(); k++) {
		scaled<unsigned long, 65535> t(2 * (*k).second);
		t /= targets[(*k).first].count + s.size();
		results.emplace_back(targets[(*k).first].text, t);
		}

	sort(results.rbegin(), results.rend());
}
