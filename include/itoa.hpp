/**
 * @file itoa.hpp
 * @author lymslive
 * @date 2026-01-12
 * @version 1.0.0
 *
 * @brief Integer to string conversion optimized for JSON serialization.
 *
 * @details This header provides IntegerWriter class for high-performance integer
 * serialization. When combined with high-unsafe-level string types (like JString,
 * KString), it can write integers directly from the end of the string buffer,
 * avoiding the need for temporary reverse-order buffering.
 *
 * @note This header depends on wwjson.hpp for NumberWriter base class.
 * Include this after wwjson.hpp or jbuilder.hpp.
 * */

#pragma once
#ifndef ITOA_HPP__
#define ITOA_HPP__

#include "wwjson.hpp"

namespace wwjson {

/// @brief High-performance integer writer for JSON serialization
/// @details
/// Provides optimized integer-to-string conversion. When used with string types
/// that have unsafe_level >= 4, this class writes integers directly from the
/// end of the buffer (forward writing), avoiding the overhead of reverse-order
/// buffering used in traditional itoa implementations.
///
/// @par Requirements:
/// - stringT must have unsafe_level >= 4 (e.g., JString, KString)
/// - stringT must provide reserve_ex() and unsafe_set_end() methods
///
/// @tparam stringT String type that satisfies UnsafeStringConcept with unsafe_level >= 4
template <typename stringT>
struct IntegerWriter : public NumberWriter<stringT>
{
    /// @brief Check that stringT has sufficient unsafe level for forward writing
    static_assert(detail::unsafe_level_v<stringT> >= 4,
        "IntegerWriter requires stringT with unsafe_level >= 4 (e.g., JString, KString)");
};

} // namespace wwjson

#endif /* end of include guard: ITOA_HPP__ */
