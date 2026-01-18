/**
 * @file itoa_u64.cpp
 * @brief Minimal example for uint64_t serialization
 *
 * This example demonstrates the IntegerWriter for uint64_t type.
 * Compile: g++ -O2 -DNDEBUG -I../../include itoa_u64.cpp -o itoa_u64_linux.exe
 * Assembly: objdump -d -S itoa_u64_linux.exe > itoa_u64.s
 */

#include <cstdio>
#include "jbuilder.hpp"

void itoa_const(uint64_t value)
{
    char buffer[32];
    wwjson::UnsafeBuffer ubuf(buffer);  // unsafe_level = 255
    wwjson::IntegerWriter<wwjson::UnsafeBuffer>::Output(ubuf, value);
    printf("const: %s\n", ubuf.c_str());
    printf("const: %s\n", buffer);
}

void itoa_var(uint64_t value)
{
    char buffer[32];
    wwjson::UnsafeBuffer ubuf(buffer);  // unsafe_level = 255
    wwjson::IntegerWriter<wwjson::UnsafeBuffer>::Output(ubuf, value);
    printf("var: %s\n", ubuf.c_str());
    printf("var: %s\n", buffer);
}

int main()
{
    itoa_const(11223344556677889900ull);
    uint64_t value;
    scanf("%lu", &value);
    itoa_var(value);
    return 0;
}
