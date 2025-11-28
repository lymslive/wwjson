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
#include <array>
// for std::to_chars C++17
// #include <charconv>

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
    /// Whether to auto call EscapeKey method in the Builder api.
    static constexpr bool kEscapeKey = false;
    
    /// Whether to auto call EscapeString methodes in the Builder api.
    static constexpr bool kEscapeValue = false;
    
    /// Whether to quote numeric values by default.
    static constexpr bool kQuoteNumber = false;
    
    /// Whether to allow trailing commas in arrays and objects.
    static constexpr bool kTailComma = false;
    
    /// Escape table for ASCII characters (0-127), 0 means no escape needed.
    static constexpr auto kEscapeTable = []() constexpr {
        std::array<uint8_t, 128> table{}; // Default 0 means not escape
        
        table['\n'] = 'n';
        table['\t'] = 't';
        table['\r'] = 'r';
        table['"'] = '"';
        table['\\'] = '\\';
        table['\0'] = '0';
        
        return table;
    }();

    /// Escape object key (usually no escaping needed for keys).
    static void EscapeKey(stringT& dst, const char* key, size_t len)
    {
        dst.append(key, len);
    }
    
    /// Escape string using the compile-time escape table.
    static void EscapeString(stringT& dst, const char* src, size_t len)
    {
        // Pre-allocate memory to reduce reallocations
        dst.reserve(dst.size() + len + len / 4);
        
        for (size_t i = 0; i < len; ++i)
        {
            unsigned char c = static_cast<unsigned char>(src[i]);
            
            if (c >= 128)
            {
                // May UTF-8 byte stream, append directly
                dst.push_back(c);
            }
            else
            {
                // ASCII character, check escape table once
                uint8_t escape_char = kEscapeTable[c];
                if (escape_char != 0)
                {
                    dst.push_back('\\');
                    dst.push_back(escape_char);
                }
                else
                {
                    dst.push_back(c);
                }
            }
        }
    }

};

/// Generic json builder that works with different string types and configurations.
/// Direct json string construction without DOM tree as other libs.
template<typename stringT, typename configT = BasicConfig<stringT>>
struct GenericBuilder
{
    /// The JSON string being built.
    stringT json;

    /// M0: Basic Methods
    /* ---------------------------------------------------------------------- */
    
    /// Constructor with optional initial capacity.
    GenericBuilder(int capacity = 1024)
    {
        json.reserve(capacity);
    }

    /// M1: String Interface Wrapper Methods
    /* ---------------------------------------------------------------------- */
    
    /// Append a single character to JSON string.
    void PutChar(char c) { json.push_back(c); }
    
    /// Fix trailing character if it matches expected, replace with replacement.
    void FixTail(char expected, char replacement)
    {
        if (!json.empty() && json.back() == expected)
        {
            json.back() = replacement;
        }
        else
        {
            json.push_back(replacement);
        }
    }
    
    /// Append C-string to JSON string.
    void Append(const char* str) { json.append(str); }
    
    /// Append C-string with length to JSON string.
    void Append(const char* str, size_t len) { json.append(str, len); }
    
    /// Append std::string to JSON string.
    void Append(const std::string& str) { json.append(str); }
    
    /// Check if JSON string is empty.
    bool Empty() const { return json.empty(); }
    
    /// Get JSON string size.
    size_t Size() const { return json.size(); }
    
    /// Get reference to last character.
    char& Back() { return json.back(); }
    
    /// Get const reference to last character.
    const char& Back() const { return json.back(); }
    
    /// Get reference to first character.
    char& Front() { return json.front(); }
    
    /// Get const reference to first character.
    const char& Front() const { return json.front(); }
    
    /// Push back character (alias for PutChar).
    void PushBack(char c) { PutChar(c); }
    
    /// Clear JSON string.
    void Clear() { json.clear(); }

    /// M2: JSON Character-level Methods
    /* ---------------------------------------------------------------------- */
    
    /// Append comma separator.
    void PutNext() { PutChar(','); }

    /// Alias for PutNext().
    void SepItem() { PutNext(); }

    /// Begin array with key.
    void BeginArray(const char* pszKey)
    {
        PutKey(pszKey);
        BeginArray();
    }

    /// Begin array without key.
    void BeginArray() { PutChar('['); }

    /// End array, handling trailing comma based on config.
    void EndArray()
    {
        if constexpr (configT::kTailComma)
        {
            PutChar(']');
        }
        else
        {
            FixTail(',', ']');
        }
    }

    /// End array with additional separator. Suggest pass `true` but not used.
    void EndArray(bool /*hasNext*/)
    {
        EndArray();
        SepItem();
    }

    /// Append empty array.
    void EmptyArray() { Append("[]"); }

    /// Begin object with key.
    void BeginObject(const char* pszKey)
    {
        PutKey(pszKey);
        BeginObject();
    }

    /// Begin object without key.
    void BeginObject() { PutChar('{'); }

    /// End object, handling trailing comma based on config.
    void EndObject()
    {
        if constexpr (configT::kTailComma)
        {
            PutChar('}');
        }
        else
        {
            FixTail(',', '}');
        }
    }

    /// End object with additional separator. Suggest pass `true` but not used.
    void EndObject(bool /*hasNext*/)
    {
        EndObject();
        SepItem();
    }

    /// Append empty object.
    void EmptyObject() { Append("{}"); }

    /// Append newline character.
    void EndLine() { PutChar('\n'); }

    /// M3: JSON Scalar Value Methods
    /* ---------------------------------------------------------------------- */
    
    /// Append null value to JSON.
    void PutNull() { Append("null"); }

    /// Append null value to JSON (nullptr_t overload).
    void PutValue(std::nullptr_t) { PutNull(); }

    /// Append boolean value to JSON.
    void PutValue(bool tf)
    {
        if (tf)
        {
            Append("true");
        }
        else
        {
            Append("false");
        }
    }

    /// Append C-string value with length to JSON with quotes.
    void PutValue(const char* pszVal, size_t len)
    {
        PutChar('"');
        if constexpr (configT::kEscapeValue)
        {
            configT::EscapeString(json, pszVal, len);
        }
        else
        {
            Append(pszVal, len);
        }
        PutChar('"');
    }

    /// Append C-string value to JSON with quotes.
    void PutValue(const char* pszVal)
    {
        PutValue(pszVal, ::strlen(pszVal));
    }

    /// Append std::string value to JSON with quotes.
    void PutValue(const std::string& strValue)
    {
        PutValue(strValue.c_str(), strValue.length());
    }

    /// Append integral number value to JSON.
    template <typename numberT>
    std::enable_if_t<std::is_integral_v<numberT>, void>
    /*void*/ PutValue(numberT nValue)
    {
        // std::array<char, 32> buffer;
        // auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), nValue);
        // Append(buffer.data(), ptr - buffer.data());
        Append(std::to_string(nValue));
    }

    /// Append floating-point number value to JSON.
    template <typename numberT>
    std::enable_if_t<std::is_floating_point_v<numberT>, void>
    /*void*/ PutValue(numberT nValue)
    {
        Append(std::to_string(nValue));
    }
    
    /// Append object key with quotes and colon.
    void PutKey(const char* pszKey, size_t len)
    {
        PutChar('"');
        if constexpr (configT::kEscapeKey)
        {
            configT::EscapeKey(json, pszKey, len);
        }
        else
        {
            Append(pszKey, len);
        }
        PutChar('"');
        PutChar(':');
    }

    /// Append object key with quotes and colon.
    void PutKey(const char* pszKey)
    {
        PutKey(pszKey, ::strlen(pszKey));
    }

    /// Append object key with quotes and colon.
    void PutKey(const std::string& strKey)
    {
        PutKey(strKey.c_str(), strKey.length());
    }

    /// Append numeric value to JSON, handling quoting based on config.
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    PutNumber(numberT value)
    {
        if constexpr (configT::kQuoteNumber)
        {
            PutChar('"');
            PutValue(value);
            PutChar('"');
        }
        else
        {
            PutValue(value);
        }
    }

    /// M4: JSON Array Element Methods
    /* ---------------------------------------------------------------------- */
    
    /// Add numeric item to array.
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddItem(numberT value)
    {
        PutNumber(value);
        SepItem();
    }

    /// Add numeric item as quoted string to array.
    /// Suggest pass `true` as last argument but not used.
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddItem(numberT value, bool /*asString*/)
    {
        PutChar('"');
        PutValue(value);
        PutChar('"');
        SepItem();
    }

    /// Add string(and bool) item to array.
    template<typename... Args>
    void AddItem(Args&&... args)
    {
        PutValue(std::forward<Args>(args)...);
        SepItem();
    }

    /// M5: JSON Object Element Methods
    /* ---------------------------------------------------------------------- */
    
    /// Add numeric member to object.
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddMember(const char* pszKey, numberT value)
    {
        PutKey(pszKey);
        PutNumber(value);
        SepItem();
    }

    /// Add numeric member as quoted string to object.
    /// Suggest pass `true` as last argument but not used.
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddMember(const char* pszKey, numberT value, bool /*asString*/)
    {
        PutKey(pszKey);
        PutChar('"');
        PutValue(value);
        PutChar('"');
        SepItem();
    }

    /// Add string(and bool) member to object.
    template<typename... Args>
    void AddMember(const char* pszKey, Args&&... args)
    {
        PutKey(pszKey);
        PutValue(std::forward<Args>(args)...);
        SepItem();
    }

    /// M6: String Escaping Methods
    /* ---------------------------------------------------------------------- */

    /// Add C-string item with length after escaping.
    /// Use configed escape method.
    void AddItemEscape(const char* value, size_t len)
    {
        PutChar('"');
        configT::EscapeString(json, value, len);
        PutChar('"');
        SepItem();
    }

    /// Add C-string item after escaping.
    void AddItemEscape(const char* value)
    {
        AddItemEscape(value, ::strlen(value));
    }

    /// Add string item after escaping.
    void AddItemEscape(const std::string& value)
    {
        AddItemEscape(value.c_str(), value.length());
    }
    
    /// Add C-string member with length after escaping.
    void AddMemberEscape(const char* pszKey, const char* value, size_t len)
    {
        PutKey(pszKey);
        AddItemEscape(value, len);
    }

    /// Add C-string member after escaping.
    void AddMemberEscape(const char* pszKey, const char* value)
    {
        AddMemberEscape(pszKey, value, ::strlen(value));
    }

    /// Add string member after escaping.
    void AddMemberEscape(const char* pszKey, const std::string& value)
    {
        AddMemberEscape(pszKey, value.c_str(), value.length());
    }

    /// M7: Scope Creation Methods
    /* ---------------------------------------------------------------------- */
    
    /// Create a scoped GenericArray that auto-closes when destroyed.
    GenericArray<stringT, configT> ScopeArray(bool hasNext = false);
    
    /// Create a scoped GenericArray with key that auto-closes when destroyed.
    GenericArray<stringT, configT> ScopeArray(const char* pszKey, bool hasNext = false);
    
    /// Create a scoped GenericObject that auto-closes when destroyed.
    GenericObject<stringT, configT> ScopeObject(bool hasNext = false);
    
    /// Create a scoped GenericObject with key that auto-closes when destroyed.
    GenericObject<stringT, configT> ScopeObject(const char* pszKey, bool hasNext = false);

    /// M8: Advanced Methods
    /* ---------------------------------------------------------------------- */
    
    /// Reopen object {} or array [] to add more fields.
    bool Reopen()
    {
        if (Empty())
        {
            return false;
        }
        
        char lastChar = Back();
        if (lastChar == '}' || lastChar == ']')
        {
            Back() = ',';
            return true;
        }
        
        return false;
    }

    /// Merge two JSON serialized strings (objects or arrays).
    /// Simple algorithm: only checking closing/opening pairs, 
    /// change `*self}{that*` or `*self][that*` to `*self,that*` .
    bool Merge(const GenericBuilder<stringT, configT>& that)
    {
        if (Empty())
        {
            json = that.json;
            return true;
        }
        
        if (that.Empty())
        {
            return true;
        }
        
        char selfLast = Back();
        char thatFirst = that.Front();
        
        if ((selfLast == '}' && thatFirst == '{') || (selfLast == ']' && thatFirst == '['))
        {
            Back() = ',';
            Append(that.json.c_str() + 1, that.Size() - 1);
            return true;
        }
        
        return false;
    }

    /// Static method version, merge two objects {} or arrays [].
    /// Simple algorithm: only checking closing/opening pairs, 
    /// change `*self}{that*` or `*self][that*` to `*self,that*` .
    static bool Merge(stringT& self, const stringT& that)
    {
        if (self.empty())
        {
            self = that;
            return true;
        }
        
        if (that.empty())
        {
            return true;
        }
        
        char selfLast = self.back();
        char thatFirst = that.front();
        
        if ((selfLast == '}' && thatFirst == '{') || (selfLast == ']' && thatFirst == '['))
        {
            self.back() = ',';
            self.append(that.c_str() + 1, that.size() - 1);
            return true;
        }
        
        return false;
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
