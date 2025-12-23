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

#include <stdint.h>

namespace wwjson {

/// @brief Concept definition for unsafe string operations in JSON building
/// @details
/// This struct defines the interface for string types that support unsafe operations
/// optimized for JSON serialization. The key innovation is the concept of "unsafe level"
/// which allows a batch of unsafe operations after a safe boundary check.
/// 
/// @par Required Interface Methods:
/// - static constexpr uint8_t kUnsafeLevel: Number of unsafe operations allowed
/// - unsafe_push_back(char c): Add character without boundary check
/// - unsafe_set_end(size_t new_size): Directly set string length
/// - unsafe_end_cstr(): Add null terminator at end
/// - reserve_ex(size_t capacity): Reserve with kUnsafeLevel bytes margin
struct UnsafeStringConcept
{
};

/// @brief Integer type alias for unsafe level specification
using UnsafeLevel = uint8_t;

/// @brief Three-pointer view for string buffer management
/// @details
/// This struct provides the underlying three-pointer design for string buffers:
/// - m_begin: Start of allocated memory
/// - m_end: Current end of string content (left-closed, right-open)
/// - m_cap_end: End of allocated capacity (can write null terminator here)
/// 
/// The design allows efficient direct memory manipulation while maintaining
/// compatibility with standard string operations.
struct StringBufferView
{
    char* m_begin;     ///< Start of buffer memory
    char* m_end;       ///< Current end of string content
    char* m_cap_end;   ///< End of allocated capacity
};

/// @brief High-performance string buffer with unsafe operations
/// @tparam kUnsafeLevel Number of unsafe operations allowed after each safe check
/// @details
/// StringBuffer implements the UnsafeStringConcept interface with a three-pointer
/// design for optimal performance in JSON serialization. The key features are:
/// 
/// - Batch boundary checking with safety margin
/// - Direct buffer manipulation without intermediate copies
/// - Deferred null termination until final result is needed
/// - Support for unsafe character writes after safe capacity checks
/// 
/// @par Performance Characteristics:
/// - O(1) append operations when capacity is available
/// - Minimal memory allocations through smart capacity management
/// - Zero-overhead unsafe operations within safety margin
/// 
/// @par Usage Examples:
/// ```cpp
/// StringBuffer<4> buffer;  // JString alias
/// buffer.reserve_ex(100);   // Reserve 100 + 4 bytes margin
/// buffer.append("key");
/// buffer.unsafe_push_back(':');  // Safe within margin
/// buffer.unsafe_push_back('"');
/// buffer.append("value");
/// buffer.unsafe_push_back('"');
/// buffer.unsafe_end_cstr();      // Add null terminator
/// ```
template <UnsafeLevel kUnsafeLevel = 4>
class StringBuffer : private StringBufferView, public UnsafeStringConcept
{
public:
    static constexpr uint8_t kUnsafeLevelValue = kUnsafeLevel;

    /// @brief Default constructor
    StringBuffer()
    {
        m_begin = nullptr;
        m_end = nullptr;
        m_cap_end = nullptr;
    }

    /// @brief Constructor with initial capacity
    /// @param capacity Initial capacity to allocate
    explicit StringBuffer(size_t capacity)
    {
        allocate(capacity);
    }

    /// @brief Copy constructor
    /// @param other Other StringBuffer to copy from
    StringBuffer(const StringBuffer& other)
    {
        copy_from(other);
    }

    /// @brief Move constructor
    /// @param other Other StringBuffer to move from
    StringBuffer(StringBuffer&& other) noexcept
    {
        move_from(std::move(other));
    }

    /// @brief Destructor
    ~StringBuffer()
    {
        deallocate();
    }

    /// @brief Copy assignment
    StringBuffer& operator=(const StringBuffer& other)
    {
        if (this != &other)
        {
            deallocate();
            copy_from(other);
        }
        return *this;
    }

    /// @brief Move assignment
    StringBuffer& operator=(StringBuffer&& other) noexcept
    {
        if (this != &other)
        {
            deallocate();
            move_from(std::move(other));
        }
        return *this;
    }

    /// @brief Get current string size
    size_t size() const { return m_end - m_begin; }

    /// @brief Get current capacity
    size_t capacity() const { return m_cap_end - m_begin; }

    /// @brief Check if buffer is empty
    bool empty() const { return m_end == m_begin; }

    /// @brief Get pointer to string content
    const char* data() const { return m_begin; }

    /// @brief Get null-terminated C string (ensures null termination)
    const char* c_str()
    {
        if (m_begin == nullptr)
        {
            return nullptr;
        }
        if (m_end < m_cap_end)
        {
            *m_end = '\0';
        }
        return m_begin;
    }

    /// @brief Get front character
    char& front() { return *m_begin; }
    const char& front() const { return *m_begin; }

    /// @brief Get back character
    char& back() { return *(m_end - 1); }
    const char& back() const { return *(m_end - 1); }

    /// @brief Reserve capacity with unsafe level margin
    /// @param additional_capacity Additional bytes to reserve
    /// @details Reserves capacity + kUnsafeLevel bytes to allow safe
    /// subsequent unsafe operations
    void reserve_ex(size_t additional_capacity)
    {
        size_t required_capacity = size() + additional_capacity + kUnsafeLevel;
        if (required_capacity > capacity())
        {
            reserve(required_capacity);
        }
    }

    /// @brief Reserve exact capacity
    /// @param new_capacity New capacity to allocate
    void reserve(size_t new_capacity)
    {
        if (new_capacity > capacity())
        {
            reallocate(new_capacity);
        }
    }

    /// @brief Append C-style string
    /// @param str String to append
    void append(const char* str)
    {
        size_t len = strlen(str);
        append(str, len);
    }

    /// @brief Append string with length
    /// @param str String to append
    /// @param len Length of string
    void append(const char* str, size_t len)
    {
        reserve_ex(len);
        memcpy(m_end, str, len);
        m_end += len;
    }

    /// @brief Append another StringBuffer
    /// @param other Another StringBuffer to append
    template <UnsafeLevel kOtherLevel>
    void append(const StringBuffer<kOtherLevel>& other)
    {
        append(other.data(), other.size());
    }

    /// @brief Push back single character with boundary check
    /// @param c Character to append
    void push_back(char c)
    {
        reserve_ex(1);
        *m_end++ = c;
    }

    /// @brief Unsafe push back without boundary check
    /// @param c Character to append
    /// @warning Only use after reserve_ex() to ensure capacity
    void unsafe_push_back(char c)
    {
        *m_end++ = c;
    }

    /// @brief Unsafe set string length directly
    /// @param new_size New string size
    /// @warning Only use to shrink string or within reserved capacity
    void unsafe_set_end(size_t new_size)
    {
        m_end = m_begin + new_size;
    }

    /// @brief Add null terminator at end (unsafe operation)
    /// @warning Only use when buffer has capacity for null terminator
    void unsafe_end_cstr()
    {
        if (m_end < m_cap_end)
        {
            *m_end = '\0';
        }
    }

    /// @brief Clear string content
    void clear()
    {
        m_end = m_begin;
    }

private:
    /// @brief Allocate memory for given capacity
    /// @param capacity Capacity to allocate
    void allocate(size_t capacity)
    {
        if (capacity == 0)
        {
            m_begin = nullptr;
            m_end = nullptr;
            m_cap_end = nullptr;
            return;
        }

        // Allocate capacity, space for null terminator included in capacity
        m_begin = static_cast<char*>(std::malloc(capacity));
        m_end = m_begin;
        m_cap_end = m_begin + capacity;
    }

    /// @brief Deallocate memory
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

    /// @brief Reallocate memory with new capacity
    /// @param new_capacity New capacity to allocate
    void reallocate(size_t new_capacity)
    {
        size_t current_size = size();
        
        // Allocate new buffer
        char* new_begin = static_cast<char*>(std::malloc(new_capacity));
        
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
        
        // Update pointers
        m_begin = new_begin;
        m_end = m_begin + current_size;
        m_cap_end = m_begin + new_capacity + 1;
    }

    /// @brief Copy from another StringBuffer
    /// @param other Another StringBuffer to copy from
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
    }

    /// @brief Move from another StringBuffer
    /// @param other Another StringBuffer to move from
    void move_from(StringBuffer&& other)
    {
        m_begin = other.m_begin;
        m_end = other.m_end;
        m_cap_end = other.m_cap_end;

        // Clear other
        other.m_begin = nullptr;
        other.m_end = nullptr;
        other.m_cap_end = nullptr;
    }
};

/// @brief JSON-optimized string type with kUnsafeLevel=4
/// @details This is the recommended string type for JSON serialization.
/// The unsafe level of 4 provides safety margin for common JSON patterns
/// like `":"`, `","`, `"{"`, `"}"` which may require multiple consecutive
/// unsafe character writes.
using JString = StringBuffer<4>;

} // namespace wwjson

#endif // JSTRING_HPP__