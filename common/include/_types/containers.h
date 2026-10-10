/*
    This file is part of openapi_cpp_helper.

    sasCore is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCore.  If not, see <http://www.gnu.org/licenses/>
 */

#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <set>

namespace _types
{

template<typename T>
using array = std::vector<T>;

template<typename T>
using array_ref = array<T>;

template<typename KeyT, typename ValueT>
using map = std::map<KeyT, ValueT>;

template<typename KeyT, typename ValueT>
using map_ref = map<KeyT, ValueT>;

template<typename KeyT, typename ValueT>
using unordered_map = std::unordered_map<KeyT, ValueT>;

template<typename KeyT, typename ValueT>
using unordered_map_ref = unordered_map<KeyT, ValueT>;

template<typename T>
using set = std::set<T>;

template<typename T>
using set_ref = set<T>;

}
