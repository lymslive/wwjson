/**
 * @file itoa_back.cpp
 * @brief Minimal example for NumberWriter backward serialization
 *
 * This example demonstrates the NumberWriter for uint32_t type,
 * focusing on reverse-order buffer writing and inline analysis.
 *
 * Key differences from IntegerWriter:
 * - NumberWriter writes to temporary buffer then appends (backward)
 * - IntegerWriter writes directly to output string (forward)
 *
 * Compile: g++ -O2 -DNDEBUG -I../../include itoa_back.cpp -o itoa_back_linux.exe
 * Assembly: objdump -d -S itoa_back_linux.exe > itoa_back.s
 */

#include <cstdio>
#include "jbuilder.hpp"

// Convert uint32_t to decimal string using NumberWriter (backward buffer writing)
// Returns length of the string
static int nw_itoa_u32(uint32_t value, char* buf, size_t bufsz)
{
    wwjson::UnsafeBuffer ubuf(buf, bufsz);
    wwjson::NumberWriter<wwjson::UnsafeBuffer>::WriteUnsigned(ubuf, value);
    printf("%s\n", ubuf.c_str());
    return static_cast<int>(ubuf.size());
}

int main()
{
    char buffer[16];

    // First: use literal value (compile-time constant)
    // This should allow compiler to optimize the number conversion
    uint32_t literal_value = 1122334455;
    int len1 = nw_itoa_u32(literal_value, buffer, sizeof(buffer));
    printf("Literal value: %u -> %s (len=%d)\n", literal_value, buffer, len1);

    // Second: read value from user input (runtime variable)
    // This tests compiler optimization with unknown runtime value
    printf("Enter an integer: ");
    uint32_t user_value = 0;
    if (scanf("%u", &user_value) == 1)
    {
        int len2 = nw_itoa_u32(user_value, buffer, sizeof(buffer));
        printf("User input: %u -> %s (len=%d)\n", user_value, buffer, len2);
    }

    return 0;
}
