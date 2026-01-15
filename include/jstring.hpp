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
/// which indicates how many bytes can be written using unsafe methods after a safe check.
///
/// @par Required Interface Methods:
/// - static constexpr uint8_t kUnsafeLevel: Number of bytes that can be written unsafely
/// - unsafe_push_back(char): Add character without boundary check
/// - unsafe_append(const char*, size_t): Add string without boundary check
/// - unsafe_set_end(char*): Directly set the end pointer of string
/// - unsafe_resize(size): Directly set the size of string
/// - unsafe_end_cstr(): Add null terminator at end
/// - reserve_ex(size_t extra): Check if buffer has space to write extra bytes, return true if available
///
/// @par Unsafe Level Definition:
/// kUnsafeLevel specifies how many additional bytes can be written using unsafe methods
/// (like unsafe_push_back) after calling reserve_ex(). This provides a safety margin
/// for common JSON patterns that need multiple consecutive character writes.
/// Example: kUnsafeLevel=4 means after reserve_ex(100), you can safely write 104 bytes
/// (100 + 4) using unsafe operations before another check is needed.
/// The null terminator space is always additional (+1 byte).
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
/// Avoid performing write operations using both the buffer view and the memory 
/// management object that it borrows from at the same time.
class BufferView : public UnsafeStringConcept
{
protected:
    char* m_begin = nullptr;     ///< Start of buffer memory
    char* m_end = nullptr;       ///< Current end of string content
    char* m_cap_end = nullptr;   ///< End of allocated capacity

public:
    static constexpr uint8_t kUnsafeLevel = 0;

    /// @{ M0: Constructors and assignment operators

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
    explicit BufferView(char (&dst)[N]) : BufferView(dst, N)
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
    /// Does NOT update string's size. After writing, do NOT call dst.resize()
    /// as it will overwrite BufferView's data with '\0'.
    explicit BufferView(std::string& dst) : BufferView(dst.data(), dst.capacity())
    {
        resize(dst.size());
    }

    /// @brief Constructor from std::vector<char>
    /// @param dst Reference to std::vector<char> (must have reserved capacity)
    /// @warning
    /// Does NOT update vector's size. After writing, do NOT call dst.resize()
    /// as it will overwrite BufferView's data with '\0'.
    explicit BufferView(std::vector<char>& dst) : BufferView(dst.data(), dst.capacity())
    {
        resize(dst.size());
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

    /// @}
    /* ---------------------------------------------------------------------- */
    /// @{ M1: Capacity and size queries

    /// Read-only access methods
    size_t size() const { return m_end - m_begin; }
    size_t capacity() const { return m_cap_end - m_begin; }
    bool empty() const { return m_end == m_begin; }

    /// Check if buffer is exactly full (no remaining space)
    bool full() const { return m_end == m_cap_end; }

    /// Check if write operations have exceeded buffer capacity
    bool overflow() const { return m_end > m_cap_end; }

    /// @brief Check remaining available bytes in buffer
    /// @return
    ///   - Positive value: bytes available from m_end to m_cap_end
    ///   - Zero: buffer is exactly full (m_end == m_cap_end)
    ///   - Negative value: overflow detected (m_end > m_cap_end, value = overflowed bytes)
    int64_t reserve_ex() const
    {
        return static_cast<int64_t>(m_cap_end - m_end);
    }

    /// @brief Check if buffer has space to write n bytes
    /// @param n Number of bytes to check space for
    /// @return true if remaining space >= n, false otherwise (including overflow)
    bool reserve_ex(size_t n) const
    {
        return reserve_ex() >= static_cast<int64_t>(n);
    }

    /// no-ops as not owned memory.
    void reserve(size_t) const {}

    /// Set the end pointer with bounds checking.
    void set_end(char* new_end)
    {
        if (new_end < m_begin || new_end > m_cap_end) { return; }
        unsafe_set_end(new_end);
    }

    /// Safe resize with bounds checking.
    void resize(size_t new_size)
    {
        if (m_begin + new_size > m_cap_end) { return; }
        unsafe_resize(new_size);
    }

    /// Clear content, reset end pointer to the begin.
    void clear() { unsafe_resize(0); }

    /// @}
    /* ---------------------------------------------------------------------- */
    /// @{ M2: Edge pointer and element access

    explicit operator bool() const { return m_begin != nullptr; }
    const char* data() const { return m_begin; }

    char* begin() { return m_begin; }
    const char* begin() const { return m_begin; }
    char* end() { return m_end; }
    const char* end() const { return m_end; }
    char* cap_end() { return m_cap_end; }
    const char* cap_end() const { return m_cap_end; }

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

    /// Remove the last character from the string if not empty.
    void pop_back()
    {
        if (m_end > m_begin)
        {
            --m_end;
        }
    }

    /// @}
    /* ---------------------------------------------------------------------- */
    /// @{ M3: String conversion

    /// Get C-style string (adds null terminator if space available)
    const char* c_str()
    {
        if (m_begin == nullptr) { return ""; }
        end_cstr();
        return m_begin;
    }

    /// brief Convert to std::string
    std::string str() const
    {
        return std::string(m_begin, size());
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

    /// @}
    /* ---------------------------------------------------------------------- */
    /// @{ M4: Safe write operations (with boundary checks)

    /// Add null terminator with bounds checking
    void end_cstr()
    {
        if (m_end > m_cap_end) { return; }
        unsafe_end_cstr();
    }

    /// Fill remaining space with character ch (like memset) but doesn't change size.
    void fill(char ch)
    {
        assert(m_begin != nullptr && "BufferView::fill() called on null buffer");
        if (full()) return;
        size_t available = m_cap_end - m_end;
        ::memset(m_end, ch, available);
    }

    /// @brief Fill count characters with ch (safely truncated at cap_end)
    /// @param ch Character to fill
    /// @param count Number of characters to fill
    /// @details Fills count characters starting from m_end, moves end pointer.
    /// No capacity expansion - count is safely bounded by available capacity.
    void fill(char ch, size_t count)
    {
        assert(m_begin != nullptr && "BufferView::fill() called on null buffer");
        if (full()) return;
        size_t available = m_cap_end - m_end;
        if (count > available) { count = available; }
        ::memset(m_end, ch, count);
        m_end += count;
    }

    /// @brief Push back a charactor with bounds checking
    /// @param c Character to append
    /// @details Checks bounds, doesn't write if at capacity.
    /// m_end can advance to m_cap_end, but not beyond.
    /// m_cap_end position is reserved for '\0' terminator.
    void push_back(char c)
    {
        if (m_end + 1 > m_cap_end) { return; }
        unsafe_push_back(c);
    }

    /// Append count copies of character ch with safe bounds checking
    void append(size_t count, char ch)
    {
        if (m_end + count > m_cap_end) { return; }
        unsafe_fill(ch, count);
    }

    /// Append sting `str` of `len` lenth with bounds checking
    void append(const char* str, size_t len)
    {
        if (m_end + len > m_cap_end) { return; }
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

    void append(const BufferView& other)
    {
        append(other.data(), other.size());
    }

    /// @}
    /* ---------------------------------------------------------------------- */
    /// @{ M5: Unsafe write operations (no boundary checks)

    void unsafe_end_cstr()
    {
        assert(m_begin != nullptr && "BufferView::unsafe_end_cstr() called on null buffer");
        *m_end = '\0';
    }

    void unsafe_fill(char ch, size_t count)
    {
        assert(m_begin != nullptr && "BufferView::unsafe_fill() called on null buffer");
        ::memset(m_end, ch, count);
        m_end += count;
    }

    void unsafe_push_back(char c)
    {
        assert(m_begin != nullptr && "BufferView::unsafe_push_back() called on null buffer");
        *m_end++ = c;
    }

    void unsafe_append(const char* str, size_t len)
    {
        assert(m_begin != nullptr && "BufferView::unsafe_append() called on null buffer");
        ::memcpy(m_end, str, len);
        m_end += len;
    }

    void unsafe_resize(size_t new_size)
    {
        assert(m_begin != nullptr && "BufferView::unsafe_resize() called on null buffer");
        m_end = m_begin + new_size;
    }

    void unsafe_set_end(char* new_end)
    {
        assert(m_begin != nullptr && "BufferView::unsafe_set_end() called on null buffer");
        m_end = new_end;
    }

    /// @}
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

    /// Inherit all constructors and reserve_ex() from BufferView.
    using BufferView::BufferView;
    using BufferView::reserve_ex;

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

    /// Assume buffer has enough space in unsafe mode and return true.
    bool reserve_ex(size_t n) const
    {
        (void)n;  // Suppress unused parameter warning
        return true;
    }
};

/// @brief High-performance string buffer with unsafe operations
/// @tparam kUnsafeLevel Number of additional bytes that can be written unsafely after a safe check
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
/// - Growth strategy: Exponential (2x) until 8MB, then linear +8MB
/// - Alignment: Memory is aligned to improve efficiency
/// - Capacity invariant: capacity() = m_cap_end - m_begin
/// - Always allocates at least capacity() + 1 bytes for null terminator
/// - m_cap_end is within allocated memory, position reserved for potential null terminator
/// - The byte at m_cap_end is written with '\0' on allocation for safety
///
/// @par kUnsafeLevel Semantics:
/// kUnsafeLevel specifies how many additional bytes can be written using unsafe methods
/// (like unsafe_push_back) after calling reserve_ex(). This provides a safety margin
/// for common JSON patterns that need multiple consecutive character writes.
/// Example: For kUnsafeLevel=4, after reserve_ex(100):
///   - capacity() returns >=104 (100 user + 4 margin)
///   - Actual allocation: aligned to >= 105 bytes (104 + 1 null terminator)
///   - You can write 104 bytes using unsafe operations safely
///   - The null terminator space is always additional (+1 byte)
/// This definition allows kUnsafeLevel=0 to represent behavior similar to std::string.
///
/// @par Maximum Level (0xFF / 255):
/// When LEVEL equals 255 (0xFF), the buffer operates in almost single-allocation mode:
///   - push_back/append will not trigger automatic reallocation (no boundary check overhead)
///   - reserve_ex() returns true and allows explicit capacity expansion
///   - reserve() is allowed for explicit capacity expansion (use sparingly)
///   - User should provide sufficient initial capacity in constructor
///   - Memory is owned (unlike UnsafeBuffer which borrows external memory)
/// Use this mode when you know the approximate buffer size upfront and want to avoid
/// automatic reallocation overhead. Explicit reserve() calls are still supported for
/// flexibility when needed. See KString type alias for convenience.
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

    using BufferView::reserve_ex;

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

    bool reserve_ex(size_t add_capacity)
    {
        try
        {
            reserve(size() + add_capacity);
            return true;
        }
        catch (const std::bad_alloc&)
        {
            return false;
        }
    }

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
        if (str == nullptr) { return; }
        append(str, ::strlen(str));
    }

    void append(const char* str, size_t len)
    {
        if constexpr (LEVEL < 0xFF)
        {
            reserve_ex(len);
        }
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

    void append(const BufferView& other)
    {
        append(other.data(), other.size());
    }

    void push_back(char c)
    {
        if constexpr (LEVEL < 0xFF)
        {
            reserve_ex(1);
        }
        unsafe_push_back(c);
    }

    void append(size_t count, char ch)
    {
        if constexpr (LEVEL < 0xFF)
        {
            reserve_ex(count);
        }
        unsafe_fill(ch, count);
    }

    void resize(size_t new_size)
    {
        if constexpr (LEVEL < 0xFF)
        {
            reserve(new_size);
        }
        unsafe_resize(new_size);
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
        if (m_begin == nullptr)
        {
            throw std::bad_alloc();
        }
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

        // Use realloc to attempt in-place expansion, avoiding memcpy when possible.
        // realloc behavior:
        //   - Returns original ptr if in-place expansion succeeds
        //   - Returns new ptr with data copied if relocation needed
        //   - Returns nullptr on failure (original pointer remains valid)
        char* new_begin = static_cast<char*>(std::realloc(m_begin, alloc_size));

        if (new_begin == nullptr)
        {
            throw std::bad_alloc();
        }

        // Update pointers - handle both in-place and relocated cases
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

/// @brief Maximum unsafe level string buffer without automatically allocation
/// @details StringBuffer with kUnsafeLevel=255, optimized for scenarios where
/// it is known the maximum buffer size upfront and want to avoid boundary
/// check overhead.
using KString = StringBuffer<255>;

} // namespace wwjson

#endif // JSTRING_HPP__
