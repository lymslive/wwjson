#include "couttast/tinytast.hpp"
#include "test_util.h"
#include "jstring.hpp"
#include <charconv>  // for std::to_chars
#include <system_error>  // for std::make_error_code
#include <string>
#include <string_view>

using namespace wwjson;

DEF_TAST(jstring_basic_construct, "JString 基础构造测试")
{
    // 默认构造
    JString buffer1;
    COUT(buffer1.empty(), true);
    COUT(buffer1.size(), 0);
    COUT(buffer1.capacity(), 0);
    COUT((void*)buffer1.data(), nullptr);
    // 未分配内存，但 c_str 返回非 nullptr, 指向静态空串
    COUT(buffer1.c_str() != nullptr, true);
    
    // 带容量构造
    JString buffer2(100);
    COUT(buffer2.empty(), true);
    COUT(buffer2.size(), 0);
    COUT(buffer2.capacity() >= 100, true);
    COUT((void*)buffer2.data() != nullptr, true);
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
    
    // 复制构造
    JString copy(original);
    COUT(copy.size(), original.size());
    COUT(strcmp(copy.data(), original.data()), 0);
    COUT(copy.data() != original.data(), true); // 不同内存
    
    // 复制赋值
    JString copy2;
    copy2 = original;
    COUT(copy2.size(), original.size());
    COUT(strcmp(copy2.data(), original.data()), 0);
    COUT(copy2.data() != original.data(), true); // 不同内存
    
    // 移动构造
    JString moved(std::move(original));
    COUT(moved.size(), 11);
    COUT(strcmp(moved.data(), "Hello World"), 0);
    COUT(original.empty(), true); // 原对象应为空
    COUT(original.data() == nullptr, true);
    
    // 移动赋值
    JString moved2;
    moved2 = std::move(moved);
    COUT(moved2.size(), 11);
    COUT(strcmp(moved2.data(), "Hello World"), 0);
    COUT(moved.empty(), true); // 原对象应为空
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

DEF_TAST(jstring_buffer_view, "StringBufferView 转换测试")
{
    JString buffer;
    buffer.append("test content");

    // 使用 reinterpret_cast 转换为 StringBufferView
    // 现在使用 public 继承，static_cast 可用
    const StringBufferView& view = static_cast<const StringBufferView&>(buffer);

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
    JString buffer;
    
    size_t initial_capacity = buffer.capacity();
    COUT(initial_capacity, 0);
    
    // 小追加应触发分配
    buffer.append("small");
    COUT(buffer.capacity() > 0, true);
    
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
        std::string fill(buffer.capacity(), 'x');
        buffer.append(fill.c_str(), fill.size());
        size_t old_size = buffer.size();

        // 最少申请 256 ，容量 255
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
    }
}

DEF_TAST(jstring_memory_alignment, "内存对齐和最小分配测试")
{
    // 测试最小分配大小 (256 字节)
    {
        JString buffer(100);
        COUT(buffer.capacity(), 255);
    }
}

DEF_TAST(jstring_invariants, "StringBuffer 不变量测试")
{
    {
        JString buffer;
        buffer.reserve(100);

        // 不变量 1: capacity() == m_cap_end - m_begin
        // 这通过 capacity() 的实现保证

        // 不变量 2: capacity() >= size()
        COUT(buffer.capacity() >= buffer.size(), true);

        buffer.append("hello");
        COUT(buffer.capacity() >= buffer.size(), true);

        // 不变量 3: m_end 指向当前内容末尾
        // 这通过 append 实现

        // 不变量 4: unsafe_end_cstr 可以在 m_end <= m_cap_end 时调用
        buffer.unsafe_end_cstr();
        COUT(strcmp(buffer.c_str(), "hello"), 0);
    }

    // 测试 m_cap_end 初始化为 \0
    {
        JString buffer(100);
        // 分配后 m_cap_end 应该被初始化为 '\0'
        // 这在 allocate() 中实现
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
        buffer.StringBufferView::unsafe_append(extra, len);

        COUT(strncmp(buffer.data() + buffer.size() - len, extra, len), 0);
        COUT(strncmp(buffer.c_str(), "Unsafe append: test data", buffer.size()), 0);
    }
}

DEF_TAST(jstring_to_string_view, "StringBufferView 隐式转换为 std::string_view")
{
    JString buffer;
    buffer.append("test content");

    // 隐式转换为 std::string_view
    std::string_view sv = buffer;

    COUT(sv.size(), buffer.size());
    COUT(sv.data(), buffer.data());
    COUT(sv.compare("test content"), 0);

    // 通过引用转换
    const StringBufferView& view = buffer;
    std::string_view sv2 = view;

    COUT(sv2.size(), buffer.size());
    COUT(sv2.data(), buffer.data());
    COUT(sv2.compare("test content"), 0);

    // 测试空字符串
    JString empty_buffer;
    std::string_view empty_sv = empty_buffer;
    COUT(empty_sv.size(), 0);
}

DEF_TAST(jstring_to_string, "StringBufferView 显式转换为 std::string")
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

