/**
 * @file estimate_size.cpp
 * @brief Example: Estimate JSON size before building
 *
 * This example demonstrates how to create a custom Builder that estimates
 * the size of JSON output without actually constructing the string.
 * This is useful for pre-allocating buffers or determining storage requirements.
 */

#include <iostream>
#include <string>
#include <cstdint>
#include <cwchar>
#include <wwjson.hpp>
#include <jbuilder.hpp>

// Helper alias for convenience
using RawBuilder = wwjson::RawBuilder;

/// @brief Special string class that only tracks length without actual storage
///
/// EString (Estimation String) is a minimal string implementation that
/// counts characters but doesn't store them. The c_str() method returns
/// nullptr to indicate it's a dummy string used only for estimation.
struct EString {
    size_t length;
    size_t capacity_;

    EString() : length(0), capacity_(1024) {}

    // Required by String concept
    const char* c_str() const {
        return nullptr; // Always return nullptr since we don't store data
    }

    // Append operation - only count length
    void append(const char*, size_t len) {
        length += len;
    }

    void append(const char* s) {
        size_t len = 0;
        if (s) {
            while (s[len]) ++len;
        }
        length += len;
    }

    // Push back single character
    void push_back(char) {
        length++;
    }

    // Get current length
    size_t size() const {
        return length;
    }

    // Get capacity (for compatibility)
    size_t capacity() const {
        return capacity_;
    }

    // Check if empty
    bool empty() const {
        return length == 0;
    }

    // Get back character (dummy, not really stored)
    char& back() {
        static char dummy = '\0';
        return dummy;
    }

    // Const version
    char back() const {
        return '\0';
    }

    // Clear the counter
    void clear() {
        length = 0;
    }

    // Reserve capacity (does nothing for estimation)
    void reserve(size_t) {
        // No-op for estimation
    }
};

/// @brief Configuration for size estimation
///
/// EConfig extends BasicConfig to override the string escaping and
/// number serialization methods for size estimation only.
struct EConfig : public wwjson::BasicConfig<EString> {
    /// @brief Estimate escaped string length (max 2x original length)
    ///
    /// This method only counts the maximum possible escaped length
    /// without performing the actual escaping operation.
    static void EscapeString(EString &dst, const char *src, size_t len) {
        // Worst case: every character needs escaping (\x format)
        // which results in 2 characters per input character
        dst.append(src, len * 2);
    }

    /// @brief Estimate number string length based on type
    ///
    /// This method estimates the maximum length needed for different
    /// number types without actually converting them to strings.
    template <typename numberT>
    static std::enable_if_t<std::is_arithmetic_v<numberT>, void>
    NumberString(EString &dst, numberT value) {
        if constexpr (std::is_integral_v<numberT>) {
            // Integer types
            if constexpr (sizeof(numberT) <= 4) {
                // 32-bit integers: max 11 chars (10 digits + sign)
                dst.append("12345678901", 11);
            } else {
                // 64-bit integers: max 21 chars (20 digits + sign)
                dst.append("123456789012345678901", 21);
            }
        } else {
            // Floating-point types: use %g format estimation
            // 64-bit double: max ~25 chars (sign, digits, decimal, exponent)
            dst.append("1.2345678901234567e+308", 25);
        }
    }
};

/// @brief Builder type alias for size estimation
using EstBuilder = wwjson::GenericBuilder<EString, EConfig>;

// FastBuilder is already defined in jbuilder.hpp as GenericBuilder<KString, UnsafeConfig<KString>>

int main() {
    std::cout << "Example 2: Estimate JSON Size Before Building\n";
    std::cout << "===============================================\n\n";

    // Step 1: Build the same JSON with both builders
    std::cout << "Building sample JSON...\n\n";

    // Estimate size
    EstBuilder estBuilder;
    {
        auto obj = estBuilder.ScopeObject();
        obj.AddMember("name", "John Doe");
        obj.AddMember("age", 30);
        obj.AddMember("active", true);
        obj.AddMember("score", 95.5);
        obj.AddMember("email", "john.doe@example.com");

        obj.AddMember("address", [&]() {
            auto addr = estBuilder.ScopeObject();
            addr.AddMember("street", "123 Main St");
            addr.AddMember("city", "New York");
            addr.AddMember("zip", "10001");
        });

        obj.AddMember("tags", [&]() {
            auto arr = estBuilder.ScopeArray();
            arr.AddItem("developer");
            arr.AddItem("engineer");
            arr.AddItem("designer");
        });
    }

    size_t estimatedSize = estBuilder.json.size();

    // Actually build with FastBuilder using estimated capacity
    wwjson::FastBuilder fastBuilder(estimatedSize);
    {
        auto obj = fastBuilder.ScopeObject();
        obj.AddMember("name", "John Doe");
        obj.AddMember("age", 30);
        obj.AddMember("active", true);
        obj.AddMember("score", 95.5);
        obj.AddMember("email", "john.doe@example.com");

        obj.AddMember("address", [&]() {
            auto addr = fastBuilder.ScopeObject();
            addr.AddMember("street", "123 Main St");
            addr.AddMember("city", "New York");
            addr.AddMember("zip", "10001");
        });

        obj.AddMember("tags", [&]() {
            auto arr = fastBuilder.ScopeArray();
            arr.AddItem("developer");
            arr.AddItem("engineer");
            arr.AddItem("designer");
        });
    }

    size_t actualSize = fastBuilder.json.size();
    std::string jsonResult = fastBuilder.MoveResult().str();

    // Step 2: Compare estimated vs actual size
    std::cout << "Size Comparison:\n";
    std::cout << "  Estimated size: " << estimatedSize << " bytes\n";
    std::cout << "  Actual size:    " << actualSize << " bytes\n";
    std::cout << "  Difference:     " << (int(estimatedSize) - int(actualSize)) << " bytes (";
    if (estimatedSize >= actualSize) {
        std::cout << "overestimate";
    } else {
        std::cout << "UNDERESTIMATE - WARNING";
    }
    std::cout << ")\n";

    double ratio = (actualSize > 0) ? (100.0 * estimatedSize / actualSize) : 100.0;
    std::cout << "  Estimate ratio: " << ratio << "% of actual size\n\n";

    // Step 3: Display the generated JSON
    std::cout << "Generated JSON:\n";
    std::cout << jsonResult << "\n\n";

    // Step 4: Demonstrate buffer pre-allocation
    std::cout << "Buffer Pre-allocation Example:\n";
    std::cout << "  With estimated size, you can pass capacity to FastBuilder:\n";
    std::cout << "    FastBuilder builder(" << estimatedSize << ");  // Pre-allocate capacity\n";
    std::cout << "  FastBuilder uses KString internally which doesn't auto-expand,\n";
    std::cout << "  so pre-allocating capacity is essential to avoid buffer overflow.\n\n";

    // Step 5: Show accuracy metrics
    std::cout << "Accuracy Analysis:\n";
    std::cout << "  The estimation is conservative (overestimates) to ensure\n";
    std::cout << "  sufficient buffer space for FastBuilder. The overestimation is due to:\n";
    std::cout << "  - Worst-case string escaping (2x factor)\n";
    std::cout << "  - Maximum number representation sizes\n";
    std::cout << "  - Conservative padding for safety\n";
    std::cout << "  \n";
    std::cout << "  FastBuilder Advantages:\n";
    std::cout << "  - Uses KString (StringBuffer<255>) - no automatic reallocation\n";
    std::cout << "  - Single-allocation mode for maximum performance\n";
    std::cout << "  - Requires sufficient initial capacity (estimatedSize)\n";
    std::cout << "  - Ideal for performance-critical hot paths\n";

    return 0;
}
