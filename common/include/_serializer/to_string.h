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
#include <chrono>

#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <vector>

#include "base64.h"

namespace _serializer
{

template<typename T>
std::string to_string(const T & v)
{ return std::to_string(v); }

template<>
inline std::string to_string<std::string>(const std::string & str)
{ return str; }

template<>
inline std::string to_string<bool>(const bool & b)
{ return b ? "true" : "false"; }

template<>
inline std::string to_string<std::chrono::system_clock::time_point>(const std::chrono::system_clock::time_point & tp)
{
    auto time_t_val = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_val = *std::gmtime(&time_t_val);

    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%dT%H:%M:%S");

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  tp.time_since_epoch()).count() % 1000;
    oss << '.' << std::setfill('0') << std::setw(3) << ms << 'Z';

    return oss.str();
}

template<>
inline std::string to_string<std::vector<std::byte>>(const std::vector<std::byte> & data)
{
    return to_base64(data);
}

}
