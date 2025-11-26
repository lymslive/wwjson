/**
 * @file custom_string.cpp
 * @brief Custom string class implementation
 */

#include "custom_string.h"
#include <algorithm>

namespace test {

string::string()
    : m_data(nullptr), m_size(0), m_capacity(0)
{
    m_data = new char[1];
    m_data[0] = '\0';
    m_capacity = 1;
}

string::string(size_t capacity)
    : m_data(nullptr), m_size(0), m_capacity(0)
{
    if (capacity < 1) capacity = 1;
    m_data = new char[capacity + 1];
    m_data[0] = '\0';
    m_capacity = capacity + 1;
}

string::string(const string& other)
    : m_data(nullptr), m_size(0), m_capacity(0)
{
    m_data = new char[other.m_capacity];
    std::copy(other.m_data, other.m_data + other.m_size + 1, m_data);
    m_size = other.m_size;
    m_capacity = other.m_capacity;
}

string::string(string&& other) noexcept
    : m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity)
{
    other.m_data = new char[1];
    other.m_data[0] = '\0';
    other.m_size = 0;
    other.m_capacity = 1;
}

string::~string()
{
    delete[] m_data;
}

string& string::operator=(const string& other)
{
    if (this != &other) {
        delete[] m_data;
        m_data = new char[other.m_capacity];
        std::copy(other.m_data, other.m_data + other.m_size + 1, m_data);
        m_size = other.m_size;
        m_capacity = other.m_capacity;
    }
    return *this;
}

string& string::operator=(string&& other) noexcept
{
    if (this != &other) {
        delete[] m_data;
        m_data = other.m_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        
        other.m_data = new char[1];
        other.m_data[0] = '\0';
        other.m_size = 0;
        other.m_capacity = 1;
    }
    return *this;
}

void string::grow_capacity(size_t new_capacity)
{
    char* new_data = new char[new_capacity + 1];
    std::copy(m_data, m_data + m_size, new_data);
    new_data[m_size] = '\0';
    delete[] m_data;
    m_data = new_data;
    m_capacity = new_capacity + 1;
}

void string::ensure_capacity(size_t needed)
{
    if (needed >= m_capacity) {
        size_t new_capacity = std::max(m_capacity * 2, needed + 1);
        grow_capacity(new_capacity - 1);
    }
}

void string::reserve(size_t new_capacity)
{
    if (new_capacity + 1 > m_capacity) {
        grow_capacity(new_capacity);
    }
}

void string::clear() noexcept
{
    m_data[0] = '\0';
    m_size = 0;
}

void string::push_back(char c)
{
    ensure_capacity(m_size + 1);
    m_data[m_size] = c;
    m_size++;
    m_data[m_size] = '\0';
}

void string::pop_back()
{
    if (m_size > 0) {
        m_size--;
        m_data[m_size] = '\0';
    }
}

void string::append(const char* str)
{
    if (!str) return;
    
    size_t len = std::strlen(str);
    append(str, len);
}

void string::append(const char* str, size_t len)
{
    if (!str || len == 0) return;
    
    ensure_capacity(m_size + len);
    std::copy(str, str + len, m_data + m_size);
    m_size += len;
    m_data[m_size] = '\0';
}

void string::append(const std::string& str)
{
    append(str.c_str(), str.size());
}

void string::append(const string& str)
{
    append(str.c_str(), str.size());
}

bool operator==(const string& lhs, const string& rhs)
{
    return lhs.size() == rhs.size() && 
           std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
}

bool operator==(const string& lhs, const std::string& rhs)
{
    return lhs.size() == rhs.size() && 
           std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
}

bool operator==(const std::string& lhs, const string& rhs)
{
    return lhs.size() == rhs.size() && 
           std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
}

} // namespace test