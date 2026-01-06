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

namespace wwjson {

/// @brief Optimized config for high-unsafe-level string types
/// @details
/// Inherits from BasicConfig and provides optimized implementations for
/// string escaping when string type's unsafe_level >= 4.
///
/// @par When to use:
/// - With JString (kUnsafeLevel=4) for balanced safety and performance
/// - With KString (kUnsafeLevel=255) for maximum performance
template <typename stringT>
struct UnsafeConfig : public BasicConfig<stringT>
{
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
    /// When unsafe_level >= 4, this implementation:
    /// - Writes directly to the string's internal buffer using pointer arithmetic
    /// - Uses unsafe_set_end() to update the end pointer after writing
    /// - Falls back to the parent's safe implementation for low unsafe_level types
    ///
    /// @note This avoids temporary buffer allocation for better performance.
    static void EscapeString(stringT &dst, const char *src, size_t len)
    {
        if (wwjson_unlikely(src == nullptr)) { return; }

        if constexpr (unsafe_level_v<stringT> >= 4)
        {
            // For unsafe string types: reserve 2x space and write directly
            dst.reserve(dst.size() + len * 2);

            char* buffer = const_cast<char*>(dst.data());
            char* write_ptr = buffer + dst.size();
            const char* const buffer_end = buffer + dst.capacity();

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
        else
        {
            // Fall back to parent's safe implementation for low unsafe_level types
            BasicConfig<stringT>::EscapeString(dst, src, len);
        }
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

} // namespace wwjson

#endif // JBUILDER_HPP__
