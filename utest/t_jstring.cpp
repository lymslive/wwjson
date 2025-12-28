#include "couttast/tinytast.hpp"
#include "test_util.h"
#include "jstring.hpp"
#include "couttast/couthex.hpp"
#include <charconv>  // for std::to_chars
#include <system_error>  // for std::make_error_code
#include <string>
#include <string_view>

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

DEF_TAST(bufv_write_methods, "BufferView 写入方法测试")
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

/// @}
/* ---------------------------------------------------------------------- */

/// @brief Test for BufferView borrowing container memory
/// @details Tests the behavior when BufferView borrows memory from
/// std::string or std::vector<char> and demonstrates the issue with
/// using container.resize() to sync the container's size.
/// @{

DEF_TAST(bufv_borrow_string_resize, "BufferView 借用 std::string 并验证 resize 问题")
{
    DESC("场景1: BufferView 写入超过 string 原始 size,resize 会填充默认字符");
    {
        std::string str;
        str.reserve(256);
        size_t original_capacity = str.capacity();
        size_t original_size = str.size();  // 0

        // BufferView 借用 string 的内存并写入数据
        BufferView bv(str);
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
        std::string bv_str = static_cast<std::string>(bv);

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

DEF_TAST(bufv_borrow_vector_resize, "BufferView 借用 std::vector<char> 并验证 resize 问题")
{
    DESC("场景1: BufferView 写入超过 vector 原始 size,resize 会填充默认字符");
    {
        std::vector<char> vec;
        vec.reserve(256);
        size_t original_size = vec.size();  // 0

        // BufferView 借用 vector 的内存并写入数据
        BufferView bv(vec);
        bv.append("Testing vector resize behavior with BufferView.");
        bv.end_cstr();
        size_t bv_size = bv.size();

        // 此时 vec.size() 仍然是 0
        COUT(vec.size(), 0);
        COUT(vec.data());
        COUT(bv.size(), bv_size);

        std::string bv_str = static_cast<std::string>(bv);

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

        std::string bv_str = static_cast<std::string>(bv);

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

/// @brief Test for StringBuffer/JString class
/// @{

DEF_TAST(jstring_basic_construct, "JString 基础构造测试")
{
    // 默认构造 - 现在默认申请 1024 字节，容量为 1023
    JString buffer1;
    COUT(buffer1.empty(), true);
    COUT(buffer1.size(), 0);
    COUT(buffer1.capacity(), 1023);
    COUT((void*)buffer1.data() != nullptr, true);
    // operator bool() 测试
    COUT(static_cast<bool>(buffer1), true);
    // c_str 应该正常返回
    COUT(buffer1.c_str() != nullptr, true);

    // 带容量构造
    JString buffer2(100);
    COUT(buffer2.empty(), true);
    COUT(buffer2.size(), 0);
    COUT(buffer2.capacity() >= 100, true);
    COUT((void*)buffer2.data() != nullptr, true);
    COUT(static_cast<bool>(buffer2), true);
}

DEF_TAST(jstring_append_string, "JString 字符串追加测试")
{
    JString buffer;
    
    // 追加 C 风格字符串
    buffer.append("hello");
    COUT(buffer.size(), 5);
    COUT(strncmp(buffer.data(), "hello", buffer.size()), 0);
    
    // 按长度追加
    buffer.append(" world", 6);
    COUT(buffer.size(), 11);
    COUT(strncmp(buffer.data(), "hello world", buffer.size()), 0);
    
    // 多次追加
    buffer.append("!");
    buffer.append(" JSON");
    COUT(buffer.size(), 17);
    COUT(strncmp(buffer.data(), "hello world! JSON", buffer.size()), 0);
}

DEF_TAST(jstring_push_back, "JString 单字符追加测试")
{
    JString buffer;
    
    // 单字符 push_back
    buffer.push_back('H');
    buffer.push_back('e');
    buffer.push_back('l');
    buffer.push_back('l');
    buffer.push_back('o');
    
    COUT(buffer.size(), 5);
    COUT(strncmp(buffer.data(), "Hello", buffer.size()), 0);
}

DEF_TAST(jstring_reserve, "JString 容量预留测试")
{
    JString buffer;

    // 初始预留
    buffer.reserve(100);
    COUT(buffer.capacity() >= 100 + 4, true); // capacity() should be >= requested
    COUT(buffer.empty(), true);

    // 带边界的预留
    buffer.reserve_ex(50);
    COUT(buffer.capacity() >= buffer.size() + 50 + 4, true);

    // 预留后追加
    buffer.append("This is a test string for reserve operation");
    COUT(buffer.size() > 0, true);
    COUT(buffer.size() <= buffer.capacity(), true);
}

DEF_TAST(jstring_unsafe_operations, "JString 不安全操作测试")
{
    JString buffer;
    
    // 预留足够空间进行不安全操作
    buffer.reserve_ex(10);
    
    // 安全追加
    buffer.append("key");
    COUT(buffer.size(), 3);
    
    // 边界内不安全操作
    buffer.unsafe_push_back(':');
    buffer.unsafe_push_back('"');
    buffer.unsafe_push_back('v');
    buffer.unsafe_push_back('a');
    
    COUT(buffer.size(), 7);
    COUT(strncmp(buffer.data(), "key:\"va", buffer.size()), 0);
    
    // 测试 unsafe_resize
    buffer.unsafe_resize(3);
    COUT(buffer.size(), 3);
    COUT(strncmp(buffer.data(), "key", buffer.size()), 0);
    
    // 测试 unsafe_end_cstr
    buffer.unsafe_resize(7);
    buffer.unsafe_push_back('l');
    buffer.unsafe_push_back('u');
    buffer.unsafe_push_back('e');
    buffer.unsafe_push_back('"');
    
    COUT(buffer.size(), 11);
    COUT(strncmp(buffer.c_str(), "key:\"value\"", buffer.size()), 0);
    
    // 重置并测试正确空字符结尾
    buffer.clear();
    buffer.append("test");
    buffer.unsafe_end_cstr();
    COUT(strcmp(buffer.c_str(), "test"), 0);
}

DEF_TAST(jstring_different_unsafe_levels, "不同不安全级别的 StringBuffer 测试")
{
    StringBuffer<2> buffer2;
    StringBuffer<8> buffer8;
    
    // 安全操作应该行为相同
    buffer2.append("test");
    buffer8.append("test");
    
    COUT(buffer2.size(), 4);
    COUT(buffer8.size(), 4);
    COUT(strncmp(buffer2.data(), "test", buffer2.size()), 0);
    COUT(strncmp(buffer8.data(), "test", buffer8.size()), 0);
    
    // 测试不同级别的不安全操作
    buffer2.reserve_ex(5);
    buffer8.reserve_ex(5);
    
    // 不安全 push back
    for (int i = 0; i < 3; ++i)
    {
        buffer2.unsafe_push_back('a' + i);
        buffer8.unsafe_push_back('a' + i);
    }
    
    COUT(buffer2.size(), 7);
    COUT(buffer8.size(), 7);
}

DEF_TAST(jstring_copy_move, "JString 复制和移动语义测试")
{
    JString original;
    original.append("Hello World");
    original.end_cstr();

    // 复制构造
    JString copy(original);
    COUT(copy.size(), original.size());
    COUT(strcmp(copy.data(), original.data()), 0);
    COUT(copy.data() != original.data(), true); // 不同内存
    COUT(static_cast<bool>(copy), true);

    // 复制赋值
    JString copy2;
    COUT(static_cast<bool>(copy2), true);  // 默认构造也分配内存
    copy2 = original;
    COUT(copy2.size(), original.size());
    COUT(strcmp(copy2.data(), original.data()), 0);
    COUT(copy2.data() != original.data(), true); // 不同内存
    COUT(static_cast<bool>(copy2), true);

    // 移动构造
    JString moved(std::move(original));
    COUT(moved.size(), 11);
    COUT(strcmp(moved.data(), "Hello World"), 0);
    COUT(original.empty(), true); // 原对象应为空
    COUT(original.capacity(), 0); 
    COUT(static_cast<bool>(original), false);

    // 移动赋值
    JString moved2;
    moved2 = std::move(moved);
    COUT(moved2.size(), 11);
    COUT(strcmp(moved2.data(), "Hello World"), 0);
    COUT(moved.empty(), true); // 原对象应为空
    COUT(static_cast<bool>(moved), false);
}

DEF_TAST(jstring_edge_cases, "JString 边界情况测试")
{
    JString buffer;
    
    // 空字符串操作
    COUT(buffer.empty(), true);
    COUT(buffer.size(), 0);
    
    buffer.append("test");
    COUT(buffer.size(), 4);

    // 对空缓冲区清空
    buffer.clear();
    COUT(buffer.empty(), true);
    
    // 追加空字符串
    buffer.append("");
    COUT(buffer.size(), 0);
    
    // 在空缓冲区上进行不安全操作
    buffer.reserve_ex(10);
    buffer.unsafe_resize(0);
    COUT(buffer.size(), 0);
}

DEF_TAST(jstring_json_patterns, "JString JSON 序列化模式测试")
{
    JString buffer;
    
    // 模拟 JSON 键值构建
    buffer.reserve_ex(50);
    buffer.append("\"name\"");
    
    // 使用不安全操作处理 JSON 标点
    buffer.unsafe_push_back(':');
    buffer.unsafe_push_back('"');
    buffer.append("John Doe");
    buffer.unsafe_push_back('"');
    buffer.unsafe_push_back(',');
    
    COUT(buffer.size(), 18);
    COUT(strncmp(buffer.data(), "\"name\":\"John Doe\",", buffer.size()), 0);
    
    // 继续添加更多 JSON 内容
    buffer.append("\"age\"");
    buffer.unsafe_push_back(':');
    buffer.append("30");
    
    // 添加空字符结尾得到最终 JSON 字符串
    buffer.unsafe_end_cstr();
    
    COUT(buffer.size(), 26);
    COUT(strcmp(buffer.c_str(), "\"name\":\"John Doe\",\"age\":30"), 0);
}

DEF_TAST(jstring_buffer_view, "BufferView 转换测试")
{
    JString buffer;
    buffer.append("test content");

    // 使用 reinterpret_cast 转换为 BufferView
    // 现在使用 public 继承，static_cast 可用
    const BufferView& view = static_cast<const BufferView&>(buffer);

    COUT(view.size(), buffer.size());
    COUT(view.data(), buffer.data());
    COUT(strncmp(buffer.data(), "test content", buffer.size()), 0);

    // 使用公共接口检查视图属性
    bool view_empty = view.empty();
    COUT(view_empty, false);
    COUT(view.front(), 't');
    COUT(view.back(), 't');
}

DEF_TAST(jstring_capacity_growth, "JString 容量增长测试")
{
    // 使用较小的初始容量观察扩容行为
    JString buffer(50);

    // 初始容量（50 + 4 + 1 = 55，对齐到 56，capacity = 55）
    size_t initial_capacity = buffer.capacity();
    COUT(initial_capacity, 55);
    COUT(static_cast<bool>(buffer), true);

    // 小追加不应触发重新分配
    buffer.append("small");
    COUT(buffer.capacity(), 55);

    // 大追加应触发重新分配
    size_t capacity_before = buffer.capacity();
    std::string large_content(1000, 'x');
    buffer.append(large_content.c_str(), large_content.size());

    COUT(buffer.capacity() > capacity_before, true);
    COUT(buffer.size(), 1005); // 5 + 1000
}

DEF_TAST(jstring_front_back_access, "JString 首尾字符访问测试")
{
    JString buffer;
    buffer.append("Hello");
    
    COUT(buffer.front(), 'H');
    COUT(buffer.back(), 'o');
    
    // 通过 front/back 修改
    buffer.front() = 'h';
    buffer.back() = '!';
    COUT(strncmp(buffer.data(), "hell!", buffer.size()), 0);
}

DEF_TAST(jstring_clear_operation, "JString 清空操作测试")
{
    JString buffer;
    buffer.append("some content");
    
    size_t size_before = buffer.size();
    COUT(size_before > 0, true);
    
    buffer.clear();
    COUT(buffer.empty(), true);
    COUT(buffer.size(), 0);
    
    // 清空后应该可以正常使用
    buffer.append("new content");
    COUT(buffer.empty(), false);
    COUT(strncmp(buffer.data(), "new content", buffer.size()), 0);
}

DEF_TAST(jstring_c_str_termination, "JString c_str 空字符结尾测试")
{
    JString buffer;
    buffer.append("test");

    // c_str 应该返回以空字符结尾的字符串
    const char* cstr = buffer.c_str();
    COUT(strlen(cstr), 4);
    COUT(strcmp(cstr, "test"), 0);

    // 修改内容后 c_str 仍应正确
    buffer.push_back('!');
    cstr = buffer.c_str();
    COUT(strlen(cstr), 5);
    COUT(strcmp(cstr, "test!"), 0);
}

DEF_TAST(jstring_kunsafelevel_semantics, "kUnsafeLevel 语义测试")
{
    // 测试 reserve_ex(n) 后可以写入 n 个字符，然后再调用 unsafe_push_back kUnsafeLevel 次
    {
        JString buffer(250);
        buffer.append(buffer.capacity(), 'x');
        size_t old_size = buffer.size();

        // 250 + 4 + 1 = 255，对齐到 256，capacity = 255
        COUT(old_size, 255);

        buffer.reserve_ex(10);  // 预留 10 个字符的空间 + 4 的 unsafe margin
        COUT(buffer.capacity());
        size_t cap1 = buffer.capacity();

        // 写入 10 个安全字符
        buffer.append("0123456789");
        COUT(buffer.size(), old_size + 10);
        size_t cap2 = buffer.capacity();

        // 再调用 unsafe_push_back 4 次 (kUnsafeLevel = 4)
        buffer.unsafe_push_back('A');
        buffer.unsafe_push_back('B');
        buffer.unsafe_push_back('C');
        buffer.unsafe_push_back('D');
        size_t cap3 = buffer.capacity();

        COUT(buffer.size(), old_size + 14);
        COUT(strncmp(buffer.data() + old_size, "0123456789ABCD", 14), 0);

        COUT(buffer.capacity());
        COUT(buffer.capacity() >= buffer.size(), true);

        COUT(cap1 == cap2, true);
        COUT(cap1 == cap3, true);
        COUT(static_cast<bool>(buffer), true);
    }
}

DEF_TAST(jstring_memory_alignment, "内存对齐测试")
{
    // 测试默认构造的对齐容量
    {
        JString buffer;
        COUT(buffer.capacity(), 1023);  // 1024 - 1 (null terminator)
        COUT(static_cast<bool>(buffer), true);
    }

    // 测试小容量构造的对齐（无最小分配限制）
    {
        JString buffer(100);
        // 100 + 4 + 1 = 105，对齐到 112，capacity = 111
        COUT(buffer.capacity(), 111);
    }

    // 测试指定较小容量
    {
        JString buffer(10);
        // 10 + 4 + 1 = 15，对齐到 16，capacity = 15
        COUT(buffer.capacity(), 15);
    }
}

DEF_TAST(jstring_to_chars_integration, "std::to_chars 集成测试")
{
    // 测试在 JString 原位 buffer 上使用 std::to_chars 转换整数
    // 先预留足够空间，然后从 end 处写入整数，再更新 end 位置

    {
        JString buffer;
        buffer.append("The number is: ");

        // 预留足够空间写入整数
        buffer.reserve_ex(12);  // 足够存储 INT_MAX (11 chars) + margin

        // 使用 std::to_chars 直接在 buffer 末尾写入整数
        int value = 12345678;
        std::to_chars_result result = std::to_chars(buffer.end(), buffer.cap_end(), value);

        COUT(std::make_error_code(result.ec), std::make_error_code(std::errc{}));
        COUT(result.ptr - buffer.end(), 8);  // "12345678" 的长度

        // 更新 end 位置
        buffer.set_end(result.ptr);

        COUT(buffer.size(), 23);  // "The number is: " (15) + "12345678" (8) = 23

        // 验证内容
        buffer.end_cstr();
        COUT(strcmp(buffer.c_str(), "The number is: 12345678"), 0);
    }

    // 测试负数
    {
        JString buffer;
        buffer.append("Negative: ");

        buffer.reserve_ex(12);
        int value = -98765;
        std::to_chars_result result = std::to_chars(buffer.end(), buffer.cap_end(), value);

        COUT(std::make_error_code(result.ec), std::make_error_code(std::errc{}));
        buffer.set_end(result.ptr);

        buffer.end_cstr();
        COUT(strcmp(buffer.c_str(), "Negative: -98765"), 0);
    }

    // 测试 unsigned int
    {
        JString buffer;
        buffer.append("Unsigned: ");

        buffer.reserve_ex(12);
        unsigned int value = 4294967295u;
        std::to_chars_result result = std::to_chars(buffer.end(), buffer.cap_end(), value);

        COUT(std::make_error_code(result.ec), std::make_error_code(std::errc{}));
        buffer.set_end(result.ptr);

        buffer.end_cstr();
        COUT(strcmp(buffer.c_str(), "Unsigned: 4294967295"), 0);
    }

    // 测试使用 begin() end() cap_end() 方法
    {
        JString buffer;
        buffer.append("Prefix:");

        buffer.reserve_ex(16);

        // 使用新增的 begin/end/cap_end 方法
        char* write_pos = buffer.end();
        int value = 42;
        std::to_chars_result result = std::to_chars(write_pos, buffer.cap_end(), value);

        COUT(std::make_error_code(result.ec), std::make_error_code(std::errc{}));
        buffer.set_end(result.ptr);

        COUT(strcmp(buffer.c_str(), "Prefix:42"), 0);

        // 验证 begin() 返回正确的指针
        COUT(strncmp(buffer.begin(), "Prefix:42", buffer.size()), 0);

        // 验证 end() 和 cap_end() 的关系
        COUT(buffer.end() - buffer.begin(), buffer.size());
        COUT(buffer.cap_end() - buffer.begin(), buffer.capacity());
    }

    // 测试使用 unsafe_append(char*, size)
    {
        JString buffer;
        buffer.append("Unsafe append: ");

        // 预留足够空间
        buffer.reserve_ex(20);

        // 手动构造字符串并使用 unsafe_append
        const char* extra = "test data";
        size_t len = strlen(extra);
        buffer.BufferView::unsafe_append(extra, len);

        COUT(strncmp(buffer.data() + buffer.size() - len, extra, len), 0);
        COUT(strncmp(buffer.c_str(), "Unsafe append: test data", buffer.size()), 0);
    }
}

DEF_TAST(jstring_to_string_view, "BufferView 隐式转换为 std::string_view")
{
    JString buffer;
    buffer.append("test content");

    // 隐式转换为 std::string_view
    std::string_view sv = buffer;

    COUT(sv.size(), buffer.size());
    COUT(sv.data(), buffer.data());
    COUT(sv.compare("test content"), 0);

    // 通过引用转换
    const BufferView& view = buffer;
    std::string_view sv2 = view;

    COUT(sv2.size(), buffer.size());
    COUT(sv2.data(), buffer.data());
    COUT(sv2.compare("test content"), 0);

    // 测试空字符串
    JString empty_buffer;
    COUT(empty_buffer.capacity(), 1023);  // 默认构造
    COUT(static_cast<bool>(empty_buffer), true);  // 有内存
    std::string_view empty_sv = empty_buffer;
    COUT(empty_sv.size(), 0);
}

DEF_TAST(jstring_to_string, "BufferView 显式转换为 std::string")
{
    JString buffer;
    buffer.append("test content");

    // 显式转换为 std::string
    std::string str = static_cast<std::string>(buffer);

    COUT(str.size(), buffer.size());
    COUT(str.compare("test content"), 0);

    // 验证拷贝而非引用
    COUT(str.data() != buffer.data(), true);

    // 修改原始 buffer，str 应该不受影响
    buffer.append(" more");
    COUT(str.compare("test content"), 0);
    COUT(buffer.size(), 17);

    // 测试空字符串
    JString empty_buffer;
    std::string empty_str = static_cast<std::string>(empty_buffer);
    COUT(empty_str.size(), 0);
    COUT(empty_str.empty(), true);
}

DEF_TAST(jstring_append_std_string, "StringBuffer append 支持 std::string")
{
    JString buffer;

    // 测试 std::string
    std::string str1 = "hello";
    buffer.append(str1);

    COUT(buffer.size(), 5);
    COUT(strncmp(buffer.data(), "hello", buffer.size()), 0);

    std::string str2 = " world";
    buffer.append(str2);

    COUT(buffer.size(), 11);
    COUT(strncmp(buffer.data(), "hello world", buffer.size()), 0);

    // 测试空字符串
    std::string empty_str;
    buffer.append(empty_str);
    COUT(buffer.size(), 11);
}

DEF_TAST(jstring_append_string_view, "StringBuffer append 支持 std::string_view")
{
    JString buffer;

    // 测试 std::string_view
    std::string_view sv1 = "hello";
    buffer.append(sv1);

    COUT(buffer.size(), 5);
    COUT(strncmp(buffer.data(), "hello", buffer.size()), 0);

    // 从 std::string 创建 string_view
    std::string str = " world";
    std::string_view sv2(str);
    buffer.append(sv2);

    COUT(buffer.size(), 11);
    COUT(strncmp(buffer.data(), "hello world", buffer.size()), 0);

    // 测试子串
    std::string long_str = "0123456789";
    std::string_view sub_sv = std::string_view(long_str).substr(3, 4); // "3456"
    buffer.append(sub_sv);

    COUT(buffer.size(), 15);
    COUT(strncmp(buffer.data(), "hello world3456", buffer.size()), 0);

    // 测试空 string_view
    std::string_view empty_sv;
    buffer.append(empty_sv);
    COUT(buffer.size(), 15);

    buffer.append("");
    COUT(buffer.size(), 15);
}

DEF_TAST(jstring_fill, "BufferView fill 方法测试")
{
    DESC("fill(0) 填充剩余空间为 '\\0'");
    {
        JString buffer(100);
        buffer.append("hello");
        buffer.fill(0);
        for (size_t i = buffer.size(); i < buffer.capacity(); ++i)
        {
            COUT(buffer.data()[i], '\0');
        }
        COUT(strncmp(buffer.data(), "hello", 5), 0);
    }

    DESC("fill(ch, count) 填充并移动 end 指针");
    {
        JString buffer(100);
        buffer.append("hello");
        buffer.fill('!', 3);
        COUT(buffer.size(), 8);
        COUT(strcmp(buffer.c_str(), "hello!!!"), 0);
    }

    DESC("fill 超过容量时自动截断");
    {
        JString buffer(100);
        buffer.append("test");
        size_t size_before = buffer.size();
        buffer.fill('y', 1000);
        COUT(buffer.full(), true);
        bool all_y = true;
        for (size_t i = size_before; i < buffer.capacity(); ++i)
        {
            if (buffer.data()[i] != 'y') all_y = false;
        }
        COUT(all_y, true);
    }
}

DEF_TAST(jstring_append_count_char, "StringBuffer append(count, ch) 测试")
{
    DESC("基础测试：追加多个相同字符");
    {
        JString buffer;
        buffer.append("hello");
        buffer.append(5, '!');
        COUT(buffer.size(), 10);
        COUT(strcmp(buffer.c_str(), "hello!!!!!"), 0);
    }

    DESC("超过容量时自动扩容");
    {
        JString buffer(10);
        buffer.append("test");
        buffer.append(100, 'x');
        COUT(buffer.size(), 104);
        bool all_x = true;
        for (size_t i = 4; i < 104; ++i)
        {
            if (buffer.data()[i] != 'x') all_x = false;
        }
        COUT(all_x, true);
    }

    DESC("count=0 不追加任何字符");
    {
        JString buffer;
        buffer.append("hello");
        buffer.append(0, 'x');
        COUT(buffer.size(), 5);
        COUT(strcmp(buffer.c_str(), "hello"), 0);
    }

    DESC("fill 和 append 组合使用");
    {
        JString buffer(100);
        buffer.append("prefix");
        buffer.fill('-', 5);
        buffer.append(3, '?');
        COUT(strcmp(buffer.c_str(), "prefix-----???"), 0);
    }
}

/// @}
/* ---------------------------------------------------------------------- */
