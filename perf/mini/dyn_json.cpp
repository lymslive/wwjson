/**
 * @file dyn_json.cpp
 * @brief Dynamic JSON building with variable input
 *
 * This example demonstrates JSON building with runtime variable input.
 * Key difference from builder.cpp: the code field value comes from a variable,
 * not a compile-time constant. This tests compiler optimization behavior
 * when values cannot be fully computed at compile time.
 *
 * Compile: g++ -O2 -DNDEBUG -I../../include dyn_json.cpp -o dyn_json_linux.exe
 * Assembly: objdump -d -S dyn_json_linux.exe > dyn_json.s
 */

#include <cstdio>
#include "jbuilder.hpp"

// Convert uint32_t to decimal string in buffer
// Returns length of the string
static int itoa_u32(uint32_t value, char* buf, size_t bufsz)
{
    wwjson::UnsafeBuffer ubuf(buf, bufsz);
    wwjson::IntegerWriter<wwjson::UnsafeBuffer>::Output(ubuf, value);
    return static_cast<int>(ubuf.size());
}

// Build JSON string with code value from variable
// buf_js[64] for JSON output, buf_int[16] for integer string
static void build_json(char* buf_js, const char* buf_int, size_t len_int)
{
    using namespace wwjson;
    UnsafeBuffer ubuf(buf_js, 64);
    GenericBuilder<UnsafeBuffer, UnsafeConfig<UnsafeBuffer>> builder(std::move(ubuf));

    // Build {"code":"<value>","msg":"ok"}
    builder.BeginObject();
    builder.AddMember("code", buf_int, len_int);
    builder.AddMember("msg", "ok");
    builder.EndObject();
    builder.GetResult(); // remove tail comma

    printf("%s\n", builder.json.c_str());
}

int main()
{
    char buf_js[64];
    char buf_int[16];

    // First: use literal value (compile-time constant)
    // This should behave similarly to builder.cpp
    int len1 = itoa_u32(1122334455, buf_int, sizeof(buf_int));
    build_json(buf_js, buf_int, len1);
    printf("Literal: %s\n", buf_js);

    // Second: read value from user input (runtime variable)
    // This tests compiler optimization with unknown runtime value
    printf("Enter an integer: ");
    uint32_t user_value = 0;
    if (scanf("%u", &user_value) == 1)
    {
        int len2 = itoa_u32(user_value, buf_int, sizeof(buf_int));
        build_json(buf_js, buf_int, len2);
        printf("User input: %s\n", buf_js);
    }

    return 0;
}
