#include "couttast/tinytast.hpp"
#include "test_util.h"
#include "jstring.hpp"

using namespace wwjson;

DEF_TAST(jstring_basic_construct, "JString 基础构造测试")
{
    // 默认构造
    JString buffer1;
    COUT(buffer1.empty(), true);
    COUT(buffer1.size(), 0);
    COUT(buffer1.capacity(), 0);
    COUT((void*)buffer1.data(), nullptr);
    
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
    COUT(strncmp(buffer.data(), "hello", 5), 0);
    
    // 按长度追加
    buffer.append(" world", 6);
    COUT(buffer.size(), 11);
    COUT(strncmp(buffer.data(), "hello world", 11), 0);
    
    // 多次追加
    buffer.append("!");
    buffer.append(" JSON");
    COUT(buffer.size(), 17);
    COUT(strncmp(buffer.data(), "hello world! JSON", 17), 0);
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
    COUT(strncmp(buffer.data(), "Hello", 5), 0);
}

DEF_TAST(jstring_reserve, "JString 容量预留测试")
{
    JString buffer;
    
    // 初始预留
    buffer.reserve(100);
    COUT(buffer.capacity() >= 100, true);
    COUT(buffer.empty(), true);
    
    // 带边界的预留
    buffer.reserve_ex(50);
    COUT(buffer.capacity() >= 50 + 4, true); // 4 is kUnsafeLevel for JString
    
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
    COUT(strncmp(buffer.data(), "key:\"va", 7), 0);
    
    // 测试 unsafe_set_end
    buffer.unsafe_set_end(3);
    COUT(buffer.size(), 3);
    COUT(strncmp(buffer.data(), "key", 3), 0);
    
    // 测试 unsafe_end_cstr
    buffer.unsafe_set_end(7);
    buffer.unsafe_push_back('l');
    buffer.unsafe_push_back('u');
    buffer.unsafe_push_back('e');
    buffer.unsafe_push_back('"');
    
    COUT(buffer.size(), 11);
    COUT(strncmp(buffer.c_str(), "key:\"value\"", 11), 0);
    
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
    COUT(strncmp(buffer2.data(), "test", 4), 0);
    COUT(strncmp(buffer8.data(), "test", 4), 0);
    
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
    
    // 对空缓冲区清空
    buffer.clear();
    COUT(buffer.empty(), true);
    
    // 追加空字符串
    buffer.append("");
    COUT(buffer.size(), 0);
    
    // 在空缓冲区上进行不安全操作
    buffer.reserve_ex(10);
    buffer.unsafe_set_end(0);
    COUT(buffer.size(), 0);
    
    // 空缓冲区的 c_str - 使用全新未分配内存的缓冲区
    JString empty_buffer;
    COUT(empty_buffer.c_str() == nullptr, true);
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
    COUT(strncmp(buffer.data(), "\"name\":\"John Doe\",", 19), 0);
    
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
    // 在私有继承时 static_cast 不可用
    StringBufferView view = reinterpret_cast<const StringBufferView&>(buffer);
    
    COUT(view.m_end - view.m_begin, buffer.size());
    COUT(view.m_begin, buffer.data());
    COUT(strncmp(buffer.data(), "test content", 12), 0);
    
    // 手动计算视图属性
    bool view_empty = (view.m_end == view.m_begin);
    COUT(view_empty, false);
    COUT(*view.m_begin, 't');
    COUT(*(view.m_end - 1), 't');
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
    COUT(strncmp(buffer.data(), "hell!", 5), 0);
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
    COUT(strncmp(buffer.data(), "new content", 11), 0);
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
