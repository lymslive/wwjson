/**
 * @file wwjson.hpp
 * @author lymslive
 * @date 2025-11-21
 * @brief Construrct json in raw string simply and quickly.
 *
 * @details WWJSON is a header-only C++ library for fast JSON string building without DOM tree construction.
 * It provides a simple, lightweight way to construct JSON strings directly through a builder pattern.
 * The library focuses on JSON serialization (building), not parsing, with high performance through direct string manipulation.
 * Supports nested objects and arrays with RAII-based scope management and is compatible with custom string types.
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

/// String concept struct to document required interfaces for custom string types.
/// Although C++17 doesn't support concepts, this serves as documentation.
struct StringConcept
{
    // Required interfaces that a custom string type must provide:
    // - append(const char* str)
    // - append(const char* str, size_t len)
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

/// Forward declarations for template classes.
template<typename stringT> struct BasicConfig;
template<typename stringT, typename configT> struct GenericBuilder;
template<typename stringT, typename configT> struct GenericObject;
template<typename stringT, typename configT> struct GenericArray;

/// Default escape characters constant.
constexpr const char* DEFAULT_ESCAPE_CHARS = "\\\n\t\r\"\0";

/// Basic configuration for JSON serialization.
/// Provides static methods and compile-time constants for customization.
template<typename stringT>
struct BasicConfig
{
    /// Whether to always escape string values in AddMember methods.
    static constexpr bool kAlwaysEscape = false;
    
    /// Whether to quote numeric values by default.
    static constexpr bool kQuoteNumber = false;
    
    /// Whether to allow trailing commas in arrays and objects.
    static constexpr bool kTailComma = false;
    
    /// Escape a specific single character `ec` in string `src`, write to `dst`.
    /// eg: only need to escpae '"' when sub json as a string field.
    /// Note: simple algorithm by inserting back slash, so not suitable for
    /// escaping \n \t ect.
    static void EscapeString(stringT& dst, const char* src, size_t len, char ec)
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
    
    /// Escape each character of `ecs` in string `src`, write to `dst`.
    static void EscapeString(stringT& dst, const char* src, size_t len, const char* ecs = DEFAULT_ESCAPE_CHARS)
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

    /// Escape object key with single character (default NO escaping).
    static void EscapeKey(stringT& dst, const char* key, size_t len, char ec)
    {
        dst.append(key, len);
    }

    /// Escape object key with multiple characters (default NO escaping).
    static void EscapeKey(stringT& dst, const char* key, size_t len, const char* ecs = DEFAULT_ESCAPE_CHARS)
    {
        dst.append(key, len);
    }

};

/// Generic json builder that works with different string types and configurations.
/// Direct json string construction without DOM tree as other libs.
template<typename stringT, typename configT = BasicConfig<stringT>>
struct GenericBuilder
{
    /// The JSON string being built.
    stringT json;

    /// Constructor with optional initial capacity.
    GenericBuilder(int capacity = 1024)
    {
        json.reserve(capacity);
    }

    /// Append null value to JSON.
    void PutNull()
    {
        json.append("null");
    }

    /// Append boolean value to JSON.
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

    /// Append C-string value to JSON with quotes.
    void PutValue(const char* pszVal)
    {
        json.push_back('"');
        if constexpr (configT::kAlwaysEscape)
        {
            configT::EscapeString(json, pszVal, strlen(pszVal), DEFAULT_ESCAPE_CHARS);
        }
        else
        {
            json.append(pszVal);
        }
        json.push_back('"');
    }

    /// Append C-string value with length to JSON with quotes.
    void PutValue(const char* pszVal, size_t len)
    {
        json.append("\"");
        if constexpr (configT::kAlwaysEscape)
        {
            configT::EscapeString(json, pszVal, len, DEFAULT_ESCAPE_CHARS);
        }
        else
        {
            json.append(pszVal, len);
        }
        json.append("\"");
    }

    /// Append std::string value to JSON with quotes.
    void PutValue(const std::string& strValue)
    {
        json.push_back('"');
        if constexpr (configT::kAlwaysEscape)
        {
            configT::EscapeString(json, strValue.c_str(), strValue.length(), DEFAULT_ESCAPE_CHARS);
        }
        else
        {
            json.append(strValue);
        }
        json.push_back('"');
    }

    /// Append integral number value to JSON.
    template <typename numberT>
    std::enable_if_t<std::is_integral_v<numberT>, void>
    /*void*/ PutValue(numberT nValue)
    {
        // std::array<char, 32> buffer;
        // auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), nValue);
        // json.append(buffer.data(), ptr);
        json.append(std::to_string(nValue));
    }

    /// Append floating-point number value to JSON.
    template <typename numberT>
    std::enable_if_t<std::is_floating_point_v<numberT>, void>
    /*void*/ PutValue(numberT nValue)
    {
        json.append(std::to_string(nValue));
    }

    /// Append comma separator.
    void PutComma()
    {
        json.push_back(',');
    }

    /// Alias for PutComma().
    void SepItem() { PutComma(); }

    /// Begin array with key.
    void BeginArray(const char* pszKey)
    {
        PutKey(pszKey);
        BeginArray();
    }

    /// Begin array without key.
    void BeginArray()
    {
        json.push_back('[');
    }

    /// End array, handling trailing comma based on config.
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

    /// End array with additional separator. Suggest pass `true` but not used.
    void EndArray(bool /*hasNext*/)
    {
        EndArray();
        SepItem();
    }

    /// Append empty array.
    void EmptyArray()
    {
        json.append("[]");
    }

    /// Add numeric item to array.
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

    /// Add string item to array.
    void AddItem(const std::string& value)
    {
        PutValue(value);
        SepItem();
    }

    /// Add C-string item to array.
    void AddItem(const char* value)
    {
        PutValue(value);
        SepItem();
    }

    /// Add C-string item with length to array.
    void AddItem(const char* value, size_t len)
    {
        PutValue(value, len);
        SepItem();
    }

    /// Add numeric item as quoted string to array.
    /// Suggest pass `true` as last argument but not used.
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddItem(numberT value, bool /*asString*/)
    {
        json.push_back('"');
        PutValue(value);
        json.push_back('"');
        SepItem();
    }

    /// Begin object with key.
    void BeginObject(const char* pszKey)
    {
        PutKey(pszKey);
        BeginObject();
    }

    /// Begin object without key.
    void BeginObject()
    {
        json.push_back('{');
    }

    /// End object, handling trailing comma based on config.
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

    /// End object with additional separator. Suggest pass `true` but not used.
    void EndObject(bool /*hasNext*/)
    {
        EndObject();
        SepItem();
    }

    /// Append empty object.
    void EmptyObject()
    {
        json.append("{}");
    }

    /// Append object key with quotes and colon.
    void PutKey(const char* pszKey)
    {
        json.push_back('"');
        if constexpr (configT::kAlwaysEscape)
        {
            configT::EscapeKey(json, pszKey, strlen(pszKey), DEFAULT_ESCAPE_CHARS);
        }
        else
        {
            json.append(pszKey);
        }
        json.append("\":");
    }

    /// Add numeric member to object.
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

    /// Add string member to object.
    void AddMember(const char* pszKey, const std::string& value)
    {
        PutKey(pszKey);
        PutValue(value);
        SepItem();
    }

    /// Add C-string member to object.
    void AddMember(const char* pszKey, const char* value)
    {
        PutKey(pszKey);
        PutValue(value);
        SepItem();
    }

    /// Add C-string member with length to object.
    void AddMember(const char* pszKey, const char* value, size_t len)
    {
        PutKey(pszKey);
        PutValue(value,len);
        SepItem();
    }

    /// Add numeric member as quoted string to object.
    /// Suggest pass `true` as last argument but not used.
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddMember(const char* pszKey, numberT value, bool /*asString*/)
    {
        PutKey(pszKey);
        json.push_back('"');
        PutValue(value);
        json.push_back('"');
        SepItem();
    }

    /// Add member with single character escaping.
    void AddMemberEscape(const char* pszKey, const std::string& value, char ec)
    {
        PutKey(pszKey);
        json.push_back('"');
        configT::EscapeString(json, value.c_str(), value.length(), ec);
        json.push_back('"');
        SepItem();
    }

    /// Add C-string member with single character escaping.
    void AddMemberEscape(const char* pszKey, const char* value, char ec)
    {
        PutKey(pszKey);
        json.push_back('"');
        configT::EscapeString(json, value, strlen(value), ec);
        json.push_back('"');
        SepItem();
    }

    /// Add C-string member with length and single character escaping.
    void AddMemberEscape(const char* pszKey, const char* value, size_t len, char ec)
    {
        PutKey(pszKey);
        json.push_back('"');
        configT::EscapeString(json, value, len, ec);
        json.push_back('"');
        SepItem();
    }

    /// Add member with multiple character escaping.
    void AddMemberEscape(const char* pszKey, const std::string& value, const char* ecs = DEFAULT_ESCAPE_CHARS)
    {
        PutKey(pszKey);
        json.push_back('"');
        configT::EscapeString(json, value.c_str(), value.length(), ecs);
        json.push_back('"');
        SepItem();
    }

    /// Add C-string member with multiple character escaping.
    void AddMemberEscape(const char* pszKey, const char* value, const char* ecs = DEFAULT_ESCAPE_CHARS)
    {
        PutKey(pszKey);
        json.push_back('"');
        configT::EscapeString(json, value, strlen(value), ecs);
        json.push_back('"');
        SepItem();
    }

    /// Add C-string member with length and multiple character escaping.
    void AddMemberEscape(const char* pszKey, const char* value, size_t len, const char* ecs = DEFAULT_ESCAPE_CHARS)
    {
        PutKey(pszKey);
        json.push_back('"');
        configT::EscapeString(json, value, len, ecs);
        json.push_back('"');
        SepItem();
    }

    /// Add item with single character escaping.
    void AddItemEscape(const std::string& value, char ec)
    {
        json.push_back('"');
        configT::EscapeString(json, value.c_str(), value.length(), ec);
        json.push_back('"');
        SepItem();
    }

    /// Add item with multiple character escaping.
    void AddItemEscape(const std::string& value, const char* ecs = DEFAULT_ESCAPE_CHARS)
    {
        json.push_back('"');
        configT::EscapeString(json, value.c_str(), value.length(), ecs);
        json.push_back('"');
        SepItem();
    }

    /// Add C-string item with length and single character escaping.
    void AddItemEscape(const char* value, size_t len, char ec)
    {
        json.push_back('"');
        configT::EscapeString(json, value, len, ec);
        json.push_back('"');
        SepItem();
    }

    /// Add C-string item with length and multiple character escaping.
    void AddItemEscape(const char* value, size_t len, const char* ecs = DEFAULT_ESCAPE_CHARS)
    {
        json.push_back('"');
        configT::EscapeString(json, value, len, ecs);
        json.push_back('"');
        SepItem();
    }

    /// Add C-string item with single character escaping.
    void AddItemEscape(const char* value, char ec)
    {
        AddItemEscape(value, strlen(value), ec);
    }

    /// Add C-string item with multiple character escaping.
    void AddItemEscape(const char* value, const char* ecs = DEFAULT_ESCAPE_CHARS)
    {
        AddItemEscape(value, strlen(value), ecs);
    }

    /// Append newline character.
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

    /// Static method version, merge two object {}.
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

    /// Create a scoped GenericArray that auto-closes when destroyed.
    GenericArray<stringT, configT> ScopeArray(bool hasNext = false);
    
    /// Create a scoped GenericArray with key that auto-closes when destroyed.
    GenericArray<stringT, configT> ScopeArray(const char* pszKey, bool hasNext = false);
    
    /// Create a scoped GenericObject that auto-closes when destroyed.
    GenericObject<stringT, configT> ScopeObject(bool hasNext = false);
    
    /// Create a scoped GenericObject with key that auto-closes when destroyed.
    GenericObject<stringT, configT> ScopeObject(const char* pszKey, bool hasNext = false);
};

/// Auto open and close object {}.
template<typename stringT, typename configT>
struct GenericObject
{
private:
    /// Reference to the builder.
    GenericBuilder<stringT, configT>& m_builder;
    /// Whether to add separator after closing.
    bool m_next;

public:
    /// Constructor without key.
    GenericObject(GenericBuilder<stringT, configT>& build, bool hasNext = false) : m_builder(build), m_next(hasNext)
    {
        m_builder.BeginObject();
    }

    /// Constructor with key.
    GenericObject(GenericBuilder<stringT, configT>& build, const char* pszKey, bool hasNext = false) : m_builder(build), m_next(hasNext)
    {
        m_builder.PutKey(pszKey);
        m_builder.BeginObject();
    }

    /// Destructor auto-closes object.
    ~GenericObject()
    {
        m_builder.EndObject();
        if (m_next)
        {
            m_builder.SepItem();
        }
    }

    /// Forward to builder method.
    template <typename... Args>
    void AddMember(Args&&... args)
    {
        m_builder.AddMember(std::forward<Args>(args)...);
    }

    /// Forward to builder AddMemberEscape method.
    template <typename... Args>
    void AddMemberEscape(Args&&... args)
    {
        m_builder.AddMemberEscape(std::forward<Args>(args)...);
    }
};

/// Auto open and close array[].
template<typename stringT, typename configT>
struct GenericArray
{
private:
    /// Reference to the builder.
    GenericBuilder<stringT, configT>& m_builder;
    /// Whether to add separator after closing.
    bool m_next;

public:
    /// Constructor without key.
    GenericArray(GenericBuilder<stringT, configT>& build, bool hasNext = false) : m_builder(build), m_next(hasNext)
    {
        m_builder.BeginArray();
    }

    /// Constructor with key.
    GenericArray(GenericBuilder<stringT, configT>& build, const char* pszKey, bool hasNext = false) : m_builder(build), m_next(hasNext)
    {
        m_builder.PutKey(pszKey);
        m_builder.BeginArray();
    }

    /// Destructor auto-closes array.
    ~GenericArray()
    {
        m_builder.EndArray();
        if (m_next)
        {
            m_builder.SepItem();
        }
    }

    /// Forward to builder method.
    template <typename... Args>
    void AddItem(Args&&... args)
    {
        m_builder.AddItem(std::forward<Args>(args)...);
    }

    /// Forward to builder AddItemEscape method.
    template <typename... Args>
    void AddItemEscape(Args&&... args)
    {
        m_builder.AddItemEscape(std::forward<Args>(args)...);
    }
};

/// Add scope methods to GenericBuilder.
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

/// Type aliases for backward compatibility and common usage.
using RawBuilder = GenericBuilder<std::string>;
using RawObject = GenericObject<std::string, BasicConfig<std::string>>;
using RawArray = GenericArray<std::string, BasicConfig<std::string>>;

} /* end of wwjson:: */ 

#endif /* end of include guard: WWJSON_HPP__ */
