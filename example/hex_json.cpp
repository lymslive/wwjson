/**
 * @file hex_json.cpp
 * @brief Example: Build JSON with hexadecimal string and integer representation
 *
 * This example demonstrates how to create a custom configuration (HexConfig)
 * that converts all strings and integers to hexadecimal representation.
 *
 * - Strings: Each byte is converted to two hexadecimal digits
 * - Integers: Converted to hexadecimal with '0x' prefix
 * - Floating-point: Formatted using %g (standard format)
 */

#include <wwjson/wwjson.hpp>
#include <wwjson/jstring.hpp>

#include <iostream>
#include <string>
#include <cstdint>
#include <array>
#include <cwchar>

// Helper alias for convenience
using RawBuilder = wwjson::RawBuilder;

/// @brief Configuration for hexadecimal JSON representation
///
/// HexConfig extends BasicConfig to customize string escaping and number
/// serialization for hexadecimal representation.
/// This is a template to work with different string types (std::string, JString, etc.)
template <typename stringT>
struct HexConfig : public wwjson::BasicConfig<stringT> {
    /// Enable automatic string value escaping
    static constexpr bool kEscapeValue = true;

    /// Quote numeric values as strings (since we're adding 0x prefix)
    static constexpr bool kQuoteNumber = true;

    /// @brief Convert string bytes to hexadecimal representation
    ///
    /// Each byte in the input string is converted to two hexadecimal
    /// digits (0-9, a-f). Non-ASCII bytes are also converted.
    static void EscapeString(stringT &dst, const char *src, size_t len) {
        if (src == nullptr) { return; }

        static constexpr char hex_table[] = "0123456789abcdef";

        for (size_t i = 0; i < len; ++i) {
            unsigned char c = static_cast<unsigned char>(src[i]);
            dst.push_back(hex_table[c >> 4]);   // High nibble
            dst.push_back(hex_table[c & 0x0F]); // Low nibble
        }
    }

    /// @brief Convert numbers to hexadecimal representation (integers) or standard format (floats)
    ///
    /// - Integers: Converted to hexadecimal with '0x' prefix
    /// - Floating-point: Formatted using %g (standard format)
    template <typename numberT>
    static std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    NumberString(stringT &dst, numberT value) {
        if constexpr (std::is_integral_v<numberT>) {
            // Integer: convert to hexadecimal with 0x prefix
            dst.append("0x");

            if (value == 0) {
                dst.push_back('0');
                return;
            }

            // Determine sign and work with positive value
            bool is_negative = false;
            uint64_t abs_value;

            if constexpr (std::is_signed_v<numberT>) {
                if (value < 0) {
                    is_negative = true;
                    abs_value = static_cast<uint64_t>(-static_cast<int64_t>(value));
                } else {
                    abs_value = static_cast<uint64_t>(value);
                }
            } else {
                abs_value = static_cast<uint64_t>(value);
            }

            // Calculate number of hex digits
            uint64_t temp = abs_value;
            int digit_count = 0;
            while (temp != 0) {
                temp >>= 4;
                digit_count++;
            }

            // Build hex string in reverse order
            std::array<char, 16> hex_digits;
            int pos = digit_count;
            while (abs_value != 0 && pos > 0) {
                uint8_t nibble = static_cast<uint8_t>(abs_value & 0x0F);
                hex_digits[--pos] = "0123456789abcdef"[nibble];
                abs_value >>= 4;
            }

            // Append hex digits
            for (int i = 0; i < digit_count; ++i) {
                dst.push_back(hex_digits[i]);
            }

        } else {
            // Floating-point: use standard %g format
            // This uses the parent implementation
            wwjson::BasicConfig<stringT>::NumberString(dst, value);
        }
    }
};

int main() {
    std::cout << "Example 3: Hexadecimal JSON Representation\n";
    std::cout << "===========================================\n\n";

    // Step 1: Build JSON with HexConfig using JString
    std::cout << "Building JSON with HexConfig...\n\n";

    using HexBuilder = wwjson::GenericBuilder<std::string, HexConfig<std::string>>;

    HexBuilder hexBuilder;
    {
        auto obj = hexBuilder.ScopeObject();

        // String example: each byte becomes 2 hex digits
        obj.AddMember("message", "Hello");
        // "Hello" in hex: 48 65 6c 6c 6f -> "48656c6c6f"

        // Integer examples: converted to hexadecimal with 0x prefix
        obj.AddMember("zero", 0);
        obj.AddMember("small_positive", 10);   // 0xa
        obj.AddMember("small_negative", -10);  // 0xa
        obj.AddMember("large_positive", 255);  // 0xff
        obj.AddMember("large_negative", -255); // 0xff
        obj.AddMember("big_number", 4095);    // 0xfff
        obj.AddMember("bigger_number", 65535); // 0xffff

        // Floating-point: uses standard %g format
        obj.AddMember("float_value", 3.14);
        obj.AddMember("double_value", 2.718281828);

        // Complex nested structure
        obj.AddMember("nested", [&]() {
            auto nested = hexBuilder.ScopeObject();
            nested.AddMember("name", "Test");
            nested.AddMember("code", 42);
        });

        // Array with mixed types
        obj.AddMember("array", [&]() {
            auto arr = hexBuilder.ScopeArray();
            arr.AddItem("ABC");
            arr.AddItem(123);
            arr.AddItem(1.5);
        });
    }

    std::string hexJson = hexBuilder.MoveResult();

    std::cout << "Hexadecimal JSON Output:\n";
    std::cout << hexJson << "\n\n";

    std::cout << "JSON Length: " << hexJson.length() << " characters\n\n";

    // Step 2: Show what each string looks like in hex
    std::cout << "String Conversion Examples:\n";
    std::cout << "  \"Hello\" -> \"";
    std::string hexHello;
    HexConfig<std::string>::EscapeString(hexHello, "Hello", 5);
    std::cout << hexHello << "\"\n";

    std::cout << "  \"ABC\" -> \"";
    std::string hexABC;
    HexConfig<std::string>::EscapeString(hexABC, "ABC", 3);
    std::cout << hexABC << "\" (A=0x41, B=0x42, C=0x43)\n";

    std::cout << "  \"Test\" -> \"";
    std::string hexTest;
    HexConfig<std::string>::EscapeString(hexTest, "Test", 4);
    std::cout << hexTest << "\"\n\n";

    // Step 3: Show integer conversions
    std::cout << "Integer Conversion Examples:\n";
    std::cout << "  10 -> \"";
    std::string hex10;
    HexConfig<std::string>::NumberString(hex10, 10);
    std::cout << hex10 << "\" (decimal 10 = hex 0xa)\n";

    std::cout << "  255 -> \"";
    std::string hex255;
    HexConfig<std::string>::NumberString(hex255, 255);
    std::cout << hex255 << "\" (decimal 255 = hex 0xff)\n";

    std::cout << "  4095 -> \"";
    std::string hex4095;
    HexConfig<std::string>::NumberString(hex4095, 4095);
    std::cout << hex4095 << "\" (decimal 4095 = hex 0xfff)\n";

    std::cout << "  65535 -> \"";
    std::string hex65535;
    HexConfig<std::string>::NumberString(hex65535, 65535);
    std::cout << hex65535 << "\" (decimal 65535 = hex 0xffff)\n\n";

    // Step 4: Use JString with HexConfig for comparison
    std::cout << "Using JString with HexConfig:\n";
    using JString = wwjson::JString;
    using JHexBuilder = wwjson::GenericBuilder<JString, HexConfig<JString>>;

    JHexBuilder jHexBuilder;
    {
        auto obj = jHexBuilder.ScopeObject();
        obj.AddMember("string", "JString");
        obj.AddMember("number", 255);
    }
    jHexBuilder.GetResult();

    std::cout << "JString capacity: " << jHexBuilder.json.capacity() << "\n";
    std::cout << "JString size: " << jHexBuilder.json.size() << "\n";
    std::cout << "JString result: " << jHexBuilder.json.c_str() << "\n\n";

    // Step 5: Explain the configuration
    std::cout << "Configuration Details:\n";
    std::cout << "  HexConfig<std::string>::kEscapeValue = true (string escaping enabled)\n";
    std::cout << "  HexConfig<std::string>::kQuoteNumber = true (numbers quoted as strings)\n";
    std::cout << "  EscapeString converts each byte to 2 hex digits\n";
    std::cout << "  NumberString converts integers to hex (0x prefix), floats use %g\n";

    return 0;
}
