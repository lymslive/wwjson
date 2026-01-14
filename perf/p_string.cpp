#include "couttast/tinytast.hpp"
#include "relative_perf.h"

#include "argv.h"

#include "wwjson.hpp"
#include "yyjson.h"
#include "jbuilder.hpp"

#include <string>
#include <type_traits>
#include <vector>

// String serialization functions for performance testing
namespace test
{
namespace wwjson
{

void BuildStringObject(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::RawBuilder builder(size_k << 10);
    builder.BeginObject();

    for (int i = 0; i < count; i++)
    {
        std::string value = std::to_string(start + i);
        std::string key = "k" + value;
        builder.AddMember(key, value);
    }

    builder.EndObject();
    dst = builder.MoveResult();
}

void BuildStringObjectJString(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::Builder builder(size_k << 10);
    builder.BeginObject();

    for (int i = 0; i < count; i++)
    {
        std::string value = std::to_string(start + i);
        std::string key = "k" + value;
        builder.AddMember(key, value);
    }

    builder.EndObject();
    dst = builder.MoveResult();
}

void BuildStringObjectKString(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::FastBuilder builder(size_k << 10);
    builder.BeginObject();

    for (int i = 0; i < count; i++)
    {
        std::string value = std::to_string(start + i);
        std::string key = "k" + value;
        builder.AddMember(key, value);
    }

    builder.EndObject();
    dst = builder.MoveResult();
}

void BuildEscapeObject(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::RawBuilder builder(size_k << 10);
    builder.BeginObject();

    const std::string sub_json = "{\"key\":\"value\"}";

    for (int i = 0; i < count; i++)
    {
        std::string key = "k" + std::to_string(start + i);
        builder.AddMemberEscape(key, sub_json);
    }

    builder.EndObject();
    dst = builder.MoveResult();
}

void BuildEscapeObjectJString(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::Builder builder(size_k << 10);
    builder.BeginObject();

    const std::string sub_json = "{\"key\":\"value\"}";

    for (int i = 0; i < count; i++)
    {
        std::string key = "k" + std::to_string(start + i);
        builder.AddMemberEscape(key, sub_json);
    }

    builder.EndObject();
    dst = builder.MoveResult();
}

void BuildEscapeObjectKString(std::string &dst, int start, int count, int size_k = 1)
{
    ::wwjson::FastBuilder builder(size_k << 10);
    builder.BeginObject();

    const std::string sub_json = "{\"key\":\"value\"}";

    for (int i = 0; i < count; i++)
    {
        std::string key = "k" + std::to_string(start + i);
        builder.AddMemberEscape(key, sub_json);
    }

    builder.EndObject();
    dst = builder.MoveResult();
}

} // namespace wwjson

namespace yyjson
{

void BuildStringObject(std::string &dst, int start, int count)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc)
    {
        dst = "{}";
        return;
    }

    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    for (int i = 0; i < count; i++)
    {
        std::string value = std::to_string(start + i);
        std::string key = "k" + value;
        yyjson_mut_val *key_str =
            yyjson_mut_strncpy(doc, key.c_str(), key.length());
        yyjson_mut_val *val_str =
            yyjson_mut_strncpy(doc, value.c_str(), value.length());
        yyjson_mut_obj_add(root, key_str, val_str);
    }

    char *json_str = yyjson_mut_write(doc, YYJSON_WRITE_NOFLAG, NULL);
    if (json_str)
    {
        dst = json_str;
        free(json_str);
    }
    else
    {
        dst = "{}";
    }

    yyjson_mut_doc_free(doc);
}

void BuildEscapeObject(std::string &dst, int start, int count)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    if (!doc)
    {
        dst = "{}";
        return;
    }

    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    const std::string sub_json = "{\"key\":\"value\"}";

    for (int i = 0; i < count; i++)
    {
        std::string key = "k" + std::to_string(start + i);
        yyjson_mut_val *key_str =
            yyjson_mut_strncpy(doc, key.c_str(), key.length());
        yyjson_mut_val *val_str =
            yyjson_mut_strncpy(doc, sub_json.c_str(), sub_json.length());
        yyjson_mut_obj_add(root, key_str, val_str);
    }

    char *json_str = yyjson_mut_write(doc, YYJSON_WRITE_NOFLAG, NULL);
    if (json_str)
    {
        dst = json_str;
        free(json_str);
    }
    else
    {
        dst = "{}";
    }

    yyjson_mut_doc_free(doc);
}

} // namespace yyjson
} // namespace test

// Relative performance tests using RelativeTimer
namespace test::perf
{

// Relative performance test for string object building
struct StringObjectRelativeTest : public test::perf::RelativeTimer<StringObjectRelativeTest>
{
    int start;
    int count;
    int size_kb;
    std::string wwjson_result;
    std::string yyjson_result;

    StringObjectRelativeTest(int s, int n, int size = 1) : start(s), count(n), size_kb(size)
    {
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildStringObject(temp, start, count, 1);
            size_kb = (temp.size() / 1024) + 1;
        }
    }

    void methodA()
    {
        test::wwjson::BuildStringObject(wwjson_result, start, count, size_kb);
    }

    void methodB()
    {
        test::yyjson::BuildStringObject(yyjson_result, start, count);
    }

    bool methodVerify()
    {
        methodA();
        methodB();
        return wwjson_result == yyjson_result;
    }

    static const char* testName() { return "StringObject Relative Test"; }
    static const char* labelA() { return "wwjson"; }
    static const char* labelB() { return "yyjson"; }
};

// Relative performance test for escaped string object building
struct EscapeObjectRelativeTest : public test::perf::RelativeTimer<EscapeObjectRelativeTest>
{
    int start;
    int count;
    int size_kb;
    std::string wwjson_result;
    std::string yyjson_result;

    EscapeObjectRelativeTest(int s, int n, int size = 1) : start(s), count(n), size_kb(size)
    {
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildEscapeObject(temp, start, count, 1);
            size_kb = (temp.size() / 1024) + 1;
        }
    }

    void methodA()
    {
        test::wwjson::BuildEscapeObject(wwjson_result, start, count, size_kb);
    }

    void methodB()
    {
        test::yyjson::BuildEscapeObject(yyjson_result, start, count);
    }

    bool methodVerify()
    {
        methodA();
        methodB();
        return wwjson_result == yyjson_result;
    }

    static const char* testName() { return "EscapeObject Relative Test"; }
    static const char* labelA() { return "wwjson"; }
    static const char* labelB() { return "yyjson"; }
};

// Relative performance test: JString (Builder) vs std::string (RawBuilder)
// FIX: Swapped methodA/methodB to fix the inverted ratio issue
struct StringObjectJStringRelativeTest
    : public test::perf::RelativeTimer<StringObjectJStringRelativeTest>
{
    int start;
    int count;
    int size_kb;
    std::string raw_result;
    std::string jstring_result;

    StringObjectJStringRelativeTest(int s, int n, int size = 1) : start(s), count(n), size_kb(size)
    {
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildStringObject(temp, start, count, 1);
            size_kb = (temp.size() / 1024) + 1;
        }
    }

    // Method A: Builder with JString (faster, should be methodA)
    void methodA()
    {
        test::wwjson::BuildStringObjectJString(jstring_result, start, count, size_kb);
    }

    // Method B: RawBuilder with std::string (slower, should be methodB)
    void methodB()
    {
        test::wwjson::BuildStringObject(raw_result, start, count, size_kb);
    }

    bool methodVerify()
    {
        methodA();
        methodB();
        return raw_result == jstring_result;
    }

    static const char* testName() { return "StringObject JString Relative Test"; }
    static const char* labelA() { return "JString"; }
    static const char* labelB() { return "std::string"; }
};

// Relative performance test: KString (FastBuilder) vs std::string (RawBuilder)
// FIX: Swapped methodA/methodB to fix the inverted ratio issue
struct StringObjectKStringRelativeTest
    : public test::perf::RelativeTimer<StringObjectKStringRelativeTest>
{
    int start;
    int count;
    int size_kb;
    std::string raw_result;
    std::string kstring_result;

    StringObjectKStringRelativeTest(int s, int n, int size = 1) : start(s), count(n), size_kb(size)
    {
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildStringObject(temp, start, count, 1);
            size_kb = (temp.size() / 1024) + 1;
        }
    }

    // Method A: FastBuilder with KString (faster, should be methodA)
    void methodA()
    {
        test::wwjson::BuildStringObjectKString(kstring_result, start, count, size_kb);
    }

    // Method B: RawBuilder with std::string (slower, should be methodB)
    void methodB()
    {
        test::wwjson::BuildStringObject(raw_result, start, count, size_kb);
    }

    bool methodVerify()
    {
        methodA();
        methodB();
        return raw_result == kstring_result;
    }

    static const char* testName() { return "StringObject KString Relative Test"; }
    static const char* labelA() { return "KString"; }
    static const char* labelB() { return "std::string"; }
};

// Relative performance test: JString (Builder) vs std::string (RawBuilder) for escape
// FIX: Swapped methodA/methodB to fix the inverted ratio issue
struct EscapeObjectJStringRelativeTest
    : public test::perf::RelativeTimer<EscapeObjectJStringRelativeTest>
{
    int start;
    int count;
    int size_kb;
    std::string raw_result;
    std::string jstring_result;

    EscapeObjectJStringRelativeTest(int s, int n, int size = 1) : start(s), count(n), size_kb(size)
    {
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildEscapeObject(temp, start, count, 1);
            size_kb = (temp.size() / 1024) + 1;
        }
    }

    // Method A: Builder with JString (faster, should be methodA)
    void methodA()
    {
        test::wwjson::BuildEscapeObjectJString(jstring_result, start, count, size_kb);
    }

    // Method B: RawBuilder with std::string (slower, should be methodB)
    void methodB()
    {
        test::wwjson::BuildEscapeObject(raw_result, start, count, size_kb);
    }

    bool methodVerify()
    {
        methodA();
        methodB();
        return raw_result == jstring_result;
    }

    static const char* testName() { return "EscapeObject JString Relative Test"; }
    static const char* labelA() { return "JString"; }
    static const char* labelB() { return "std::string"; }
};

// Relative performance test: KString (FastBuilder) vs std::string (RawBuilder) for escape
// FIX: Swapped methodA/methodB to fix the inverted ratio issue
struct EscapeObjectKStringRelativeTest
    : public test::perf::RelativeTimer<EscapeObjectKStringRelativeTest>
{
    int start;
    int count;
    int size_kb;
    std::string raw_result;
    std::string kstring_result;

    EscapeObjectKStringRelativeTest(int s, int n, int size = 1) : start(s), count(n), size_kb(size)
    {
        if (size_kb == 1)
        {
            std::string temp;
            test::wwjson::BuildEscapeObject(temp, start, count, 1);
            size_kb = (temp.size() / 1024) + 1;
        }
    }

    // Method A: FastBuilder with KString (faster, should be methodA)
    void methodA()
    {
        test::wwjson::BuildEscapeObjectKString(kstring_result, start, count, size_kb);
    }

    // Method B: RawBuilder with std::string (slower, should be methodB)
    void methodB()
    {
        test::wwjson::BuildEscapeObject(raw_result, start, count, size_kb);
    }

    bool methodVerify()
    {
        methodA();
        methodB();
        return raw_result == kstring_result;
    }

    static const char* testName() { return "EscapeObject KString Relative Test"; }
    static const char* labelA() { return "KString"; }
    static const char* labelB() { return "std::string"; }
};

} // namespace test::perf

/* ============================================================ */
/* Relative performance test */

DEF_TAST(string_object_relative, "字符串对象构建相对性能测试（wwjson vs yyjson）")
{
    test::CArgv argv;

    std::vector<int> test_counts = {10, 50, 100, 500};
    test_counts.push_back(argv.items);

    DESC("=== Testing wwjson (RawBuilder) vs yyjson ===");
    DESC("");
    for (int n : test_counts)
    {
        test::perf::StringObjectRelativeTest test(argv.start, n, 1);
        double ratio = test.runAndPrint(
            "String Object Test (items=" + std::to_string(n) + ")",
            "wwjson", "yyjson",
            argv.loop, 10
        );
        COUT(ratio < 1.0, true);

        DESC("");
    }

    DESC("=== Testing JString (Builder) vs std::string (RawBuilder) ===");
    DESC("");
    for (int n : test_counts)
    {
        // FIX: Now methodA=JString (faster), methodB=std::string (slower)
        // So ratio = timeA/timeB < 1.0 when JString is faster
        test::perf::StringObjectJStringRelativeTest test(argv.start, n, 1);
        double ratio = test.runAndPrint(
            "String Object JString Test (items=" + std::to_string(n) + ")",
            "JString", "std::string",
            argv.loop, 10
        );
        COUT(ratio < 1.0, true);

        DESC("");
    }

    DESC("=== Testing KString (FastBuilder) vs std::string (RawBuilder) ===");
    DESC("");
    for (int n : test_counts)
    {
        // FIX: Now methodA=KString (faster), methodB=std::string (slower)
        // So ratio = timeA/timeB < 1.0 when KString is faster
        test::perf::StringObjectKStringRelativeTest test(argv.start, n, 1);
        double ratio = test.runAndPrint(
            "String Object KString Test (items=" + std::to_string(n) + ")",
            "KString", "std::string",
            argv.loop, 10
        );
        COUT(ratio < 1.0, true);

        DESC("");
    }
}

DEF_TAST(string_escape_relative, "转义字符串对象构建相对性能测试（wwjson vs yyjson）")
{
    test::CArgv argv;

    std::vector<int> test_counts = {10, 50, 100, 500};
    test_counts.push_back(argv.items);

    DESC("=== Testing wwjson (RawBuilder) vs yyjson ===");
    DESC("");
    for (int n : test_counts)
    {
        test::perf::EscapeObjectRelativeTest test(argv.start, n, 1);
        double ratio = test.runAndPrint(
            "Escape Object Test (items=" + std::to_string(n) + ")",
            "wwjson", "yyjson",
            argv.loop, 10
        );
        COUT(ratio < 1.0, true);

        DESC("");
    }

    DESC("=== Testing JString (Builder) vs std::string (RawBuilder) ===");
    DESC("");
    for (int n : test_counts)
    {
        // FIX: Now methodA=JString (faster), methodB=std::string (slower)
        // So ratio = timeA/timeB < 1.0 when JString is faster
        test::perf::EscapeObjectJStringRelativeTest test(argv.start, n, 1);
        double ratio = test.runAndPrint(
            "Escape Object JString Test (items=" + std::to_string(n) + ")",
            "JString", "std::string",
            argv.loop, 10
        );
        COUT(ratio < 1.0, true);

        DESC("");
    }

    DESC("=== Testing KString (FastBuilder) vs std::string (RawBuilder) ===");
    DESC("");
    for (int n : test_counts)
    {
        // FIX: Now methodA=KString (faster), methodB=std::string (slower)
        // So ratio = timeA/timeB < 1.0 when KString is faster
        test::perf::EscapeObjectKStringRelativeTest test(argv.start, n, 1);
        double ratio = test.runAndPrint(
            "Escape Object KString Test (items=" + std::to_string(n) + ")",
            "KString", "std::string",
            argv.loop, 10
        );
        COUT(ratio < 1.0, true);

        DESC("");
    }
}
