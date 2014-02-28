//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Additional support for incomplete      *
//               *    C++11.                                 *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#ifndef __INCLUDE_SUPPORT11_H__
#define __INCLUDE_SUPPORT11_H__

#include <utility>
#include <unordered_map>
#include <unordered_set>

template<typename _Key, typename _Value, typename... _Args>
inline std::pair<typename std::unordered_map<_Key, _Value>::iterator, bool>
um_emplace(std::unordered_map<_Key, _Value>& _obj, _Args&&... _args)
{
	return _obj.insert(typename std::unordered_map<_Key, _Value>::
	    value_type(std::forward<_Args>(_args)...));
}

template<typename _Value, typename... _Args>
inline std::pair<typename std::unordered_set<_Value>::iterator, bool>
us_emplace(std::unordered_set<_Value>& _obj, _Args&&... _args)
{
	return _obj.insert(typename std::unordered_set<_Value>::
	    value_type(std::forward<_Args>(_args)...));
}

#endif
