/**
 * @file builder.cpp
 * @brief Minimal example for JSON building
 *
 * This example demonstrates the GenericBuilder for minimal JSON construction.
 * Compile: g++ -O2 -DNDEBUG -I../../include builder.cpp -o builder_linux.exe
 * Assembly: objdump -d -S builder_linux.exe > builder.s
 */

#include <cstdio>
#include "jbuilder.hpp"

int main()
{
    using namespace wwjson;
    char buffer[64];
    UnsafeBuffer ubuf(buffer);  // unsafe_level = 255
    GenericBuilder<UnsafeBuffer, UnsafeConfig<UnsafeBuffer>> builder(std::move(ubuf));

    // Build {"code":"0","msg":"ok"}
    builder.BeginObject();
    builder.AddMember("code", "0");
    builder.AddMember("msg", "ok");
    builder.EndObject();
    builder.GetResult(); // remove tail comma

    printf("%s\n", builder.json.c_str());
    printf("%s\n", buffer);
    return 0;
}
