/**
 * @file external.hpp
 * @author lymslive
 * @date 2026-01-20
 * @version 1.0.0
 *
 * @brief External library adapters for high-performance floating-point serialization.
 *
 * @details
 * This header provides integration adapters for third-party libraries that implement
 * optimized double-to-ASCII (DTOA) conversion. These can be used as alternatives to
 * the standard std::to_chars or snprintf for floating-point serialization in UnsafeConfig.
 *
 * @par Supported Libraries:
 * - rapidjson: Uses internal dtoa implementation for high-performance conversion
 * - fmt: Uses fmt library's optimized formatting for precise output
 * - yyjson: Uses yyjson's fast floating-point conversion (Schubfach algorithm)
 *
 * @par Usage:
 * Define one of the following macros before including this header:
 * - WWJSON_USE_RAPIDJSON_DTOA: Use rapidjson's internal dtoa
 * - WWJSON_USE_FMTLIB_DTOA: Use fmt library for formatting
 *
 * @note
 * The external::NumberWriter is a type alias that selects the appropriate implementation
 * based on the defined macros. Integer serialization is delegated to the internal
 * IntegerWriter implementation.
 *
 * @warning
 * These external adapters require stringT with unsafe_level >= 4 (e.g., JString, KString).
 * They use in-place writing directly to the string buffer for maximum performance.
 */

#pragma once
#ifndef WWJSON_EXTERNAL_HPP__
#define WWJSON_EXTERNAL_HPP__

#include "wwjson.hpp"
#include "itoa.hpp"

// Include external library headers based on which DTOA library is enabled
#if defined(WWJSON_USE_RAPIDJSON_DTOA)
#include <rapidjson/internal/dtoa.h>
#endif

#if defined(WWJSON_USE_FMTLIB_DTOA)
#include <fmt/format.h>
#endif

#if defined(WWJSON_USE_YYJSON_DTOA)
#include <yyjson.h>
#endif

namespace wwjson
{

namespace external
{

// ============================================================================
// rapidjson namespace - NumberWriter implementation
// ============================================================================

#if defined(WWJSON_USE_RAPIDJSON_DTOA)

namespace rapidjson
{

/// @brief High-performance double serialization using rapidjson's internal dtoa
/// @tparam stringT String type (must have unsafe_level >= 4)
template <typename stringT>
struct NumberWriter
{
    static_assert(detail::unsafe_level_v<stringT> >= 4,
        "external::rapidjson::NumberWriter requires stringT with unsafe_level >= 4");

    /// @brief Output double value using rapidjson's dtoa
    static void Output(stringT& dst, double value)
    {
        char* buffer = dst.end();
        char* end = ::rapidjson::internal::dtoa(value, buffer);
        dst.unsafe_set_end(end);
    }
};

} // namespace rapidjson

#endif // WWJSON_USE_RAPIDJSON_DTOA

// ============================================================================
// fmt namespace - NumberWriter implementation
// ============================================================================

#if defined(WWJSON_USE_FMTLIB_DTOA)

namespace fmt
{

/// @brief High-performance double serialization using fmt library
/// @tparam stringT String type (must have unsafe_level >= 4)
template <typename stringT>
struct NumberWriter
{
    static_assert(detail::unsafe_level_v<stringT> >= 4,
        "external::fmt::NumberWriter requires stringT with unsafe_level >= 4");

    /// @brief Output double value using fmt library
    static void Output(stringT& dst, double value)
    {
        char* buffer = dst.end();
        char* end = ::fmt::format_to(buffer, "{}", value);
        dst.unsafe_set_end(end);
    }
};

} // namespace fmt

#endif // WWJSON_USE_FMTLIB_DTOA

// ============================================================================
// yyjson namespace - NumberWriter implementation
// ============================================================================

#if defined(WWJSON_USE_YYJSON_DTOA)

namespace yyjson
{

/// @brief High-performance double serialization using yyjson's dtoa
/// @tparam stringT String type (must have unsafe_level >= 4)
template <typename stringT>
struct NumberWriter
{
    static_assert(detail::unsafe_level_v<stringT> >= 4,
        "external::yyjson::NumberWriter requires stringT with unsafe_level >= 4");

    /// @brief Output double value using yyjson_dtoa
    static void Output(stringT& dst, double value)
    {
        char* buffer = dst.end();
        char* end = ::yyjson_dtoa(value, buffer);
        dst.unsafe_set_end(end);
    }
};

} // namespace yyjson

#endif // WWJSON_USE_YYJSON_DTOA

// ============================================================================
// NumberWriter type alias - selects implementation based on defined macros
// ============================================================================

#if defined(WWJSON_USE_RAPIDJSON_DTOA)
template <typename stringT>
using NumberWriter = rapidjson::NumberWriter<stringT>;

#elif defined(WWJSON_USE_FMTLIB_DTOA)
template <typename stringT>
using NumberWriter = fmt::NumberWriter<stringT>;

#elif defined(WWJSON_USE_YYJSON_DTOA)
template <typename stringT>
using NumberWriter = yyjson::NumberWriter<stringT>;
#endif

} // namespace external

} // namespace wwjson

#endif /* end of include guard: WWJSON_EXTERNAL_HPP__ */
