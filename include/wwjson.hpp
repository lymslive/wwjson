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
#include <charconv>
#include <cmath>

#include <string.h>
#include <stdint.h>

/// High precision floating-point serialization control.
/// Define this macro to use simple %g format (shorter but less precise).
/// Default is high precision %.17g for better accuracy.
/// Note: This setting is ignored if std::to_chars is available for floating-point types.
#ifndef WWJSON_USE_SIMPLE_FLOAT_FORMAT
    #define WWJSON_USE_SIMPLE_FLOAT_FORMAT 0
#endif

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

namespace detail
{

template <typename T>
using has_fp_to_chars = decltype(
    std::to_chars(std::declval<char*>(), std::declval<char*>(), std::declval<T>()
    //  , std::declval<std::chars_format>()
    )
);

template <typename T, typename = void>
struct supports_to_chars_float : std::false_type {};

template <typename T>
struct supports_to_chars_float<T, std::void_t<has_fp_to_chars<T>>>
    : std::true_type {};

} // namespace detail

/// Detect if std::to_chars supports floating-point types.
template <typename T>
constexpr bool has_float_to_chars_v =
    std::is_floating_point_v<std::remove_cv_t<T>> &&
    detail::supports_to_chars_float<std::remove_cv_t<T>>::value;

#if WWJSON_USE_SIMPLE_FLOAT_FORMAT
constexpr bool use_simple_float_format = !has_float_to_chars_v<double>;
#else
constexpr bool use_simple_float_format = false;
#endif

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

/// High-performance number writer for integer types.
/// Uses small integer caching strategy for fast serialization.
template<typename stringT>
struct NumberWriter
{
    /// Structure to store a pair of digit characters.
    struct DigitPair {
        char high;
        char low;
        constexpr DigitPair() : high('0'), low('0') {}
        constexpr DigitPair(char h, char l) : high(h), low(l) {}
    };
    
    /// 0-99 digit pairs for fast lookup.
    alignas(64) static constexpr std::array<DigitPair, 100> kDigitPairs = []() {
        std::array<DigitPair, 100> table{};
        for (int i = 0; i < 100; ++i) {
            table[i] = DigitPair('0' + (i / 10), '0' + (i % 10));
        }
        return table;
    }();
    
    /// Write small integers (0-9999) to output string.
    template<typename intT>
    static inline std::enable_if_t<std::is_integral_v<intT>, void>
    WriteSmall(stringT& dst, intT value)
    {
        if (value < 100) {
            if (value < 10) {
                dst.push_back(static_cast<char>('0' + value));
            } else {
                const DigitPair& pair = kDigitPairs[static_cast<std::size_t>(value)];
                dst.push_back(pair.high);
                dst.push_back(pair.low);
            }
            return;
        }
        
        if (value < 1000) {
            dst.push_back(static_cast<char>('0' + (value / 100)));
            value %= 100;
            const DigitPair& pair = kDigitPairs[static_cast<std::size_t>(value)];
            dst.push_back(pair.high);
            dst.push_back(pair.low);
            return;
        }
        
        // 4 digits (1000-9999)
        uint32_t q = static_cast<uint32_t>(value / 100);
        uint32_t r = static_cast<uint32_t>(value % 100);
        const DigitPair& pair_q = kDigitPairs[q];
        const DigitPair& pair_r = kDigitPairs[r];
        dst.push_back(pair_q.high);
        dst.push_back(pair_q.low);
        dst.push_back(pair_r.high);
        dst.push_back(pair_r.low);
    }
    
    template<typename intT>
    static std::enable_if_t<std::is_integral_v<intT>, void>
    WriteUnsigned(stringT& dst, intT value)
    {
        if (value < 10000) {
            return WriteSmall(dst, value);
        }
        
        constexpr int max_len = std::numeric_limits<intT>::digits10 + 1;
        char buffer[max_len];
        char* const buffer_end = buffer + max_len;
        char* ptr = buffer_end;
        
        while (value >= 10000) {
            uint32_t chunk = static_cast<uint32_t>(value % 10000);
            value /= 10000;
            
            uint32_t high = chunk / 100;
            uint32_t low = chunk % 100;
            
            // Use value is bit faster than reference here, after testing
            DigitPair pair_low = kDigitPairs[low];
            DigitPair pair_high = kDigitPairs[high];
            
            *(--ptr) = pair_low.low;
            *(--ptr) = pair_low.high;
            *(--ptr) = pair_high.low;
            *(--ptr) = pair_high.high;
        }
        
        WriteSmall(dst, value);
        dst.append(ptr, buffer_end - ptr);
    }
    
    template<typename intT>
    static std::enable_if_t<std::is_integral_v<intT>, void>
    WriteSigned(stringT& dst, intT value)
    {
        using UnsignedT = std::make_unsigned_t<intT>;
        
        if (value < 0) {
            dst.push_back('-');
            WriteUnsigned(dst, static_cast<UnsignedT>(-value));
        } else {
            WriteUnsigned(dst, static_cast<UnsignedT>(value));
        }
    }
    
    /// Write small floating-point numbers efficiently.
    /// Returns true if the number was handled, false if fallback is needed.
    static bool WriteSmall(stringT& dst, double value)
    {
        constexpr double max_precise_double = 9007199254740992.0;
        if (wwjson_unlikely(value > max_precise_double)) {
            return false;
        }
        
        uint64_t integer_part = static_cast<uint64_t>(value);
        double fractional_part = value - static_cast<double>(integer_part);
        if (fractional_part == 0.0) {
            WriteUnsigned(dst, integer_part);
            return true;
        }
        
        // Multiply fractional part by 10000 and check if it's an integer
        double scaled_fractional = fractional_part * 10000.0;
        uint64_t scaled_int = static_cast<uint64_t>(scaled_fractional + 0.5);
        double error_check = scaled_fractional - static_cast<double>(scaled_int);
        double tolerance = 1.0e-8; // allow some tolerace after scaled
        if (std::abs(error_check) > tolerance) {
            return false; 
        }
        
        if (scaled_int == 10000) {
            ++integer_part;
            WriteUnsigned(dst, integer_part);
            return true;
        }

        // Now we have a valid fixed-point number with at most 4 decimal places
        WriteUnsigned(dst, integer_part);
        dst.push_back('.');
        
        uint32_t frac_part = static_cast<uint32_t>(scaled_int);
        uint32_t q = static_cast<uint32_t>(frac_part / 100);
        uint32_t r = static_cast<uint32_t>(frac_part % 100);
        const DigitPair& pair_q = kDigitPairs[q];
        const DigitPair& pair_r = kDigitPairs[r];

        if (r == 0) {
            dst.push_back(pair_q.high);
            if (pair_q.low != '0') {
                dst.push_back(pair_q.low);
            }
        }
        else {
            dst.push_back(pair_q.high);
            dst.push_back(pair_q.low);
            if (pair_r.low != '0') {
                dst.push_back(pair_r.high);
                dst.push_back(pair_r.low);
            }
            else {
                dst.push_back(pair_r.high);
            }
        }

        return true;
    }

    /// Converts integer values to their string representation.
    template<typename intT>
    static std::enable_if_t<std::is_integral_v<intT>, void> 
    Output(stringT& dst, intT value)
    {
        if constexpr (std::is_signed_v<intT>) {
            WriteSigned(dst, value);
        } else {
            WriteUnsigned(dst, value);
        }
    }
    
    /// Converts floating-point values to their string representation.
    /// NaN and Infinity are output as "null" (JSON has no representation for these values).
    template<typename floatT>
    static std::enable_if_t<std::is_floating_point_v<floatT>, void>
    Output(stringT& dst, floatT value)
    {
        if (wwjson_unlikely(std::isnan(value))) {
            dst.append("null", 4);
            return;
        }
        if (wwjson_unlikely(std::isinf(value))) {
            dst.append("null", 4);
            return;
        }
        
        if (value < 0) {
            dst.push_back('-');
            value = -value;
        }
        
        // Try optimized path for small fixed-point numbers
        if (WriteSmall(dst, value)) {
            return;
        }
        
        // --- Normal path for regular numbers ---
        if constexpr (has_float_to_chars_v<floatT>) {
            char buffer[256];
            auto result = std::to_chars(buffer, buffer + sizeof(buffer), value); // , std::chars_format::general
            if (result.ec == std::errc{}) {
                dst.append(buffer, static_cast<size_t>(result.ptr - buffer));
                return;
            }
        }
        
        // --- Fallback path without std::to_chars ---
        char buffer[256];
        int len = 0;
        
#if WWJSON_USE_SIMPLE_FLOAT_FORMAT
        // Simple format - shorter output
        len = std::snprintf(buffer, sizeof(buffer), "%g", value);
#else
        // High precision format - more accurate
        if constexpr (std::is_same_v<floatT, float>) {
            len = std::snprintf(buffer, sizeof(buffer), "%.9g", value);
        } else if constexpr (std::is_same_v<floatT, double>) {
            len = std::snprintf(buffer, sizeof(buffer), "%.17g", value);
        } else if constexpr (std::is_same_v<floatT, long double>) {
            len = std::snprintf(buffer, sizeof(buffer), "%.21Lg", value);
        }
#endif
        
        if (wwjson_likely(len > 0 && static_cast<size_t>(len) < sizeof(buffer))) {
            dst.append(buffer, static_cast<size_t>(len));
            return;
        }
        
        dst.append(std::to_string(value));
    }
};

/// Configuration for JSON serialization with customizable options.
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
        std::array<uint8_t, 128> table{};
        
        // Set non-printable control characters (0x01-0x1F) to '.'
        for (int i = 0x01; i <= 0x1F; ++i) {
            table[i] = '.';
        }
        
        // C/C++ standard escape sequences
        table[0x00] = '0';  // \0
        table[0x07] = 'a';  // \a
        table[0x08] = 'b';  // \b
        table[0x09] = 't';  // \t
        table[0x0A] = 'n';  // \n
        table[0x0B] = 'v';  // \v
        table[0x0C] = 'f';  // \f
        table[0x0D] = 'r';  // \r
        
        // JSON special characters
        table['"'] = '"';   // \"
        table['\\'] = '\\'; // \\\\
        
        // DEL character
        table[0x7F] = '.';  // DEL
        
        return table;
    }();

    /// Not Escape object key by default, but can customize.
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
    
    template<typename numberT>
    static std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    NumberString(stringT& dst, numberT value)
    {
        NumberWriter<stringT>::Output(dst, value);
    }

};

/// JSON builder that works with different string types and configurations.
/// Direct JSON string construction without DOM tree.
template<typename stringT, typename configT = BasicConfig<stringT>>
struct GenericBuilder
{
    stringT json;
    using builder_type = GenericBuilder<stringT, configT>;

    /// M0: Basic Methods
    /* ---------------------------------------------------------------------- */
    
    GenericBuilder(size_t capacity = 1024) { json.reserve(capacity); }
    GenericBuilder(const GenericBuilder& other) = default;
    GenericBuilder(GenericBuilder&& other) noexcept = default;

    /// Get built json string.
    const stringT& GetResult() const { return json; }
    
    /// Fix to valid json and return, removes trailing comma if any.
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

    /// Fix to valid json and return as rvalue to transfer ownership.
    stringT&& MoveResult() { return std::move(GetResult()); }

    /// M1: String Interface Wrapper Methods
    /* ---------------------------------------------------------------------- */
    
    void PutChar(char c) { json.push_back(c); }
    
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
    
    void Append(const char* str)
    {
        if (wwjson_unlikely(str == nullptr)) { return; }
        json.append(str);
    }
    
    void Append(const char* str, size_t len)
    {
        if (wwjson_unlikely(str == nullptr)) { return; }
        json.append(str, len);
    }
    
    void Append(const std::string& str) { json.append(str); }
    
    bool Empty() const { return json.empty(); }
    operator bool() const { return !Empty(); }
    size_t Size() const { return json.size(); }
    char& Back() { return json.back(); }
    const char& Back() const { return json.back(); }
    char& Front() { return json.front(); }
    const char& Front() const { return json.front(); }
    void PushBack(char c) { PutChar(c); }
    void Clear() { json.clear(); }

    /// M2: JSON Character-level Methods
    /* ---------------------------------------------------------------------- */
    
    void PutNext() { PutChar(','); }
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
    /// Always add comma after closing, usually needed for sub array.
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

    void EmptyArray() { Append("[]"); }

    /// Append a single char '{' as begin an object.
    /// Can used to begin root object, or nest object as item in parent array.
    void BeginObject() { PutChar('{'); }

    /// Append string `"key":{`, to add a child object in it's parent object.
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

    void EndLine() { PutChar('\n'); }

    /// M3: JSON Scalar Value and Low-level Methods
    /* ---------------------------------------------------------------------- */
    
    void PutNull() { Append("null"); }
    void PutValue(std::nullptr_t) { PutNull(); }

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

    /// Append string  to JSON with quotes, may escape by config.
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

    void PutValue(const char* pszVal)
    {
        if (wwjson_unlikely(pszVal == nullptr)) { return; }
        PutValue(pszVal, ::strlen(pszVal));
    }

    void PutValue(const std::string& strValue)
    {
        PutValue(strValue.c_str(), strValue.length());
    }

    void PutValue(const std::string_view& strValue)
    {
        PutValue(strValue.data(), strValue.length());
    }

    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    PutValue(numberT nValue)
    {
        configT::NumberString(json, nValue);
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

    void PutKey(const char* pszKey)
    {
        if (wwjson_unlikely(pszKey == nullptr)) { return; }
        PutKey(pszKey, ::strlen(pszKey));
    }

    void PutKey(const std::string& strKey)
    {
        PutKey(strKey.c_str(), strKey.length());
    }

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

    void PutSub(const char* pszSub)
    {
        if (wwjson_unlikely(pszSub == nullptr)) { return; }
        PutSub(pszSub, ::strlen(pszSub));
    }

    void PutSub(const std::string& strSub)
    {
        PutSub(strSub.c_str(), strSub.length());
    }

    void PutSub(const std::string_view& strSub)
    {
        PutSub(strSub.data(), strSub.length());
    }

    /// M4: JSON Array and Object Element Methods
    /* ---------------------------------------------------------------------- */
    
    /// Add numeric item to array.
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    AddItem(numberT value)
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
    AddItem(numberT value, bool /*asString*/)
    {
        PutChar('"');
        PutValue(value);
        PutChar('"');
        SepItem();
    }

    /// Add string(or other supported type of) item to array.
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

    /// Add member to object with key and value.
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

    /// Force to escape string and add to array.
    void AddItemEscape(const char* value, size_t len)
    {
        if (wwjson_unlikely(value == nullptr)) { return; }
        PutChar('"');
        configT::EscapeString(json, value, len);
        PutChar('"');
        SepItem();
    }

    void AddItemEscape(const char* value)
    {
        if (wwjson_unlikely(value == nullptr)) { return; }
        AddItemEscape(value, ::strlen(value));
    }

    void AddItemEscape(const std::string& value)
    {
        AddItemEscape(value.c_str(), value.length());
    }
    
    void AddItemEscape(const std::string_view& value)
    {
        AddItemEscape(value.data(), value.length());
    }
    
    /// Force to escape string value and add to object.
    /// Note: Not force to escape key, only refer to configT::kEscapeKey.
    template<typename keyT, typename... Args>
    std::enable_if_t<is_key_v<keyT>, void>
    AddMemberEscape(keyT&& key, Args&&... args)
    {
        PutKey(std::forward<keyT>(key));
        AddItemEscape(std::forward<Args>(args)...);
    }

    /// M6: Special Member Functions and Operator Overloads
    /* ---------------------------------------------------------------------- */
    
    GenericBuilder& operator=(const GenericBuilder& other) = default;
    GenericBuilder& operator=(GenericBuilder&& other) noexcept = default;

    /// Just returns *this for assignment.
    GenericBuilder& operator[](int /* index */)
    {
        return *this;
    }

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
    
    /// Behaves as BeginArray but auto EndArray when destroyed.
    GenericArray<stringT, configT> ScopeArray();
    template<typename keyT>
    std::enable_if_t<is_key_v<keyT>, GenericArray<stringT, configT>>
    ScopeArray(keyT&& key);
    
    /// Behaves as BeginObject but auto EndObject when destroyed.
    GenericObject<stringT, configT> ScopeObject();
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

/// RAII array that auto-closes when destroyed.
template<typename stringT, typename configT>
struct GenericArray
{
private:
    GenericBuilder<stringT, configT>& m_builder;

public:
    GenericArray(GenericBuilder<stringT, configT>& build) : m_builder(build)
    {
        m_builder.BeginArray();
    }

    template<typename keyT>
    GenericArray(GenericBuilder<stringT, configT>& build, keyT&& key) : m_builder(build)
    {
        m_builder.PutKey(std::forward<keyT>(key));
        m_builder.BeginArray();
    }

    ~GenericArray() { m_builder.EndArray(); }

    template <typename... Args>
    void AddItem(Args&&... args)
    {
        m_builder.AddItem(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void AddItemEscape(Args&&... args)
    {
        m_builder.AddItemEscape(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void AddItemSub(Args&&... args)
    {
        m_builder.AddItemSub(std::forward<Args>(args)...);
    }

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

    template<typename... Args>
    auto ScopeArray(Args&&... args)
    {
        return m_builder.ScopeArray(std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    auto ScopeObject(Args&&... args)
    {
        return m_builder.ScopeObject(std::forward<Args>(args)...);
    }
    
    constexpr operator bool() const { return true; }
};

/// RAII object that auto-closes when destroyed.
template<typename stringT, typename configT>
struct GenericObject
{
private:
    GenericBuilder<stringT, configT>& m_builder;

public:
    GenericObject(GenericBuilder<stringT, configT>& build) : m_builder(build)
    {
        m_builder.BeginObject();
    }

    template<typename keyT>
    GenericObject(GenericBuilder<stringT, configT>& build, keyT&& key) : m_builder(build)
    {
        m_builder.PutKey(std::forward<keyT>(key));
        m_builder.BeginObject();
    }

    ~GenericObject() { m_builder.EndObject(); }

    template <typename... Args>
    void AddMember(Args&&... args)
    {
        m_builder.AddMember(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void AddMemberEscape(Args&&... args)
    {
        m_builder.AddMemberEscape(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void AddMemberSub(Args&&... args)
    {
        m_builder.AddMemberSub(std::forward<Args>(args)...);
    }

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
            m_builder.AddItem(std::forward<keyT>(key));
        }
        return *this;
    }
    
    /// Stream operator<< for values used after inserting key.
    template<typename T>
    std::enable_if_t<!is_key_v<T>, GenericObject&>
    operator<<(const T& value)
    {
        m_builder.AddItem(value);
        return *this;
    }

    template<typename... Args>
    auto ScopeArray(Args&&... args)
    {
        return m_builder.ScopeArray(std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    auto ScopeObject(Args&&... args)
    {
        return m_builder.ScopeObject(std::forward<Args>(args)...);
    }
    
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

// Type aliases for common usage
using RawBuilder = GenericBuilder<std::string>;
using RawObject = GenericObject<std::string, BasicConfig<std::string>>;
using RawArray = GenericArray<std::string, BasicConfig<std::string>>;

} /* end of wwjson:: */ 

#endif /* end of include guard: WWJSON_HPP__ */
