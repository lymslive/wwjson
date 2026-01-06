/**
 * @file struct_to_json.cpp
 * @brief Example: Convert nested structures to JSON
 *
 * This example demonstrates how to convert complex nested C++ structures
 * to JSON using wwjson library. Each struct has a toJson method that accepts
 * a RawBuilder reference and writes its members to the JSON string being built.
 * The top-level struct also provides an overload that returns a std::string.
 */

#include <wwjson/wwjson.hpp>

#include <iostream>
#include <string>
#include <vector>

// Helper alias for convenience
using RawBuilder = wwjson::RawBuilder;

// Address structure (nested in Publisher)
struct Address {
    std::string street;
    std::string city;
    std::string country;

    // Convert address to JSON using the provided builder
    void toJson(RawBuilder& builder) const {
        builder.BeginObject();
        builder.AddMember("street", street);
        builder.AddMember("city", city);
        builder.AddMember("country", country);
        builder.EndObject();
    }
};

// Publisher structure (contains nested Address)
struct Publisher {
    std::string name;
    Address address;

    void toJson(RawBuilder& builder) const {
        builder.BeginObject();
        builder.AddMember("name", name);
        builder.AddMember("address", [&builder, this](){
            address.toJson(builder);
        });
        builder.EndObject();
    }
};

// Author structure (contains nested Publisher)
struct Author {
    std::string name;
    std::string email;
    Publisher publisher;

    void toJson(RawBuilder& builder) const {
        builder.BeginObject();
        builder.AddMember("name", name);
        builder.AddMember("email", email);
        builder.AddMember("publisher", [&builder, this](){
            publisher.toJson(builder);
        });
        builder.EndObject();
    }
};

// Book structure (top-level, contains vector of Authors)
struct Book {
    std::string title;
    std::string isbn;
    double price;
    std::vector<Author> authors;
    int publishedYear;
    int pageCount;
    std::string genre;

    // Overload: Convert book to JSON using the provided builder
    void toJson(RawBuilder& builder) const {
        builder.BeginObject();

        builder.AddMember("title", title);
        builder.AddMember("isbn", isbn);
        builder.AddMember("price", price);
        builder.AddMember("publishedYear", publishedYear);
        builder.AddMember("pageCount", pageCount);
        builder.AddMember("genre", genre);

        // Add authors array - lambda creates scope with automatic []
        builder.AddMember("authors", [&builder, this](){
            builder.BeginArray();
            for (const auto& author : authors) {
                author.toJson(builder);
            }
            builder.EndArray();
        });

        builder.EndObject();
    }

    // Overload: Convert book to JSON string (no parameters, returns std::string)
    std::string toJson() const {
        RawBuilder builder;
        toJson(builder);
        return builder.MoveResult();
    }
};

int main() {
    std::cout << "Example 1: Multi-level Structure to JSON\n";
    std::cout << "==========================================\n\n";

    // Create a sample book with nested structures
    Book book;
    book.title = "Modern C++ Design";
    book.isbn = "978-0-201-70431-0";
    book.price = 49.99;
    book.publishedYear = 2001;
    book.pageCount = 352;
    book.genre = "Programming";

    // Create authors with nested publishers and addresses
    Author author1;
    author1.name = "Andrei Alexandrescu";
    author1.email = "andrei@example.com";

    Publisher publisher1;
    publisher1.name = "Addison-Wesley Professional";

    Address address1;
    address1.street = "75 Arlington Street";
    address1.city = "Boston";
    address1.country = "USA";

    publisher1.address = address1;
    author1.publisher = publisher1;

    Author author2;
    author2.name = "Scott Meyers";
    author2.email = "scott@example.com";

    // Share the same publisher for second author
    author2.publisher = publisher1;

    book.authors.push_back(author1);
    book.authors.push_back(author2);

    // Convert book to JSON string
    std::string json = book.toJson();

    std::cout << "Book Information:\n";
    std::cout << "  Title: " << book.title << "\n";
    std::cout << "  Authors: " << book.authors.size() << "\n\n";

    std::cout << "Generated JSON:\n";
    std::cout << json << "\n\n";

    std::cout << "JSON Length: " << json.length() << " characters\n";

    return 0;
}
