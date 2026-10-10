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

#include <string>

namespace _types
{

#if __cplusplus >= 202002L
inline const char * u8_literal(const char8_t * s)
{ return reinterpret_cast<const char*>(s); }
#else
constexpr const char * u8_literal(const char * s)
{ return s; }
#endif

using string = std::string;
using string_ref = string;

}
