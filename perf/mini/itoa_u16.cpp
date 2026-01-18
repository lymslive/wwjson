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

void itoa_const(uint16_t value)
{
    char buffer[16];
    wwjson::UnsafeBuffer ubuf(buffer);  // unsafe_level = 255
    wwjson::IntegerWriter<wwjson::UnsafeBuffer>::Output(ubuf, value);
    printf("const: %s\n", ubuf.c_str());
    printf("const: %s\n", buffer);
}

void itoa_var(uint16_t value)
{
    char buffer[16];
    wwjson::UnsafeBuffer ubuf(buffer);  // unsafe_level = 255
    wwjson::IntegerWriter<wwjson::UnsafeBuffer>::Output(ubuf, value);
    printf("var: %s\n", ubuf.c_str());
    printf("var: %s\n", buffer);
}

int main()
{
    itoa_const(12233);
    uint16_t value;
    scanf("%hu", &value);
    itoa_var(value);
    return 0;
}
