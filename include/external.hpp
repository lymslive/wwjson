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
        if (wwjson_unlikely(std::isnan(value)))
        {
            dst.unsafe_append("null", 4);
            return;
        }
        if (wwjson_unlikely(std::isinf(value)))
        {
            dst.unsafe_append("null", 4);
            return;
        }

        if (value < 0)
        {
            dst.unsafe_push_back('-');
            value = -value;
        }

        dst.reserve_ex(32);
        char* buffer = dst.end();
        const char* result = ::rapidjson::internal::Dtoa(buffer, value);

        if (result)
        {
            size_t len = ::strlen(result);
            dst.unsafe_set_end(buffer + len);
        }
        else
        {
            wwjson::NumberWriter<stringT>::Output(dst, value);
        }
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
        if (wwjson_unlikely(std::isnan(value)))
        {
            dst.unsafe_append("null", 4);
            return;
        }
        if (wwjson_unlikely(std::isinf(value)))
        {
            dst.unsafe_append("null", 4);
            return;
        }

        if (value < 0)
        {
            dst.unsafe_push_back('-');
            value = -value;
        }

        // Reserve space and write directly to buffer end for maximum performance
        dst.reserve_ex(32);
        char* write_ptr = dst.end();
        // Use fmt's default format which produces shortest representation
        auto end = ::fmt::format_to(write_ptr, "{}", value);
        size_t len = static_cast<size_t>(end - write_ptr);
        dst.unsafe_set_end(write_ptr + len);
    }
};

} // namespace fmt

#endif // WWJSON_USE_FMTLIB_DTOA

// ============================================================================
// NumberWriter type alias - selects implementation based on defined macros
// ============================================================================

#if defined(WWJSON_USE_RAPIDJSON_DTOA)
template <typename stringT>
using NumberWriter = rapidjson::NumberWriter<stringT>;

#elif defined(WWJSON_USE_FMTLIB_DTOA)
template <typename stringT>
using NumberWriter = fmt::NumberWriter<stringT>;
#endif

} // namespace external

} // namespace wwjson

#endif /* end of include guard: WWJSON_EXTERNAL_HPP__ */
