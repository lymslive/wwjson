/**
 * @file custom_string.h
 * @brief Custom string class for testing wwjson library compatibility
 */

#pragma once
#ifndef CUSTOM_STRING_H__
#define CUSTOM_STRING_H__

#include <string>
#include <cstring>
#include <algorithm>

namespace test {

/// Custom string class that implements std::string interface
/// Compatible with wwjson::StringConcept requirements
class string
{
private:
    char* m_data;
    size_t m_size;
    size_t m_capacity;

    void grow_capacity(size_t new_capacity);
    void ensure_capacity(size_t needed);

public:
    // Constructors
    string();
    explicit string(size_t capacity);
    string(const string& other);
    string(string&& other) noexcept;
    ~string();

    // Assignment operators
    string& operator=(const string& other);
    string& operator=(string&& other) noexcept;

    // Capacity operations
    size_t size() const noexcept { return m_size; }
    size_t capacity() const noexcept { return m_capacity; }
    bool empty() const noexcept { return m_size == 0; }
    void reserve(size_t new_capacity);
    void clear() noexcept;

    // Element access
    char& front() { return m_data[0]; }
    const char& front() const { return m_data[0]; }
    char& back() { return m_data[m_size > 0 ? m_size - 1 : 0]; }
    const char& back() const { return m_data[m_size > 0 ? m_size - 1 : 0]; }
    const char* c_str() const noexcept { return m_data; }

    // Modifiers
    void push_back(char c);
    void pop_back();

    // Append operations
    void append(const char* str);
    void append(const char* str, size_t len);
    void append(const std::string& str);
    void append(const string& str);

    // Utility
    friend bool operator==(const string& lhs, const string& rhs);
    friend bool operator==(const string& lhs, const std::string& rhs);
    friend bool operator==(const std::string& lhs, const string& rhs);
};

} // namespace test

#endif // CUSTOM_STRING_H__