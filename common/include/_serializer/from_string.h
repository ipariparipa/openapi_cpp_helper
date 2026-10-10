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
#include <vector>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <cmath>

#include "base64.h"

namespace _serializer
{

template<typename T>
T from_string(const std::string &);

template<>
inline std::string from_string<std::string>(const std::string & str)
{ return str; }

template<>
inline int32_t from_string<int32_t>(const std::string & str)
{ return std::stoi(str); }

template<>
inline int64_t from_string<int64_t>(const std::string & str)
{ return std::stoll(str); }

template<>
inline float from_string<float>(const std::string & str)
{ return std::stof(str); }

template<>
inline double from_string<double>(const std::string & str)
{ return std::stod(str); }

template<>
inline bool from_string<bool>(const std::string & str)
{ return str != "0" || str == "true" || str == "yes"; }


template<>
inline std::chrono::system_clock::time_point from_string<std::chrono::system_clock::time_point>(const std::string & isoStr)
{
    if (isoStr.size() < 19 || isoStr[10] != 'T')
        throw std::invalid_argument("Invalid ISO format");

    int year, month, day, hour, min, sec = 0;
    double prec_s = 0.0;
    char dash1, dash2, colon1, colon2 /*, dot*/ = 0;

    std::istringstream iss(isoStr);
    iss >> year >> dash1 >> month >> dash2 >> day
        >> std::ws >> hour >> colon1 >> min >> colon2 >> sec;

    if (isoStr.size() >= 21 && isoStr[19] == '.')
    {
        int p = 0;
        size_t pos = 20;
        while (pos < isoStr.size() && std::isdigit(isoStr[pos]))
            p = p * 10 + (isoStr[pos++] - '0');

        prec_s = double(p) / double(std::pow(10, pos-20));
    }

    std::tm tm_val = {};
    tm_val.tm_year = year - 1900;
    tm_val.tm_mon  = month - 1;
    tm_val.tm_mday = day;
    tm_val.tm_hour = hour;
    tm_val.tm_min  = min;
    tm_val.tm_sec  = sec;
    tm_val.tm_isdst = 0;

    auto time_t_val = std::mktime(&tm_val);
    auto tp = std::chrono::system_clock::from_time_t(time_t_val);

    if (prec_s > 0.0)
        tp += std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(prec_s * 1000000000.0));

    return tp;
}

template<>
inline std::vector<std::byte> from_string<std::vector<std::byte>>(const std::string & encoded)
{
    std::vector<std::byte> ret;
    if(auto err= from_base64<std::vector<std::byte>>(encoded, ret); err)
        throw std::runtime_error(err);
    return ret;
}

}
