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

#include "_serializer/json.h"
#include "_serializer/base64.h"
#include "_serializer/to_string.h"
#include "_serializer/from_string.h"

#include <chrono>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>

namespace rapidjson_serializer
{

namespace _internal
{

template<typename Base, typename P, typename... Ts>
std::shared_ptr<Base> first_cast(const P& p)
{
    std::shared_ptr<Base> ret;
    ((ret = std::dynamic_pointer_cast<Ts>(p)) || ...);
    return ret;
}

}

class abstract_base
{
public:
    virtual inline ~abstract_base() = default;
    virtual const std::shared_ptr<rapidjson::Value> & val() const = 0;
    virtual const std::shared_ptr<rapidjson::Document> & doc() const = 0;
};

template<typename T = abstract_base>
std::shared_ptr<T> cast_to(const _serializer::json::value::ref & in);

class document;
class object;
class array;
template<typename BaseValueT = _serializer::json::value>
class base_value : public BaseValueT, public abstract_base
{
    std::shared_ptr<rapidjson::Document> m_doc;
    std::shared_ptr<rapidjson::Value> m_value;

public:
    base_value(const std::shared_ptr<rapidjson::Document> & doc)
        : m_doc(doc)
        , m_value(new rapidjson::Value())
    { }

    base_value(const std::shared_ptr<rapidjson::Document> & doc, const std::shared_ptr<rapidjson::Value> & value)
        : m_doc(doc)
        , m_value(value)
    { }

    virtual ~base_value() override = default;

    bool is_object() const override
    { return val()->IsObject(); }

    bool is_array() const override
    { return val()->IsArray(); }

    bool is_number() const override
    { return val()->IsNumber(); }

    bool is_bool() const override
    { return val()->IsBool(); }

    bool is_string() const override
    { return val()->IsString(); }

    std::shared_ptr<_serializer::json::document> get_document() override;
    std::shared_ptr<_serializer::json::object> as_object() override;
    std::shared_ptr<_serializer::json::array> as_array() override;

    void set_null() override
    { val()->SetNull(); }

    bool is_null() const override
    { return val()->IsNull(); }

    bool copy_from(const _serializer::json::value::ref & v) override;

    const std::shared_ptr<rapidjson::Value> & val() const override
    { return m_value; }

    const std::shared_ptr<rapidjson::Document> & doc() const override
    { return m_doc; }
};


class document final : public base_value<_serializer::json::document>
{
public:
    inline document(const std::shared_ptr<rapidjson::Document> & doc = std::make_shared<rapidjson::Document>())
        : base_value(doc, doc)
    { }

    inline ~document() final = default;

    inline std::string content() const final
    {
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> w(buf);
        return doc()->Accept(w) ? buf.GetString() : std::string();
    }

    inline bool content(const std::string & str) final
    {
        return !doc()->Parse(str.c_str()).HasParseError();
    }

    inline bool content(const std::string_view & str) final
    {
        return !doc()->Parse(str.data(), str.size()).HasParseError();
    }

    inline bool content(std::ostream & stream) final
    {
        rapidjson::OStreamWrapper s(stream);
        rapidjson::Writer<rapidjson::OStreamWrapper> w(s);
        return doc()->Accept(w);
    }

    inline bool content(std::istream & stream) final
    {
        rapidjson::IStreamWrapper s(stream);
        return !doc()->ParseStream(s).HasParseError();
    }
};

class value final : public base_value<>
{
public:
    inline value(const std::shared_ptr<rapidjson::Document> & doc)
        : base_value(doc)
    { }

    inline value(const std::shared_ptr<rapidjson::Document> & doc, const std::shared_ptr<rapidjson::Value> & value)
        : base_value(doc, value)
    { }

    inline ~value() final = default;
};

class object final : public base_value<_serializer::json::object>
{
public:
    inline object(const std::shared_ptr<rapidjson::Document> & doc)
        : base_value(doc)
    { val()->SetObject(); }

    inline object(const std::shared_ptr<rapidjson::Document> & doc, const std::shared_ptr<rapidjson::Value> & value)
        : base_value(doc, value)
    { assert(val()->IsObject()); }

    inline ~object() final = default;

    void inline add(const std::string & name, const _serializer::json::value::ref & v) final
    {
        if(auto vv = cast_to(v))
            val()->AddMember(rapidjson::Value(name.c_str(), doc()->GetAllocator()), *vv->val(), doc()->GetAllocator());
    }

    std::unordered_map<std::string, _serializer::json::value::ref> inline get_members() const final
    {
        std::unordered_map<std::string, value::ref> ret;
        for(auto it = val()->MemberBegin(); it != val()->MemberEnd(); ++it)
            ret.insert(std::pair<std::string, _serializer::json::value::ref>(it->name.GetString(), std::make_shared<rapidjson_serializer::value>(doc(), std::shared_ptr<rapidjson::Value>(&it->value, [](...){}))));

        return ret;
    }

    value::ref inline member(const std::string & n) const final
    {
        if(auto it = val()->FindMember(n.c_str()); it != val()->MemberEnd())
            return std::make_shared<rapidjson_serializer::value>(doc(), std::shared_ptr<rapidjson::Value>(&it->value, [](...){}));

        return nullptr;
    }
};

class array final : public base_value<_serializer::json::array>
{
public:
    inline array(const std::shared_ptr<rapidjson::Document> & doc)
        : base_value(doc)
    { val()->SetArray(); }

    inline array(const std::shared_ptr<rapidjson::Document> & doc, const std::shared_ptr<rapidjson::Value> & value)
        : base_value(doc, value)
    { assert(val()->IsArray()); }

    inline ~array() final = default;

   void inline push(const _serializer::json::value::ref & v) final
    {
       if(auto vv = cast_to(v))
            val()->PushBack(*vv->val(), doc()->GetAllocator());
    }

    std::vector<_serializer::json::value::ref> inline get_items() const
    {
        std::vector<value::ref> ret;
        for(auto it = val()->Begin(); it != val()->End(); ++it)
            ret.push_back(std::make_shared<rapidjson_serializer::value>(doc(), std::shared_ptr<rapidjson::Value>(&*it, [](...){})));

        return ret;
    }

    size_t size() const final
    {
        return val()->Size();
    }

};

template<typename T>
std::shared_ptr<T> cast_to(const _serializer::json::value::ref & in)
{
    return std::dynamic_pointer_cast<T>(_internal::first_cast<abstract_base, _serializer::json::value::ref,
                                                              base_value<_serializer::json::document>,
                                                              base_value<_serializer::json::object>,
                                                              base_value<_serializer::json::array>,
                                                              base_value<_serializer::json::value>
                                                              >(in));
}

template<typename BaseValueT>
bool base_value<BaseValueT>::copy_from(const _serializer::json::value::ref &v)
{
    if(auto vv = cast_to(v))
        val()->CopyFrom(*vv->val(), doc()->GetAllocator());

    return true;
}

template<typename BaseValueT>
std::shared_ptr<_serializer::json::document> base_value<BaseValueT>::get_document()
{
    return std::make_shared<document>(doc());
}

template<typename BaseValueT>
std::shared_ptr<_serializer::json::object> base_value<BaseValueT>::as_object() //override
{
    if(!m_value->IsObject())
        m_value->SetObject();
    return std::make_shared<object>(doc(), val());
}

template<typename BaseValueT>
std::shared_ptr<_serializer::json::array> base_value<BaseValueT>::as_array() //override
{
    if(!m_value->IsArray())
        m_value->SetArray();
    return std::make_shared<array>(doc(), val());
}

template<typename T>
void value_set(const std::shared_ptr<abstract_base> & b, const T & v)
{ b->val()->Set<T>(v); }

template<typename T>
std::optional<T> value_get(const std::shared_ptr<abstract_base> & b)
{ return b->val()->Is<T>() ? b->val()->Get<T>() : std::optional<T>(); }

template<>
inline void value_set<std::string>(const std::shared_ptr<abstract_base> & b, const std::string & v)
{ b->val()->SetString(v.c_str(), b->doc()->GetAllocator()); }

template<>
inline void value_set<bool>(const std::shared_ptr<abstract_base> & b, const bool & v)
{ b->val()->SetBool(v); }

template<>
inline void value_set<long long>(const std::shared_ptr<abstract_base> & b, const long long & v)
{ b->val()->SetInt64(v); }

template<>
inline void value_set<std::vector<std::byte>>(const std::shared_ptr<abstract_base> & b, const std::vector<std::byte> & v)
{ b->val()->SetString(_serializer::to_base64(v).c_str(), b->doc()->GetAllocator()); }

template<>
inline void value_set<std::chrono::system_clock::time_point>(const std::shared_ptr<abstract_base> & b, const std::chrono::system_clock::time_point & v)
{ b->val()->SetString(_serializer::to_string(v).c_str(), b->doc()->GetAllocator()); }

template<>
inline std::optional<std::string> value_get<std::string>(const std::shared_ptr<abstract_base> & b)
{ return b->val()->IsString() ? b->val()->GetString() : std::optional<std::string>(); }

template<>
inline std::optional<long long> value_get<long long>(const std::shared_ptr<abstract_base> & b)
{ return b->val()->IsInt64() ? b->val()->GetInt64() : std::optional<long long>(); }

template<>
inline std::optional<std::vector<std::byte>> value_get<std::vector<std::byte>>(const std::shared_ptr<abstract_base> & b)
{
    if(std::vector<std::byte> ret; b->val()->IsString() && !_serializer::from_base64<std::vector<std::byte>, std::string>(b->val()->GetString(), ret))
        return ret;

    return std::nullopt;
}

template<>
inline std::optional<std::chrono::system_clock::time_point> value_get<std::chrono::system_clock::time_point>(const std::shared_ptr<abstract_base> & b)
{
    try
    {
        return b->val()->IsString() ? _serializer::from_string<std::chrono::system_clock::time_point>(b->val()->GetString()) : std::optional<std::chrono::system_clock::time_point>();
    } catch (...)
    {
        return std::nullopt;
    }
}

}

namespace _serializer { namespace json
{

template<typename T>
value::ref to_value(const document::ref & d, const value::ref & v, const T & s)
{
    (void)d;

    if(auto vv = rapidjson_serializer::cast_to<>(v))
    {
        rapidjson_serializer::value_set<T>(vv, s);
        return v;
    }

    return { };
}

template<typename T>
bool from_value(const value::ref & v, T & t)
{
    if(auto vv = rapidjson_serializer::cast_to<rapidjson_serializer::value>(v))
        if(auto r = rapidjson_serializer::value_get<T>(vv))
        {
            t = *r;
            return true;
        }

    return false;
}

inline document::ref make_document()
{
    return std::make_shared<rapidjson_serializer::document>();
}

inline value::ref make_null(const document::ref & d)
{
    if(auto b = rapidjson_serializer::cast_to(d))
    {
        auto ret = std::make_shared<rapidjson_serializer::value>(b->doc());
        ret->set_null();
        return ret;
    }

    return { };
}

inline array::ref make_array(const document::ref & d)
{
    if(auto b = rapidjson_serializer::cast_to(d))
        return std::make_shared<rapidjson_serializer::array>(b->doc());

    return { };
}

inline object::ref make_object(const document::ref & d)
{
    if(auto b = rapidjson_serializer::cast_to(d))
        return std::make_shared<rapidjson_serializer::object>(b->doc());

    return { };
}

inline value::ref make_value(const document::ref & d)
{
    if(auto b = rapidjson_serializer::cast_to(d))
        return std::make_shared<rapidjson_serializer::value>(b->doc());

    return { };
}

template<typename T>
bool value_is(const value::ref & v)
{
    if(auto b = rapidjson_serializer::cast_to(v))
        return b->val()->Is<T>();

    return false;
}

template<>
inline bool value_is<std::string>(const value::ref & v)
{
    if(auto b = rapidjson_serializer::cast_to(v))
        return b->val()->IsString();

    return false;
}

template<>
inline bool value_is<long long>(const value::ref & v)
{
    if(auto b = rapidjson_serializer::cast_to(v))
        return b->val()->IsInt64();

    return { };
}

template<>
inline bool value_is<bool>(const value::ref & v)
{
    if(auto b = rapidjson_serializer::cast_to(v))
        return b->val()->IsBool();

    return { };
}

}}
