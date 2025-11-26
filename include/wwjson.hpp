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
#include <string>
// for std::to_chars C++17
// #include <charconv>
// #include <array>

namespace wwjson
{

/// String concept struct to document required interfaces for custom string types
/// Although C++17 doesn't support concepts, this serves as documentation
struct StringConcept
{
    // Required interfaces that a custom string type must provide:
    // - append(const char* str)
    // - append(const char* str, size_t len)
    // - append(const std::string& str)
    // - push_back(char c)
    // - clear()
    // - empty() const
    // - size() const
    // - c_str() const
    // - front() and back()
    // - reserve(size_t capacity)
    // - Constructor that takes capacity or default constructor
    // Note: Custom string types should provide these same interfaces as std::string
};

// Forward declarations
template<typename stringT> struct BasicConfig;
template<typename stringT, typename configT> struct GenericBuilder;
template<typename stringT, typename configT> struct GenericObject;
template<typename stringT, typename configT> struct GenericArray;

/// Basic configuration for JSON serialization
/// Provides static methods and compile-time constants for customization
template<typename stringT>
struct BasicConfig
{
    /// Whether to always escape string values in AddMember methods
    static constexpr bool kAlwaysEscape = false;
    
    /// Whether to quote numeric values by default
    static constexpr bool kQuoteNumber = false;
    
    /// Whether to allow trailing commas in arrays and objects
    static constexpr bool kTailComma = false;
    
    /// Escape object key (currently just copy without escaping)
    static void EscapeKey(stringT& dst, const char* key, size_t len)
    {
        dst.append(key, len);
    }
    
    /// Escape string value (calls EscapeString)
    static void EscapeValue(stringT& dst, const char* value, size_t len)
    {
        EscapeString(dst, value, len);
    }
    
    /// Escape string with single character
    static void EscapeString(stringT& dst, const char* src, size_t len, char ec = '"')
    {
        for (size_t i = 0; i < len; ++i)
        {
            char c = src[i];
            if (c == ec)
            {
                dst.push_back('\\');
            }
            dst.push_back(c);
        }
    }
    
    /// Escape string with multiple characters
    static void EscapeString(stringT& dst, const char* src, size_t len, const char* ecs)
    {
        for (size_t i = 0; i < len; ++i)
        {
            char c = src[i];
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
};

/// Generic json builder that works with different string types and configurations
/// Direct json string construction without DOM by any lib.
template<typename stringT, typename configT = BasicConfig<stringT>>
struct GenericBuilder
{
    stringT json;

    GenericBuilder(int capacity = 1024)
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
        if constexpr (configT::kTailComma)
        {
            json.push_back(']');
        }
        else
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
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddItem(numberT value)
    {
        if constexpr (configT::kQuoteNumber)
        {
            json.push_back('"');
            PutValue(value);
            json.push_back('"');
        }
        else
        {
            PutValue(value);
        }
        SepItem();
    }

    void AddItem(const std::string& value)
    {
        PutValue(value);
        SepItem();
    }

    /// Add item as string even if is_arithmetic_v.
    /// The bool parameter is only for overload resolution, always quotes the value.
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddItem(numberT value, bool)
    {
        (void)value; // Avoid unused parameter warning when optimized
        json.push_back('"');
        PutValue(value);
        json.push_back('"');
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
        if constexpr (configT::kTailComma)
        {
            json.push_back('}');
        }
        else
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
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddMember(const char* pszKey, numberT value)
    {
        PutKey(pszKey);
        if constexpr (configT::kQuoteNumber)
        {
            json.push_back('"');
            PutValue(value);
            json.push_back('"');
        }
        else
        {
            PutValue(value);
        }
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

    /// Add member as string even if is_arithmetic_v.
    /// The bool parameter is only for overload resolution, always quotes the value.
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddMember(const char* pszKey, numberT value, bool)
    {
        (void)value; // Avoid unused parameter warning when optimized
        PutKey(pszKey);
        json.push_back('"');
        PutValue(value);
        json.push_back('"');
        SepItem();
    }

    // String escaping support is now provided by configT::EscapeString

    void AddMemberEscape(const char* pszKey, const std::string& value, char ec = '"')
    {
        PutKey(pszKey);
        json.push_back('"');
        configT::EscapeString(json, value.c_str(), value.length(), ec);
        json.push_back('"');
        SepItem();
    }

    void AddMemberEscape(const char* pszKey, const std::string& value, const char* ecs)
    {
        PutKey(pszKey);
        json.push_back('"');
        configT::EscapeString(json, value.c_str(), value.length(), ecs);
        json.push_back('"');
        SepItem();
    }

    // Add item escape methods for consistency
    void AddItemEscape(const std::string& value, char ec = '"')
    {
        json.push_back('"');
        configT::EscapeString(json, value.c_str(), value.length(), ec);
        json.push_back('"');
        SepItem();
    }

    void AddItemEscape(const std::string& value, const char* ecs)
    {
        json.push_back('"');
        configT::EscapeString(json, value.c_str(), value.length(), ecs);
        json.push_back('"');
        SepItem();
    }

    void AddItemEscape(const char* value, size_t len, char ec = '"')
    {
        json.push_back('"');
        configT::EscapeString(json, value, len, ec);
        json.push_back('"');
        SepItem();
    }

    void AddItemEscape(const char* value, size_t len, const char* ecs)
    {
        json.push_back('"');
        configT::EscapeString(json, value, len, ecs);
        json.push_back('"');
        SepItem();
    }

    void AddItemEscape(const char* value, char ec = '"')
    {
        AddItemEscape(value, strlen(value), ec);
    }

    void AddItemEscape(const char* value, const char* ecs)
    {
        AddItemEscape(value, strlen(value), ecs);
    }

    void EndLine()
    {
        json.push_back('\n');
    }

    /// Reopen object {} to add more fields.
    void ReopenObject()
    {
        if (!json.empty() && json.back() == '}')
        {
            json.back() = ',';
        }
    }

    /// Merge two JSON object serialized strings.
    void Merge(const GenericBuilder<stringT, configT>& that)
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

    /// Static method version, merge two object {}
    static void MergeObject(stringT& self, const stringT& that)
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

    /// Create a scoped GenericArray that auto-closes when destroyed
    GenericArray<stringT, configT> ScopeArray(bool hasNext = false);
    
    /// Create a scoped GenericArray with key that auto-closes when destroyed
    GenericArray<stringT, configT> ScopeArray(const char* pszKey, bool hasNext = false);
    
    /// Create a scoped GenericObject that auto-closes when destroyed
    GenericObject<stringT, configT> ScopeObject(bool hasNext = false);
    
    /// Create a scoped GenericObject with key that auto-closes when destroyed
    GenericObject<stringT, configT> ScopeObject(const char* pszKey, bool hasNext = false);
};

/// Auto open and close object {}
template<typename stringT, typename configT>
struct GenericObject
{
private:
    GenericBuilder<stringT, configT>& m_builder;
    bool m_next;

public:
    GenericObject(GenericBuilder<stringT, configT>& build, bool hasNext = false) : m_builder(build), m_next(hasNext)
    {
        m_builder.BeginObject();
    }
    GenericObject(GenericBuilder<stringT, configT>& build, const char* pszKey, bool hasNext = false) : m_builder(build), m_next(hasNext)
    {
        m_builder.PutKey(pszKey);
        m_builder.BeginObject();
    }
    ~GenericObject()
    {
        m_builder.EndObject();
        if (m_next)
        {
            m_builder.SepItem();
        }
    }

    // Forward to builder method
    template <typename... Args>
    void AddMember(Args&&... args)
    {
        m_builder.AddMember(std::forward<Args>(args)...);
    }
};

/// Auto open and close array[]
template<typename stringT, typename configT>
struct GenericArray
{
private:
    GenericBuilder<stringT, configT>& m_builder;
    bool m_next;

public:
    GenericArray(GenericBuilder<stringT, configT>& build, bool hasNext = false) : m_builder(build), m_next(hasNext)
    {
        m_builder.BeginArray();
    }
    GenericArray(GenericBuilder<stringT, configT>& build, const char* pszKey, bool hasNext = false) : m_builder(build), m_next(hasNext)
    {
        m_builder.PutKey(pszKey);
        m_builder.BeginArray();
    }
    ~GenericArray()
    {
        m_builder.EndArray();
        if (m_next)
        {
            m_builder.SepItem();
        }
    }

    // Forward to builder method
    template <typename... Args>
    void AddItem(Args&&... args)
    {
        m_builder.AddItem(std::forward<Args>(args)...);
    }
};

/// Add scope methods to GenericBuilder
template<typename stringT, typename configT>
inline GenericArray<stringT, configT> GenericBuilder<stringT, configT>::ScopeArray(bool hasNext)
{
    return GenericArray<stringT, configT>(*this, hasNext);
}

template<typename stringT, typename configT>
inline GenericArray<stringT, configT> GenericBuilder<stringT, configT>::ScopeArray(const char* pszKey, bool hasNext)
{
    return GenericArray<stringT, configT>(*this, pszKey, hasNext);
}

template<typename stringT, typename configT>
inline GenericObject<stringT, configT> GenericBuilder<stringT, configT>::ScopeObject(bool hasNext)
{
    return GenericObject<stringT, configT>(*this, hasNext);
}

template<typename stringT, typename configT>
inline GenericObject<stringT, configT> GenericBuilder<stringT, configT>::ScopeObject(const char* pszKey, bool hasNext)
{
    return GenericObject<stringT, configT>(*this, pszKey, hasNext);
}

// Type aliases for backward compatibility and common usage
using RawBuilder = GenericBuilder<std::string>;
using RawObject = GenericObject<std::string, BasicConfig<std::string>>;
using RawArray = GenericArray<std::string, BasicConfig<std::string>>;

} /* end of wwjson:: */ 

#endif /* end of include guard: WWJSON_HPP__ */
