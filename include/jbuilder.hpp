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
using Builder = GenericBuilder<JString>;

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
/// KString buffer(8192);  // Pre-allocate 8KB
/// FastBuilder builder(buffer);
/// builder.BeginObject();
/// builder.AddMember("key", "value");
/// builder.EndObject();
/// auto json = builder.MoveResult();
/// @endcode
using FastBuilder = GenericBuilder<KString>;

/// @brief RAII object wrapper for Builder
using JObject = GenericObject<JString, BasicConfig<JString>>;

/// @brief RAII array wrapper for Builder
using JArray = GenericArray<JString, BasicConfig<JString>>;

/// @brief RAII object wrapper for FastBuilder
using FastObject = GenericObject<KString, BasicConfig<KString>>;

/// @brief RAII array wrapper for FastBuilder
using FastArray = GenericArray<KString, BasicConfig<KString>>;

} // namespace wwjson

#endif // JBUILDER_HPP__
