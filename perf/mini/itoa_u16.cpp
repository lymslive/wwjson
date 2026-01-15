/**
 * @file itoa_u16.cpp
 * @brief Minimal example for uint16_t serialization
 *
 * This example demonstrates the IntegerWriter for uint16_t type.
 * Compile: g++ -O2 -DNDEBUG -I../../include itoa_u16.cpp -o itoa_u16_linux.exe
 * Assembly: objdump -d -S itoa_u16_linux.exe > itoa_u16.s
 */

#include <cstdio>
#include "jbuilder.hpp"

int main()
{
    uint16_t value = 12233;
    char buffer[16];
    wwjson::UnsafeBuffer ubuf(buffer);  // unsafe_level = 255
    wwjson::IntegerWriter<wwjson::UnsafeBuffer>::Output(ubuf, value);
    printf("%s\n", ubuf.c_str());
    printf("%s\n", buffer);
    return 0;
}
