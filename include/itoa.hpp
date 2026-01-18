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
namespace detail {

/// @brief Output single digit (0-9)
template <typename stringT>
void OutputDigit(stringT& dst, uint8_t value)
{
    assert(value < 10 && "Value must be 0-9");
    dst.unsafe_push_back('0' + value);
}

/// @brief Output 2-digit number (00-99) using digit pair lookup
template <typename stringT>
void Output2Digits(stringT& dst, uint8_t value)
{
    assert(value < 100 && "Value must be 0-99");
    const char* digit = &NumberWriter<stringT>::kDigitPairs[value].high;
    dst.unsafe_append(digit, 2);
}

/// @brief Compile-time power of 10 calculation
template <uint8_t DIGIT>
constexpr uint64_t kPow10 = []() {
    static_assert(DIGIT <= 16, "Pow10 only valid for 0-16");
    uint64_t result = 1;
    for (uint8_t i = 0; i < DIGIT; ++i) result *= 10;
    return result;
}();

/// @brief Forward-writing unsigned integer helper with digit and high/low control
///
/// @tparam stringT String buffer type (must have unsafe_level >= 4)
/// @tparam DIGIT Number of digits to write (must be power of 2: 2, 4, 8, 16...)
/// @tparam HIGH If true, this is the high part and may write fewer digits
///
/// @details This template implements a recursive divide-and-conquer algorithm for
/// writing unsigned integers to a string buffer. The algorithm works as follows:
///
/// 1. **Divide-and-Conquer Strategy**: For numbers with DIGIT > 2, the value is split
///    into two halves: high part (value / 10^(DIGIT/2)) and low part (value % 10^(DIGIT/2)).
///    Each half is processed recursively with DIGIT/2.
///
/// 2. **Recursive Termination**: When DIGIT == 2, the recursion terminates by directly
///    calling OutputDigit() (for HIGH=true and value < 10) or Output2Digits().
///
/// 3. **HIGH Flag Control**:
///    - HIGH=false: Always writes exactly DIGIT digits (e.g., "05" for 5 with DIGIT=2)
///    - HIGH=true: May write fewer digits, omitting leading zeros (e.g., "5" for 5)
///    - When HIGH=true and value < 10^(DIGIT/2), it recurses with DIGIT/2 to avoid leading zeros
///
/// 4. **Multiplication Optimization**: For specific divisors, multiplication with bit shifts
///    replaces division for better performance:
///    - Dividing by 100: `(value * 5243) >> 19`
///    - Dividing by 10000: `(value * 109951163ULL) >> 40`
///
/// This design enables fast integer serialization when combined with string types that
/// support unsafe_level >= 4 (e.g., JString, KString), allowing forward writing without
/// temporary reverse buffering.
template <typename stringT, uint8_t DIGIT, bool HIGH>
struct UnsignedWriter
{
    static_assert(DIGIT >= 2 && (DIGIT & (DIGIT - 1)) == 0,
        "DIGIT must be power of 2 (2, 4, 8, 16...)");
    static_assert(DIGIT <= 16, "DIGIT > 16 not supported");

    /// @brief Output value with specified digit count using forward writing
    /// @param dst Destination string buffer
    /// @param value Value to output (must be < 10^DIGIT)
    template <typename uintT>
    static void Output(stringT& dst, uintT value)
    {
        constexpr uint64_t kHalf = kPow10<DIGIT / 2>;  // 10^(DIGIT/2)
        constexpr uint64_t kMax = kPow10<DIGIT>;       // 10^DIGIT
        assert(static_cast<uint64_t>(value) < kMax && "value must be < 10^DIGIT");

        if constexpr (HIGH)
        {
            // HIGH=true: High part, may write fewer digits; value must be > 0
            assert(value > 0 && "HIGH=true requires value > 0");
            if constexpr (DIGIT == 2)
            {
                // Base case: 2 digits, may be 1 or 2
                if (value < 10)
                {
                    OutputDigit(dst, static_cast<uint8_t>(value));
                }
                else
                {
                    Output2Digits(dst, static_cast<uint8_t>(value));
                }
            }
            else // DIGIT > 2
            {
                if (value < kHalf)
                {
                    // Recurse to smaller digit count, keep HIGH flag
                    UnsignedWriter<stringT, DIGIT / 2, true>::Output(dst, value);
                }
                else
                {
                    // Split into two halves
                    uintT high; // value / kHalf;
                    if constexpr (kHalf == 100)
                    {
                        high = (value * 5243) >> 19;
                    }
                    else if constexpr (kHalf == 10000)
                    {
                        high = (value * 109951163ULL) >> 40;
                    }
                    else
                    {
                        high = value / kHalf;
                    }

                    uintT low = value - high * kHalf; // value % kHalf
                    UnsignedWriter<stringT, DIGIT / 2, true>::Output(dst, high);
                    UnsignedWriter<stringT, DIGIT / 2, false>::Output(dst, low);
                }
            }
        }
        else
        {
            if constexpr (DIGIT == 2)
            {
                // Base case: always write 2 digits
                Output2Digits(dst, static_cast<uint8_t>(value));
            }
            else // DIGIT > 2
            {
                // Split into two halves, both use HIGH=false
                uintT high; // value / kHalf;
                if constexpr (kHalf == 100)
                {
                    high = (value * 5243) >> 19;
                }
                else if constexpr (kHalf == 10000)
                {
                    high = (value * 109951163ULL) >> 40;
                }
                else
                {
                    high = value / kHalf;
                }

                uintT low = value - high * kHalf; // value % kHalf
                UnsignedWriter<stringT, DIGIT / 2, false>::Output(dst, high);
                UnsignedWriter<stringT, DIGIT / 2, false>::Output(dst, low);
            }
        }
    }
};

} // namespace detail

// ============================================================================
// Main IntegerWriter class
// ============================================================================

/// @brief High-performance integer serialization for high-unsafe-level string types
///
/// @tparam stringT String buffer type (must have unsafe_level >= 4)
///
/// @details IntegerWriter provides optimized integer-to-string conversion for
/// serialization to JSON. It is designed to work with string types that support
/// high-unsafe-level operations (unsafe_level >= 4), such as JString and KString.
///
/// **Design Purpose**:
/// - Enable direct writing of integer values into string buffers without
///   requiring temporary reverse-order buffering
/// - Achieve maximum performance by leveraging unsafe string operations
/// - Support all standard integer types (int8_t through uint64_t)
///
/// **Core Mechanism**:
/// - Uses UnsignedWriter's recursive divide-and-conquer algorithm to write
///   digits in forward order (from most significant to least significant)
/// - Utilizes stringT's unsafe_push_back() and unsafe_append() methods for
///   zero-copy direct writing
/// - Eliminates the need for a temporary buffer by writing directly to the
///   destination string
///
/// **Performance Advantages**:
/// - No temporary buffer allocation or copying
/// - Forward writing eliminates the need to reverse digit order
/// - Multiplication optimizations replace division for common divisors
/// - Compile-time template specialization for different integer types
///
/// **Type Requirements**:
/// - stringT must have `unsafe_push_back(char)` method
/// - stringT must have `unsafe_append(const char*, size_t)` method
/// - stringT must satisfy `unsafe_level_v<stringT> >= 4`
/// - Recommended types: JString, KString
template <typename stringT>
struct IntegerWriter
{
    static_assert(detail::unsafe_level_v<stringT> >= 4,
        "IntegerWriter requires stringT with unsafe_level >= 4 (e.g., JString, KString)");

    // =====================================================================
    // WriteUnsigned methods for each unsigned type
    // =====================================================================

    /// @brief Serialize uint8_t (0-255) to string
    ///
    /// @param dst Destination string buffer
    /// @param value Unsigned 8-bit integer value (0-255)
    ///
    /// @details Implements value range decomposition with optimized handling:
    /// - 0-9: Single digit via OutputDigit()
    /// - 10-99: Two digits via Output2Digits()
    /// - 100-199: Three digits, optimized (high=1, low=value-100)
    /// - 200-255: Three digits, optimized (high=2, low=value-200)
    ///
    /// The 100-255 range uses branch optimization instead of division/multiplication,
    /// as high can only be 1 or 2 for uint8_t values.
    static void WriteUnsigned(stringT& dst, uint8_t value)
    {
        if (value < 10)
        {
            detail::OutputDigit(dst, value);
        }
        else if (value < 100)
        {
            detail::Output2Digits(dst, value);
        }
        else if (value < 200)
        {
            // 100-199: high=1, low=value-100
            detail::OutputDigit(dst, 1);
            detail::Output2Digits(dst, value - 100);
        }
        else
        {
            // 200-255: high=2, low=value-200
            detail::OutputDigit(dst, 2);
            detail::Output2Digits(dst, value - 200);
        }
    }

    /// @brief Serialize uint16_t (0-65535) to string
    ///
    /// @param dst Destination string buffer
    /// @param value Unsigned 16-bit integer value (0-65535)
    ///
    /// @details Implements value range decomposition:
    /// - 0-9999: Up to 4 digits using UnsignedWriter<4, true>
    /// - 10000-65535: 1 digit + 4 digits using multiplication optimization
    ///   (high = (value * 109951163) >> 40, low = value - high * 10000)
    static void WriteUnsigned(stringT& dst, uint16_t value)
    {
        if (value < 10000)
        {
            // 0-9999: 4 digits or fewer
            detail::UnsignedWriter<stringT, 4, true>::Output(dst, value);
        }
        else
        {
            // 10000-65535: 1 digit + 4 digits
            // uint16_t high = value / 10000;
            // uint16_t low = value % 10000;
            uint16_t high = static_cast<uint16_t>((value * 109951163ULL) >> 40);
            uint16_t low = value - high * 10000;
            detail::OutputDigit(dst, static_cast<uint8_t>(high));
            detail::UnsignedWriter<stringT, 4, false>::Output(dst, low);
        }
    }

    /// @brief Serialize uint32_t (0-4294967295) to string
    ///
    /// @param dst Destination string buffer
    /// @param value Unsigned 32-bit integer value (0-4294967295)
    ///
    /// @details Implements value range decomposition:
    /// - 0-9999: Up to 4 digits using UnsignedWriter<4, true>
    /// - 10000-99999999: 4-8 digits with multiplication optimization
    ///   (high = (value * 109951163) >> 40)
    /// - 100000000-4294967295: 2-10 digits using triple-level decomposition
    static void WriteUnsigned(stringT& dst, uint32_t value)
    {
        if (value < 10000)
        {
            detail::UnsignedWriter<stringT, 4, true>::Output(dst, value);
        }
        else if (value < detail::kPow10<8>)
        {
            uint32_t high = (value * 109951163ULL) >> 40;
            uint32_t low  = value - high * 10000;
            detail::UnsignedWriter<stringT, 4, true>::Output(dst, high);
            detail::UnsignedWriter<stringT, 4, false>::Output(dst, low);
        }
        else
        {
            // 100000000-4294967295: 2 digit + 8 digits
            uint32_t tmp = value / 10000;
            uint32_t low  = value % 10000;
            uint32_t high = (tmp * 109951163ULL) >> 40;
            uint32_t mid = tmp - high * 10000;
            detail::UnsignedWriter<stringT, 2, true>::Output(dst, high);
            detail::UnsignedWriter<stringT, 4, false>::Output(dst, mid);
            detail::UnsignedWriter<stringT, 4, false>::Output(dst, low);
        }
    }

    /// @brief Serialize uint64_t (0-18446744073709551615) to string
    ///
    /// @param dst Destination string buffer
    /// @param value Unsigned 64-bit integer value (0-18446744073709551615)
    ///
    /// @details Implements value range decomposition:
    /// - 0-99999999: Up to 8 digits using UnsignedWriter<8, true>
    /// - 100000000-9999999999999999: 8-16 digits using standard division
    /// - 10^16-18446744073709551615: 16-20 digits using triple-level decomposition
    static void WriteUnsigned(stringT& dst, uint64_t value)
    {
        if (value < detail::kPow10<8>)
        {
            detail::UnsignedWriter<stringT, 8, true>::Output(dst, value);
        }
        else if (value < detail::kPow10<16>)
        {
            uint32_t high = value / detail::kPow10<8>;
            uint32_t low =  value % detail::kPow10<8>;
            detail::UnsignedWriter<stringT, 8, true>::Output(dst, high);
            detail::UnsignedWriter<stringT, 8, false>::Output(dst, low);
        }
        else
        {
            // 10^16-18446744073709551615: 4 digits + 16 digits
            uint64_t tmp = value / detail::kPow10<8>;
            uint32_t low =  value % detail::kPow10<8>;
            uint32_t high = tmp / detail::kPow10<8>;
            uint32_t mid =  tmp % detail::kPow10<8>;

            detail::UnsignedWriter<stringT, 4, true>::Output(dst, high);
            detail::UnsignedWriter<stringT, 8, false>::Output(dst, mid);
            detail::UnsignedWriter<stringT, 8, false>::Output(dst, low);
        }
    }

    // =====================================================================
    // Unified Output interface
    // =====================================================================

    /// @brief Output any integer type (signed or unsigned)
    template <typename intT>
    static void Output(stringT& dst, intT value)
    {
        if (value == 0)
        {
            dst.unsafe_push_back('0');
            return;
        }

        using UnsignedT = std::make_unsigned_t<intT>;
        if constexpr (std::is_signed_v<intT>)
        {
            if (value < 0)
            {
                dst.unsafe_push_back('-');
                WriteUnsigned(dst, static_cast<UnsignedT>(-value));
            }
            else
            {
                WriteUnsigned(dst, static_cast<UnsignedT>(value));
            }
        }
        else
        {
            WriteUnsigned(dst, static_cast<UnsignedT>(value));
        }
    }
};

} // namespace wwjson

#endif /* end of include guard: ITOA_HPP__ */
