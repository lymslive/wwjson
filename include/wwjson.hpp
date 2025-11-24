/**
 * @file wwjson.hpp 
 * @author lymslive
 * @date 2025-11-21
 * @brief Deal with json in raw string simply and quickly in limited case.
 * */

#pragma once
#ifndef WWJSON_HPP__
#define WWJSON_HPP__

#include <type_traits>
// for std::to_chars C++17
// #include <charconv>
#include <array>

namespace wwjson
{

/// Builder json string directlly without DOM by any lib.
struct RawJsonBuilder
{
    std::string json;

    RawJsonBuilder(int capacity = 1024)
    {
        json.reserve(capacity);
    }

    void PutNull()
    {
        json.append("null");
    }

    void PutValue(bool tf)
    {
        if (tf)
        {
            json.append("true");
        }
        else
        {
            json.append("false");
        }
    }

    void PutValue(const char* pszVal)
    {
        // json.append("\"");
        json.push_back('"');
        json.append(pszVal);
        json.push_back('"');
        // json.append("\"");
    }

    void PutValue(const char* pszVal,size_t len)
    {
        json.append("\"");
        json.append(pszVal,len);
        json.append("\"");
    }

    void PutValue(const std::string& strValue)
    {
        json.push_back('"');
        json.append(strValue);
        json.push_back('"');
    }

    // is_arithmetic_v
    template <typename numberT>
    std::enable_if_t<std::is_integral_v<numberT>, void>
    /*void*/ PutValue(numberT nValue)
    {
        // std::array<char, 32> buffer;
        // auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), nValue);
        // json.append(buffer.data(), ptr);
        json.append(std::to_string(nValue));
    }

    template <typename numberT>
    std::enable_if_t<std::is_floating_point_v<numberT>, void>
    /*void*/ PutValue(numberT nValue)
    {
        json.append(std::to_string(nValue));
    }

    void PutComma()
    {
        json.push_back(',');
    }

    void SepItem() { PutComma(); }

    void BeginArray(const char* pszKey)
    {
        PutKey(pszKey);
        BeginArray();
    }
    void BeginArray()
    {
        json.push_back('[');
    }
    void EndArray()
    {
        if (json.back() == ',')
        {
            json.back() = ']';
        }
        else
        {
            json.push_back(']');
        }
    }
    void EndArray(bool hasNext)
    {
        EndArray();
        SepItem();
    }
    void EmptyArray()
    {
        json.append("[]");
    }

    /// Add item to array.
    template <typename T>
    void AddItem(T value)
    {
        PutValue(value);
        SepItem();
    }

    void AddItem(const std::string& value)
    {
        PutValue(value);
        SepItem();
    }

    // Add member as string event if is_arithmetic_v
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddItem(numberT value, bool str)
    {
        if (str)
        {
            json.push_back('"');
        }
        PutValue(value);
        if (str)
        {
            json.push_back('"');
        }
        SepItem();
    }

    void BeginObject(const char* pszKey)
    {
        PutKey(pszKey);
        BeginObject();
    }
    void BeginObject()
    {
        json.push_back('{');
    }
    void EndObject()
    {
        if (json.back() == ',')
        {
            json.back() = '}';
        }
        else
        {
            json.push_back('}');
        }
    }
    void EndObject(bool hasNext)
    {
        EndObject();
        SepItem();
    }
    void EmptyObject()
    {
        json.append("{}");
    }

    void PutKey(const char* pszKey)
    {
        // json.append("\"");
        json.push_back('"');
        json.append(pszKey);
        json.append("\":");
    }

    /// Add member to object.
    template <typename T>
    void AddMember(const char* pszKey, T value)
    {
        PutKey(pszKey);
        PutValue(value);
        SepItem();
    }

    void AddMember(const char* pszKey, const std::string& value)
    {
        PutKey(pszKey);
        PutValue(value);
        SepItem();
    }

    void AddMember(const char* pszKey, const char* value,size_t len)
    {
        PutKey(pszKey);
        PutValue(value,len);
        SepItem();
    }

    // Add member as string event if is_arithmetic_v
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddMember(const char* pszKey, numberT value, bool str)
    {
        PutKey(pszKey);
        json.push_back('"');
        PutValue(value);
        json.push_back('"');
        SepItem();
    }

    // 字符串转义支持
    // 只判断转义单个特殊字符，默认双引号
    static void EscapeString(const std::string& src, std::string& dst, char ec = '"')
    {
        for (auto c : src)
        {
            if (c == ec)
            {
                dst.push_back('\\');
            }
            dst.push_back(c);
        }
    }

    // 判断转义多个字符，如 "\"\\\n\t" 等
    static void EscapeString(const std::string& src, std::string& dst, const char* ecs)
    {
        for (auto c : src)
        {
            bool escaped = false;
            for (const char* p = ecs; *p != '\0'; ++p)
            {
                if (c == *p)
                {
                    dst.push_back('\\');
                    if (c == '\n')
                    {
                        dst.push_back('n');
                    }
                    else if (c == '\t')
                    {
                        dst.push_back('t');
                    }
                    else if (c == '\r')
                    {
                        dst.push_back('r');
                    }
                    else
                    {
                        dst.push_back(c);
                    }
                    escaped = true;
                    break;
                }
            }
            if (!escaped)
            {
                dst.push_back(c);
            }
        }
    }

    void AddMemberEscape(const char* pszKey, const std::string& value, char ec = '"')
    {
        PutKey(pszKey);
        json.push_back('"');
        EscapeString(value, json, ec);
        json.push_back('"');
        SepItem();
    }

    void AddMemberEscape(const char* pszKey, const std::string& value, const char* ecs)
    {
        PutKey(pszKey);
        json.push_back('"');
        EscapeString(value, json, ecs);
        json.push_back('"');
        SepItem();
    }

    void EndLine()
    {
        json.push_back('\n');
    }

    /// 重新打开对象 {}，为后续再增加字段.
    void ReopenObject()
    {
        if (!json.empty() && json.back() == '}')
        {
            json.back() = ',';
        }
    }

    /// 合并两 json 对象的序列化字符串.
    void Merge(const RawJsonBuilder& that)
    {
        if (json.size() < 2)
        {
            json = that.json;
        }
        else if (that.json.size() > 2 && that.json.front() == '{')
        {
            ReopenObject();
            json.append(that.json.c_str() + 1, that.json.size() - 1);
        }
    }

    /// 静态方法版本，合并两个 object {}
    static void MergeObject(std::string& self, const std::string& that)
    {
        if (self.size() < 2)
        {
            self = that;
        }
        else if (that.size() > 2 && that.front() == '{')
        {
            if (self.back() == '}')
            {
                self.back() = ',';
            }
            self.append(that.c_str() + 1, that.size() - 1);
        }
    }
};

/// Auto open and close object {}
struct RawJsonObject
{
    RawJsonBuilder& m_build;
    bool m_next = false;

    RawJsonObject(RawJsonBuilder& build, bool hasNext = false) : m_build(build), m_next(hasNext)
    {
        m_build.BeginObject();
    }
    RawJsonObject(RawJsonBuilder& build, const char* pszKey, bool hasNext = false) : m_build(build), m_next(hasNext)
    {
        m_build.PutKey(pszKey);
        m_build.BeginObject();
    }
    ~RawJsonObject()
    {
        m_build.EndObject();
        if (m_next)
        {
            m_build.SepItem();
        }
    }

    // Forward to builder method
    template <typename... Args>
    void AddMember(Args&&... args)
    {
        m_build.AddMember(std::forward<Args>(args)...);
    }
};

/// Auto open and close array[]
struct RawJsonArray
{
    RawJsonBuilder& m_build;
    bool m_next = false;

    RawJsonArray(RawJsonBuilder& build, bool hasNext = false) : m_build(build), m_next(hasNext)
    {
        m_build.BeginArray();
    }
    RawJsonArray(RawJsonBuilder& build, const char* pszKey, bool hasNext = false) : m_build(build), m_next(hasNext)
    {
        m_build.PutKey(pszKey);
        m_build.BeginArray();
    }
    ~RawJsonArray()
    {
        m_build.EndArray();
        if (m_next)
        {
            m_build.SepItem();
        }
    }

    // Forward to builder method
    template <typename... Args>
    void AddItem(Args&&... args)
    {
        m_build.AddItem(std::forward<Args>(args)...);
    }
};

} /* end of wwjson:: */ 

#endif /* end of include guard: WWJSON_HPP__ */
