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
