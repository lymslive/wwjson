/**
 * @file t_bufferview.cpp
 * @author lymslive
 * @date 2025-12-29
 * @brief Tests for BufferView and UnsafeBuffer classes from include/jstring.hpp
 */
#include "couttast/tinytast.hpp"
#include "jstring.hpp"
#include "couttast/couthex.hpp"
#include <array>
#include <charconv>
#include <string>
#include <vector>

using namespace wwjson;

/// @brief Test for BufferView base class
/// @{
DEF_TAST(bufv_layout, "BufferView 布局大小测试")
{
    size_t ptr = sizeof((void*)nullptr);
    COUT(sizeof(BufferView), 3 * ptr);
    COUT(sizeof(JString), 3 * ptr);
    COUT(sizeof(UnsafeBuffer), 3 * ptr);
}

DEF_TAST(bufv_invariants, "BufferView 不变关系式测试")
{
    char buffer[256];
    BufferView view(buffer, 256);

    DESC("初始构造状态");
    {
        // capacity == cap_end - begin
        COUT(view.capacity(), 255);
        COUT(view.capacity(), view.cap_end() - view.begin());

        // size = end - begin
        // empty() == (m_end == m_begin)
        COUT(view.size(), 0);
        COUT(view.size(), view.end() - view.begin());
        COUT(view.empty(), true);

        // full() == !empty() (初始为空，不满)
        COUT(view.full(), false);

        // reserve_ex = cap_end - end
        COUT(view.reserve_ex(), view.cap_end() - view.end());
        COUT(view.reserve_ex(), view.capacity() - view.size());
        COUT(view.reserve_ex(), 255);

        // reserve_ex() == 0 当且仅当 full() 为真
        COUT(view.reserve_ex() == 0, view.full());

        // size() == capacity() 当且仅当 full() 为真
        COUT(view.size() == view.capacity(), view.full());

        // 零字符封尾
        COUT(*view.cap_end() == '\0', true);
    }

    DESC("写入一些内容");
    {
        view.unsafe_append("hello", 5);

        COUT(view.empty(), false);
        COUT(view.size(), 5);

        COUT(view.full(), false);
        COUT(view.reserve_ex() > 0, true);
        COUT(view.reserve_ex() > 0, !view.full());
        COUT(view.capacity(), view.cap_end() - view.begin());
        COUT(view.size(), view.end() - view.begin());
        COUT(view.reserve_ex(), view.cap_end() - view.end());
        COUT(view.reserve_ex(), view.capacity() - view.size());

        COUT(*view.cap_end() == '\0', true);
    }

    DESC("清空内容");
    {
        view.clear();
        COUT(view.size(), 0);
        COUT(view.empty(), true);
        COUT(view.size() == 0, view.empty());
        COUT(view.size() == 0, !view.full());
        COUT(view.reserve_ex(), view.capacity() - view.size());
        COUT(*view.cap_end() == '\0', true);
    }

    DESC("填满内容");
    {
        view.fill('x');
        COUT(view.size(), 0);
        COUT(view.empty(), true);

        view.fill('x', view.capacity());
        COUT(view.size(), view.capacity());
        COUT(view.empty(), false);
        COUT(view.full(), true);
        COUT(view.reserve_ex(), 0);

        COUT(view.capacity(), view.cap_end() - view.begin());
        COUT(view.size(), view.end() - view.begin());
        COUT(view.reserve_ex(), view.cap_end() - view.end());
        COUT(view.reserve_ex(), view.capacity() - view.size());
        COUT(*view.cap_end() == '\0', true);
    }
}

DEF_TAST(bufv_constructors, "BufferView 构造函数测试")
{
    DESC("constructor from char* and size");
    {
        char buffer[256];
        BufferView bv(buffer, 256);

        COUT(bv.size(), 0);
        COUT(bv.capacity(), 255);
        COUT(static_cast<bool>(bv), true);

        bv.append("hello");
        COUT(bv.size(), 5);
        COUT(strncmp(bv.data(), "hello", 5), 0);
    }

    DESC("constructor from C array");
    {
        char buffer[128];
        BufferView bv(buffer);

        COUT(bv.capacity(), 127);

        bv.append("test");
        COUT(bv.size(), 4);
        COUT(strncmp(bv.data(), "test", 4), 0);
    }

    DESC("constructor from std::array");
    {
        std::array<char, 64> arr;
        BufferView bv(arr);

        COUT(bv.capacity(), 63);

        bv.append("array test");
        COUT(bv.size(), 10);
        COUT(strncmp(bv.data(), "array test", 10), 0);
    }

    DESC("constructor from std::string");
    {
        std::string str;
        str.reserve(512);

        BufferView bv(str);
        // Note: BufferView will always use last byte for \0
        COUT(bv.capacity(), str.capacity() - 1);

        bv.append("borrowed string");
        COUT(bv.size(), 15);

        // Note: str.size() is still 0, BufferView doesn't sync size
        COUT(str.size(), 0);
        COUT(strncmp(bv.data(), "borrowed string", 15), 0);
    }

    DESC("constructor from std::vector<char>");
    {
        std::vector<char> vec;
        vec.reserve(1024);

        BufferView bv(vec);
        COUT(bv.capacity(), vec.capacity() - 1);

        bv.append("vector content");
        COUT(bv.size(), 14);

        // Note: vec.size() is still 0, BufferView doesn't sync size
        COUT(vec.size(), 0);
        COUT(strncmp(bv.data(), "vector content", 14), 0);
    }
}

DEF_TAST(bufv_move_constructor, "BufferView 移动构造测试")
{
    char buffer[64];
    BufferView bv1(buffer);
    bv1.append("test data longer");

    // Move construct
    BufferView bv2(std::move(bv1));

    // bv2 should have data
    COUT(bv2.size(), 16);
    COUT(std::string(bv2), "test data longer");

    // bv1 should be empty (nullptr pointers)
    COUT(static_cast<bool>(bv1), false);
    COUT(bv1.size(), 0);
    COUT(bv1.capacity(), 0);

    // Move assignment
    BufferView bv3;
    bv3 = std::move(bv2);
    COUT(bv3.size(), 16);
    COUT(std::string(bv3), "test data longer");
    COUT(static_cast<bool>(bv2), false);
    COUT(bv2.size(), 0);
    COUT(bv2.capacity(), 0);
}

/// 测试 M2: Edge pointer and element access 方法组
DEF_TAST(bufv_ends_access, "BufferView 两端指针及元素获取")
{
    DESC("edge pointer access");
    {
        char buffer[64];
        BufferView bv(buffer);
        bv.append("hello world");

        COUT(static_cast<void*>(bv.begin()) == static_cast<void*>(buffer), true);
        COUT(static_cast<void*>(bv.end()) == static_cast<void*>(buffer + 11), true);
        COUT(static_cast<void*>(bv.cap_end()) == static_cast<void*>(buffer + 63), true);
        COUT(bv.capacity(), 63);

        const char* const_begin = static_cast<const BufferView&>(bv).begin();
        const char* const_end = static_cast<const BufferView&>(bv).end();
        const char* const_cap_end = static_cast<const BufferView&>(bv).cap_end();
        COUT(const_begin == bv.begin(), true);
        COUT(const_end == bv.end(), true);
        COUT(const_cap_end == bv.cap_end(), true);
    }

    DESC("element access: front and back");
    {
        char buffer[32];
        BufferView bv(buffer);
        bv.append("test");

        COUT(bv.front(), 't');
        COUT(bv.back(), 't');

        bv.append("ing");
        COUT(bv.front(), 't');
        COUT(bv.back(), 'g');

        const char& const_front = static_cast<const BufferView&>(bv).front();
        const char& const_back = static_cast<const BufferView&>(bv).back();
        COUT(const_front, 't');
        COUT(const_back, 'g');

        bv.pop_back();
        COUT(bv.back(), 'n');
        bv.pop_back();
        COUT(bv.back(), 'i');
        COUT(bv.size(), 5);
    }

    DESC("bool operator");
    {
        char buffer[32];
        BufferView bv(buffer);
        COUT(static_cast<bool>(bv), true);

        BufferView empty_bv;
        COUT(static_cast<bool>(empty_bv), false);

        const BufferView& const_bv = bv;
        COUT(static_cast<bool>(const_bv), true);
    }

    DESC("edge pointer on empty buffer");
    {
        char buffer[32];
        BufferView bv(buffer);

        COUT(static_cast<void*>(bv.begin()) == static_cast<void*>(buffer), true);
        COUT(static_cast<void*>(bv.end()) == static_cast<void*>(buffer), true);
        COUT(bv.begin() == bv.end(), true);
    }

    DESC("element access modifications");
    {
        char buffer[32];
        BufferView bv(buffer);
        bv.append("abcde");

        bv.front() = 'A';
        bv.back() = 'E';
        COUT(bv.front(), 'A');
        COUT(bv.back(), 'E');
        COUT(std::string(bv), "AbcdE");
    }
}

/// 测试 M3: String conversion 方法组
DEF_TAST(bufv_str_converstion, "BufferView 转换字符串")
{
    DESC("c_str conversion with null terminator");
    {
        char buffer[64];
        BufferView bv(buffer);
        bv.append("hello");

        const char* cstr = bv.c_str();
        COUT(strcmp(cstr, "hello") == 0, true);
        COUT(cstr[5], '\0');

        bv.append(" world");
        cstr = bv.c_str();
        COUT(strcmp(cstr, "hello world") == 0, true);
        COUT(cstr[11], '\0');
    }

    DESC("str method conversion");
    {
        char buffer[64];
        BufferView bv(buffer);
        bv.append("test string");

        std::string result = bv.str();
        COUT(result, std::string("test string"));
        COUT(result.size(), 11);
    }

    DESC("implicit string_view conversion");
    {
        char buffer[64];
        BufferView bv(buffer);
        bv.append("string_view");

        std::string_view sv = bv;
        COUT(sv == std::string_view("string_view"), true);
        COUT(sv.size(), 11);

        std::string from_sv(sv);
        COUT(from_sv, std::string("string_view"));
    }

    DESC("explicit std::string conversion");
    {
        char buffer[64];
        BufferView bv(buffer);
        bv.append("explicit string");

        std::string s = static_cast<std::string>(bv);
        COUT(s, std::string("explicit string"));
        COUT(s.size(), 15);
    }

    DESC("c_str on empty buffer");
    {
        char buffer[32];
        BufferView bv(buffer);

        const char* cstr = bv.c_str();
        COUT(strcmp(cstr, "") == 0, true);
    }

    DESC("c_str on null buffer");
    {
        BufferView bv;

        const char* cstr = bv.c_str();
        COUT(strcmp(cstr, "") == 0, true);
    }

    DESC("str and conversions preserve buffer content");
    {
        char buffer[128];
        BufferView bv(buffer);
        bv.append("buffer content");

        std::string str1 = bv.str();
        std::string_view sv = bv;
        std::string str2 = static_cast<std::string>(bv);

        COUT(str1 == str2, true);
        COUT(std::string(sv) == str1, true);
        COUT(str1.size(), 14);
    }
}

/// 测试 M4: Safe write operations 方法组
DEF_TAST(bufv_write_safe, "BufferView 写入方法测试")
{
    DESC("push_back with boundary checking");
    {
        char buffer[16];
        BufferView bv(buffer);

        bv.push_back('H');
        bv.push_back('e');
        bv.push_back('l');
        bv.push_back('l');
        bv.push_back('o');

        COUT(bv.size(), 5);
        COUT(std::string(bv), "Hello");

        // Try push_back beyond capacity in safe mode
        for (int i = 0; i < 20; ++i) bv.push_back('x');
        COUT(bv.size(), bv.capacity());
        COUT(bv.overflow(), false);
        COUT(bv.reserve_ex(), 0);  // Full
    }

    DESC("append with boundary checking");
    {
        char buffer[32];
        BufferView bv(buffer);

        bv.append("safe write");
        COUT(bv.size(), 10);
        COUT(std::string(bv), std::string("safe write"));

        // Try to write beyond capacity - should be rejected
        bv.append("this text is way too long and should be rejected");
        COUT(bv.overflow(), false);  // Overflow not triggered (write rejected)
        COUT(bv.reserve_ex(), 21);  // 31 capacity - 10 written = 21 remaining
        COUT(bv.size(), 10);        // Size unchanged
    }

    DESC("append(count, ch) with boundary checking");
    {
        char buffer[32];
        BufferView bv(buffer);

        bv.append("hello");
        bv.append(5, '!');
        COUT(bv.size(), 10);
        COUT(std::string(bv), "hello!!!!!");

        // Try to append beyond capacity - should be rejected
        bv.append(50, 'x');
        COUT(bv.size(), 10);  // Unchanged
        COUT(bv.overflow(), false);
        COUT(bv.reserve_ex(), 21);  // 31 capacity - 10 written = 21 remaining
    }

    DESC("fill with boundary checking");
    {
        char buffer[64];
        BufferView bv(buffer);

        bv.append("prefix");
        bv.fill('-', 5);
        COUT(bv.size(), 11);
        COUT(std::string(bv), "prefix-----");

        // Fill rest of buffer using remaining capacity
        size_t before_size = bv.size();
        size_t remaining = bv.capacity() - before_size;
        bv.fill('x', remaining);  // Fill remaining
        COUT(bv.full(), true);
        COUT(bv.overflow(), false);
        COUT(bv.reserve_ex(), 0);  // Full
    }

    DESC("resize with boundary checking");
    {
        char buffer[64];
        BufferView bv(buffer);

        bv.append("test content");
        COUT(bv.size(), 12);

        bv.resize(5);
        COUT(bv.size(), 5);
        COUT(std::string(bv, 0, 5), std::string("test "));

        // Try to resize beyond capacity - should be rejected
        bv.resize(100);
        COUT(bv.size(), 5);  // Unchanged
        COUT(bv.overflow(), false);
        COUT(bv.reserve_ex(), 58);  // 63 capacity - 5 written = 58 remaining
    }
}

/// 测试 M5: Unsafe write operations 方法组
DEF_TAST(bufv_write_unsafe, "BufferView 不检查边界的写入方法测试")
{
    DESC("unsafe_push_back");
    {
        char buffer[8];
        BufferView bv(buffer);

        bv.unsafe_push_back('H');
        bv.unsafe_push_back('e');
        bv.unsafe_push_back('l');
        bv.unsafe_push_back('l');
        bv.unsafe_push_back('o');

        COUT(bv.size(), 5);
        COUT(std::string(bv), std::string("Hello"));
    }

    DESC("unsafe_push_back overflow");
    {
        char buffer[4+4];
        BufferView bv(buffer, 4);

        bv.unsafe_push_back('H');
        bv.unsafe_push_back('e');
        bv.unsafe_push_back('l');
        bv.unsafe_push_back('l');
        bv.unsafe_push_back('o');

        COUT(bv.capacity(), 3);
        COUT(bv.size(), 5);
        COUT(std::string(bv), std::string("Hello"));
    }

    DESC("unsafe_append with length");
    {
        char buffer[64];
        BufferView bv(buffer);

        bv.unsafe_append("unsafe", 6);
        COUT(bv.size(), 6);
        COUT(std::string(bv), std::string("unsafe"));

        bv.unsafe_append(" append", 7);
        COUT(bv.size(), 13);
        COUT(std::string(bv), std::string("unsafe append"));
    }

    DESC("unsafe_fill");
    {
        char buffer[64];
        BufferView bv(buffer);

        bv.append("prefix");
        bv.unsafe_fill('-', 5);
        COUT(bv.size(), 11);
        COUT(std::string(bv), std::string("prefix-----"));

        bv.unsafe_fill('x', 5);
        COUT(bv.size(), 16);
        COUT(std::string(bv), std::string("prefix-----xxxxx"));
    }

    DESC("unsafe_resize");
    {
        char buffer[64];
        BufferView bv(buffer);

        bv.append("test content");
        COUT(bv.size(), 12);

        bv.unsafe_resize(5);
        COUT(bv.size(), 5);
        COUT(std::string(bv, 0, 5), std::string("test "));

        bv.unsafe_resize(10);
        COUT(bv.size(), 10);
        COUT(bv.capacity(), 63);
    }

    DESC("unsafe_set_end");
    {
        char buffer[64];
        BufferView bv(buffer);
        bv.append("hello world");

        char* old_end = bv.end();
        bv.unsafe_set_end(bv.begin() + 5);
        COUT(bv.size(), 5);
        COUT(std::string(bv), std::string("hello"));

        bv.unsafe_set_end(old_end);
        COUT(bv.size(), 11);
        COUT(std::string(bv), std::string("hello world"));
    }

    DESC("unsafe_end_cstr");
    {
        char buffer[64];
        BufferView bv(buffer);
        bv.append("no null");

        bv.unsafe_end_cstr();
        COUT(bv.size(), 7);
        COUT(*bv.end(), '\0');

        const char* cstr = bv.c_str();
        COUT(strcmp(cstr, "no null") == 0, true);
    }

    DESC("unsafe operations can cause overflow (with protected buffer)");
    {
        char buffer[16 + 32];  // Extra 32 bytes to protect against real overflow
        BufferView bv(buffer, 16);  // Only use first 16 bytes

        // Safe write first
        bv.append("safe");
        COUT(bv.overflow(), false);

        // Unsafe write beyond capacity (but within protected buffer)
        for (int i = 0; i < 20; ++i) {
            bv.unsafe_push_back('x');
        }

        COUT(bv.overflow(), true);
        COUT(bv.reserve_ex(), -9);  // 15 capacity - 24 written = -9 overflow
        COUT(bv.capacity(), 15);
        COUT(bv.size(), 24);
        COUT(bv.str(), "safexxxxxxxxxxxxxxxxxxxx");
    }
}

/// 测试 BufferView append 各种字符串重载：
/// c-string, std::string, std::string_view, BufferView 及其了类 JString
DEF_TAST(bufv_append_string, "BufferView append 字符串重载")
{
    DESC("append c-string");
    {
        char buffer[64];
        BufferView bv(buffer);

        bv.append("hello");
        COUT(bv.size(), 5);
        COUT(std::string(bv), std::string("hello"));

        bv.append(" world");
        COUT(bv.size(), 11);
        COUT(std::string(bv), std::string("hello world"));
    }

    DESC("append c-string with length");
    {
        char buffer[64];
        BufferView bv(buffer);

        bv.append("hello world", 5);
        COUT(bv.size(), 5);
        COUT(std::string(bv), std::string("hello"));

        bv.append(" world!", 6);
        COUT(bv.size(), 11);
        COUT(std::string(bv), std::string("hello world"));
    }

    DESC("append std::string");
    {
        char buffer[64];
        BufferView bv(buffer);

        std::string str1 = "test";
        bv.append(str1);
        COUT(bv.size(), 4);
        COUT(std::string(bv), std::string("test"));

        std::string str2 = " string";
        bv.append(str2);
        COUT(bv.size(), 11);
        COUT(std::string(bv), std::string("test string"));
    }

    DESC("append std::string_view");
    {
        char buffer[64];
        BufferView bv(buffer);

        std::string_view sv1 = "string_view";
        bv.append(sv1);
        COUT(bv.size(), 11);
        COUT(std::string(bv), std::string("string_view"));

        std::string str = " append";
        std::string_view sv2 = str;
        bv.append(sv2);
        COUT(bv.size(), 18);
        COUT(std::string(bv), std::string("string_view append"));
    }

    DESC("append another BufferView");
    {
        char buffer1[64];
        char buffer2[64];
        BufferView bv1(buffer1);
        BufferView bv2(buffer2);

        bv1.append("first");
        bv2.append("second");

        bv1.append(bv2);
        COUT(bv1.size(), 11);
        COUT(std::string(bv1), std::string("firstsecond"));
    }

    DESC("append JString (StringBuffer)");
    {
        char buffer[128];
        BufferView bv(buffer);

        JString js;
        js.append("JString");
        bv.append(js);
        COUT(bv.size(), 7);
        COUT(std::string(bv), std::string("JString"));

        js.append(" content");
        bv.append(js);
        COUT(bv.size(), 22);
        COUT(std::string(bv), std::string("JStringJString content"));
    }

    DESC("append multiple string types in sequence");
    {
        char buffer[128];
        BufferView bv(buffer);

        bv.append("c-string");
        bv.append(std::string(" std::string"));
        bv.append(std::string_view(" std::string_view"));

        char temp[32];
        BufferView bv2(temp);
        bv2.append(" BufferView");
        bv.append(bv2);

        COUT(bv.size(), 48);
        COUT(std::string(bv), std::string("c-string std::string std::string_view BufferView"));
    }

    DESC("append nullptr c-string (should be ignored)");
    {
        char buffer[64];
        BufferView bv(buffer);

        bv.append("hello");
        bv.append(static_cast<const char*>(nullptr));
        COUT(bv.size(), 5);
        COUT(std::string(bv), std::string("hello"));
    }
}

/// @}
/* ---------------------------------------------------------------------- */

/// @brief Test BufferView usage issues
/// @{

/// 借用 std::string 注意事项：
/// - BufferView 的 capacity 容量比 std::string 少 1
/// - 通过 BufferView 写入内容不会反映到 std::string 的 size 中
/// - 通过 std::string 写入内容会与 BufferView 互相覆盖
/// - 使用 std::string resize 也可能会清空 BufferView 写入的内容
DEF_TAST(bufv_borrow_string, "BufferView 借用 std::string 并验证 resize 问题")
{
    DESC("场景1: BufferView 写入超过 string 原始 size,resize 会填充默认字符");
    {
        std::string str;
        str.reserve(256);
        size_t original_capacity = str.capacity();
        size_t original_size = str.size();  // 0

        // BufferView 借用 string 的内存并写入数据
        BufferView bv(str);
        COUT(bv.capacity(), str.capacity() - 1);

        bv.append("Hello, World! This is a test string for BufferView.");
        bv.end_cstr();
        size_t bv_size = bv.size();

        // 此时 str.size() 仍然是 0,但内存中已经有数据
        COUT(str.size(), 0);
        COUT(str.c_str());
        COUT(bv.size(), bv_size);

        // 先保存 BufferView 写入的内容
        std::string bv_str = static_cast<std::string>(bv);

        // 调用 str.resize(bv.size())
        // 注意: resize(0 -> 51) 会在 0-50 位置填充默认字符 '\0',覆盖了 BufferView 写入的数据!
        str.resize(bv_size);

        // 验证: string 的内容可能已经被破坏
        COUT(str.size(), bv_size);
        bool content_matches = (str.compare(bv_str) == 0);
        COUT(content_matches, false);  // 预期为 false,因为 resize 覆盖了内容

        // 使用 COUT_HEX 打印十六进制内容
        DESC("BufferView 的内容 (十六进制):");
        COUT_HEX(bv_str);
        DESC("string 的内容 (resize 后,十六进制):");
        COUT_HEX(str);

        // 统计非 '\0' 字符数量
        size_t non_zero_count = 0;
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] != '\0') non_zero_count++;
        }
        COUT(non_zero_count, 0);  // 所有字符都被 '\0' 覆盖
    }

    DESC("场景2: 验证原始 string 有数据时的情况");
    {
        std::string str = "prefix";
        str.reserve(256);
        size_t original_size = str.size();  // 6

        // BufferView 借用 string 的内存并写入数据
        BufferView bv(str);
        bv.append("Hello, BufferView!");
        bv.end_cstr();
        size_t bv_size = bv.size();

        // 此时 str.size() 仍然是 6
        COUT(str.size(), original_size);
        COUT(str.c_str());
        COUT(bv.size(), bv_size);

        // 先保存 BufferView 写入的内容
        std::string bv_str = bv.c_str();

        // 调用 str.resize(bv.size())
        // 注意: resize(6 -> 18) 会保留 0-5 的内容,但在 6-17 位置填充 '\0'
        str.resize(bv_size);

        // 验证: 前缀 "prefix" 应该保留,但后面的内容被 '\0' 覆盖
        COUT(str.size(), bv_size);
        bool prefix_preserved = (str.compare(0, 6, "prefix") == 0);
        COUT(prefix_preserved, true);  // 前缀保留

        bool content_matches = (str.compare(bv_str) == 0);
        COUT(content_matches, false);  // 整体内容不匹配

        DESC("内容对比 (十六进制):");
        DESC("BufferView:");
        COUT_HEX(bv_str);
        DESC("string (resize 后):");
        COUT_HEX(str);
    }
}

/// 借用 std::vector<char> 注意事项：与 std::string 类似
DEF_TAST(bufv_borrow_vector, "BufferView 借用 std::vector<char> 并验证 resize 问题")
{
    DESC("场景1: BufferView 写入超过 vector 原始 size,resize 会填充默认字符");
    {
        std::vector<char> vec;
        vec.reserve(256);
        size_t original_size = vec.size();  // 0

        // BufferView 借用 vector 的内存并写入数据
        BufferView bv(vec);
        COUT(bv.capacity(), vec.capacity() - 1);

        bv.append("Testing vector resize behavior with BufferView.");
        bv.end_cstr();
        size_t bv_size = bv.size();

        // 此时 vec.size() 仍然是 0
        COUT(vec.size(), 0);
        COUT(vec.data());
        COUT(bv.size(), bv_size);

        std::string bv_str = bv.str();

        // 调用 vec.resize(bv.size())
        // 注意: resize(0 -> 44) 会在所有位置填充 char() 即 '\0'
        vec.resize(bv_size);

        // 验证: vector 的内容应该全是 '\0'
        COUT(vec.size(), bv_size);

        // 检查是否全为 '\0'
        bool all_zeros = true;
        for (size_t i = 0; i < vec.size(); ++i) {
            if (vec[i] != '\0') {
                all_zeros = false;
                break;
            }
        }
        COUT(all_zeros, true);  // 预期为 true,所有内容被 '\0' 覆盖

        // 打印内容
        std::string vec_str(vec.data(), vec.size());
        DESC("内容对比 (十六进制):");
        DESC("BufferView:");
        COUT_HEX(bv_str);
        DESC("vector:");
        COUT_HEX(vec_str);
    }

    DESC("场景2: 验证原始 vector 有数据时的情况");
    {
        std::vector<char> vec = {'p', 'r', 'e', 'f', 'i', 'x'};
        vec.reserve(256);
        size_t original_size = vec.size();  // 6

        // BufferView 借用 vector 的内存并写入数据
        BufferView bv(vec);
        bv.append("Hello, Vector!");
        bv.end_cstr();
        size_t bv_size = bv.size();

        // 此时 vec.size() 仍然是 6
        COUT(vec.size(), original_size);
        COUT(vec.data());
        COUT(bv.size(), bv_size);

        std::string bv_str = bv.str();

        // 调用 vec.resize(bv.size())
        // 前缀保留,后面填充 '\0'
        vec.resize(bv_size);

        // 验证前缀保留
        bool prefix_preserved = true;
        std::string prefix = "prefix";
        for (size_t i = 0; i < 6; ++i) {
            if (vec[i] != prefix[i]) {
                prefix_preserved = false;
                break;
            }
        }
        COUT(prefix_preserved, true);

        // 验证后面全是 '\0'
        bool suffix_all_zeros = true;
        for (size_t i = 6; i < vec.size(); ++i) {
            if (vec[i] != '\0') {
                suffix_all_zeros = false;
                break;
            }
        }
        COUT(suffix_all_zeros, true);

        // 打印内容
        std::string vec_str(vec.data(), vec.size());
        DESC("内容对比 (十六进制):");
        DESC("BufferView:");
        COUT_HEX(bv_str);
        DESC("vector (resize 后):");
        COUT_HEX(vec_str);
    }
}

/// 使用 std::to_chars 与 snprintf 在 BufferView 的内存中直接写入.
/// 调用外部方法写入内容后，需要调用 resize 或 set_end 方法调整 size.
DEF_TAST(bufv_extern_write, "BufferView 与外部方法写入集成协作")
{
    DESC("using std::to_chars to write numbers");
    {
        char buffer[64];
        BufferView bv(buffer);

        int value = 42;
        auto result = std::to_chars(bv.begin(), bv.cap_end(), value);
        size_t len = result.ptr - bv.begin();

        bv.resize(len);
        COUT(bv.size(), 2);
        COUT(std::string(bv), std::string("42"));

        // Write another number
        value = 12345;
        result = std::to_chars(bv.end(), bv.cap_end(), value);
        len = result.ptr - bv.end();
        bv.unsafe_set_end(result.ptr);
        COUT(bv.size(), 7);
        COUT(std::string(bv), std::string("4212345"));
    }

    DESC("using snprintf to write formatted string");
    {
        char buffer[64];
        BufferView bv(buffer);

        int written = snprintf(bv.begin(), bv.capacity() + 1, "%s %d", "value", 100);
        bv.resize(written);
        COUT(bv.size(), 9);
        COUT(std::string(bv), std::string("value 100"));

        // Append another formatted string
        written = snprintf(bv.end(), bv.reserve_ex() + 1, " %s", "appended");
        bv.unsafe_set_end(bv.end() + written);
        COUT(bv.size(), 18);
        COUT(std::string(bv), std::string("value 100 appended"));
    }

    DESC("using memcpy to copy data");
    {
        char buffer[64];
        BufferView bv(buffer);

        const char* src = "copied data";
        size_t len = strlen(src);
        memcpy(bv.begin(), src, len);
        bv.resize(len);

        COUT(bv.size(), 11);
        COUT(std::string(bv), std::string("copied data"));
    }

    DESC("using strcpy/strncpy to write string");
    {
        char buffer[64];
        BufferView bv(buffer);

        strcpy(bv.begin(), "strcpy test");
        bv.resize(strlen("strcpy test"));

        COUT(bv.size(), 11);
        COUT(std::string(bv), std::string("strcpy test"));

        strncpy(bv.end(), "append", bv.reserve_ex());
        size_t append_len = strlen("append");
        bv.unsafe_set_end(bv.end() + append_len);

        COUT(bv.size(), 17);
        COUT(std::string(bv), std::string("strcpy testappend"));
    }

    DESC("set_end vs resize after external write");
    {
        char buffer[64];
        BufferView bv(buffer);
        bv.append("base");

        // External write using memcpy
        const char* more = " content";
        memcpy(bv.end(), more, strlen(more));

        // Using unsafe_set_end
        bv.unsafe_set_end(bv.end() + strlen(more));
        COUT(bv.size(), 12);
        COUT(std::string(bv), std::string("base content"));

        // Using resize (safe version)
        const char* even_more = " appended";
        size_t len = strlen(even_more);
        memcpy(bv.end(), even_more, len);
        bv.resize(bv.size() + len);

        COUT(bv.size(), 21);
        COUT(std::string(bv), std::string("base content appended"));
    }
}

/// BufferView '\0' 字符封端注意事项：
/// - 初始化不清零，仅 cap_end 置零
/// - 写入方法 push_back/append 后不保证有空字符结束
/// - 显式调用 end_cstr() 会有空字符结束
/// - 调用 c_str() 方法也保证有空字符结束
DEF_TAST(bufv_null_end, "BufferView 空字符封端测试")
{
    DESC("initialization: only cap_end is set to zero");
    {
        char buffer[32];
        BufferView bv(buffer);
        bv.fill('@');  // Fill with known character for testing

        COUT(*bv.cap_end() == '\0', true);
        COUT(bv.empty(), true);
        COUT(std::string(bv), std::string(""));  // Empty string

        DESC("Buffer content after fill with '@':");
        COUT_HEX(std::string(bv.data(), bv.capacity()));
        COUT(bv.data());
    }

    DESC("push_back doesn't guarantee null terminator");
    {
        char buffer[32];
        BufferView bv(buffer);
        bv.fill('@');

        bv.push_back('H');
        bv.push_back('e');
        bv.push_back('l');
        bv.push_back('l');
        bv.push_back('o');

        COUT(bv.size(), 5);
        COUT(std::string(bv), std::string("Hello"));

        DESC("Buffer content (expect 'Hello@...'):");
        COUT_HEX(std::string(bv.data(), bv.capacity()));
        COUT(bv.data());

        COUT(bv.back(), 'o');
        COUT(*bv.end(), '@');
        COUT(*bv.cap_end() == '\0', true);  // cap_end is always null
    }

    DESC("append doesn't guarantee null terminator");
    {
        char buffer[32];
        BufferView bv(buffer);
        bv.fill('@');

        bv.append("test");

        COUT(bv.size(), 4);
        COUT(std::string(bv), std::string("test"));

        DESC("Buffer content (expect 'test@@...'):");
        COUT_HEX(std::string(bv.data(), bv.capacity()));
        COUT(bv.data());

        COUT(bv.back(), 't');
        COUT(*bv.end(), '@');
        COUT(*bv.cap_end() == '\0', true);  // cap_end is always null
    }

    DESC("explicit end_cstr() adds null terminator");
    {
        char buffer[32];
        BufferView bv(buffer);
        bv.fill('@');

        bv.append("hello");
        bv.end_cstr();

        COUT(bv.size(), 5);
        COUT(*bv.end() == '\0', true);  // Now end() is null

        DESC("Buffer content (expect 'hello\0@...'):");
        COUT_HEX(std::string(bv.data(), bv.capacity()));

        const char* cstr = bv.c_str();
        COUT(strcmp(cstr, "hello") == 0, true);
    }

    DESC("c_str() ensures null terminator");
    {
        char buffer[32];
        BufferView bv(buffer);
        bv.fill('@');

        bv.append("world");

        const char* cstr = bv.c_str();
        COUT(bv.size(), 5);
        COUT(*bv.end() == '\0', true);  // c_str() ensures null terminator

        DESC("Buffer content after c_str() (expect 'world\0@...'):");
        COUT_HEX(std::string(bv.data(), bv.capacity()));

        COUT(strcmp(cstr, "world") == 0, true);
    }

    DESC("end_cstr() with no space available");
    {
        char buffer[5];
        BufferView bv(buffer);
        bv.fill('@');

        bv.append("1234");  // Fill to capacity (4 out of 4)
        bv.end_cstr();     // No space for null terminator

        COUT(bv.full(), true);
        COUT(bv.size(), 4);

        DESC("Buffer content full (expect '1234\0'):");
        COUT_HEX(std::string(bv.data(), bv.capacity()));

        // cap_end should still be null from initialization
        COUT(*bv.cap_end() == '\0', true);

        const char* cstr = bv.c_str();
        COUT(strlen(cstr), 4);
    }

    DESC("multiple writes need explicit end_cstr()");
    {
        char buffer[32];
        BufferView bv(buffer);
        bv.fill('@');

        bv.append("first");
        bv.append("second");

        DESC("Buffer before end_cstr() (expect 'firstsecond@@...'):");
        COUT_HEX(std::string(bv.data(), bv.capacity()));

        // After multiple appends, call end_cstr() to ensure null termination
        bv.end_cstr();

        COUT(bv.size(), 11);
        COUT(*bv.end() == '\0', true);

        DESC("Buffer after end_cstr() (expect 'firstsecond\0@...'):");
        COUT_HEX(std::string(bv.data(), bv.capacity()));

        const char* cstr = bv.c_str();
        COUT(strcmp(cstr, "firstsecond") == 0, true);
    }

    DESC("clear doesn't reset cap_end null terminator");
    {
        char buffer[32];
        BufferView bv(buffer);
        bv.fill('@');

        bv.append("data");
        bv.end_cstr();

        bv.clear();
        COUT(bv.size(), 0);
        COUT(bv.empty(), true);

        DESC("Buffer after clear (expect '@@@@...\0'):");
        COUT_HEX(std::string(bv.data(), bv.capacity()));

        // cap_end should still be null
        COUT(*bv.cap_end() == '\0', true);
    }

    DESC("resize preserves cap_end null terminator");
    {
        char buffer[32];
        BufferView bv(buffer);
        bv.fill('@');

        bv.append("original data");
        bv.resize(5);

        COUT(bv.size(), 5);
        COUT(*bv.cap_end() == '\0', true);  // cap_end is always null

        DESC("Buffer after resize (expect 'origi@@@@@...\0'):");
        COUT_HEX(std::string(bv.data(), bv.capacity()));
    }
}

/// @}
/* ---------------------------------------------------------------------- */

/// @brief Test for UnsafeBuffer class
/// @{

DEF_TAST(ubuf_constructors, "UnsafeBuffer 继承构造函数测试")
{
    DESC("constructor from char* and size");
    {
        char buffer[256];
        UnsafeBuffer bv(buffer, 256);

        COUT(bv.size(), 0);
        COUT(bv.capacity(), 255);
        COUT(static_cast<bool>(bv), true);

        bv.append("hello");
        COUT(bv.size(), 5);
        COUT(strncmp(bv.data(), "hello", 5), 0);
    }

    DESC("constructor from C array");
    {
        char buffer[128];
        UnsafeBuffer bv(buffer);

        COUT(bv.capacity(), 127);

        bv.append("test");
        COUT(bv.size(), 4);
        COUT(strncmp(bv.data(), "test", 4), 0);
    }

    DESC("constructor from std::array");
    {
        std::array<char, 64> arr;
        UnsafeBuffer bv(arr);

        COUT(bv.capacity(), 63);

        bv.append("array test");
        COUT(bv.size(), 10);
        COUT(strncmp(bv.data(), "array test", 10), 0);
    }

    DESC("constructor from std::string");
    {
        std::string str;
        str.reserve(512);

        UnsafeBuffer bv(str);
        // Note: UnsafeBuffer will always use last byte for \0
        COUT(bv.capacity(), str.capacity() - 1);

        bv.append("borrowed string");
        COUT(bv.size(), 15);

        // Note: str.size() is still 0, UnsafeBuffer doesn't sync size
        COUT(str.size(), 0);
        COUT(strncmp(bv.data(), "borrowed string", 15), 0);
    }

    DESC("constructor from std::vector<char>");
    {
        std::vector<char> vec;
        vec.reserve(1024);

        UnsafeBuffer bv(vec);
        COUT(bv.capacity(), vec.capacity() - 1);

        bv.append("vector content");
        COUT(bv.size(), 14);

        // Note: vec.size() is still 0, UnsafeBuffer doesn't sync size
        COUT(vec.size(), 0);
        COUT(strncmp(bv.data(), "vector content", 14), 0);
    }
}

DEF_TAST(ubuf_move_constructor, "UnsafeBuffer 移动构造测试")
{
    char buffer[64];
    UnsafeBuffer ub1(buffer);
    ub1.append("test data longer");

    // Move construct
    UnsafeBuffer ub2(std::move(ub1));

    // ub2 should have data
    COUT(ub2.size(), 16);
    COUT(std::string(ub2), "test data longer");

    // ub1 should be empty (nullptr pointers)
    COUT(static_cast<bool>(ub1), false);
    COUT(ub1.size(), 0);
    COUT(ub1.capacity(), 0);

    // Move assignment
    UnsafeBuffer ub3;
    ub3 = std::move(ub2);
    COUT(ub3.size(), 16);
    COUT(std::string(ub3), "test data longer");
    COUT(static_cast<bool>(ub2), false);
    COUT(ub2.size(), 0);
    COUT(ub2.capacity(), 0);
}

DEF_TAST(ubuf_write_methods, "UnsafeBuffer 写入方法测试")
{
    DESC("push_back without boundary checking");
    {
        char buffer[16 + 32];
        UnsafeBuffer ub(buffer, 16);

        ub.push_back('A');
        ub.push_back('B');
        ub.push_back('C');

        COUT(ub.size(), 3);

        // Unsafe mode will allow overflow
        for (int i = 0; i < 20; ++i) ub.push_back('x');
        COUT(ub.size(), 23);
        COUT(ub.overflow(), true);
        COUT(ub.reserve_ex(), -8);  // 15 capacity - 23 written = -8 overflow
    }

    DESC("append without boundary checking");
    {
        char buffer[64 + 96];
        UnsafeBuffer ub(buffer, 64);

        ub.append("unsafe");
        COUT(ub.size(), 6);
        COUT(std::string(ub, 0, 6), std::string("unsafe"));

        // Writing beyond capacity will cause overflow
        std::string long_text(100, 'x');
        ub.append(long_text);
        COUT(ub.overflow(), true);  // Overflow occurred
        COUT(ub.reserve_ex(), -43);  // 63 capacity - 106 written = -43 overflow
    }

    DESC("append(count, ch) without boundary checking");
    {
        char buffer[32 + 64];
        UnsafeBuffer ub(buffer, 32);

        ub.append("hello");
        ub.append(5, '!');
        COUT(ub.size(), 10);
        COUT(std::string(ub), "hello!!!!!");

        // Unsafe mode will allow overflow
        ub.append(50, 'x');
        COUT(ub.overflow(), true);
        COUT(ub.reserve_ex(), -29);  // 31 capacity - 60 written = -29 overflow
    }

    DESC("fill without boundary checking");
    {
        char buffer[64 + 96];
        UnsafeBuffer ub(buffer, 64);

        ub.append("prefix");
        ub.fill('-', 5);
        COUT(ub.size(), 11);

        // Unsafe mode allows any fill size
        // Note: fill is safe (truncates), not overflow
        ub.fill('x', 100);
        // 63 capacity - 111 written = -48 overflow
        COUT(ub.overflow(), false);
        COUT(ub.full(), true);
    }

    DESC("resize without boundary checking");
    {
        char buffer[64 + 96];
        UnsafeBuffer ub(buffer, 64);

        ub.append("test content");
        COUT(ub.size(), 12);

        ub.resize(5);
        COUT(ub.size(), 5);

        // Unsafe mode will allow overflow
        ub.resize(100);
        COUT(ub.overflow(), true);
        COUT(ub.reserve_ex(), -37);  // 63 capacity - 100 written = -37 overflow
    }
}

/// @}
/* ---------------------------------------------------------------------- */
