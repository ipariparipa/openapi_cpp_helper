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

#include <memory>
#include <vector>
#include <map>
#include <set>
#include <optional>
#include <unordered_map>
#include <string>

#include "_serializer/base64.h"
#include "assert.h"

#include "../_types/common.h"
#include "../_types/value.h"
#include "../_types/file.h"

namespace _serializer { namespace json
{

class object;
class array;
class document;
class value : public _types::object
{
public:
    using ref = std::shared_ptr<value>;
    virtual inline ~value() override = default;
    virtual bool is_object() const = 0;
    virtual bool is_array() const = 0;
    virtual bool is_number() const = 0;
    virtual bool is_bool() const = 0;
    virtual bool is_string() const = 0;
    virtual std::shared_ptr<_serializer::json::document> get_document() = 0;
    virtual std::shared_ptr<_serializer::json::object> as_object() = 0;
    virtual std::shared_ptr<_serializer::json::array> as_array() = 0;
    virtual void set_null() = 0;
    virtual bool is_null() const = 0;
    virtual bool copy_from(const value::ref &) = 0;
};

class document : public value
{
public:
    using ref = std::shared_ptr<document>;
    virtual inline ~document() = default;
    virtual std::string content() const = 0;
    virtual bool content(const std::string &) = 0;
    virtual bool content(const std::string_view &) = 0;
    virtual bool content(std::ostream &) = 0;
    virtual bool content(std::istream &) = 0;
};

class object : public value
{
public:
    using ref = std::shared_ptr<object>;
    virtual inline ~object() = default;
    virtual void add(const std::string & name, const value::ref &) = 0;
    virtual std::unordered_map<std::string, value::ref> get_members() const = 0;
    virtual value::ref member(const std::string & n) const = 0;
};

class array : public value
{
public:
    using ref = std::shared_ptr<array>;
    virtual inline ~array() = default;
    virtual void push(const value::ref &) = 0;
    virtual std::vector<value::ref> get_items() const = 0;
    virtual size_t size() const = 0;
};

document::ref make_document(); //to be implemented
value::ref make_null(const document::ref &); //to be implemented
array::ref make_array(const document::ref &); //to be implemented
object::ref make_object(const document::ref &); //to be implemented
value::ref make_value(const document::ref &); //to be implemented

inline value::ref to_null(const value::ref & v)
{ v->set_null(); return v; }

template<typename T>
value::ref to_value(const document::ref &, const value::ref &, const T &); //to be implemented

template<>
inline value::ref to_value<_types::object_ref>(const document::ref &, const value::ref & v, const _types::object_ref & s)
{
    if(auto sv = std::dynamic_pointer_cast<_serializer::json::value>(s); sv && sv->is_object())
        return v->copy_from(sv) ? v : nullptr;

    return nullptr;
}

template<>
inline value::ref to_value<_types::value_ref>(const document::ref &, const value::ref & v, const _types::value_ref & s)
{
    if(auto sv = std::dynamic_pointer_cast<_serializer::json::value>(s); sv)
        return v->copy_from(sv) ? v : nullptr;

    return nullptr;
}

template<>
inline value::ref to_value<_types::byte_array>(const document::ref & d, const value::ref & v, const _types::byte_array & s)
{
    return to_value(d, v, to_base64(s));
}

template<>
inline value::ref to_value<_types::file>(const document::ref & d, const value::ref & v, const _types::file & s)
{
    std::visit([&](auto && arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>)
            v->set_null();
        else
            to_value<T>(d, v, arg);
    }, s);

    return v;
}

template<typename T>
value::ref to_value(const document::ref & d, const value::ref &v, const std::vector<T> & s)
{
    auto arr = v->as_array();
    for(const auto & i : s)
        arr->push(to_value(d, make_value(d), i));

    return arr;
}

template<typename T>
value::ref to_value(const document::ref & d, const value::ref & v, const std::set<T> & s)
{
    auto arr = v->as_array();
    for(const auto & i : s)
        arr->push(to_value(d, make_value(d), i));

    return arr;
}

template<typename T>
value::ref to_value(const document::ref & d, const value::ref &v, const std::map<std::string, T> & s)
{
    auto obj = v->as_object();
    for(const auto & i : s)
        obj->add(i.first.c_str(), to_value(d, make_value(d), i.second));

    return obj;
}

template<typename T>
value::ref to_value(const document::ref & d, const value::ref &v, const std::optional<T> & s)
{
    return v ? to_value<T>(d, v, *s) : to_null(v);
}

template<typename T>
value::ref to_value(const document::ref & d, const T & s)
{ return to_value<T>(d, d, s); }

template<typename T>
bool from_value(const value::ref &, T & ret); //to be implemented

template<>
inline bool from_value<_types::object_ref>(const value::ref & v, _types::object_ref & ret)
{
    assert(v);

    if(!v->is_object())
        return false;

    if(auto doc = _serializer::json::make_document(); !doc->copy_from(v))
        return false;
    else
        ret = doc->as_object();

    return true;
}

template<>
inline bool from_value<_types::value_ref>(const value::ref & v, _types::value_ref & ret)
{
    assert(v);

    if(auto doc = _serializer::json::make_document(); !doc->copy_from(v))
        return false;
    else
        ret = doc;

    return true;
}

template<>
inline bool from_value<_types::byte_array>(const value::ref & v, _types::byte_array & ret)
{
    assert(v);

    if(_types::string str; from_value(v, str))
        if(auto err = from_base64<_types::byte_array>(str, ret); !err)
            return true;

    return false;
}

template<>
inline bool from_value<_types::file>(const value::ref & v, _types::file & ret)
{
    assert(v);

    if(auto & r = ret.emplace<_types::byte_array>(); from_value(v, r))
        return true;

    if(auto & r = ret.emplace<_types::string>(); from_value(v, r))
        return true;

    return false;
}

template<typename T>
bool from_value(const value::ref & v, std::vector<T> & ret)
{
    assert(v);

    if(!v->is_array())
        return false;

    auto arr = v->as_array();
    auto vals = arr->get_items();
    ret.resize(vals.size());
    for(size_t l = vals.size(), i = 0; i < l; ++i)
        if(auto & r = ret[i] = _types::default_value<T>(); !from_value(vals[i], r))
            return false;

    return true;
}

template<typename T>
bool from_value(const value::ref & v, std::set<T> & ret)
{
    assert(v);

    if(!v->is_array())
        return false;

    auto arr = v->as_array();
    auto vals = arr->get_items();
    for(size_t l = vals.size(), i = 0; i < l; ++i)
        if(auto & r = ret.emplace(_types::default_value<T>()); from_value(vals[i], r))
            ret.insert(r);
        else
            return false;

    return true;
}

template<typename T>
bool from_value(const value::ref & v, std::map<std::string, T> & ret)
{
    assert(v);

    if(!v->is_object())
        return false;

    auto obj = v->as_object();
    auto vals = obj->get_members();
    for(const auto & m : vals)
        if(auto & r = ret[m.first] = _types::default_value<T>(); !from_value(m.second, r))
            return false;

    return true;
}

template<typename T>
bool from_value(const value::ref & v, std::optional<T> & ret)
{
    assert(v);

    if(v->is_null())
    {
        ret.reset();
        return true;
    }

    return from_value(v, ret.emplace(_types::default_value<T>()));
}

template<typename T>
bool value_is(const value::ref & v); // to be implemented

}}
