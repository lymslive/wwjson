/**
 * @file jbuilder.hpp
 * @author lymslive
 * @date 2025-12-30
 * @version 1.0.0
 *
 * @brief Combined JSON builder with optimized string types
 *
 * @details This header provides convenient type aliases that combine wwjson.hpp's
 * GenericBuilder with jstring.hpp's optimized string buffers (JString, KString).
 *
 * @note This header depends on both wwjson.hpp and jstring.hpp. It can be used
 * as a replacement for wwjson.hpp when you want better performance with the
 * optimized StringBuffer types.
 *
 * @par Type Aliases:
 * - **Builder**: GenericBuilder<JString> - Standard JSON building with safety margin
 * - **FastBuilder**: GenericBuilder<KString> - Maximum performance with single allocation
 * - **JObject**: GenericObject<JString> - RAII object wrapper for Builder
 * - **JArray**: GenericArray<JString> - RAII array wrapper for Builder
 * - **FastObject**: GenericObject<KString> - RAII object wrapper for FastBuilder
 * - **FastArray**: GenericArray<KString> - RAII array wrapper for FastBuilder
 */

#pragma once
#ifndef JBUILDER_HPP__
#define JBUILDER_HPP__

#include "wwjson.hpp"
#include "jstring.hpp"
#include "itoa.hpp"

#include <optional>

// Define WWJSON_USE_EXTERNAL_DTOA based on compile definitions from CMake
#if defined(WWJSON_USE_RAPIDJSON_DTOA) || defined(WWJSON_USE_FMTLIB_DTOA)
#define WWJSON_USE_EXTERNAL_DTOA 1
#include "external.hpp"
#else
#define WWJSON_USE_EXTERNAL_DTOA 0
#endif

namespace wwjson {

/// @brief Optimized config for high-unsafe-level string types
/// @details
/// Inherits from BasicConfig and provides optimized implementations for
/// string escaping when string type's unsafe_level >= 4.
///
/// @par When to use:
/// - With JString (kUnsafeLevel=4) for balanced safety and performance
/// - With KString (kUnsafeLevel=255) for maximum performance
///
/// @note UnsafeConfig requires stringT to have unsafe_level >= 4.
/// Using it with std::string or other low-unsafe-level types will cause a compile error.
template <typename stringT>
struct UnsafeConfig : public BasicConfig<stringT>
{
    static_assert(detail::unsafe_level_v<stringT> >= 4,
        "UnsafeConfig requires stringT with unsafe_level >= 4 (e.g., JString, KString). "
        "Use BasicConfig<std::string> for standard string types.");

    /// @brief Escape object key using the optimized EscapeString implementation
    /// @note
    /// Must override EscapeKey explicitly because static methods don't have
    /// polymorphism. Without this override, the base class EscapeKey would
    /// call BasicConfig::EscapeString instead of this derived class's EscapeString.
    static void EscapeKey(stringT &dst, const char *key, size_t len)
    {
        EscapeString(dst, key, len);
    }

    /// @brief Optimized string escaping for high-unsafe-level types
    /// @details
    /// Writes directly to the string's internal buffer using pointer arithmetic
    /// and uses unsafe_set_end() to update the end pointer after writing.
    /// This avoids temporary buffer allocation for better performance.
    ///
    /// @note Requires stringT with unsafe_level >= 4.
    static void EscapeString(stringT &dst, const char *src, size_t len)
    {
        if (wwjson_unlikely(src == nullptr)) { return; }
        if (wwjson_unlikely(len == 0)) { return; }

        // For unsafe string types: reserve 2x space and write directly
        dst.reserve_ex(len * 2);

        char* write_ptr = dst.end();

        for (size_t i = 0; i < len; ++i)
        {
            unsigned char c = static_cast<unsigned char>(src[i]);

            if (wwjson_unlikely(c >= 128))
            {
                // UTF-8 character - pass through unchanged
                *write_ptr++ = c;
            }
            else
            {
                // ASCII character - check escape table
                uint8_t escape_char = BasicConfig<stringT>::kEscapeTable[c];
                if (wwjson_unlikely(escape_char != 0))
                {
                    *write_ptr++ = '\\';
                    *write_ptr++ = escape_char;
                }
                else
                {
                    // No escaping needed
                    *write_ptr++ = c;
                }
            }
        }

        // Update end pointer using unsafe_set_end
        dst.unsafe_set_end(write_ptr);
    }

    /// @brief Integer serialization to string buffer.
    /// @tparam intT Integer type (signed or unsigned)
    /// @param[out] dst Destination string buffer
    /// @param value Integer value to serialize
    template <typename intT>
    static std::enable_if_t<std::is_integral_v<intT>, void>
    NumberString(stringT &dst, intT value)
    {
        constexpr size_t max_digits = std::numeric_limits<intT>::digits10 + 2;
        dst.reserve_ex(max_digits);
        IntegerWriter<stringT>::Output(dst, value);
    }

    /// @brief Floating-point serialization.
    /// @tparam floatT Floating-point type
    /// @param[out] dst Destination string buffer
    /// @param value Floating-point value to serialize
    /// @note Handles nan and inf as "null" per JSON specification
    template <typename floatT>
    static std::enable_if_t<std::is_floating_point_v<floatT>, void>
    NumberString(stringT &dst, floatT value)
    {
        if (wwjson_unlikely(std::isnan(value) || std::isinf(value)))
        {
            dst.reserve_ex(4);
            dst.unsafe_append("null", 4);
            return;
        }

        dst.reserve_ex(64);
#if WWJSON_USE_EXTERNAL_DTOA
        external::NumberWriter<stringT>::Output(dst, value);
#else
        NumberWriter<stringT>::Output(dst, value);
#endif
    }
};

/// @brief Builder using JString (StringBuffer<4>) for JSON construction
/// @details
/// This is the recommended builder type for most JSON serialization tasks.
/// It provides a good balance between safety and performance with a 4-byte
/// unsafe level that handles common JSON patterns efficiently.
///
/// @par When to use Builder:
/// - General-purpose JSON construction
/// - When buffer size is unknown upfront
/// - When safety margin is desired
///
/// @par Example:
/// @code
/// Builder builder;
/// builder.BeginObject();
/// builder.AddMember("name", "value");
/// builder.EndObject();
/// auto json = builder.MoveResult();
/// @endcode
using Builder = GenericBuilder<JString, UnsafeConfig<JString>>;

/// @brief Builder using KString (StringBuffer<255>) for maximum performance
/// @details
/// This builder uses KString which operates in single-allocation mode.
/// No reallocation occurs, but the caller must provide sufficient initial capacity.
///
/// @par When to use FastBuilder:
/// - When maximum performance is required
/// - When maximum buffer size is known upfront
/// - In performance-critical hot paths
///
/// @par Example:
/// @code
/// FastBuilder builder(8192);  // Pre-allocate 8KB
/// builder.BeginObject();
/// builder.AddMember("key", "value");
/// builder.EndObject();
/// auto json = builder.MoveResult();
/// @endcode
using FastBuilder = GenericBuilder<KString, UnsafeConfig<KString>>;

/// @brief RAII object wrapper for Builder
using JObject = GenericObject<JString, UnsafeConfig<JString>>;

/// @brief RAII array wrapper for Builder
using JArray = GenericArray<JString, UnsafeConfig<JString>>;

/// @brief RAII object wrapper for FastBuilder
using FastObject = GenericObject<KString, UnsafeConfig<KString>>;

/// @brief RAII array wrapper for FastBuilder
using FastArray = GenericArray<KString, UnsafeConfig<KString>>;

// ============================================================================
// to_json Helper Functions - Simplified struct-to-JSON serialization
// ============================================================================

namespace detail {

/// @brief Type trait to detect if a type is an associative container (map)
/// @details Detects containers with key_type and mapped_type where key_type is a string type
template <typename T, typename = void>
struct is_map : std::false_type {};

template <typename T>
struct is_map<T, std::void_t<
    typename T::key_type,
    typename T::mapped_type,
    std::enable_if_t<is_key_v<typename T::key_type>>
>> : std::true_type {};

/// @brief Compile-time check for map types
template <typename T>
inline constexpr bool is_map_v = is_map<T>::value;

/// @brief Type trait to detect if a type is a sequence container (vector-like)
/// @details Detects containers with begin(), end(), and value_type, excluding strings and maps
template <typename T, typename = void>
struct is_vector : std::false_type {};

template <typename T>
struct is_vector<T, std::void_t<
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end()),
    typename T::value_type
>> : std::integral_constant<bool,
    !is_key_v<T> &&
    !is_map_v<T>
> {};

/// @brief Compile-time check for vector types
template <typename T>
inline constexpr bool is_vector_v = is_vector<T>::value;

/// @brief Type trait to detect std::optional
template <typename T, typename = void>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

/// @brief Compile-time check for optional types
template <typename T>
inline constexpr bool is_optional_v = is_optional<T>::value;

/// @brief Marker class to indicate "not a key" for to_json_impl
/// @details Used to differentiate between AddMember (with key) and AddItem (without key)
struct NotKey {};

/// @brief Check if type is a scalar (string, number, bool)
/// @note Uses is_key from wwjson.hpp (strings) + arithmetic types + bool
template <typename T>
inline constexpr bool is_scalar_v =
    is_key_v<T> || std::is_arithmetic_v<std::decay_t<T>>;

/// @brief Unified to_json_impl function with compile-time key detection
/// @tparam builderT GenericBuilder type
/// @tparam keyT Key type (is_key for member, NotKey for array element)
/// @tparam valueT Value type to serialize
/// @param builder Reference to the JSON builder
/// @param key JSON object key (field name) or NotKey marker
/// @param value Value to serialize
template <typename builderT, typename keyT, typename valueT>
void to_json_impl(builderT& builder, keyT&& key, valueT&& value)
{
    using decayT = std::decay_t<valueT>;
    constexpr bool has_key = is_key_v<keyT>;

    if constexpr (is_optional_v<decayT>) {
        // Optional: serialize based on whether it has a value
        if constexpr (has_key) {
            if (value.has_value()) {
                to_json_impl(builder, std::forward<keyT>(key), value.value());
            } else {
                builder.AddMember(std::forward<keyT>(key), nullptr); // null
            }
        } else {
            if (value.has_value()) {
                to_json_impl(builder, std::forward<keyT>(key), value.value());
            } else {
                builder.AddItem(nullptr); // null
            }
        }
    }
    else if constexpr (is_scalar_v<decayT>) {
        // Scalar: use AddMember or AddItem based on key type
        if constexpr (has_key) {
            builder.AddMember(std::forward<keyT>(key), std::forward<valueT>(value));
        } else {
            builder.AddItem(std::forward<valueT>(value));
        }
    }
    else if constexpr (is_map_v<decayT>) {
        // Map: build JSON object
        if constexpr (has_key) {
            builder.AddMember(std::forward<keyT>(key));
        }
        builder.BeginObject();
        for (const auto& [k, v] : value) {
            to_json_impl(builder, k, v);
        }
        builder.EndObject();
    }
    else if constexpr (is_vector_v<decayT>) {
        // Sequence container: build JSON array
        if constexpr (has_key) {
            builder.AddMember(std::forward<keyT>(key));
        }
        builder.BeginArray();
        for (const auto& elem : value) {
            to_json_impl(builder, NotKey{}, elem);
        }
        builder.EndArray();
    }
    else {
        // Struct: assume has to_json(builder) method
        if constexpr (has_key) {
            builder.AddMember(std::forward<keyT>(key));
        }
        builder.BeginObject();
        value.to_json(builder);
        builder.EndObject();
    }
}

} // namespace detail

/// @brief Serialize a value with a key
/// @tparam builderT GenericBuilder type
/// @tparam valueT Scalar, container, or struct type
/// @param builder Reference to the JSON builder
/// @param key JSON object key (field name)
/// @param value Value to serialize
template <typename builderT, typename valueT>
void to_json(builderT& builder, const char* key, valueT&& value)
{
    detail::to_json_impl(builder, key, std::forward<valueT>(value));
}

/// @brief Serialize a value without a key (for array elements)
/// @tparam builderT GenericBuilder type
/// @tparam valueT Scalar, container, or struct type
/// @param builder Reference to the JSON builder
/// @param value Value to serialize
template <typename builderT, typename valueT>
void to_json(builderT& builder, valueT&& value)
{
    detail::to_json_impl(builder, detail::NotKey{}, std::forward<valueT>(value));
}

/// @brief Serialize a struct to JSON string using default Builder
/// @tparam structT Type with to_json(builder) method
/// @param st Struct instance to serialize
/// @return JSON string representation
template <typename structT>
std::string to_json(const structT& st)
{
    Builder builder;
    builder.BeginObject();
    st.to_json(builder);
    builder.EndObject();
    return builder.MoveResult().str();
}

// ============================================================================
// TO_JSON Macro - Simplified field serialization
// ============================================================================

/// @brief Macro to simplify struct field serialization
/// @details Expands to: wwjson::to_json(builder, "field_name", field_value)
/// @par Example:
/// @code
/// struct Person {
///     std::string name;
///     int age;
///
///     void toJson(Builder& builder) const {
///         builder.BeginObject();
///         TO_JSON(name);
///         TO_JSON(age);
///         builder.EndObject();
///     }
/// };
/// @endcode
/// @note Uses # operator to convert field name to string literal automatically
#ifndef TO_JSON
#define TO_JSON(field) wwjson::to_json(builder, #field, field)
#else
#pragma message("WARNING: TO_JSON macro is already defined elsewhere")
#endif

} // namespace wwjson

#endif // JBUILDER_HPP__
