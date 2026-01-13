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
    dst.unsafe_push_back(digit[0]);
    dst.unsafe_push_back(digit[1]);
}

/// @brief Compile-time power of 10 calculation
template <uint8_t DIGIT>
constexpr uint32_t kPow10 = []() {
    static_assert(DIGIT <= 9, "Pow10 only valid for 0-9");
    uint32_t result = 1;
    for (uint8_t i = 0; i < DIGIT; ++i) result *= 10;
    return result;
}();

/// @brief Forward-writing unsigned integer helper with digit and high/low control
/// @tparam stringT String buffer type (must have unsafe_level >= 4)
/// @tparam DIGIT Number of digits to write (must be power of 2: 2, 4, 8...)
/// @tparam HIGH If true, this is the high part and may write fewer digits
template <typename stringT, uint8_t DIGIT, bool HIGH>
struct UnsignedWriter
{
    static_assert(DIGIT >= 2 && (DIGIT & (DIGIT - 1)) == 0,
        "DIGIT must be power of 2 (2, 4, 8...)");
    static_assert(DIGIT <= 8, "DIGIT > 8 not supported");

    /// @brief Output value with specified digit count using forward writing
    /// @param dst Destination string buffer
    /// @param value Value to output (must be < 10^DIGIT)
    template <typename uintT>
    static void Output(stringT& dst, uintT value)
    {
        constexpr uint32_t kHalf = kPow10<DIGIT / 2>;  // 10^(DIGIT/2)
        constexpr uint32_t kMax = kPow10<DIGIT>;      // 10^DIGIT
        assert(static_cast<uint64_t>(value) < kMax && "value must be < 10^DIGIT");

        if constexpr (HIGH)
        {
            // HIGH=true: High part, may write fewer digits
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
                    uintT high = value / kHalf;
                    uintT low = value % kHalf;
                    UnsignedWriter<stringT, DIGIT / 2, true>::Output(dst, high);
                    UnsignedWriter<stringT, DIGIT / 2, false>::Output(dst, low);
                }
            }
        }
        else
        {
            // HIGH=false: Low part, must write full digit count
            if constexpr (DIGIT == 2)
            {
                // Base case: always write 2 digits
                Output2Digits(dst, static_cast<uint8_t>(value));
            }
            else // DIGIT > 2
            {
                // Split into two halves, both use HIGH=false
                uintT high = value / kHalf;
                uintT low = value % kHalf;
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

template <typename stringT>
struct IntegerWriter
{
    static_assert(detail::unsafe_level_v<stringT> >= 4,
        "IntegerWriter requires stringT with unsafe_level >= 4 (e.g., JString, KString)");

    // =====================================================================
    // WriteUnsigned methods for each unsigned type
    // =====================================================================

    /// @brief Write unsigned 8-bit integer
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
        else
        {
            // 100-255: write 1 digit + 2 digits
            uint8_t high = value / 100;
            uint8_t low = value % 100;
            detail::OutputDigit(dst, high);
            detail::Output2Digits(dst, low);
        }
    }

    /// @brief Write unsigned 16-bit integer
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
            uint16_t high = value / 10000;
            uint16_t low = value % 10000;
            detail::OutputDigit(dst, static_cast<uint8_t>(high));
            detail::UnsignedWriter<stringT, 4, false>::Output(dst, low);
        }
    }

    /// @brief Write unsigned 32-bit integer
    static void WriteUnsigned(stringT& dst, uint32_t value)
    {
        if (value < 10000)
        {
            // 0-9999: 4 digits or fewer
            detail::UnsignedWriter<stringT, 4, true>::Output(dst, value);
        }
        else if (value < 100000000)  // 10^8
        {
            // 10000-99999999: 4 digits + 4 digits
            uint32_t high = value / 10000;
            uint32_t low = value % 10000;
            detail::UnsignedWriter<stringT, 4, true>::Output(dst, high);
            detail::UnsignedWriter<stringT, 4, false>::Output(dst, low);
        }
        else
        {
            // 100000000-4294967295: 2 digit + 8 digits
            uint32_t high = value / 100000000;  // 1-42
            uint32_t low = value % 100000000;
            detail::UnsignedWriter<stringT, 2, true>::Output(dst, high);
            detail::UnsignedWriter<stringT, 8, false>::Output(dst, low);
        }
    }

    /// @brief Write unsigned 64-bit integer
    static void WriteUnsigned(stringT& dst, uint64_t value)
    {
        // Fallback to base class for 64-bit
        NumberWriter<stringT>::Output(dst, value);
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
