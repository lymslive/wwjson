/**
 * @file struct_to_json.cpp
 * @brief Example: Convert nested structures to JSON using wwjson::to_json API
 *
 * This example demonstrates the simplified struct-to-JSON serialization using
 * the wwjson::to_json helper functions and TO_JSON macro. Two approaches are shown:
 * 1. Using wwjson::to_json(builder, key, value) - explicit API calls
 * 2. Using TO_JSON(field) macro - concise field serialization
 *
 * @note Struct's to_json methods should NOT include BeginObject/EndObject -
 * the jbuilder.hpp helper functions handle that automatically.
 */

#include <wwjson/jbuilder.hpp>

#include <iostream>
#include <string>
#include <vector>

// Helper alias for convenience
using Builder = wwjson::Builder;

// ============================================================================
// Approach 1: Using wwjson::to_json() explicit API
// ============================================================================

// Address structure (nested in Publisher)
struct Address1 {
    std::string street;
    std::string city;
    std::string country;

    // to_json method - note: no BeginObject/EndObject (handled by helper)
    void to_json(Builder& builder) const {
        wwjson::to_json(builder, "street", street);
        wwjson::to_json(builder, "city", city);
        wwjson::to_json(builder, "country", country);
    }
};

// Publisher structure (contains nested Address)
struct Publisher1 {
    std::string name;
    Address1 address;

    void to_json(Builder& builder) const {
        wwjson::to_json(builder, "name", name);
        wwjson::to_json(builder, "address", address);  // Nested struct via to_json
    }
};

// Author structure (contains nested Publisher)
struct Author1 {
    std::string name;
    std::string email;
    Publisher1 publisher;

    void to_json(Builder& builder) const {
        wwjson::to_json(builder, "name", name);
        wwjson::to_json(builder, "email", email);
        wwjson::to_json(builder, "publisher", publisher);  // Nested struct
    }
};

// Book structure (top-level, contains vector of Authors)
struct Book1 {
    std::string title;
    std::string isbn;
    double price;
    std::vector<Author1> authors;
    int publishedYear;
    int pageCount;
    std::string genre;

    // to_json method - note: no BeginObject/EndObject
    void to_json(Builder& builder) const {
        wwjson::to_json(builder, "title", title);
        wwjson::to_json(builder, "isbn", isbn);
        wwjson::to_json(builder, "price", price);
        wwjson::to_json(builder, "publishedYear", publishedYear);
        wwjson::to_json(builder, "pageCount", pageCount);
        wwjson::to_json(builder, "genre", genre);
        wwjson::to_json(builder, "authors", authors);  // Vector via to_json
    }

    // Single-parameter overload: convert to JSON string directly
    std::string to_json() const {
        return wwjson::to_json(*this);
    }
};

// ============================================================================
// Approach 2: Using TO_JSON macro
// ============================================================================

// Address structure for Approach 2
struct Address2 {
    std::string street;
    std::string city;
    std::string country;

    void to_json(Builder& builder) const {
        TO_JSON(street);
        TO_JSON(city);
        TO_JSON(country);
    }
};

// Publisher structure for Approach 2
struct Publisher2 {
    std::string name;
    Address2 address;

    void to_json(Builder& builder) const {
        TO_JSON(name);
        TO_JSON(address);  // TO_JSON works with nested structs too
    }
};

// Author structure for Approach 2
struct Author2 {
    std::string name;
    std::string email;
    Publisher2 publisher;

    void to_json(Builder& builder) const {
        TO_JSON(name);
        TO_JSON(email);
        TO_JSON(publisher);
    }
};

// Book structure for Approach 2
struct Book2 {
    std::string title;
    std::string isbn;
    double price;
    std::vector<Author2> authors;
    int publishedYear;
    int pageCount;
    std::string genre;

    void to_json(Builder& builder) const {
        TO_JSON(title);
        TO_JSON(isbn);
        TO_JSON(price);
        TO_JSON(publishedYear);
        TO_JSON(pageCount);
        TO_JSON(genre);
        TO_JSON(authors);  // TO_JSON works with containers too
    }

    // The top to_json may not needed.
    // Directly use wwjson::to_json is also simple.
//  std::string to_json() const {
//      return wwjson::to_json(*this);
//  }
};

// ============================================================================
// Helper functions to create sample data
// ============================================================================

Author1 createAuthor1(const std::string& name, const std::string& email,
                      const Publisher1& publisher) {
    Author1 author;
    author.name = name;
    author.email = email;
    author.publisher = publisher;
    return author;
}

Book1 createBook1() {
    Publisher1 publisher;
    publisher.name = "Addison-Wesley Professional";
    publisher.address.street = "75 Arlington Street";
    publisher.address.city = "Boston";
    publisher.address.country = "USA";

    Book1 book;
    book.title = "Modern C++ Design";
    book.isbn = "978-0-201-70431-0";
    book.price = 49.99;
    book.publishedYear = 2001;
    book.pageCount = 352;
    book.genre = "Programming";
    book.authors.push_back(createAuthor1("Andrei Alexandrescu", "andrei@example.com", publisher));
    book.authors.push_back(createAuthor1("Scott Meyers", "scott@example.com", publisher));
    return book;
}

Author2 createAuthor2(const std::string& name, const std::string& email,
                      const Publisher2& publisher) {
    Author2 author;
    author.name = name;
    author.email = email;
    author.publisher = publisher;
    return author;
}

Book2 createBook2() {
    Publisher2 publisher;
    publisher.name = "Addison-Wesley Professional";
    publisher.address.street = "75 Arlington Street";
    publisher.address.city = "Boston";
    publisher.address.country = "USA";

    Book2 book;
    book.title = "Effective C++";
    book.isbn = "978-0-32-133487-9";
    book.price = 54.99;
    book.publishedYear = 2005;
    book.pageCount = 320;
    book.genre = "Programming";
    book.authors.push_back(createAuthor2("Scott Meyers", "scott@example.com", publisher));
    return book;
}

// ============================================================================
// Main function
// ============================================================================

int main() {
    std::cout << "Example: Struct to JSON with wwjson::to_json API\n";
    std::cout << "================================================\n\n";

    // Approach 1: Using wwjson::to_json() explicit API
    std::cout << "Approach 1: Using wwjson::to_json() explicit API\n";
    std::cout << "-----------------------------------------------\n";
    Book1 book1 = createBook1();
    std::string json1 = book1.to_json();
    std::cout << "Book Information:\n";
    std::cout << "  Title: " << book1.title << "\n";
    std::cout << "  Authors: " << book1.authors.size() << "\n\n";
    std::cout << "Generated JSON:\n" << json1 << "\n\n";
    std::cout << "JSON Length: " << json1.length() << " characters\n\n";

    // Approach 2: Using TO_JSON macro
    std::cout << "Approach 2: Using TO_JSON macro\n";
    std::cout << "--------------------------------\n";
    Book2 book2 = createBook2();
    std::string json2 = wwjson::to_json(book2);
    std::cout << "Book Information:\n";
    std::cout << "  Title: " << book2.title << "\n";
    std::cout << "  Authors: " << book2.authors.size() << "\n\n";
    std::cout << "Generated JSON:\n" << json2 << "\n\n";
    std::cout << "JSON Length: " << json2.length() << " characters\n";

    return 0;
}
