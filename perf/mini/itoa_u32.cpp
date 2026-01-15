/**
 * @file itoa_u32.cpp
 * @brief Minimal example for uint32_t serialization
 *
 * This example demonstrates the IntegerWriter for uint32_t type.
 * Compile: g++ -O2 -I../../include itoa_u32.cpp -o itoa_u32_linux.exe
 * Assembly: objdump -d -S itoa_u32_linux.exe > itoa_u32.s
 */

#include <cstdio>
#include "jbuilder.hpp"

int main()
{
    uint32_t value = 1122334455;
    char buffer[16];
    wwjson::UnsafeBuffer ubuf(buffer);  // unsafe_level = 255
    wwjson::IntegerWriter<wwjson::UnsafeBuffer>::Output(ubuf, value);
    printf("%s\n", ubuf.c_str());
    printf("%s\n", buffer);
    return 0;
}
