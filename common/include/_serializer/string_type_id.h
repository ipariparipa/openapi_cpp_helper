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

#include <cstdint>
#include <string_view>

namespace _serializer
{

using string_type_id_t = std::uint64_t;

template<string_type_id_t BeginV, string_type_id_t ModV>
constexpr string_type_id_t string_type_id(std::string_view s)
{
    string_type_id_t h = BeginV;

    for (char c : s)
    {
        h ^= static_cast<unsigned char>(c);
        h *= ModV;
    }

    return h;
}

}
