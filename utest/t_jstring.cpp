#include "couttast/tinytast.hpp"
#include "test_util.h"
#include "jstring.hpp"
#include "couttast/couthex.hpp"
#include <charconv>  // for std::to_chars
#include <system_error>  // for std::make_error_code
#include <string>
#include <string_view>

using namespace wwjson;

/// @brief Test for StringBuffer/JString class
/// @{

DEF_TAST(jstr_construct, "JString 基础构造测试")
{
    DESC("默认构造: 申请 1024 字节，容量 1023");
    {
        JString buffer;
        COUT(buffer.empty(), true);
        COUT(buffer.size(), 0);
        COUT(buffer.capacity(), 1023);
        COUT((void*)buffer.data() != nullptr, true);
        COUT(static_cast<bool>(buffer), true);
        COUT(buffer.c_str() != nullptr, true);
    }

    DESC("带容量构造: 向上对齐 8 字节");
    {
        JString buffer(100);
        COUT(buffer.empty(), true);
        COUT(buffer.size(), 0);
        COUT(buffer.capacity() >= 100, true);
        // 100 + 4 + 1 = 105，对齐到 112，capacity = 111
        COUT(buffer.capacity(), 111);
        COUT((void*)buffer.data() != nullptr, true);
        COUT(static_cast<bool>(buffer), true);

        // 显然可当基类使用
        const BufferView& view = buffer;
        COUT(view.reserve_ex(), buffer.capacity());
    }

    DESC("最小容量构造: 向上对齐 8 字节");
    {
        JString buffer(0);
        COUT(buffer.empty(), true);
        COUT(buffer.size(), 0);
        COUT(buffer.capacity() > 0, true);
        COUT(buffer.capacity(), 8-1);

        // 第一次自动扩容
        void* ptrBefore = buffer.begin();
        COUT(ptrBefore);
        buffer.append(8, 'x');
        COUT(buffer.capacity(), 16-1);
        void* ptrAfter = buffer.begin();
        COUT(ptrAfter);
        COUT(ptrAfter == ptrBefore);
    }
}

DEF_TAST(jstr_copy_move, "JString 复制和移动语义测试")
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

DEF_TAST(jstr_append_string, "JString 字符串追加测试")
{
    DESC("追加 C 风格字符串");
    {
        JString buffer;

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

    DESC("追加 std::string");
    {
        JString buffer;

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

    DESC("追加 std::string_view");
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
}

DEF_TAST(jstr_push_char, "JString 单字符追加测试")
{
    DESC("push back");
    {
        JString buffer(0); // minimize intitial capacity
        COUT(buffer.capacity(), 8-1);

        buffer.push_back('H');
        buffer.push_back('e');
        buffer.push_back('l');
        buffer.push_back('l');
        buffer.push_back('o');

        COUT(buffer.size(), 5);
        COUT(buffer.str(), "Hello");

        for (int i = 0; i < 10; ++i)
        {
            buffer.push_back('H');
            buffer.push_back('e');
            buffer.push_back('l');
            buffer.push_back('l');
            buffer.push_back('o');
        }

        COUT(buffer.size(), 55);
        COUT(buffer.capacity());
        COUT(buffer.capacity() > 55, true);
    }

    DESC("append count of char");
    {
        JString buffer(0);
        buffer.append("hello");
        buffer.append(5, '!');
        COUT(buffer.size(), 10);
        COUT(strcmp(buffer.c_str(), "hello!!!!!"), 0);

        buffer.clear();
        COUT(buffer.size(), 0);
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

    DESC("append count=0 不追加任何字符");
    {
        JString buffer;
        buffer.append("hello");
        buffer.append(0, 'x');
        COUT(buffer.size(), 5);
        COUT(strcmp(buffer.c_str(), "hello"), 0);
    }
}

DEF_TAST(jstr_edge_cases, "JString 边界情况测试")
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


DEF_TAST(jstr_capacity_growth, "JString 容量增长测试")
{
    DESC("使用最小的初始容量观察扩容行为，指数增长上限为 1024");
    {
        // 使用最小初始容量
        JString buffer(0);
        COUT(buffer.capacity(), 8-1);  // 最小对齐到 8 字节

        // 记录容量变化序列
        DESC("指数增长阶段：容量翻倍直到达到上限 1024");
        std::vector<size_t> capacities;

        size_t prev_cap = 0;
        while (buffer.size() < 5000)
        {
            buffer.push_back('x');
            capacities.push_back(buffer.capacity());
            size_t cur_cap = buffer.capacity();
            if (cur_cap > prev_cap)
            {
                DESC("size=%zu; capacity=%zu", buffer.size(), cur_cap);
                prev_cap = cur_cap;
            }
        }

        DESC("验证扩容序列符合预期");
        // 初始容量: 7 (8-1)
        COUT(capacities[0], 7);
        COUT(buffer.size() >= capacities[0], true);

        // 验证容量增长模式
        // 预期序列: 7 -> 15 -> 31 -> 63 -> 127 -> 255 -> 511 -> 1023 (指数增长)
        // 之后: 1023 -> 1023+1024=2047 -> 2047+1024=3071 ... (线性增长)

        // 找到所有不同的容量值
        std::vector<size_t> unique_caps;
        size_t last_cap = 0;
        for (size_t cap : capacities)
        {
            if (cap != last_cap)
            {
                unique_caps.push_back(cap);
                last_cap = cap;
            }
        }

        // 验证指数增长阶段
        bool exponential_growh_correct = true;
        if (unique_caps.size() >= 8)
        {
            COUT(unique_caps[0], 7);    // 8-1
            COUT(unique_caps[1], 15);   // 16-1
            COUT(unique_caps[2], 31);   // 32-1
            COUT(unique_caps[3], 63);   // 64-1
            COUT(unique_caps[4], 127);  // 128-1
            COUT(unique_caps[5], 255);  // 256-1
            COUT(unique_caps[6], 511);  // 512-1
            COUT(unique_caps[7], 1023);  // 1024-1
        }
        else
        {
            exponential_growh_correct = false;
        }

        // 验证线性增长阶段（超过 1024 后每次 +1024）
        bool linear_growth_correct = true;
        if (unique_caps.size() >= 9)
        {
            // 从指数增长切换到线性增长
            size_t exp_max = 1023;  // 1024-1
            size_t linear_step = 1024;  // JSTRING_MAX_EXP_ALLOC_SIZE

            // 检查后续容量
            for (size_t i = 8; i < unique_caps.size(); ++i)
            {
                size_t expected = exp_max + (i - 7) * linear_step;
                COUT(unique_caps[i], expected);
                if (unique_caps[i] != expected)
                {
                    linear_growth_correct = false;
                }
            }
        }

        COUT(exponential_growh_correct, true);
        COUT(linear_growth_correct, true);
    }

    DESC("验证 reserve 触发扩容");
    {
        JString buffer(0);
        COUT(buffer.capacity(), 7);

        // 第一次扩容
        buffer.reserve(8);  // 需要 8 + kUnsafeLevel(4) + 1 = 13，对齐到 16
        COUT(buffer.capacity(), 15);

        // 第二次扩容（指数增长）
        buffer.reserve(20);  // 需要 20 + 4 + 1 = 25，翻倍扩容到 32
        COUT(buffer.capacity(), 31);

        // 触发到上限
        buffer.reserve(1024);  // 需要 1024 + 4 + 1 = 1029，扩容到 1032
        COUT(buffer.capacity(), 1031);

        // 超过上限，线性增长
        buffer.reserve(2000);  // 需要 2000 + 4 + 1 = 2005，线性增长 1024+1024=2048
        COUT(buffer.capacity() >= 2047, true);  // 2048-1
    }

    DESC("验证 append 触发自动扩容");
    {
        JString buffer(0);
        size_t initial_cap = buffer.capacity();

        // 填满当前容量
        buffer.fill('x', buffer.capacity());
        COUT(buffer.full(), true);

        // 追加一个字符，触发扩容
        buffer.append("!");
        COUT(buffer.capacity() > initial_cap, true);
        COUT(buffer.capacity() >= initial_cap * 2 - 1, true);  // 指数增长
    }
}

DEF_TAST(jstr_reserve, "JString 容量预留测试")
{
    JString buffer(0);
    COUT(buffer.capacity(), 8-1);

    // 初始预留
    buffer.reserve(100);
    COUT(buffer.capacity() >= 100 + 4, true); // capacity() should be >= requested
    COUT(buffer.capacity(), 112-1);
    COUT(buffer.empty(), true);

    buffer.fill('x', buffer.capacity());
    COUT(buffer.full(), true);

    // 保存扩容前信息
    void* ptrBefore = buffer.begin();
    std::string strBefore = buffer.str();
    COUT(ptrBefore);

    // 预留额外字节，翻倍扩容
    buffer.reserve_ex(50);
    COUT(buffer.capacity() >= buffer.size() + 50 + 4, true);
    COUT(buffer.capacity(), (112*2)-1);

    // 扩容后信息
    void* ptrAfter = buffer.begin();
    std::string strAfter = buffer.str();
    COUT(ptrAfter);
    COUT(strAfter == strBefore, true);
    COUT(ptrAfter == ptrBefore);

    // 预留后追加
    size_t sizeBefore = buffer.size();
    buffer.append("This is a test string for reserve operation");
    COUT(buffer.size() <= buffer.capacity(), true);
    COUT(buffer.capacity(), (112*2)-1);
    COUT(::strcmp(buffer.c_str() + sizeBefore, "This is a test string for reserve operation"), 0);
}

DEF_TAST(jstr_unsafe_operations, "JString 不安全操作测试")
{
    JString buffer(0);
    
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

DEF_TAST(jstr_unsafe_levels, "StringBuffer 不安全级别语义测试")
{
    StringBuffer<2> buffer2(4);
    StringBuffer<8> buffer8(4);
    
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
    
    // reserve_ex(5) 之后，都可以不安全 push back 5 次
    for (int i = 0; i < 5; ++i)
    {
        buffer2.unsafe_push_back('a' + i);
        buffer8.unsafe_push_back('a' + i);
    }
    
    COUT(buffer2.size(), 9);
    COUT(buffer8.size(), 9);

    // 允许额外的不安全写入次数
    for (int i = 0; i < 2; ++i)
    {
        buffer2.unsafe_push_back('!');
    }

    for (int i = 0; i < 8; ++i)
    {
        buffer8.unsafe_push_back('!');
    }

    COUT(buffer2.size() <= buffer2.capacity(), true);
    COUT(buffer8.size() <= buffer8.capacity(), true);
    COUT(buffer2.overflow(), false);
    COUT(buffer8.overflow(), false);

    // unsafe level = 4
    JString buffer(0);
    buffer.fill('x', buffer.capacity());
    COUT(buffer.full(), true);
    COUT(buffer.capacity(), 8-1);
    COUT(buffer.size(), buffer.capacity());

    buffer.append("Hello");
    COUT(buffer.size(), 7+5);
    COUT(buffer.capacity(), 24-1); // 7+5+4=16 +1-> 24 
    for (int i = 0; i < 4; ++i)
    {
        // unsafe level = 4, 允许 unsafe 再写入 4 字节
        // 由于向上对齐，也许可以写入更多，但不保证安全
        buffer.unsafe_push_back('!');
    }
    COUT(buffer.size(), 7+5+4);
    COUT(buffer.capacity(), 24-1);
}

DEF_TAST(jstr_json_patterns, "JString JSON 序列化模式测试")
{
    JString buffer(0);
    
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

DEF_TAST(jstr_extern_write, "StringBuffer 与外部方法写入集成协作")
{
    DESC("using std::to_chars to write numbers");
    {
        JString buffer(0);

        int value = 4213;
        buffer.reserve_ex(4);
        auto result = std::to_chars(buffer.begin(), buffer.cap_end(), value);
        size_t len = result.ptr - buffer.begin();

        buffer.resize(len);
        COUT(buffer.size(), 4);
        COUT(buffer.str(), "4213");

        // Write another number
        value = 12345678;
        buffer.reserve_ex(8);
        result = std::to_chars(buffer.end(), buffer.cap_end(), value);
        len = result.ptr - buffer.end();
        buffer.set_end(result.ptr);
        COUT(buffer.size(), 12);
        COUT(buffer.str(), "421312345678");
    }

    DESC("using snprintf to write formatted string");
    {
        JString buffer;

        // 默认构造有 1023 字节，这里不 reserve_ex 也行
        int written = snprintf(buffer.begin(), buffer.capacity() + 1, "%s %d", "value", 100);
        buffer.unsafe_resize(written);
        COUT(buffer.size(), 9);
        COUT(buffer.str(), "value 100");

        // Append another formatted string
        written = snprintf(buffer.end(), buffer.reserve_ex() + 1, " %s", "appended");
        buffer.unsafe_set_end(buffer.end() + written);
        COUT(buffer.size(), 18);
        COUT(buffer.str(), "value 100 appended");
    }
}

/// @}
/* ---------------------------------------------------------------------- */

/// @brief Test for StringBuffer/KString class
/// @{

DEF_TAST(kstr_construct, "KString 基础构造测试")
{
    DESC("KString(0) 最少申请 256 字节，容量 255");
    {
        KString buffer(0);
        COUT(buffer.empty(), true);
        COUT(buffer.size(), 0);
        COUT(buffer.capacity(), 255);
        COUT((void*)buffer.data() != nullptr, true);
        COUT(static_cast<bool>(buffer), true);
    }

    DESC("KString 默认构造仍是 1024 字节");
    {
        KString buffer;
        COUT(buffer.empty(), true);
        COUT(buffer.size(), 0);
        COUT(buffer.capacity(), 1023);  // 1024 - 1 for null terminator
    }

    DESC("KString(4k) 传较大初始容量");
    {
        KString buffer(4096);
        COUT(buffer.empty(), true);
        COUT(buffer.size(), 0);
        COUT(buffer.capacity() >= 4096, true);
        COUT(buffer.capacity(), 4096 + 255);  // capacity + unsafe level margin
    }

    DESC("基本写入操作");
    {
        KString buffer(256);
        buffer.append("hello");
        COUT(buffer.size(), 5);
        COUT(strcmp(buffer.c_str(), "hello"), 0);

        buffer.push_back(' ');
        buffer.push_back('w');
        buffer.push_back('o');
        buffer.push_back('r');
        buffer.push_back('l');
        buffer.push_back('d');
        COUT(buffer.size(), 11);
        COUT(strcmp(buffer.c_str(), "hello world"), 0);
    }
}

DEF_TAST(kstr_reach_full, "KString 写满对比测试")
{
    DESC("相同初始容量，相同写入操作");
    {
        size_t init_cap = 256;

        // KString: 写满后 full() = true，不会扩容
        KString kstr(init_cap);
        size_t kstr_cap = kstr.capacity();
        for (size_t i = 0; i < kstr_cap; ++i) { kstr.push_back('x'); }
        COUT(kstr.full(), true);
        COUT(kstr.capacity(), kstr_cap);  // 容量不变

        // JString: 写满后会自动扩容，full() 不会 true
        JString jstr(init_cap);
        size_t jstr_cap = jstr.capacity();
        for (size_t i = 0; i < jstr_cap; ++i) { jstr.push_back('x'); }
        COUT(jstr.full(), false);
        COUT(jstr.capacity() > jstr_cap, true);  // 已扩容

        // StringBuffer<254>: 同样会自动扩容
        StringBuffer<254> sb(init_cap);
        size_t sb_cap = sb.capacity();
        for (size_t i = 0; i < sb_cap; ++i) { sb.push_back('x'); }
        COUT(sb.full(), false);
        COUT(sb.capacity() > sb_cap, true);  // 已扩容
    }

    DESC("KString 多次追加不扩容");
    {
        KString buffer(128);
        size_t init_cap = buffer.capacity();

        buffer.append("hello");
        buffer.append(" world");
        buffer.append(" this is a test");

        COUT(buffer.full(), false);
        COUT(buffer.capacity(), init_cap);  // 容量不变

        // 继续追加直到写满
        while (!buffer.full()) { buffer.push_back('x'); }
        COUT(buffer.full(), true);
        COUT(buffer.capacity(), init_cap);  // 仍未扩容
    }

    DESC("KString 写满后显式 reserve 可扩容");
    {
        KString buffer(256);
        size_t init_cap = buffer.capacity();

        // 写满
        while (!buffer.full()) { buffer.push_back('x'); }
        COUT(buffer.full(), true);
        COUT(buffer.capacity(), init_cap);

        // 显式调用 reserve 扩容
        buffer.reserve(init_cap * 2);
        COUT(buffer.capacity() > init_cap, true);
        COUT(buffer.full(), false);

        // 扩容后可以继续写入
        size_t write_count = 100;
        buffer.append(write_count, 'y');
        COUT(buffer.size(), init_cap + write_count);
        COUT(buffer.capacity() >= init_cap + write_count, true);
        COUT(buffer.full(), false);
        COUT(buffer.overflow(), false);

        // 检查数据正确性
        bool all_x = true;
        for (size_t i = 0; i < init_cap; ++i)
        {
            if (buffer.data()[i] != 'x') all_x = false;
        }
        bool all_y = true;
        for (size_t i = init_cap; i < init_cap + write_count; ++i)
        {
            if (buffer.data()[i] != 'y') all_y = false;
        }
        COUT(all_x, true);
        COUT(all_y, true);
    }
}

/// @}
/* ---------------------------------------------------------------------- */
