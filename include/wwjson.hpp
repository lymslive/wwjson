/**
 * @file wwjson.hpp
 * @author lymslive
 * @date 2025-11-21
 * @last_modified 2025-12-13
 * @brief Construrct json in raw string simply and quickly.
 *
 * @details WWJSON is a header-only C++ library for fast JSON string building
 * without DOM tree construction. It provides a simple, lightweight way to
 * construct JSON strings directly through a builder pattern. The library
 * focuses on JSON serialization (building), not parsing, with high performance
 * through direct string manipulation. Supports nested objects and arrays with
 * RAII-based scope management and is compatible with custom string types.
 * */

#pragma once
#ifndef WWJSON_HPP__
#define WWJSON_HPP__

// Branch prediction macros for performance optimization
#ifndef wwjson_likely
#if defined(__GNUC__) || defined(__clang__)
#define wwjson_likely(x) __builtin_expect(!!(x), 1)
#define wwjson_unlikely(x) __builtin_expect(!!(x), 0)
#else
#define wwjson_likely(x) (x)
#define wwjson_unlikely(x) (x)
#endif
#endif

#include <array>
#include <charconv>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

#include <stdint.h>
#include <string.h>

/// High precision floating-point serialization control.
/// Define this macro to use simple %g format (shorter but less precise).
/// Default is high precision %.17g for better accuracy.
/// Note: This setting is ignored if std::to_chars is available for
/// floating-point types.
#ifndef WWJSON_USE_SIMPLE_FLOAT_FORMAT
#define WWJSON_USE_SIMPLE_FLOAT_FORMAT 0
#endif

namespace wwjson
{

/// Forward declarations for template classes.
template <typename stringT> struct BasicConfig;
template <typename stringT, typename configT> struct GenericBuilder;
template <typename stringT, typename configT> struct GenericObject;
template <typename stringT, typename configT> struct GenericArray;

/// Type trait to detect supported key types (const char*, std::string,
/// std::string_view).
template <typename T> struct is_key : std::false_type
{
};
template <> struct is_key<const char *> : std::true_type
{
};
template <> struct is_key<char *> : std::true_type
{
};
template <> struct is_key<std::string> : std::true_type
{
};
template <> struct is_key<std::string_view> : std::true_type
{
};

template <typename T>
inline constexpr bool is_key_v = is_key<std::decay_t<T>>::value;

namespace detail
{

template <typename T>
using has_fp_to_chars = decltype(std::to_chars(
    std::declval<char *>(), std::declval<char *>(), std::declval<T>()
    //  , std::declval<std::chars_format>()
    ));

template <typename T, typename = void>
struct supports_to_chars_float : std::false_type
{
};

template <typename T>
struct supports_to_chars_float<T, std::void_t<has_fp_to_chars<T>>>
    : std::true_type
{
};

} // namespace detail

/// Detect if std::to_chars supports floating-point types.
template <typename T>
constexpr bool has_float_to_chars_v =
    std::is_floating_point_v<std::remove_cv_t<T>>
        &&detail::supports_to_chars_float<std::remove_cv_t<T>>::value;

#if WWJSON_USE_SIMPLE_FLOAT_FORMAT
constexpr bool use_simple_float_format = !has_float_to_chars_v<double>;
#else
constexpr bool use_simple_float_format = false;
#endif

/// @brief Concept documentation for custom string types
/// @details
/// This struct serves as documentation for the interface requirements of custom
/// string types used with WWJSON. While C++17 doesn't support true concepts,
/// this outlines the minimum interface that custom string implementations must
/// provide to work seamlessly with WWJSON builders.
///
/// @note Custom string types should provide the same core interface as std::string
/// to ensure compatibility with all WWJSON operations.
///
/// @par Required Interface Methods:
/// - append(const char* str)
/// - append(const char* str, size_t len)
/// - push_back(char c)
/// - clear()
/// - empty() const
/// - size() const
/// - c_str() const
/// - front() and back()
/// - reserve(size_t capacity)
/// - Constructor that takes capacity or default constructor
struct StringConcept
{
};

/// @brief High-performance number writer for JSON serialization
/// @details
/// Provides optimized number-to-string conversion for both integer and floating-point
/// types used in JSON construction. This class implements several performance
/// optimizations:
/// 
/// - **Digit Pair Caching**: Pre-computed character pairs for numbers 0-99
/// - **Branch Prediction**: Uses likely/unlikely macros for hot path optimization
/// - **Stack Allocation**: Fixed-size buffers for small numbers
/// - **Vectorized Access**: Aligned memory access for cache efficiency
/// 
/// @par Performance Characteristics:
/// - O(1) for numbers < 100 (direct lookup)
/// - O(log n) for larger numbers (digit-by-digit processing)
/// - Zero-allocation for most common cases
/// 
/// @tparam stringT String type that satisfies StringConcept interface
template <typename stringT> struct NumberWriter
{
    /// @brief Storage for pre-computed digit character pairs
    /// @details Optimized lookup table containing character representations for
    /// numbers 0-99. Each pair stores the tens and ones digits as characters.
    /// This allows O(1) lookup for the most common number ranges in JSON.
    struct DigitPair
    {
        char high;  ///< Tens digit character ('0'-'9')
        char low;   ///< Ones digit character ('0'-'9')
        
        /// @brief Default constructor initializes to "00"
        constexpr DigitPair() : high('0'), low('0') {}
        
        /// @brief Construct from two digit characters
        /// @param h Tens digit character
        /// @param l Ones digit character
        constexpr DigitPair(char h, char l) : high(h), low(l) {}
    };

    /// @brief Pre-computed lookup table for digits 0-99
    /// @details Static constexpr table containing all possible two-digit combinations.
    /// Aligned to 64-byte boundary for optimal cache performance.
    /// Initialized at compile-time to avoid runtime overhead.
    alignas(64) static constexpr std::array<DigitPair, 100> kDigitPairs = []() {
        std::array<DigitPair, 100> table{};
        for (int i = 0; i < 100; ++i)
        {
            table[i] = DigitPair('0' + (i / 10), '0' + (i % 10));
        }
        return table;
    }();

    /// @brief Convert unsigned integer to string representation
    /// @details
    /// Optimized algorithm for converting unsigned integers to decimal strings.
    /// Uses a hybrid approach with different strategies based on number magnitude:
    ///
    /// - **Small numbers (< 100)**: Direct lookup via pre-computed digit pairs
    /// - **Large numbers (≥ 100)**: Buffer-based processing with digit pair optimization
    ///
    /// @par Algorithm Details:
    /// 1. Fast path for numbers 0-99 using digit pair lookup
    /// 2. General algorithm for larger numbers:
    ///    - Process numbers in chunks of 100 for efficiency
    ///    - Use digit pair lookup for each chunk
    ///    - Build string from right to left in fixed buffer
    ///    - Append final result to destination string
    ///
    /// @param[out] dst Destination string buffer
    /// @param value Unsigned integer value to convert
    /// 
    /// @tparam intT Unsigned integer type (uint8_t, uint16_t, uint32_t, uint64_t)
    template <typename intT>
    static std::enable_if_t<std::is_integral_v<intT>, void>
    WriteUnsigned(stringT &dst, intT value)
    {
        // Fast path: numbers 0-99 (most common in JSON)
        if (value < 100)
        {
            if (value < 10)
            {
                // Single digit: direct character conversion
                dst.push_back(static_cast<char>('0' + value));
            }
            else
            {
                // Two digits: use pre-computed digit pair lookup
                const DigitPair &pair = kDigitPairs[static_cast<std::size_t>(value)];
                dst.append(&pair.high, 2);
            }
            return;
        }

        // General path: numbers >= 100
        constexpr int max_len = std::numeric_limits<intT>::digits10 + 1;
        char buffer[max_len];
        char *const buffer_end = buffer + max_len;
        char *ptr = buffer_end;

        // Process number in chunks of 100 for efficiency
        while (value >= 100)
        {
            uint32_t chunk = static_cast<uint32_t>(value % 100);
            value /= 100;

            // Use digit pair lookup for each chunk
            const DigitPair &pair = kDigitPairs[chunk];
            *(--ptr) = pair.low;
            *(--ptr) = pair.high;
        }

        // Handle final 1-2 digit chunk
        if (value < 10)
        {
            *(--ptr) = static_cast<char>('0' + value);
        }
        else
        {
            const DigitPair &pair = kDigitPairs[static_cast<std::size_t>(value)];
            *(--ptr) = pair.low;
            *(--ptr) = pair.high;
        }

        // Append completed string to destination
        dst.append(ptr, buffer_end - ptr);
    }

    template <typename intT>
    static std::enable_if_t<std::is_integral_v<intT>, void>
    WriteSigned(stringT &dst, intT value)
    {
        using UnsignedT = std::make_unsigned_t<intT>;

        if (value < 0)
        {
            dst.push_back('-');
            WriteUnsigned(dst, static_cast<UnsignedT>(-value));
        }
        else
        {
            WriteUnsigned(dst, static_cast<UnsignedT>(value));
        }
    }

    /// @brief Optimized floating-point serialization for small fixed-point numbers
    /// @param[out] dst Destination string buffer
    /// @param value Floating-point value to serialize, should be positive
    /// @return true if optimized path used, false if standard conversion needed
    /// @details
    /// Provides high-performance serialization for floating-point numbers that can
    /// be represented exactly as fixed-point decimals with up to 4 decimal places.
    /// This optimization handles the most common case of decimal numbers in JSON
    /// without the overhead of general floating-point conversion.
    ///
    /// @par Optimization Criteria:
    /// - **Range**: |value| ≤ 2^53 (9,007,199,254,740,992) for exact integer representation
    /// - **Precision**: Up to 4 decimal places with exact representation
    /// - **Fractional Check**: 0.0001 tolerance after scaling by 10,000
    ///
    /// @par Algorithm:
    /// 1. Check if value is within representable integer range
    /// 2. Separate integer and fractional parts
    /// 3. Scale fractional part by 10,000 for fixed-point processing
    /// 4. Validate exact representability within tolerance
    /// 5. Format using optimized digit pair lookup
    ///
    /// @par Performance Benefits:
    /// - **Zero allocations**: Uses stack buffers only
    /// - **Fast path**: Avoids expensive sprintf/to_chars for common cases
    /// - **Exact representation**: No rounding errors for representable values
    /// - **Predictable output**: Consistent decimal formatting
    ///
    /// @par Examples:
    /// - `3.14` → `"3.14"` (optimized)
    /// - `0.001` → `"0.001"` (optimized)
    /// - `123.4567` → `"123.4567"` (optimized)
    /// - `1.23456789` → falls back to standard conversion
    ///
    /// @note This optimization significantly improves performance for typical
    /// decimal numbers in JSON (prices, coordinates, measurements, etc.)
    static bool WriteSmall(stringT &dst, double value)
    {
        constexpr double max_precise_double = 9007199254740992.0; // 2^53
        
        // Check if value is within exact representable range
        if (wwjson_unlikely(value > max_precise_double))
        {
            return false; // Outside optimization range
        }

        uint64_t integer_part = static_cast<uint64_t>(value);
        double fractional_part = value - static_cast<double>(integer_part);
        
        // Handle pure integers efficiently
        if (fractional_part == 0.0)
        {
            WriteUnsigned(dst, integer_part);
            return true;
        }

        // Scale fractional part by 10,000 for fixed-point processing
        double scaled_fractional = fractional_part * 10000.0;
        uint32_t scaled_int = static_cast<uint32_t>(scaled_fractional + 0.5);
        
        // Validate exact representability within tolerance
        double error_check = scaled_fractional - static_cast<double>(scaled_int);
        double tolerance = 1.0e-8; // Allow tolerance after scaling
        if (std::abs(error_check) > tolerance)
        {
            return false; // Not exactly representable as fixed-point
        }

        // Handle rounding case (e.g., 0.9999 → 1.0000)
        if (wwjson_unlikely(scaled_int == 10000))
        {
            ++integer_part;
            WriteUnsigned(dst, integer_part);
            return true;
        }

        WriteUnsigned(dst, integer_part);

        const DigitPair &pair_q = kDigitPairs[scaled_int / 100];
        const DigitPair &pair_r = kDigitPairs[scaled_int % 100];
        
        char buffer[5];
        char* ptr = buffer;
        *ptr++ = '.';
        *ptr++ = pair_q.high;  // Thousands
        *ptr++ = pair_q.low;   // Hundreds
        *ptr++ = pair_r.high;  // Tens
        *ptr++ = pair_r.low;   // Ones
        
        --ptr;
        while (*ptr == '0') { --ptr; }
        dst.append(buffer, ptr - buffer + 1);

        return true;
    }

    /// Converts integer values to their string representation.
    template <typename intT>
    static std::enable_if_t<std::is_integral_v<intT>, void> 
    Output(stringT &dst, intT value)
    {
        if constexpr (std::is_signed_v<intT>)
        {
            WriteSigned(dst, value);
        }
        else
        {
            WriteUnsigned(dst, value);
        }
    }

    /// Converts floating-point values to their string representation.
    /// NaN and Infinity are output as "null" (JSON has no representation for
    /// these values).
    template <typename floatT>
    static std::enable_if_t<std::is_floating_point_v<floatT>, void>
    Output(stringT &dst, floatT value)
    {
        if (wwjson_unlikely(std::isnan(value)))
        {
            dst.append("null", 4);
            return;
        }
        if (wwjson_unlikely(std::isinf(value)))
        {
            dst.append("null", 4);
            return;
        }

        if (value < 0)
        {
            dst.push_back('-');
            value = -value;
        }

        // Try optimized path for small fixed-point numbers
        if (WriteSmall(dst, value))
        {
            return;
        }

        // --- Normal path for regular numbers ---
        if constexpr (has_float_to_chars_v<floatT>)
        {
            char buffer[256];
            auto result = std::to_chars(buffer, buffer + sizeof(buffer), value); // , std::chars_format::general
            if (result.ec == std::errc{})
            {
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
        if constexpr (std::is_same_v<floatT, float>)
        {
            len = std::snprintf(buffer, sizeof(buffer), "%.9g", value);
        }
        else if constexpr (std::is_same_v<floatT, double>)
        {
            len = std::snprintf(buffer, sizeof(buffer), "%.17g", value);
        }
        else if constexpr (std::is_same_v<floatT, long double>)
        {
            len = std::snprintf(buffer, sizeof(buffer), "%.21Lg", value);
        }
#endif

        if (wwjson_likely(len > 0 && static_cast<size_t>(len) < sizeof(buffer)))
        {
            dst.append(buffer, static_cast<size_t>(len));
            return;
        }

        dst.append(std::to_string(value));
    }
};

/// @brief Configuration template for JSON serialization behavior
/// @details
/// Provides compile-time configuration options to customize JSON serialization
/// behavior. This template allows fine-grained control over string escaping,
/// number formatting, and JSON syntax generation without runtime overhead.
/// 
/// @par Configuration Options:
/// - **kEscapeKey**: Controls automatic escaping of object keys
/// - **kEscapeValue**: Controls automatic escaping of string values  
/// - **kQuoteNumber**: Controls whether numeric values are quoted as strings
/// - **kTailComma**: Controls generation of trailing commas in arrays/objects
///
/// @note All options are evaluated at compile-time using constexpr, ensuring
/// zero runtime overhead for configuration decisions.
///
/// @par Customization Example:
/// @code
/// struct MyConfig : BasicConfig<std::string> {
///     static constexpr bool kEscapeKey = true;
///     static constexpr bool kQuoteNumber = true;
/// };
/// @endcode
///
/// @tparam stringT String type that satisfies StringConcept interface
template <typename stringT> struct BasicConfig
{
    /// @brief Enable automatic key escaping in object member operations
    /// @details
    /// When true, object keys are automatically escaped using the EscapeKey
    /// method. This ensures that keys containing special characters (quotes,
    /// backslashes, etc.) are properly escaped for valid JSON.
    /// 
    /// @note Default is false for performance - keys are typically simple
    /// identifiers in most use cases.
    static constexpr bool kEscapeKey = false;

    /// @brief Enable automatic string value escaping
    /// @details
    /// When true, string values are automatically escaped using the EscapeString
    /// method. This ensures that strings containing special characters are
    /// properly escaped for valid JSON.
    /// 
    /// @note Default is false for performance - string escaping can be
    /// expensive for large strings and is often unnecessary.
    static constexpr bool kEscapeValue = false;

    /// @brief Quote numeric values as strings
    /// @details
    /// When true, numeric values are wrapped in quotes, treating them as
    /// strings rather than JSON numbers. This is useful when strict number
    /// representation is needed or to preserve exact formatting.
    /// 
    /// @note Default is false - JSON numbers are more compact and widely
    /// supported than quoted numbers.
    static constexpr bool kQuoteNumber = false;

    /// @brief Allow trailing commas in JSON arrays and objects
    /// @details
    /// When true, trailing commas are allowed after the last element in arrays
    /// and objects. While not valid in strict JSON, this is permitted in
    /// JavaScript and many modern parsers.
    /// 
    /// @note Default is false for strict JSON compliance. Setting to true
    /// can improve performance by avoiding comma removal logic.
    static constexpr bool kTailComma = false;

    /// @brief Compile-time escape table for ASCII character processing
    /// @details
    /// Static constexpr table mapping ASCII characters (0-127) to their escape
    /// representations. A value of 0 indicates no escape is needed. This table
    /// provides efficient O(1) escape lookup during JSON string processing.
    ///
    /// @par Escape Strategy:
    /// - **Control Characters (0x01-0x1F)**: Replaced with '.' for readability
    /// - **Standard Escapes**: Uses C/C++ escape sequences (\0, \a, \b, \t, \n, \v, \f, \r)
    /// - **JSON Special**: Properly escapes quotes and backslashes
    /// - **Non-ASCII (≥128)**: Passed through unchanged for UTF-8 compatibility
    /// - **DEL (0x7F)**: Replaced with '.' for consistency
    ///
    /// @note The choice to use '.' for control characters prioritizes readability
    /// over strict JSON compliance. For applications requiring strict compliance,
    /// consider implementing a custom escape strategy.
    ///
    /// @see EscapeString() for the main string escaping implementation
    static constexpr auto kEscapeTable = []() constexpr
    {
        std::array<uint8_t, 128> table{};

        // Set non-printable control characters (0x01-0x1F) to '.'
        for (int i = 0x01; i <= 0x1F; ++i)
        {
            table[i] = '.';
        }

        // C/C++ standard escape sequences
        table[0x00] = '0'; // \0 (null character)
        table[0x07] = 'a'; // \a (bell)
        table[0x08] = 'b'; // \b (backspace)
        table[0x09] = 't'; // \t (tab)
        table[0x0A] = 'n'; // \n (newline)
        table[0x0B] = 'v'; // \v (vertical tab)
        table[0x0C] = 'f'; // \f (form feed)
        table[0x0D] = 'r'; // \r (carriage return)

        // JSON special characters
        table['"'] = '"';   // \" (escaped quote)
        table['\\'] = '\\'; // \\ (escaped backslash)

        // DEL character (0x7F)
        table[0x7F] = '.'; // DEL

        return table;
    }();

    /// Escape object key using the same implementation as EscapeString.
    static void EscapeKey(stringT &dst, const char *key, size_t len)
    {
        EscapeString(dst, key, len);
    }

    /// @brief Escape string using compile-time escape table with performance optimizations
    /// @param[out] dst Destination string buffer for escaped output
    /// @param src Source string to escape (must not be nullptr)
    /// @param len Length of source string
    /// @details
    /// High-performance string escaping implementation optimized for JSON serialization.
    /// Uses compile-time escape table for O(1) character lookup and hybrid memory
    /// allocation strategy for optimal performance across different string sizes.
    ///
    /// @par Performance Optimizations:
    /// - **Compile-time Escape Table**: O(1) character-to-escape mapping
    /// - **Hybrid Memory Allocation**: Stack buffer for small strings, heap for large
    /// - **Branch Prediction Hints**: Likely/unlikely macros for hot paths
    /// - **Single-pass Processing**: One character-by-character scan
    /// - **UTF-8 Passthrough**: Non-ASCII characters passed through unchanged
    ///
    /// @par Memory Strategy:
    /// - **Small strings (≤256 bytes)**: Uses stack-allocated buffer
    /// - **Large strings (>256 bytes)**: Heap allocation with 2x size buffer
    /// - **Worst-case expansion**: String can double in size during escaping
    ///
    /// @par Complexity:
    /// - **Time**: O(n) where n is input string length
    /// - **Space**: O(n) worst-case (escaped string may be 2x larger)
    ///
    /// @warning This function assumes UTF-8 encoding for non-ASCII characters.
    /// Non-ASCII characters (bytes ≥128) are passed through unchanged.
    static void EscapeString(stringT &dst, const char *src, size_t len)
    {
        if (wwjson_unlikely(src == nullptr)) { return; }
        
        // Allocate temporary buffer on stack for small strings, heap for large ones
        constexpr size_t stack_buffer_size = 256;
        char stack_buffer[stack_buffer_size];
        char *buffer = stack_buffer;
        std::unique_ptr<char[]> heap_buffer;

        size_t buffer_capacity = len * 2; // Worst case: every char needs escaping
        if (wwjson_unlikely(buffer_capacity > stack_buffer_size))
        {
            heap_buffer = std::make_unique<char[]>(buffer_capacity);
            buffer = heap_buffer.get();
        }

        char *ptr = buffer;
        const char *const buffer_end = buffer + buffer_capacity;

        for (size_t i = 0; i < len; ++i)
        {
            unsigned char c = static_cast<unsigned char>(src[i]);

            if (wwjson_unlikely(c >= 128))
            {
                // UTF-8 character - pass through unchanged
                *ptr++ = c;
            }
            else
            {
                // ASCII character - check escape table
                uint8_t escape_char = kEscapeTable[c];
                if (wwjson_unlikely(escape_char != 0))
                {
                    *ptr++ = '\\';
                    *ptr++ = escape_char;
                }
                else
                {
                    // No escaping needed
                    *ptr++ = c;
                }
            }
        }

        dst.append(buffer, ptr - buffer);
    }

    template <typename numberT>
    static std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    NumberString(stringT &dst, numberT value)
    {
        NumberWriter<stringT>::Output(dst, value);
    }
};

/// @brief Main JSON builder for constructing JSON strings without DOM trees
/// @details
/// GenericBuilder provides a high-performance interface for constructing JSON
/// strings through direct string manipulation. It eliminates the overhead of
/// building intermediate DOM representations and provides fine-grained control
/// over JSON generation.
///
/// @par Key Features:
/// - **Header-only**: No external dependencies beyond C++ standard library
/// - **Template-based**: Works with any string type satisfying StringConcept
/// - **RAII Support**: Scope-based management with GenericArray and GenericObject
/// - **Performance**: Optimized for high-throughput JSON generation
/// - **Configurable**: Customizable behavior through configT template parameter
///
/// @par Usage Pattern:
/// @code
/// RawBuilder builder;
/// builder.BeginObject();
/// builder.AddMember("name", "value");
/// builder.AddMember("count", 42);
/// builder.EndObject();
/// auto result = builder.MoveResult(); // {"name":"value","count":42}
/// @endcode
///
/// @tparam stringT String type (std::string, custom types)
/// @tparam configT Configuration type (defaults to BasicConfig<stringT>)
template <typename stringT, typename configT = BasicConfig<stringT>>
struct GenericBuilder
{
    stringT json; ///< Internal string buffer storing the JSON being constructed
    using builder_type = GenericBuilder<stringT, configT>; ///< Type alias for this builder

    /// @brief M0: Basic construction and lifecycle methods
    /// @{

    /// @brief Copy constructor
    GenericBuilder(const GenericBuilder &other) = default;
    
    /// @brief Move constructor
    GenericBuilder(GenericBuilder &&other) noexcept = default;

    /// @brief Default constructor with optional capacity hint
    /// @param capacity Initial capacity reservation for the JSON string
    /// @note Pre-allocates buffer space to reduce reallocations during construction
    GenericBuilder(size_t capacity = 1024) { json.reserve(capacity); }
    
    /// @brief Constructor with prefix string
    /// @param prefix Initial string content (e.g., HTTP headers, JSONP wrapper)
    /// @param capacity Additional capacity to reserve beyond prefix length
    GenericBuilder(const stringT& prefix, size_t capacity = 1024) 
        : json(prefix) 
    { 
        Reserve(capacity); 
    }
    
    /// @brief Constructor with movable prefix string
    /// @param prefix Movable string content
    /// @param capacity Additional capacity to reserve beyond prefix length
    GenericBuilder(stringT&& prefix, size_t capacity = 1024) 
        : json(std::move(prefix)) 
    { 
        Reserve(capacity); 
    }
    
    /// @brief Reserve additional string capacity
    /// @param additional_capacity Additional bytes to reserve
    /// @note This is additive to the current size, not an absolute capacity
    void Reserve(size_t additional_capacity) 
    { 
        if (wwjson_unlikely(additional_capacity == 0)) return;
        json.reserve(json.size() + additional_capacity); 
    }

    /// @brief Get const reference to the built JSON string
    /// @return const reference to the internal JSON string
    /// @note This method does not perform any cleanup - use GetResult() for cleaned output
    const stringT &GetResult() const { return json; }

    /// @brief Get mutable reference to the built JSON string with cleanup
    /// @return mutable reference to the internal JSON string
    /// @details Removes trailing commas if present to ensure valid JSON output.
    /// This operation ignores the kTailComma configuration setting to guarantee
    /// valid JSON output.
    stringT &GetResult()
    {
        if (wwjson_unlikely(json.empty())) { return json; }
        if (json.back() == ',')
        {
            json.pop_back();
        }
        return json;
    }

    /// @brief Move the JSON string result to transfer ownership
    /// @return rvalue reference to the built JSON string
    /// @note This is the recommended way to extract the final result
    stringT &&MoveResult() { return std::move(GetResult()); }
    
    /// @}

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

    void Append(const char *str)
    {
        if (wwjson_unlikely(str == nullptr)) { return; }
        json.append(str);
    }

    void Append(const char *str, size_t len)
    {
        if (wwjson_unlikely(str == nullptr)) { return; }
        json.append(str, len);
    }

    void Append(const std::string &str) { json.append(str); }

    bool Empty() const { return json.empty(); }
    operator bool() const { return !Empty(); }
    size_t Size() const { return json.size(); }
    char &Back() { return json.back(); }
    const char &Back() const { return json.back(); }
    char &Front() { return json.front(); }
    const char &Front() const { return json.front(); }
    void PushBack(char c) { PutChar(c); }
    void Clear() { json.clear(); }

    /// M2: JSON Character-level Methods
    /* ---------------------------------------------------------------------- */

    void PutNext() { PutChar(','); }
    void SepItem() { PutNext(); }

    /// @brief Begin a JSON array with opening bracket '['
    /// @details
    /// Appends the opening bracket for a JSON array. This can be used to:
    /// - Start a root-level array
    /// - Create a nested array within another array
    /// - Create a nested array within an object member
    ///
    /// @par Usage Example:
    /// @code
    /// builder.BeginArray();
    /// builder.AddItem("element1");
    /// builder.AddItem("element2");
    /// builder.EndArray(); // ["element1","element2"]
    /// @endcode
    void BeginArray() { PutChar('['); }

    /// @brief Begin a JSON array with a specified object key
    /// @tparam keyT Key type (const char*, std::string, std::string_view)
    /// @param key Object key name for the array
    /// @details
    /// Appends a quoted key followed by a colon and opening bracket:
    /// `"key":[`
    /// This is equivalent to calling PutKey(key) followed by BeginArray().
    ///
    /// @par Usage Example:
    /// @code
    /// builder.BeginObject();
    /// builder.BeginArray("items"); // "items":[
    /// builder.AddItem("value1");
    /// builder.AddItem("value2");
    /// builder.EndArray();
    /// builder.EndObject(); // {"items":["value1","value2"]}
    /// @endcode
    template <typename keyT>
    std::enable_if_t<is_key_v<keyT>, void> BeginArray(keyT &&key)
    {
        PutKey(std::forward<keyT>(key));
        BeginArray();
    }

    /// @brief End a JSON array with closing bracket ']'
    /// @details
    /// Appends the closing bracket for a JSON array. Handles trailing comma
    /// according to the configuration:
    /// - If kTailComma is true: adds ']' directly
    /// - If kTailComma is false: replaces trailing comma with ']'
    /// @note Always adds a separator comma after closing for consistency with
    /// AddItem semantics.
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

    /// @brief Create an empty JSON array "[]"
    /// @details
    /// Optimized method to append an empty array literal without the overhead
    /// of BeginArray()/EndArray() calls.
    void EmptyArray() { Append("[]"); }

    /// @brief Begin a JSON object with opening brace '{'
    /// @details
    /// Appends the opening brace for a JSON object. This can be used to:
    /// - Start a root-level object
    /// - Create a nested object within an array
    /// - Create a nested object within another object member
    ///
    /// @par Usage Example:
    /// @code
    /// builder.BeginObject();
    /// builder.AddMember("name", "value");
    /// builder.AddMember("count", 42);
    /// builder.EndObject(); // {"name":"value","count":42}
    /// @endcode
    void BeginObject() { PutChar('{'); }

    /// @brief Begin a JSON object with a specified object key
    /// @tparam keyT Key type (const char*, std::string, std::string_view)
    /// @param key Object key name for the nested object
    /// @details
    /// Appends a quoted key followed by a colon and opening brace:
    /// `"key":{`
    /// This is equivalent to calling PutKey(key) followed by BeginObject().
    ///
    /// @par Usage Example:
    /// @code
    /// builder.BeginObject();
    /// builder.BeginObject("config"); // "config":{
    /// builder.AddMember("debug", true);
    /// builder.EndObject();
    /// builder.EndObject(); // {"config":{"debug":true}}
    /// @endcode
    template <typename keyT>
    std::enable_if_t<is_key_v<keyT>, void> BeginObject(keyT &&key)
    {
        PutKey(std::forward<keyT>(key));
        BeginObject();
    }

    /// @brief End a JSON object with closing brace '}'
    /// @details
    /// Appends the closing brace for a JSON object. Handles trailing comma
    /// according to the configuration:
    /// - If kTailComma is true: adds '}' directly
    /// - If kTailComma is false: replaces trailing comma with '}'
    /// @note Always adds a separator comma after closing for consistency with
    /// AddItem semantics.
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

    /// @brief json root which default object, can pass `[` for array root.
    /// @note NOT check the open bracket, accpet any char.
    void BeginRoot(char bracket = '{') { PutChar(bracket); }

    /// @brief json root which default object without adding comma.
    /// @note NOT check the close bracket, accpet any char.
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
    void PutValue(const char *pszVal, size_t len)
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

    void PutValue(const char *pszVal)
    {
        if (wwjson_unlikely(pszVal == nullptr)) { return; }
        PutValue(pszVal, ::strlen(pszVal));
    }

    void PutValue(const std::string &strValue)
    {
        PutValue(strValue.c_str(), strValue.length());
    }

    void PutValue(const std::string_view &strValue)
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
    void PutKey(const char *pszKey, size_t len)
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

    void PutKey(const char *pszKey)
    {
        if (wwjson_unlikely(pszKey == nullptr)) { return; }
        PutKey(pszKey, ::strlen(pszKey));
    }

    void PutKey(const std::string &strKey)
    {
        PutKey(strKey.c_str(), strKey.length());
    }

    void PutKey(const std::string_view &strKey)
    {
        PutKey(strKey.data(), strKey.length());
    }

    /// Append JSON sub-string (raw JSON content) without quotes or escaping.
    /// User is responsible for ensuring the input is valid JSON.
    void PutSub(const char *pszSub, size_t len)
    {
        if (wwjson_unlikely(pszSub == nullptr)) { return; }
        Append(pszSub, len);
    }

    void PutSub(const char *pszSub)
    {
        if (wwjson_unlikely(pszSub == nullptr)) { return; }
        PutSub(pszSub, ::strlen(pszSub));
    }

    void PutSub(const std::string &strSub)
    {
        PutSub(strSub.c_str(), strSub.length());
    }

    void PutSub(const std::string_view &strSub)
    {
        PutSub(strSub.data(), strSub.length());
    }

    /// M4: JSON Array and Object Element Methods
    /* ---------------------------------------------------------------------- */

    /// Add numeric item to array.
    template <typename numberT>
    std::enable_if_t<std::is_arithmetic_v<numberT>, void> AddItem(numberT value)
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
    template <typename... Args> void AddItem(Args &&... args)
    {
        PutValue(std::forward<Args>(args)...);
        SepItem();
    }

    /// Add item to array using callable function with GenericBuilder reference
    /// parameter. Would pass current builder to the provided function to
    /// produce a sub json.
    template <typename Func>
    std::enable_if_t<std::is_invocable_v<Func, builder_type &>, void>
    AddItem(Func &&func)
    {
        func(*this);
        if (wwjson_likely(!Empty()) && wwjson_unlikely(Back() != ','))
        {
            SepItem();
        }
    }

    /// Add item to array using callable function with no parameters.
    /// Usually use lambda captures current builder to produce sub json.
    template <typename Func>
    std::enable_if_t<std::is_invocable_v<Func> &&
                         !std::is_invocable_v<Func, builder_type &>,
                     void>
    AddItem(Func &&func)
    {
        func();
        if (wwjson_likely(!Empty()) && wwjson_unlikely(Back() != ','))
        {
            SepItem();
        }
    }

    /// @brief Add a member to a JSON object with key and value
    /// @tparam keyT Key type (const char*, std::string, std::string_view)
    /// @tparam Args Value argument types
    /// @param key Object key name
    /// @param args Value arguments ( forwarded to AddItem )
    /// @details
    /// Adds a key-value pair to the current JSON object. The key is automatically
    /// quoted and escaped according to configuration, followed by a colon and
    /// the value. This is the primary method for populating JSON objects.
    ///
    /// @par Supported Value Types:
    /// - Numeric types (int, float, double, etc.)
    /// - String types (const char*, std::string, std::string_view)
    /// - Boolean values (true/false)
    /// - Null values (nullptr)
    /// - Callable functions for nested structures
    ///
    /// @par Usage Examples:
    /// @code
    /// builder.BeginObject();
    /// builder.AddMember("name", "John");          // "name":"John"
    /// builder.AddMember("age", 30);               // "age":30
    /// builder.AddMember("active", true);          // "active":true
    /// builder.AddMember("balance", 123.45);       // "balance":123.45
    /// builder.AddMember("data", [&]() {           // "data":{...}
    ///     auto obj = builder.ScopeObject();
    ///     obj.AddMember("id", 123);
    /// });
    /// builder.EndObject();
    /// @endcode
    ///
    /// @note This method does not support length-specified key overloads
    /// (pszKey, len) as the length parameter would conflict with the variadic args.
    template <typename keyT, typename... Args>
    std::enable_if_t<is_key_v<keyT>, void> AddMember(keyT &&key, Args &&... args)
    {
        PutKey(std::forward<keyT>(key));
        AddItem(std::forward<Args>(args)...);
    }

    /// @brief Add only a key to a JSON object (for deferred value assignment)
    /// @tparam keyT Key type (const char*, std::string, std::string_view)
    /// @param key Object key name
    /// @details
    /// Adds only the quoted key and colon to the JSON object, without a value.
    /// This enables the pattern of separating key specification from value
    /// assignment, which can improve code readability in complex scenarios.
    ///
    /// @par Equivalent Alternative:
    /// This is functionally equivalent to BeginObject(key) but allows the key
    /// and value assignment to be separated for better code organization.
    ///
    /// @par Usage Example:
    /// @code
    /// builder.BeginObject();
    /// builder.AddMember("user");        // "user":
    /// builder.BeginObject();            // {
    /// builder.AddMember("name", "Alice");
    /// builder.AddMember("age", 25);
    /// builder.EndObject();              // }
    /// builder.EndObject();              // {"user":{"name":"Alice","age":25}}
    /// @endcode
    template <typename keyT>
    std::enable_if_t<is_key_v<keyT>, void> AddMember(keyT &&key)
    {
        PutKey(std::forward<keyT>(key));
    }

    /// M5: String Escaping Methods
    /* ---------------------------------------------------------------------- */

    /// Force to escape string and add to array.
    void AddItemEscape(const char *value, size_t len)
    {
        if (wwjson_unlikely(value == nullptr)) { return; }
        PutChar('"');
        configT::EscapeString(json, value, len);
        PutChar('"');
        SepItem();
    }

    void AddItemEscape(const char *value)
    {
        if (wwjson_unlikely(value == nullptr)) { return; }
        AddItemEscape(value, ::strlen(value));
    }

    void AddItemEscape(const std::string &value)
    {
        AddItemEscape(value.c_str(), value.length());
    }

    void AddItemEscape(const std::string_view &value)
    {
        AddItemEscape(value.data(), value.length());
    }

    /// Force to escape string value and add to object.
    /// Note: Not force to escape key, only refer to configT::kEscapeKey.
    template <typename keyT, typename... Args>
    std::enable_if_t<is_key_v<keyT>, void>
    AddMemberEscape(keyT&& key, Args&&... args)
    {
        PutKey(std::forward<keyT>(key));
        AddItemEscape(std::forward<Args>(args)...);
    }

    /// Force to escape key and add to object without value.
    /// This enables pattern: AddMemberEscape(key) + BeginObject()
    /// which escapes the key name before creating nested object.
    template <typename keyT>
    std::enable_if_t<is_key_v<keyT>, void> AddMemberEscape(keyT&& key)
    {
        PutChar('"');
        if constexpr (std::is_pointer_v<std::decay_t<keyT>>)
        {
            // const char*
            configT::EscapeKey(json, std::forward<keyT>(key), ::strlen(key));
        }
        else
        {
            // std::string or std::string_view
            configT::EscapeKey(json, key.data(), key.length());
        }
        PutChar('"');
        PutChar(':');
    }

    /// M6: Special Member Functions and Operator Overloads
    /* ---------------------------------------------------------------------- */

    GenericBuilder &operator=(const GenericBuilder &other) = default;
    GenericBuilder &operator=(GenericBuilder &&other) noexcept = default;

    /// Just returns *this for assignment.
    GenericBuilder &operator[](int /* index */) { return *this; }

    /// Sets up the key and returns *this for assignment.
    template <typename keyT>
    std::enable_if_t<is_key_v<keyT>, GenericBuilder &> operator[](keyT &&key)
    {
        PutKey(std::forward<keyT>(key));
        return *this;
    }

    /// Generic assignment operator that delegates to AddItem.
    /// Enables builder["key"] = value and builder[index] = value syntax.
    template <typename T> GenericBuilder &operator=(const T &value)
    {
        AddItem(value);
        return *this;
    }

    /// M7: Scope Creation Methods
    /* ---------------------------------------------------------------------- */

    /// @brief Create a scoped array that auto-closes when destroyed (RAII)
    /// @return GenericArray RAII wrapper that auto-closes the array
    /// @details
    /// Creates a RAII-scoped array that automatically calls EndArray() when
    /// destroyed. This eliminates the risk of forgetting to close arrays and
    /// ensures proper JSON structure even in the presence of exceptions or
    /// early returns.
    ///
    /// @par RAII Benefits:
    /// - Exception-safe: Array is properly closed even if exceptions occur
    /// - Code clarity: Scoped construction makes nesting obvious
    ///
    /// @par Usage Example:
    /// @code
    /// builder.BeginObject();
    /// {
    ///     auto arr = builder.ScopeArray("items");
    ///     arr.AddItem("element1");
    ///     arr.AddItem("element2");
    ///     arr.AddItem(42);
    ///     // arr goes out of scope here, auto-calling EndArray()
    /// }
    /// builder.EndObject(); // {"items":["element1","element2",42]}
    /// @endcode
    GenericArray<stringT, configT> ScopeArray();
    
    /// @brief Create a scoped array with key that auto-closes when destroyed
    /// @tparam keyT Key type (const char*, std::string, std::string_view)
    /// @param key Object key name for the array
    /// @return GenericArray RAII wrapper that auto-closes the array
    /// @details
    /// Creates a RAII-scoped array with the specified object key. Equivalent
    /// to calling PutKey(key) followed by ScopeArray(), but more convenient.
    template <typename keyT>
    std::enable_if_t<is_key_v<keyT>, GenericArray<stringT, configT>>
    ScopeArray(keyT &&key);

    /// @brief Create a scoped object that auto-closes when destroyed (RAII)
    /// @return GenericObject RAII wrapper that auto-closes the object
    /// @details
    /// Creates a RAII-scoped object that automatically calls EndObject() when
    /// destroyed. This eliminates the risk of forgetting to close objects and
    /// ensures proper JSON structure even in the presence of exceptions or
    /// early returns.
    ///
    /// @par RAII Benefits:
    /// - Exception-safe: Object is properly closed even if exceptions occur
    /// - Code clarity: Scoped construction makes nesting obvious
    ///
    /// @par Usage Example:
    /// @code
    /// builder.BeginObject();
    /// {
    ///     auto obj = builder.ScopeObject("config");
    ///     obj.AddMember("debug", true);
    ///     obj.AddMember("timeout", 30);
    ///     // obj goes out of scope here, auto-calling EndObject()
    /// }
    /// builder.EndObject(); // {"config":{"debug":true,"timeout":30}}
    /// @endcode
    GenericObject<stringT, configT> ScopeObject();
    
    /// @brief Create a scoped object with key that auto-closes when destroyed
    /// @tparam keyT Key type (const char*, std::string, std::string_view)
    /// @param key Object key name for the nested object
    /// @return GenericObject RAII wrapper that auto-closes the object
    /// @details
    /// Creates a RAII-scoped object with the specified object key. Equivalent
    /// to calling PutKey(key) followed by ScopeObject(), but more convenient.
    template <typename keyT>
    std::enable_if_t<is_key_v<keyT>, GenericObject<stringT, configT>>
    ScopeObject(keyT &&key);

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

        /// @brief Merge two JSON strings (objects or arrays) into one
        /// @param that Other builder containing JSON to merge
        /// @return true if merge successful, false if incompatible JSON types
        /// @details
        /// Intelligently merges two JSON structures by detecting compatible
        /// boundaries and combining them with proper comma separation. This
        /// enables dynamic JSON composition without rebuilding from scratch.
        ///
        /// @par Merge Algorithm:
        /// 1. If either string is empty, return the other unchanged
        /// 2. Check for compatible boundary patterns:
        ///    - `}{` (object followed by object)
        ///    - `][` (array followed by array)
        /// 3. Replace boundary with comma separator
        /// 4. Concatenate the structures
        ///
        /// @par Supported Patterns:
        /// - `{"a":1}{"b":2}` → `{"a":1,"b":2}`
        /// - `[1,2][3,4]` → `[1,2,3,4]`
        ///
        /// @par Usage Example:
        /// @code
        /// GenericBuilder left;
        /// left.BeginRoot();
        /// left.AddMember("id", 1);
        /// left.EndRoot(); // {"id":1}
        ///
        /// GenericBuilder right;
        /// right.BeginRoot();
        /// right.AddMember("name", "Alice");
        /// right.EndRoot(); // {"name":"Alice"}
        ///
        /// left.Merge(right); // {"id":1,"name":"Alice"}
        /// @endcode
        ///
        /// @warning This is a simple boundary-based merge. Complex nested
        /// structures require manual construction. Not all JSON combinations
        /// are mergeable.
        bool Merge(const GenericBuilder<stringT, configT> &that)
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
            if (wwjson_likely((selfLast == '}' && thatFirst == '{') ||
                              (selfLast == ']' && thatFirst == '[')))
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
    static bool Merge(stringT &self, const stringT &that)
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
        if (wwjson_likely((selfLast == '}' && thatFirst == '{') ||
                          (selfLast == ']' && thatFirst == '[')))
        {
            self.back() = ',';
            self.append(that.c_str() + 1, that.size() - 1);
            return true;
        }

        return false;
    }

    /// Add JSON sub-string as array item without additional quotes or escaping.
    /// User is responsible for ensuring the input is valid JSON.
    template <typename... Args> void AddItemSub(Args &&... args)
    {
        PutSub(std::forward<Args>(args)...);
        SepItem();
    }

    /// Add member with JSON sub-string value without additional quotes or
    /// escaping. User is responsible for ensuring the input is valid JSON.
    template <typename keyT, typename... Args>
    std::enable_if_t<is_key_v<keyT>, void> AddMemberSub(keyT &&key, Args &&... args)
    {
        PutKey(std::forward<keyT>(key));
        AddItemSub(std::forward<Args>(args)...);
    }
};

/// @brief RAII wrapper for JSON arrays with automatic scope management
/// @details
/// Provides automatic lifecycle management for JSON arrays using RAII (Resource
/// Acquisition Is Initialization). When a GenericArray object is destroyed,
/// it automatically calls EndArray() on the associated builder, ensuring
/// proper JSON structure even in the presence of exceptions or early returns.
///
/// @par Key Benefits:
/// - **Exception Safety**: Arrays are properly closed even if exceptions occur
/// - **Code Clarity**: Scoped construction makes array boundaries obvious
/// - **Error Prevention**: Eliminates common mistakes like forgetting EndArray()
///
/// @par Usage Pattern:
/// GenericArray objects should be created within a scope (function, block, etc.)
/// and will automatically close the array when they go out of scope.
///
/// @tparam stringT String type that satisfies StringConcept interface
/// @tparam configT Configuration type for JSON serialization behavior
template <typename stringT, typename configT> struct GenericArray
{
  private:
    GenericBuilder<stringT, configT> &m_builder; ///< Reference to the parent builder

  public:
    /// @brief Construct a scoped array without a key
    /// @param build Reference to the parent GenericBuilder
    /// @details
    /// Creates a scoped array and immediately calls BeginArray() on the builder.
    /// The array will be automatically closed when this object is destroyed.
    GenericArray(GenericBuilder<stringT, configT> &build) : m_builder(build)
    {
        m_builder.BeginArray();
    }

    /// @brief Construct a scoped array with an object key
    /// @tparam keyT Key type (const char*, std::string, std::string_view)
    /// @param build Reference to the parent GenericBuilder
    /// @param key Object key name for the array
    /// @details
    /// Creates a scoped array with the specified key by calling PutKey() followed
    /// by BeginArray(). The array will be automatically closed when this object
    /// is destroyed.
    template <typename keyT>
    GenericArray(GenericBuilder<stringT, configT> &build, keyT &&key)
        : m_builder(build)
    {
        m_builder.PutKey(std::forward<keyT>(key));
        m_builder.BeginArray();
    }

    /// @brief Destructor automatically closes the array
    /// @details
    /// Called automatically when the GenericArray object goes out of scope.
    /// Ensures the JSON array is properly closed with the corresponding EndArray() call.
    ~GenericArray() { m_builder.EndArray(); }

    template <typename... Args> void AddItem(Args &&... args)
    {
        m_builder.AddItem(std::forward<Args>(args)...);
    }

    template <typename... Args> void AddItemEscape(Args &&... args)
    {
        m_builder.AddItemEscape(std::forward<Args>(args)...);
    }

    template <typename... Args> void AddItemSub(Args &&... args)
    {
        m_builder.AddItemSub(std::forward<Args>(args)...);
    }

    GenericBuilder<stringT, configT> &operator[](int /* index */)
    {
        return m_builder;
    }

    /// @brief Stream operator for fluent array element addition
    /// @tparam T Value type (any type supported by AddItem)
    /// @param value Value to add to the array
    /// @return Reference to this GenericArray for method chaining
    /// @details
    /// Provides a convenient stream-style interface for adding elements to arrays.
    /// This enables fluent, readable code for array construction with method chaining.
    ///
    /// @par Supported Types:
    /// - Numeric types (int, float, double, etc.)
    /// - String types (const char*, std::string, std::string_view)
    /// - Boolean values
    /// - Null values
    /// - Callable functions for nested structures
    ///
    /// @par Usage Examples:
    /// @code
    /// // Basic usage
    /// auto arr = builder.ScopeArray();
    /// arr << "element1" << "element2" << 42 << true;
    ///
    /// // With nested structures
    /// arr << [&]() {
    ///     auto obj = arr.ScopeObject();
    ///     obj.AddMember("nested", "value");
    /// };
    ///
    /// // Mixed with regular methods
    /// arr.AddItem("first");
    /// arr << "second" << "third";
    /// arr.AddItem(100);
    /// @endcode
    ///
    /// @note This operator is just syntactic sugar for AddItem() calls.
    /// Use whichever style is more readable for your use case.
    template <typename T> GenericArray &operator<<(const T &value)
    {
        AddItem(value);
        return *this;
    }

    template <typename... Args> auto ScopeArray(Args &&... args)
    {
        return m_builder.ScopeArray(std::forward<Args>(args)...);
    }

    template <typename... Args> auto ScopeObject(Args &&... args)
    {
        return m_builder.ScopeObject(std::forward<Args>(args)...);
    }

    constexpr operator bool() const { return true; }
};

/// @brief RAII wrapper for JSON objects with automatic scope management
/// @details
/// Provides automatic lifecycle management for JSON objects using RAII (Resource
/// Acquisition Is Initialization). When a GenericObject object is destroyed,
/// it automatically calls EndObject() on the associated builder, ensuring
/// proper JSON structure even in the presence of exceptions or early returns.
///
/// @par Key Benefits:
/// - **Exception Safety**: Objects are properly closed even if exceptions occur
/// - **Code Clarity**: Scoped construction makes object boundaries obvious
/// - **Error Prevention**: Eliminates common mistakes like forgetting EndObject()
/// - **Fluent Interface**: Supports stream-style key-value pair addition
///
/// @par Usage Pattern:
/// GenericObject objects should be created within a scope (function, block, etc.)
/// and will automatically close the object when they go out of scope.
///
/// @tparam stringT String type that satisfies StringConcept interface
/// @tparam configT Configuration type for JSON serialization behavior
template <typename stringT, typename configT> struct GenericObject
{
  private:
    GenericBuilder<stringT, configT> &m_builder; ///< Reference to the parent builder

  public:
    /// @brief Construct a scoped object without a key
    /// @param build Reference to the parent GenericBuilder
    /// @details
    /// Creates a scoped object and immediately calls BeginObject() on the builder.
    /// The object will be automatically closed when this object is destroyed.
    GenericObject(GenericBuilder<stringT, configT> &build) : m_builder(build)
    {
        m_builder.BeginObject();
    }

    /// @brief Construct a scoped object with an object key
    /// @tparam keyT Key type (const char*, std::string, std::string_view)
    /// @param build Reference to the parent GenericBuilder
    /// @param key Object key name for the nested object
    /// @details
    /// Creates a scoped object with the specified key by calling PutKey() followed
    /// by BeginObject(). The object will be automatically closed when this object
    /// is destroyed.
    template <typename keyT>
    GenericObject(GenericBuilder<stringT, configT> &build, keyT &&key)
        : m_builder(build)
    {
        m_builder.PutKey(std::forward<keyT>(key));
        m_builder.BeginObject();
    }

    /// @brief Destructor automatically closes the object
    /// @details
    /// Called automatically when the GenericObject object goes out of scope.
    /// Ensures the JSON object is properly closed with the corresponding EndObject() call.
    ~GenericObject() { m_builder.EndObject(); }

    template <typename... Args> void AddMember(Args &&... args)
    {
        m_builder.AddMember(std::forward<Args>(args)...);
    }

    template <typename... Args> void AddMemberEscape(Args &&... args)
    {
        m_builder.AddMemberEscape(std::forward<Args>(args)...);
    }

    template <typename... Args> void AddMemberSub(Args &&... args)
    {
        m_builder.AddMemberSub(std::forward<Args>(args)...);
    }

    template <typename keyT>
    std::enable_if_t<is_key_v<keyT>, GenericBuilder<stringT, configT> &>
    operator[](keyT &&key)
    {
        m_builder.PutKey(std::forward<keyT>(key));
        return m_builder;
    }

    /// @brief Stream operator for key types with intelligent context detection
    /// @tparam keyT Key type (const char*, std::string, std::string_view)
    /// @param key Key to add to the object
    /// @return Reference to this GenericObject for method chaining
    /// @details
    /// Provides intelligent key insertion that automatically detects whether
    /// a key or value is expected based on the builder's current state. This
    /// enables fluent object construction with minimal syntax.
    ///
    /// @par Context Detection Logic:
    /// - **Expecting Key**: When last character is not ':', treat input as key
    /// - **Expecting Value**: When last character is ':', treat input as value
    ///
    /// @par Usage Examples:
    /// @code
    /// auto obj = builder.ScopeObject();
    /// 
    /// // Keys are automatically detected
    /// obj << "name" << "Alice";        // "name":"Alice"
    /// obj << "age" << 25;              // "age":25
    /// obj << "active" << true;         // "active":true
    /// 
    /// // Mixed usage with regular methods
    /// obj.AddMember("id", 123);        // "id":123
    /// obj << "score" << 98.5;          // "score":98.5
    /// @endcode
    ///
    /// @note This operator intelligently switches between PutKey() and AddItem()
    /// based on the builder's state, enabling very fluent object construction.
    template <typename keyT>
    std::enable_if_t<is_key_v<keyT>, GenericObject &> operator<<(keyT &&key)
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

    /// @brief Stream operator for value types (non-key types)
    /// @tparam T Value type (numeric, string, boolean, etc.)
    /// @param value Value to add to the object
    /// @return Reference to this GenericObject for method chaining
    /// @details
    /// Adds a value to the current object. This overload is selected when the
    /// type is not a recognized key type, ensuring values are always added
    /// via AddItem() regardless of context.
    ///
    /// @par Usage Examples:
    /// @code
    /// auto obj = builder.ScopeObject();
    /// obj << "name" << "Alice";        // String value
    /// obj << "count" << 42;            // Numeric value
    /// obj << "active" << true;         // Boolean value
    /// obj << "balance" << 123.45;      // Floating-point value
    /// @endcode
    template <typename T>
    std::enable_if_t<!is_key_v<T>, GenericObject &> operator<<(const T &value)
    {
        m_builder.AddItem(value);
        return *this;
    }

    template <typename... Args> auto ScopeArray(Args &&... args)
    {
        return m_builder.ScopeArray(std::forward<Args>(args)...);
    }

    template <typename... Args> auto ScopeObject(Args &&... args)
    {
        return m_builder.ScopeObject(std::forward<Args>(args)...);
    }

    constexpr operator bool() const { return true; }
};

/// Add scope methods to GenericBuilder.
template <typename stringT, typename configT>
inline GenericArray<stringT, configT>
GenericBuilder<stringT, configT>::ScopeArray()
{
    return GenericArray<stringT, configT>(*this);
}

template <typename stringT, typename configT>
template <typename keyT>
inline std::enable_if_t<is_key_v<keyT>, GenericArray<stringT, configT>>
GenericBuilder<stringT, configT>::ScopeArray(keyT &&key)
{
    return GenericArray<stringT, configT>(*this, std::forward<keyT>(key));
}

template <typename stringT, typename configT>
inline GenericObject<stringT, configT>
GenericBuilder<stringT, configT>::ScopeObject()
{
    return GenericObject<stringT, configT>(*this);
}

template <typename stringT, typename configT>
template <typename keyT>
inline std::enable_if_t<is_key_v<keyT>, GenericObject<stringT, configT>>
GenericBuilder<stringT, configT>::ScopeObject(keyT &&key)
{
    return GenericObject<stringT, configT>(*this, std::forward<keyT>(key));
}

// Type aliases for common usage
using RawBuilder = GenericBuilder<std::string>;
using RawObject = GenericObject<std::string, BasicConfig<std::string>>;
using RawArray = GenericArray<std::string, BasicConfig<std::string>>;

} // namespace wwjson

#endif /* end of include guard: WWJSON_HPP__ */
