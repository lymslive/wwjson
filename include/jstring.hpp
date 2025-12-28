/**
 * @file jstring.hpp
 * @author lymslive
 * @date 2025-12-23
 * @version 1.0.0
 *
 * @brief High-performance string buffer class optimized for JSON serialization.
 *
 * @details This header provides a specialized string buffer implementation designed
 * specifically for JSON string construction. It addresses performance issues with
 * standard std::string by minimizing boundary checks through unsafe operations
 * and providing direct buffer manipulation capabilities.
 * */

#pragma once
#ifndef JSTRING_HPP__
#define JSTRING_HPP__

#include <cassert>
#include <cstring>
#include <memory>
#include <type_traits>
#include <string>
#include <string_view>
#include <vector>
#include <array>

#include <stdint.h>

// Compile-time configuration for memory allocation
#ifndef JSTRING_MAX_EXP_ALLOC_SIZE
#define JSTRING_MAX_EXP_ALLOC_SIZE (8 * 1024 * 1024)  // 8MB - max exponential growth
#endif

namespace wwjson {

/// @brief Concept definition for unsafe string operations in JSON building
/// @details
/// This struct defines the interface for string types that support unsafe operations
/// optimized for JSON serialization. The key innovation is the concept of "unsafe level"
/// which allows a batch of unsafe operations after a safe boundary check.
/// 
/// @par Unsafe Level Definition:
/// kUnsafeLevel defines the number of unsafe operations allowed after a safe check.
/// Example: kUnsafeLevel=4 means after reserve(n), you can call unsafe_push_back 
/// 4 times even after write n characters. 
/// The null terminator space is always additional (+1 byte).
/// 
/// @par Required Interface Methods:
/// - static constexpr uint8_t kUnsafeLevel: Number of unsafe operations allowed
/// - unsafe_push_back(char c): Add character without boundary check
/// - unsafe_set_end(size_t new_size): Directly set string length
/// - unsafe_end_cstr(): Add null terminator at end
/// - reserve(size_t capacity): Reserve with kUnsafeLevel bytes margin
struct UnsafeStringConcept
{
};

/// @brief Integer type alias for unsafe level specification
using UnsafeLevel = uint8_t;

/// @brief Three-pointer view for string buffer management (safe mode, kUnsafeLevel=0)
/// @details
/// Non-owning buffer view with three-pointer design (m_begin, m_end, m_cap_end).
///
/// Provides:
/// - **Safe write operations** with boundary checking
/// - **Unsafe write operations** without boundary checks (for high performance)
/// - **Multiple constructors**: Raw pointer, C array, std::array, std::string, std::vector
/// - **Non-owning semantics**: Copy/move transfer pointer ownership (not deallocated)
///
/// @par Warning:
/// When borrowing from std::string or std::vector, buffer view does not update
/// container's size() method. Manually call container.resize() after writing.
class BufferView : public UnsafeStringConcept
{
protected:
    char* m_begin = nullptr;     ///< Start of buffer memory
    char* m_end = nullptr;       ///< Current end of string content
    char* m_cap_end = nullptr;   ///< End of allocated capacity

public:
    static constexpr uint8_t kUnsafeLevel = 0;

    /// Default constructor - initializes all pointers to nullptr
    BufferView() = default;

    /// @brief Base class constructor for external buffer management
    /// @param dst Pointer to memory to borrow
    /// @param size Size of the memory region (must be > 0)
    /// @details
    /// Sets m_begin and m_end to dst, m_cap_end to dst + size - 1.
    /// Writes '\0' at m_cap_end for safety.
    BufferView(char* dst, size_t size)
    {
        assert(dst != nullptr && "BufferView constructor: dst cannot be nullptr");
        assert(size > 0 && "BufferView constructor: size must be > 0");
        m_begin = dst;
        m_end = dst;
        m_cap_end = dst + size - 1;
        *m_cap_end = '\0';
    }

    /// @brief Constructor from C array
    /// @param dst Reference to C array of characters
    template <size_t N>
    explicit BufferView(char (&dst)[N])
        : BufferView(dst, N)
    {
    }

    /// @brief Constructor from std::array
    /// @param dst Reference to std::array of characters
    template <size_t N>
    explicit BufferView(std::array<char, N>& dst)
        : BufferView(dst.data(), N)
    {
    }

    /// @brief Constructor from std::string
    /// @param dst Reference to std::string (must have reserved capacity)
    /// @warning
    /// Does NOT update string's size. After writing, you'll need to manually
    /// call dst.resize() if you need the string's size to reflect the content.
    explicit BufferView(std::string& dst)
        : BufferView(dst.data(), dst.capacity())
    {
    }

    /// @brief Constructor from std::vector<char>
    /// @param dst Reference to std::vector<char> (must have reserved capacity)
    /// @warning
    /// Does NOT update vector's size. After writing, you'll need to manually
    /// call dst.resize() if you need the vector's size to reflect the content.
    explicit BufferView(std::vector<char>& dst)
        : BufferView(dst.data(), dst.capacity())
    {
    }

    /// Copy constructor is deleted (non-owning semantics)
    BufferView(const BufferView&) = delete;

    /// Move constructor - transfer ownership of buffer pointers
    BufferView(BufferView&& other) noexcept
    {
        m_begin = other.m_begin;
        m_end = other.m_end;
        m_cap_end = other.m_cap_end;
        other.m_begin = nullptr;
        other.m_end = nullptr;
        other.m_cap_end = nullptr;
    }

    /// Copy assignment is deleted (non-owning semantics)
    BufferView& operator=(const BufferView&) = delete;

    /// Move assignment - transfer ownership of buffer pointers
    BufferView& operator=(BufferView&& other) noexcept
    {
        if (this != &other)
        {
            m_begin = other.m_begin;
            m_end = other.m_end;
            m_cap_end = other.m_cap_end;
            other.m_begin = nullptr;
            other.m_end = nullptr;
            other.m_cap_end = nullptr;
        }
        return *this;
    }

    /// Destructor - does nothing (doesn't own memory)
    ~BufferView() = default;

    /// Read-only access methods
    size_t size() const { return m_end - m_begin; }
    size_t capacity() const { return m_cap_end - m_begin; }
    bool empty() const { return m_end == m_begin; }
    const char* data() const { return m_begin; }

    /// Check if memory is allocated
    explicit operator bool() const { return m_begin != nullptr; }

    /// @brief Check if write operations have exceeded buffer capacity
    /// @return true if m_end is beyond m_cap_end (overflow occurred)
    bool overflow() const { return m_end > m_cap_end; }

    /// @brief Check if buffer is full (no remaining space)
    /// @return true if m_end == m_cap_end (no space left for writing)
    bool full() const { return m_end == m_cap_end; }

    /// @brief Check remaining available bytes in buffer
    /// @return
    ///   - Positive value: bytes available from m_end to m_cap_end
    ///   - Zero: buffer is exactly full (m_end == m_cap_end)
    ///   - Negative value: overflow detected (m_end > m_cap_end, value = overflowed bytes)
    int64_t reserve_ex() const
    {
        return static_cast<int64_t>(m_cap_end - m_end);
    }

    /// Iterator-like methods returning pointers
    char* begin() { return m_begin; }
    const char* begin() const { return m_begin; }
    char* end() { return m_end; }
    const char* end() const { return m_end; }
    char* cap_end() { return m_cap_end; }
    const char* cap_end() const { return m_cap_end; }

    const char* c_str()
    {
        if (m_begin == nullptr)
        {
            return "";
        }
        if (m_end < m_cap_end)
        {
            *m_end = '\0';
        }
        return m_begin;
    }

    /// Implicit conversion to std::string_view
    operator std::string_view() const
    {
        return std::string_view(m_begin, size());
    }

    /// Explicit conversion to std::string (requires copy)
    explicit operator std::string() const
    {
        return std::string(m_begin, size());
    }

    char& front() 
    { 
        return const_cast<char&>(const_cast<const BufferView*>(this)->front());
    }
    const char& front() const 
    { 
        assert(m_begin != nullptr && "BufferView::front() called on null buffer");
        assert(m_end > m_begin && "BufferView::front() called on empty buffer");
        return *m_begin; 
    }
    char& back() 
    { 
        return const_cast<char&>(const_cast<const BufferView*>(this)->back());
    }
    const char& back() const 
    { 
        assert(m_begin != nullptr && "BufferView::back() called on null buffer");
        assert(m_end > m_begin && "BufferView::back() called on empty buffer");
        return *(m_end - 1); 
    }

    /// Unsafe write methods (no boundary checks)
    void unsafe_push_back(char c)
    {
        assert(m_begin != nullptr && "BufferView::unsafe_push_back() called on null buffer");
        *m_end++ = c;
    }

    void unsafe_resize(size_t new_size)
    {
        assert(m_begin != nullptr && "BufferView::unsafe_resize() called on null buffer");
        m_end = m_begin + new_size;
    }

    /// Set end pointer directly (char* overload)
    void unsafe_set_end(char* new_end)
    {
        assert(m_begin != nullptr && "BufferView::unsafe_set_end() called on null buffer");
        m_end = new_end;
    }

    /// Unsafe append without boundary checks
    void unsafe_append(const char* str, size_t len)
    {
        assert(m_begin != nullptr && "BufferView::unsafe_append() called on null buffer");
        ::memcpy(m_end, str, len);
        m_end += len;
    }

    /// Clear content without deallocation
    void clear()
    {
        unsafe_resize(0);
    }

    /// Add null terminator at current end
    void unsafe_end_cstr()
    {
        assert(m_begin != nullptr && "BufferView::unsafe_end_cstr() called on null buffer");
        *m_end = '\0';
    }

    /// Fill remaining space with character ch (like memset)
    /// @param ch Character to fill
    /// @details Fills from m_end to m_cap_end, doesn't change size
    void fill(char ch)
    {
        assert(m_begin != nullptr && "BufferView::fill() called on null buffer");
        size_t available = m_cap_end - m_end;
        ::memset(m_end, ch, available);
    }

    /// Fill count characters with ch, safely truncates at cap_end
    /// @param ch Character to fill
    /// @param count Number of characters to fill
    /// @details Fills count characters starting from m_end, moves end pointer.
    /// No capacity expansion - count is safely bounded by available capacity.
    void fill(char ch, size_t count)
    {
        assert(m_begin != nullptr && "BufferView::fill() called on null buffer");
        size_t available = m_cap_end - m_end;
        if (count > available)
        {
            count = available;
        }
        ::memset(m_end, ch, count);
        m_end += count;
    }

    /// Unsafe fill without capacity checking
    /// @param ch Character to fill
    /// @param count Number of characters to fill
    /// @details Fills count characters starting from m_end, moves end pointer.
    /// No capacity checking - caller must ensure count <= available capacity.
    void unsafe_fill(char ch, size_t count)
    {
        assert(m_begin != nullptr && "BufferView::unsafe_fill() called on null buffer");
        ::memset(m_end, ch, count);
        m_end += count;
    }

    /// @brief Append with bounds checking (safe mode)
    /// @param str String to append
    /// @param len Length of string
    /// @details Checks bounds, doesn't write if overflow would occur.
    void append(const char* str, size_t len)
    {
        if (m_end + len > m_cap_end)
        {
            return;  // Don't write beyond capacity
        }
        unsafe_append(str, len);
    }

    void append(const char* str)
    {
        if (str == nullptr) { return; }
        append(str, ::strlen(str));
    }

    void append(const std::string& str)
    {
        append(str.data(), str.size());
    }

    void append(const std::string_view& sv)
    {
        append(sv.data(), sv.size());
    }

    /// @brief Append from another BufferView (includes StringBuffer subclasses)
    /// @param other Another BufferView to append from
    void append(const BufferView& other)
    {
        append(other.data(), other.size());
    }

    /// @brief Push back with bounds checking (safe mode)
    /// @param c Character to append
    /// @details Checks bounds, doesn't write if at capacity.
    /// m_end can advance to m_cap_end, but not beyond.
    /// m_cap_end position is reserved for '\0' terminator.
    void push_back(char c)
    {
        if (m_end + 1 > m_cap_end)
        {
            return;  // Don't write beyond capacity
        }
        unsafe_push_back(c);
    }

    /// @brief Append count copies of character ch (safe mode)
    /// @param count Number of characters to append
    /// @param ch Character to append
    /// @details Check bounds before writing.
    void append(size_t count, char ch)
    {
        if (m_end + count > m_cap_end)
        {
            return;  // Don't write beyond capacity
        }
        unsafe_fill(ch, count);
    }

    /// @brief Safe resize with bounds checking
    /// @param new_size New size
    /// @details If new_size would exceed capacity, does nothing.
    void resize(size_t new_size)
    {
        if (m_begin + new_size > m_cap_end)
        {
            return;  // Don't resize beyond capacity
        }
        unsafe_resize(new_size);
    }
};

/// @brief Unsafe buffer view with no bounds checking (kUnsafeLevel=0xFF)
/// @details
/// Inherits BufferView but overrides all write methods to call unsafe_ variants.
///
/// Provides:
/// - **No bounds checking** on any write operations (maximum performance)
/// - **Inherits all constructors** from BufferView (raw pointer, C array, std::array, etc.)
/// - **Non-owning semantics**: Same as BufferView
///
/// @par Usage Example:
/// ```cpp
/// std::string str;
/// str.reserve(1024);
/// UnsafeBuffer ub(str);  // Borrow from str, no bounds checking
/// ub.append("long text");  // Fast, unsafe write
/// ```
class UnsafeBuffer : public BufferView
{
public:
    static constexpr uint8_t kUnsafeLevel = 0xFF;

    /// Inherit all constructors from BufferView.
    using BufferView::BufferView;

    void append(const char* str, size_t len)
    {
        unsafe_append(str, len);
    }

    void append(const char* str)
    {
        if (str == nullptr) { return; }
        unsafe_append(str, ::strlen(str));
    }

    void append(const std::string& str)
    {
        unsafe_append(str.data(), str.size());
    }

    void append(const std::string_view& sv)
    {
        unsafe_append(sv.data(), sv.size());
    }

    void append(const BufferView& other)
    {
        unsafe_append(other.data(), other.size());
    }

    void push_back(char c)
    {
        unsafe_push_back(c);
    }

    void append(size_t count, char ch)
    {
        unsafe_fill(ch, count);
    }

    void resize(size_t new_size)
    {
        unsafe_resize(new_size);
    }
};

/// @brief High-performance string buffer with unsafe operations
/// @tparam kUnsafeLevel Number of unsafe operations allowed after each safe check
/// @details
/// StringBuffer implements the UnsafeStringConcept interface by inheriting from
/// BufferView. The key features are:
///
/// - Batch boundary checking with safety margin
/// - Direct buffer manipulation without intermediate copies
/// - Deferred null termination until final result is needed
/// - Support for unsafe character writes after safe capacity checks
///
/// @par Memory Allocation Policy:
/// - Minimum allocation: 256 bytes (configurable via JSTRING_MIN_ALLOC_SIZE)
/// - Growth strategy: Exponential (2x) until 8MB, then linear +8MB
/// - Alignment: Memory is aligned to improve efficiency
/// - Capacity invariant: capacity() = m_cap_end - m_begin
/// - Always allocates at least capacity() + 1 bytes for null terminator
///
/// @par kUnsafeLevel Semantics:
/// kUnsafeLevel specifies additional bytes beyond user-requested capacity.
/// Example: For kUnsafeLevel=4, after reserve(100):
///   - capacity() returns >=104 (100 user + 4 margin)
///   - Actual allocation: aligned to >= 105 bytes (104 + 1 null terminator)
///   - You can write upto 100 characters for main content, and then
///   - allow to call unsafe_push_back 4 times safely
/// This definition allows kUnsafeLevel=0 to represent behavior similar to std::string.
///
/// @par Invariants:
/// - capacity() == m_cap_end - m_begin (includes kUnsafeLevel)
/// - Actual allocation >= capacity() + 1 (for null terminator)
/// - m_cap_end is within allocated memory, position reserved for potential null terminator
/// - The byte at m_cap_end is written with '\0' on allocation for safety
///
/// @par Usage Examples:
/// ```cpp
/// StringBuffer<4> buffer;  // JString alias
/// buffer.unsafe_push_back('"');  // Assume has initial margin
/// buffer.append("key");          // Check and ensure 4 margin
/// buffer.unsafe_push_back('"');  // Safe within margin (1 of 4)
/// buffer.unsafe_push_back(':');  // (2 of 4)
/// buffer.unsafe_push_back('"');  // (3 of 4)
/// buffer.append("value");        // Check and ensure 4 margin
/// buffer.unsafe_push_back('"');  // (1 of 4)
/// buffer.unsafe_push_back(',');  // (1 of 4)
/// buffer.unsafe_end_cstr();      // Add null terminator at m_end
/// ```
template <UnsafeLevel LEVEL>
class StringBuffer : public BufferView
{
public:
    static constexpr uint8_t kUnsafeLevel = LEVEL;
    static constexpr size_t kDefaultAllocate = 1024;  // Default memory allocation size

    StringBuffer() : StringBuffer(kDefaultAllocate - kUnsafeLevel - 1)
    {
    }

    explicit StringBuffer(size_t capacity)
    {
        allocate(capacity + kUnsafeLevel + 1);
    }

    StringBuffer(const StringBuffer& other)
    {
        copy_from(other);
    }

    StringBuffer(StringBuffer&& other) noexcept
    {
        move_from(std::move(other));
    }

    ~StringBuffer() { deallocate(); }

    StringBuffer& operator=(const StringBuffer& other)
    {
        if (this != &other)
        {
            deallocate();
            copy_from(other);
        }
        return *this;
    }

    StringBuffer& operator=(StringBuffer&& other) noexcept
    {
        if (this != &other)
        {
            deallocate();
            move_from(std::move(other));
        }
        return *this;
    }

    void reserve_ex(size_t add_capacity) { reserve(size() + add_capacity); }
    void reserve(size_t new_capacity)
    {
        size_t total_capacity = new_capacity + kUnsafeLevel;
        if (total_capacity > capacity())
        {
            reallocate(total_capacity + 1);
        }
    }

    void append(const char* str)
    {
        size_t len = strlen(str);
        append(str, len);
    }

    void append(const char* str, size_t len)
    {
        reserve_ex(len);
        unsafe_append(str, len);
    }

    void append(const std::string& str)
    {
        append(str.data(), str.size());
    }

    void append(const std::string_view& sv)
    {
        append(sv.data(), sv.size());
    }

    /// @brief Append from another BufferView (includes other StringBuffer)
    /// @param other Another BufferView to append from
    void append(const BufferView& other)
    {
        append(other.data(), other.size());
    }

    void push_back(char c)
    {
        reserve_ex(1);
        unsafe_push_back(c);
    }

    /// Append count copies of character ch (like std::string::append)
    /// @param count Number of characters to append
    /// @param ch Character to append
    void append(size_t count, char ch)
    {
        reserve_ex(count);
        unsafe_fill(ch, count);
    }

    /// Safe resize with bounds checking and capacity expansion
    void resize(size_t new_size)
    {
        if (new_size > capacity())
        {
            reserve(new_size);
        }
        unsafe_resize(new_size);
    }

    /// Safe set_end with pointer bounds checking
    void set_end(char* new_end)
    {
        if (new_end < m_begin || new_end > m_cap_end)
        {
            return;
        }
        unsafe_set_end(new_end);
    }

    /// Safe end_cstr with bounds checking
    void end_cstr()
    {
        if (m_end > m_cap_end)
        {
            return;
        }
        unsafe_end_cstr();
    }

private:
    static size_t calculate_alloc_size(size_t size)
    {
        if (size == 0)
        {
            return 0;
        }
        
        const size_t alignment = 8;
        size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
        return aligned_size;
    }

    static size_t calculate_growth_size(size_t cur_size, size_t req_size)
    {
        size_t new_size = req_size;
        if (cur_size > 0)
        {
            if (cur_size < JSTRING_MAX_EXP_ALLOC_SIZE)
            {
                size_t exp_size = cur_size * 2;
                if (exp_size > JSTRING_MAX_EXP_ALLOC_SIZE)
                {
                    exp_size = JSTRING_MAX_EXP_ALLOC_SIZE;
                }
                
                // Take maximum of requested and exponential growth
                new_size = (new_size > exp_size) ? new_size : exp_size;
            }
            else
            {
                // Linear growth: add max_exp_size each time
                size_t linear_size = cur_size + JSTRING_MAX_EXP_ALLOC_SIZE;
                new_size = (new_size > linear_size) ? new_size : linear_size;
            }
        }
        
        // Align to 8-byte boundary
        const size_t alignment = 8;
        size_t aligned_size = (new_size + alignment - 1) & ~(alignment - 1);
        return aligned_size;
    }

    void allocate(size_t size)
    {
        if (size == 0)
        {
            m_begin = nullptr;
            m_end = nullptr;
            m_cap_end = nullptr;
            return;
        }

        size_t alloc_size = calculate_alloc_size(size);
        m_begin = static_cast<char*>(std::malloc(alloc_size));
        m_end = m_begin;
        m_cap_end = m_begin + alloc_size - 1;
        *m_cap_end = '\0';
    }

    void deallocate()
    {
        if (m_begin)
        {
            std::free(m_begin);
        }
        m_begin = nullptr;
        m_end = nullptr;
        m_cap_end = nullptr;
    }

    void reallocate(size_t new_size)
    {
        size_t current_size = size();
        size_t current_alloc = capacity() + 1;
        if (new_size <= current_alloc)
        {
            return;
        }

        size_t alloc_size = calculate_growth_size(current_alloc, new_size);
        char* new_begin = static_cast<char*>(std::malloc(alloc_size));
        
        // Copy existing content
        if (current_size > 0)
        {
            memcpy(new_begin, m_begin, current_size);
        }
        
        // Free old buffer
        if (m_begin)
        {
            std::free(m_begin);
        }
        
        m_begin = new_begin;
        m_end = m_begin + current_size;
        m_cap_end = m_begin + alloc_size - 1;
        *m_cap_end = '\0';
    }

    void copy_from(const StringBuffer& other)
    {
        if (other.empty())
        {
            m_begin = nullptr;
            m_end = nullptr;
            m_cap_end = nullptr;
            return;
        }

        allocate(other.size());
        append(other.data(), other.size());
        unsafe_end_cstr();
    }

    void move_from(StringBuffer&& other)
    {
        m_begin = other.m_begin;
        m_end = other.m_end;
        m_cap_end = other.m_cap_end;

        other.m_begin = nullptr;
        other.m_end = nullptr;
        other.m_cap_end = nullptr;
    }
};

/// @brief JSON-optimized string type with kUnsafeLevel=4
/// @details This is the recommended string type for JSON serialization.
/// The unsafe level of 4 provides safety margin for common JSON patterns
/// like `":"`, `","`  which may require multiple consecutive
/// unsafe character writes.
using JString = StringBuffer<4>;

} // namespace wwjson

#endif // JSTRING_HPP__
