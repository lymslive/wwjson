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

// Branch prediction macros for performance optimization
#ifndef wwjson_likely
    #if defined(__GNUC__) || defined(__clang__)
        #define wwjson_likely(x)     __builtin_expect(!!(x), 1)
        #define wwjson_unlikely(x)   __builtin_expect(!!(x), 0)
    #else
        #define wwjson_likely(x)     (x)
        #define wwjson_unlikely(x)   (x)
    #endif
#endif

#include <type_traits>
#include <string>
#include <string_view>
#include <array>
// for std::to_chars C++17
// #include <charconv>

#include <string.h>

namespace wwjson
{

/// Forward declarations for template classes.
template<typename stringT> struct BasicConfig;
template<typename stringT, typename configT> struct GenericBuilder;
template<typename stringT, typename configT> struct GenericObject;
template<typename stringT, typename configT> struct GenericArray;

/// Type trait to detect supported key types (const char*, std::string, std::string_view).
template<typename T> struct is_key : std::false_type {};
template<> struct is_key<const char*> : std::true_type {};
template<> struct is_key<char*> : std::true_type {};
template<> struct is_key<std::string> : std::true_type {};
template<> struct is_key<std::string_view> : std::true_type {};

template<typename T>
inline constexpr bool is_key_v = is_key<std::decay_t<T>>::value;

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
    /// Uses C/C++ standard escape sequences where applicable, others use '.' for non-printable chars.
    static constexpr auto kEscapeTable = []() constexpr {
        std::array<uint8_t, 128> table{}; // Default 0 means not escape
        
        // First set all non-printable control characters (0x01-0x1F) to '.'
        for (int i = 0x01; i <= 0x1F; ++i) {
            table[i] = '.';
        }
        
        // Then set specific C/C++ standard escape sequences
        table[0x00] = '0';  // \0 - null character
        table[0x07] = 'a';  // \a - bell (alert)
        table[0x08] = 'b';  // \b - backspace
        table[0x09] = 't';  // \t - horizontal tab
        table[0x0A] = 'n';  // \n - line feed (newline)
        table[0x0B] = 'v';  // \v - vertical tab
        table[0x0C] = 'f';  // \f - form feed
        table[0x0D] = 'r';  // \r - carriage return
        
        // Printable characters that need escaping in JSON
        table['"'] = '"';  // \" - double quote
        table['\\'] = '\\';  // \\ - backslash
        
        // DEL character (0x7F) - also non-printable
        table[0x7F] = '.';  // DEL - delete
        
        return table;
    }();

    /// Escape object key (usually no escaping needed for keys).
    static void EscapeKey(stringT& dst, const char* key, size_t len)
    {
        if (wwjson_unlikely(key == nullptr)) { return; }
        dst.append(key, len);
    }
    
    /// Escape string using the compile-time escape table.
    static void EscapeString(stringT& dst, const char* src, size_t len)
    {
        if (wwjson_unlikely(src == nullptr)) { return; }
        
        // Pre-allocate memory to reduce reallocations
        dst.reserve(dst.size() + len + len / 4);
        
        for (size_t i = 0; i < len; ++i)
        {
            unsigned char c = static_cast<unsigned char>(src[i]);
            
            if (wwjson_unlikely(c >= 128))
            {
                // May UTF-8 byte stream, append directly
                dst.push_back(c);
            }
            else
            {
                // ASCII character, check escape table once
                uint8_t escape_char = kEscapeTable[c];
                if (wwjson_unlikely(escape_char != 0))
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

    /// Type alias for self, useful in template metaprogramming
    using builder_type = GenericBuilder<stringT, configT>;

    /// M0: Basic Methods
    /* ---------------------------------------------------------------------- */
    
    /// Constructor with optional initial capacity.
    GenericBuilder(size_t capacity = 1024) { json.reserve(capacity); }

    /// Copy constructor.
    GenericBuilder(const GenericBuilder& other) = default;

    /// Move constructor.
    GenericBuilder(GenericBuilder&& other) noexcept = default;

    /// Get result string (const version).
    const stringT& GetResult() const { return json; }

    /// Get result string (non-const version, removes trailing comma if any).
    /// Always remove trailing comma from final result for valid JSON, Not
    /// consult configT::kTailComma.
    stringT& GetResult()
    {
        if (wwjson_unlikely(json.empty())) { return json; }
        if (json.back() == ',')
        {
            json.pop_back();
        }
        return json;
    }

    /// Move result string (transfer ownership).
    stringT&& MoveResult() { return std::move(GetResult()); }

    /// M1: String Interface Wrapper Methods
    /* ---------------------------------------------------------------------- */
    
    /// Append a single character to JSON string.
    void PutChar(char c) { json.push_back(c); }
    
    /// Fix trailing character if it matches expected, replace with replacement.
    void FixTail(char expected, char replacement)
    {
        if (wwjson_likely(!json.empty() && json.back() == expected))
        {
            json.back() = replacement;
        }
        else
        {
            json.push_back(replacement);
        }
    }
    
    /// Append C-string to JSON string.
    void Append(const char* str)
    {
        if (wwjson_unlikely(str == nullptr)) { return; }
        json.append(str);
    }
    
    /// Append C-string with length to JSON string.
    void Append(const char* str, size_t len)
    {
        if (wwjson_unlikely(str == nullptr)) { return; }
        json.append(str, len);
    }
    
    /// Append std::string to JSON string.
    void Append(const std::string& str) { json.append(str); }
    
    /// Check if JSON string is empty.
    bool Empty() const { return json.empty(); }
    
    /// Boolean conversion operator for if statement usage.
    operator bool() const { return !Empty(); }
    
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
    
    /// Append comma to separate the possible next item.
    void PutNext() { PutChar(','); }

    /// Alias for PutNext().
    void SepItem() { PutNext(); }

    /// Append a single char '[' as begin an array.
    /// Can used to begin root array, or nest array as item in parent array.
    void BeginArray() { PutChar('['); }

    /// Append string `"key":[`, to add a child array in it's parent object.
    template<typename keyT>
    std::enable_if_t<is_key_v<keyT>, void>
    BeginArray(keyT&& key)
    {
        PutKey(std::forward<keyT>(key));
        BeginArray();
    }

    /// End array, handling trailing comma based on config.
    // Always add comma after closing for consistency with AddItem.
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
        SepItem();
    }

    /// Append empty array `[]`.
    void EmptyArray() { Append("[]"); }

    /// Begin object without key.
    void BeginObject() { PutChar('{'); }

    /// Begin object with key.
    template<typename keyT>
    std::enable_if_t<is_key_v<keyT>, void>
    BeginObject(keyT&& key)
    {
        PutKey(std::forward<keyT>(key));
        BeginObject();
    }

    /// End object, handling trailing comma based on config.
    /// Always add comma after closing for consistency with AddItem.
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
        SepItem();
    }

    /// Append empty object.
    void EmptyObject() { Append("{}"); }

    /// Begin json root which default object, can pass `[` for array root.
    /// Note NOT check the open bracket, accpet any char.
    void BeginRoot(char bracket = '{') { PutChar(bracket); }

    /// End json root which default object without adding comma.
    /// Note NOT check the close bracket, accpet any char.
    void EndRoot(char bracket = '}')
    {
        if constexpr (configT::kTailComma)
        {
            PutChar(bracket);
        }
        else
        {
            FixTail(',', bracket);
        }
    }

    /// Append newline character.
    void EndLine() { PutChar('\n'); }

    /// M3: JSON Scalar Value and Low-level Methods
    /* ---------------------------------------------------------------------- */
    
    /// Append null value to JSON.
    void PutNull() { Append("null"); }

    /// Append null value to JSON (nullptr_t overload).
    void PutValue(std::nullptr_t) { PutNull(); }

    /// Append boolean value to JSON.
    void PutValue(bool tf)
    {
        if (wwjson_likely(tf))
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
        if (wwjson_unlikely(pszVal == nullptr)) { return; }
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
        if (wwjson_unlikely(pszVal == nullptr)) { return; }
        PutValue(pszVal, ::strlen(pszVal));
    }

    /// Append std::string value to JSON with quotes.
    void PutValue(const std::string& strValue)
    {
        PutValue(strValue.c_str(), strValue.length());
    }

    /// Append std::string_view value to JSON with quotes.
    void PutValue(const std::string_view& strValue)
    {
        PutValue(strValue.data(), strValue.length());
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
        if (wwjson_unlikely(pszKey == nullptr)) { return; }
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
        if (wwjson_unlikely(pszKey == nullptr)) { return; }
        PutKey(pszKey, ::strlen(pszKey));
    }

    /// Append object key with quotes and colon.
    void PutKey(const std::string& strKey)
    {
        PutKey(strKey.c_str(), strKey.length());
    }

    /// Append object key with quotes and colon.
    void PutKey(const std::string_view& strKey)
    {
        PutKey(strKey.data(), strKey.length());
    }

    /// Append JSON sub-string (raw JSON content) without quotes or escaping.
    /// User is responsible for ensuring the input is valid JSON.
    void PutSub(const char* pszSub, size_t len)
    {
        if (wwjson_unlikely(pszSub == nullptr)) { return; }
        Append(pszSub, len);
    }

    /// Append JSON sub-string (raw JSON content) without quotes or escaping.
    void PutSub(const char* pszSub)
    {
        if (wwjson_unlikely(pszSub == nullptr)) { return; }
        PutSub(pszSub, ::strlen(pszSub));
    }

    /// Append JSON sub-string (raw JSON content) without quotes or escaping.
    void PutSub(const std::string& strSub)
    {
        PutSub(strSub.c_str(), strSub.length());
    }

    /// Append JSON sub-string (raw JSON content) without quotes or escaping.
    void PutSub(const std::string_view& strSub)
    {
        PutSub(strSub.data(), strSub.length());
    }

    /// M4: JSON Array and Object Element Methods
    /* ---------------------------------------------------------------------- */
    
    /// Add numeric item to array.
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    /*void*/ AddItem(numberT value)
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

    /// Add item to array using callable function with GenericBuilder reference parameter.
    /// Would pass current builder to the provided function to produce a sub json.
    template<typename Func>
    std::enable_if_t<std::is_invocable_v<Func, builder_type&>, void>
    AddItem(Func&& func)
    {
        func(*this);
        if (wwjson_likely(!Empty()) && wwjson_unlikely(Back() != ','))
        {
            SepItem();
        }
    }

    /// Add item to array using callable function with no parameters.
    /// Usually use lambda captures current builder to produce sub json.
    template<typename Func>
    std::enable_if_t<std::is_invocable_v<Func> && !std::is_invocable_v<Func, builder_type&>, void>
    AddItem(Func&& func)
    {
        func();
        if (wwjson_likely(!Empty()) && wwjson_unlikely(Back() != ','))
        {
            SepItem();
        }
    }

    /// Add member to object with key and value (template key type).
    /// Note: The key not support (pszKey, len) argument, as len will match into args.
    template<typename keyT, typename... Args>
    std::enable_if_t<is_key_v<keyT>, void>
    AddMember(keyT&& key, Args&&... args)
    {
        PutKey(std::forward<keyT>(key));
        AddItem(std::forward<Args>(args)...);
    }

    /// M5: String Escaping Methods
    /* ---------------------------------------------------------------------- */

    /// Add C-string item with length after escaping.
    /// Use configed escape method.
    void AddItemEscape(const char* value, size_t len)
    {
        if (wwjson_unlikely(value == nullptr)) { return; }
        PutChar('"');
        configT::EscapeString(json, value, len);
        PutChar('"');
        SepItem();
    }

    /// Add C-string item after escaping.
    void AddItemEscape(const char* value)
    {
        if (wwjson_unlikely(value == nullptr)) { return; }
        AddItemEscape(value, ::strlen(value));
    }

    /// Add string item after escaping.
    void AddItemEscape(const std::string& value)
    {
        AddItemEscape(value.c_str(), value.length());
    }
    
    /// Add string_view item after escaping.
    void AddItemEscape(const std::string_view& value)
    {
        AddItemEscape(value.data(), value.length());
    }
    
    /// Add member to object with escaped string value.
    /// Handles any supported value types automatically.
    template<typename keyT, typename... Args>
    std::enable_if_t<is_key_v<keyT>, void>
    AddMemberEscape(keyT&& key, Args&&... args)
    {
        PutKey(std::forward<keyT>(key));
        AddItemEscape(std::forward<Args>(args)...);
    }

    /// M6: Special Member Functions and Operator Overloads
    /* ---------------------------------------------------------------------- */
    
    /// Copy assignment operator.
    GenericBuilder& operator=(const GenericBuilder& other) = default;

    /// Move assignment operator.
    GenericBuilder& operator=(GenericBuilder&& other) noexcept = default;

    /// Array element operator[] for integer indices.
    /// For array elements, no key setup needed, just returns *this for assignment.
    GenericBuilder& operator[](int /* index */)
    {
        return *this;
    }

    /// String key operator[] for key types.
    /// Sets up the key and returns *this for assignment.
    template<typename keyT>
    std::enable_if_t<is_key_v<keyT>, GenericBuilder&>
    operator[](keyT&& key)
    {
        PutKey(std::forward<keyT>(key));
        return *this;
    }

    /// Generic assignment operator that delegates to AddItem.
    /// Enables builder["key"] = value and builder[index] = value syntax.
    template<typename T>
    GenericBuilder& operator=(const T& value)
    {
        AddItem(value);
        return *this;
    }

    /// M7: Scope Creation Methods
    /* ---------------------------------------------------------------------- */
    
    /// Create a scoped GenericArray that auto-closes when destroyed.
    GenericArray<stringT, configT> ScopeArray();
    
    /// Create a scoped GenericArray with key that auto-closes when destroyed.
    template<typename keyT>
    std::enable_if_t<is_key_v<keyT>, GenericArray<stringT, configT>>
    ScopeArray(keyT&& key);
    
    /// Create a scoped GenericObject that auto-closes when destroyed.
    GenericObject<stringT, configT> ScopeObject();
    
    /// Create a scoped GenericObject with key that auto-closes when destroyed.
    template<typename keyT>
    std::enable_if_t<is_key_v<keyT>, GenericObject<stringT, configT>>
    ScopeObject(keyT&& key);

    /// M8: Advanced Methods
    /* ---------------------------------------------------------------------- */
    
    /// Reopen object {} or array [] to add more fields.
    bool Reopen()
    {
        if (wwjson_unlikely(Empty()))
        {
            return false;
        }
        
        char lastChar = Back();
        if (wwjson_likely(lastChar == '}' || lastChar == ']'))
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
        if (wwjson_unlikely(Empty()))
        {
            json = that.json;
            return true;
        }
        
        if (wwjson_unlikely(that.Empty()))
        {
            return true;
        }
        
        char selfLast = Back();
        char thatFirst = that.Front();
        
        if (wwjson_likely((selfLast == '}' && thatFirst == '{') || (selfLast == ']' && thatFirst == '[')))
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
        if (wwjson_unlikely(self.empty()))
        {
            self = that;
            return true;
        }
        
        if (wwjson_unlikely(that.empty()))
        {
            return true;
        }
        
        char selfLast = self.back();
        char thatFirst = that.front();
        
        if (wwjson_likely((selfLast == '}' && thatFirst == '{') || (selfLast == ']' && thatFirst == '[')))
        {
            self.back() = ',';
            self.append(that.c_str() + 1, that.size() - 1);
            return true;
        }
        
        return false;
    }

    /// Add JSON sub-string as array item without additional quotes or escaping.
    /// User is responsible for ensuring the input is valid JSON.
    template<typename... Args>
    void AddItemSub(Args&&... args)
    {
        PutSub(std::forward<Args>(args)...);
        SepItem();
    }

    /// Add member with JSON sub-string value without additional quotes or escaping.
    /// User is responsible for ensuring the input is valid JSON.
    template<typename keyT, typename... Args>
    std::enable_if_t<is_key_v<keyT>, void>
    AddMemberSub(keyT&& key, Args&&... args)
    {
        PutKey(std::forward<keyT>(key));
        AddItemSub(std::forward<Args>(args)...);
    }
};

/// Auto open and close array[].
template<typename stringT, typename configT>
struct GenericArray
{
private:
    /// Reference to the builder.
    GenericBuilder<stringT, configT>& m_builder;

public:
    /// Constructor without key.
    GenericArray(GenericBuilder<stringT, configT>& build) : m_builder(build)
    {
        m_builder.BeginArray();
    }

    /// Constructor with key.
    template<typename keyT>
    GenericArray(GenericBuilder<stringT, configT>& build, keyT&& key) : m_builder(build)
    {
        m_builder.PutKey(std::forward<keyT>(key));
        m_builder.BeginArray();
    }

    /// Destructor auto-closes array.
    ~GenericArray() { m_builder.EndArray(); }

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

    /// Forward to builder AddItemSub method.
    template <typename... Args>
    void AddItemSub(Args&&... args)
    {
        m_builder.AddItemSub(std::forward<Args>(args)...);
    }

    /// Array element operator[] for integer indices.
    /// For array elements, no key setup needed, just returns m_builder for assignment.
    GenericBuilder<stringT, configT>& operator[](int /* index */)
    {
        return m_builder;
    }

    /// Stream operator<< for chained array element addition.
    /// Enables syntax: arr << v1 << v2 << v3
    template<typename T>
    GenericArray& operator<<(const T& value)
    {
        AddItem(value);
        return *this;
    }

    /// Create a scoped GenericArray that auto-closes when destroyed.
    template<typename... Args>
    auto ScopeArray(Args&&... args)
    {
        return m_builder.ScopeArray(std::forward<Args>(args)...);
    }
    
    /// Create a scoped GenericObject that auto-closes when destroyed.
    template<typename... Args>
    auto ScopeObject(Args&&... args)
    {
        return m_builder.ScopeObject(std::forward<Args>(args)...);
    }
    
    /// Boolean conversion operator for if statement usage.
    /// Always returns true since constructor adds '['.
    constexpr operator bool() const { return true; }
};

/// Auto open and close object {}.
template<typename stringT, typename configT>
struct GenericObject
{
private:
    /// Reference to the builder.
    GenericBuilder<stringT, configT>& m_builder;

public:
    /// Constructor without key.
    GenericObject(GenericBuilder<stringT, configT>& build) : m_builder(build)
    {
        m_builder.BeginObject();
    }

    /// Constructor with key.
    template<typename keyT>
    GenericObject(GenericBuilder<stringT, configT>& build, keyT&& key) : m_builder(build)
    {
        m_builder.PutKey(std::forward<keyT>(key));
        m_builder.BeginObject();
    }

    /// Destructor auto-closes object.
    ~GenericObject() { m_builder.EndObject(); }

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

    /// Forward to builder AddMemberSub method.
    template <typename... Args>
    void AddMemberSub(Args&&... args)
    {
        m_builder.AddMemberSub(std::forward<Args>(args)...);
    }

    /// String key operator[] for key types.
    /// Sets up the key and returns m_builder for assignment.
    template<typename keyT>
    std::enable_if_t<is_key_v<keyT>, GenericBuilder<stringT, configT>&>
    operator[](keyT&& key)
    {
        m_builder.PutKey(std::forward<keyT>(key));
        return m_builder;
    }

    /// Stream operator<< for key types when expecting a key.
    /// Checks the last character in builder to determine if expecting key or value.
    template<typename keyT>
    std::enable_if_t<is_key_v<keyT>, GenericObject&>
    operator<<(keyT&& key)
    {
        if (m_builder.Back() != ':')
        {
            m_builder.PutKey(std::forward<keyT>(key));
        }
        else
        {
            // When expecting value, delegate to the value handler
            m_builder.AddItem(std::forward<keyT>(key));
        }
        return *this;
    }
    
    /// Stream operator<< for values (non-string types).
    template<typename T>
    std::enable_if_t<!is_key_v<T>, GenericObject&>
    operator<<(const T& value)
    {
        m_builder.AddItem(value);
        return *this;
    }

    /// Create a scoped GenericArray that auto-closes when destroyed.
    template<typename... Args>
    auto ScopeArray(Args&&... args)
    {
        return m_builder.ScopeArray(std::forward<Args>(args)...);
    }
    
    /// Create a scoped GenericObject that auto-closes when destroyed.
    template<typename... Args>
    auto ScopeObject(Args&&... args)
    {
        return m_builder.ScopeObject(std::forward<Args>(args)...);
    }
    
    /// Boolean conversion operator for if statement usage.
    /// Always returns true since constructor adds '{'.
    constexpr operator bool() const { return true; }
};

/// Add scope methods to GenericBuilder.
template<typename stringT, typename configT>
inline GenericArray<stringT, configT> GenericBuilder<stringT, configT>::ScopeArray()
{
    return GenericArray<stringT, configT>(*this);
}

template<typename stringT, typename configT>
template<typename keyT>
inline std::enable_if_t<is_key_v<keyT>, GenericArray<stringT, configT>>
GenericBuilder<stringT, configT>::ScopeArray(keyT&& key)
{
    return GenericArray<stringT, configT>(*this, std::forward<keyT>(key));
}

template<typename stringT, typename configT>
inline GenericObject<stringT, configT> GenericBuilder<stringT, configT>::ScopeObject()
{
    return GenericObject<stringT, configT>(*this);
}

template<typename stringT, typename configT>
template<typename keyT>
inline std::enable_if_t<is_key_v<keyT>, GenericObject<stringT, configT>>
GenericBuilder<stringT, configT>::ScopeObject(keyT&& key)
{
    return GenericObject<stringT, configT>(*this, std::forward<keyT>(key));
}

/// Type aliases for backward compatibility and common usage.
using RawBuilder = GenericBuilder<std::string>;
using RawObject = GenericObject<std::string, BasicConfig<std::string>>;
using RawArray = GenericArray<std::string, BasicConfig<std::string>>;

} /* end of wwjson:: */ 

#endif /* end of include guard: WWJSON_HPP__ */
