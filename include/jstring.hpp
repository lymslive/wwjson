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

#include <cstring>
#include <memory>
#include <type_traits>
#include <string>
#include <string_view>

#include <stdint.h>

// Compile-time configuration for memory allocation
#ifndef JSTRING_MIN_ALLOC_SIZE
#define JSTRING_MIN_ALLOC_SIZE 256  // Minimum initial allocation size
#endif

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

/// @brief Three-pointer view for string buffer management
/// @details
/// This class provides the underlying three-pointer design for string buffers:
/// - m_begin: Start of allocated memory
/// - m_end: Current end of string content (left-closed, right-open)
/// - m_cap_end: End of allocated capacity (can write null terminator here)
///
/// The design allows efficient direct memory manipulation while maintaining
/// compatibility with standard string operations. Provides read-only access
/// and unsafe write operations within the existing buffer range.
///
/// Protected members allow derived classes to directly manipulate the pointers.
class StringBufferView
{
protected:
    char* m_begin = nullptr;     ///< Start of buffer memory
    char* m_end = nullptr;       ///< Current end of string content
    char* m_cap_end = nullptr;   ///< End of allocated capacity

public:
    /// Read-only access methods
    size_t size() const { return m_end - m_begin; }
    size_t capacity() const { return m_cap_end - m_begin; }
    bool empty() const { return m_end == m_begin; }
    const char* data() const { return m_begin; }

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

    char& front() { return *m_begin; }
    const char& front() const { return *m_begin; }
    char& back() { return *(m_end - 1); }
    const char& back() const { return *(m_end - 1); }

    /// Unsafe write methods (no boundary checks)
    void unsafe_push_back(char c)
    {
        *m_end++ = c;
    }

    void unsafe_resize(size_t new_size)
    {
        m_end = m_begin + new_size;
    }

    /// Set end pointer directly (char* overload)
    void unsafe_set_end(char* new_end)
    {
        m_end = new_end;
    }

    /// Unsafe append without boundary checks
    void unsafe_append(const char* str, size_t len)
    {
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
        *m_end = '\0';
    }
};

/// @brief High-performance string buffer with unsafe operations
/// @tparam kUnsafeLevel Number of unsafe operations allowed after each safe check
/// @details
/// StringBuffer implements the UnsafeStringConcept interface by inheriting from
/// StringBufferView. The key features are:
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
class StringBuffer : public StringBufferView, public UnsafeStringConcept
{
public:
    static constexpr uint8_t kUnsafeLevel = LEVEL;

    StringBuffer()
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

    template <UnsafeLevel kOtherLevel>
    void append(const StringBuffer<kOtherLevel>& other)
    {
        append(other.data(), other.size());
    }

    void push_back(char c)
    {
        reserve_ex(1);
        unsafe_push_back(c);
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
        if (size < JSTRING_MIN_ALLOC_SIZE)
        {
            return JSTRING_MIN_ALLOC_SIZE;
        }
        
        const size_t alignment = 8;
        size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
        return aligned_size;
    }

    static size_t calculate_growth_size(size_t cur_size, size_t req_size)
    {
        size_t new_size = req_size;
        if (cur_size > JSTRING_MIN_ALLOC_SIZE)
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
        
        // Apply minimum allocation constraint
        if (new_size < JSTRING_MIN_ALLOC_SIZE)
        {
            new_size = JSTRING_MIN_ALLOC_SIZE;
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
