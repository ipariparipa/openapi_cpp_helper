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

#include <variant>
#include <functional>

namespace _types
{

template<typename ...T>
using variant = std::variant<T...>;

template<class T>
struct type_identity
{
    using type = T;
};

template<class V, class F, std::size_t... I>
void for_each_type(F&& f, std::index_sequence<I...>)
{
    (std::invoke(f, type_identity<std::variant_alternative_t<I, V>>{}), ...);
}

template<class V, class F, std::size_t I = 0>
bool for_each_type(F&& f)
{
    if constexpr (I == std::variant_size_v<V>)
        return false;
    else
        return std::invoke(f, type_identity<std::variant_alternative_t<I, V>>{})
               || for_each_type<V, F, I + 1>(std::forward<F>(f));
}

}
