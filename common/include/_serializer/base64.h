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
#include <string>

namespace _serializer
{

template<typename BinaryT, typename StringT = std::string>
StringT to_base64(const BinaryT & data)
{
    static const char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    StringT result;
    typename BinaryT::size_type i = 0;
    auto len = data.size();

    result.reserve(((len + 2) / 3) * 4);

    while (i < len)
    {
        uint32_t octet_a = i < len ? static_cast<uint32_t>(data[i++]) : 0;
        uint32_t octet_b = i < len ? static_cast<uint32_t>(data[i++]) : 0;
        uint32_t octet_c = i < len ? static_cast<uint32_t>(data[i++]) : 0;

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        result.push_back(base64Chars[(triple >> 18) & 0x3F]);
        result.push_back(base64Chars[(triple >> 12) & 0x3F]);
        result.push_back(base64Chars[(triple >> 6) & 0x3F]);
        result.push_back(base64Chars[triple & 0x3F]);
    }

    auto mod = len % 3;
    if (mod == 1)
    {
        result[result.size() - 2] = '=';
        result[result.size() - 1] = '=';
    }
    else if (mod == 2)
        result[result.size() - 1] = '=';

    return result;
}

template<typename BinaryT, typename StringT>
const char * from_base64(const StringT & encoded, BinaryT & result) //return: error text / nullptr
{
    static const char decodingTable[] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
    };

    auto len = encoded.size();
    if (len % 4 != 0)
        return "Invalid base64 length";

    typename BinaryT::size_type pad = 0;
    if (len > 0 && encoded[len - 1] == '=') pad++;
    if (len > 1 && encoded[len - 2] == '=') pad++;

    result.resize((len / 4) * 3 - pad);
    typename BinaryT::size_type outIdx = 0;

    for (size_t i = 0; i < len; i += 4)
    {
        int c0 = decodingTable[static_cast<unsigned char>(encoded[i])];
        int c1 = decodingTable[static_cast<unsigned char>(encoded[i + 1])];
        int c2 = decodingTable[static_cast<unsigned char>(encoded[i + 2])];
        int c3 = decodingTable[static_cast<unsigned char>(encoded[i + 3])];

        if (c0 < 0 || c1 < 0 || (c2 < 0 && i + 2 < len - pad) || (c3 < 0 && i + 3 < len - pad))
            return "Invalid base64 character";

        uint32_t triple = (c0 << 18) | (c1 << 12) | ((c2 > 0 ? c2 : 0) << 6) | (c3 > 0 ? c3 : 0);

        if (outIdx < result.size())
            result[outIdx++] = static_cast<typename BinaryT::value_type>((triple >> 16) & 0xFF);
        if (outIdx < result.size())
            result[outIdx++] = static_cast<typename BinaryT::value_type>((triple >> 8) & 0xFF);
        if (outIdx < result.size())
            result[outIdx++] = static_cast<typename BinaryT::value_type>(triple & 0xFF);
    }

    return nullptr; //OK
}

template<typename BinaryT, typename StringT>
BinaryT from_base64(const StringT & encoded, const char ** err)
{
    BinaryT ret;
    *err = from_base64(encoded, ret);
    return ret;
}

}
