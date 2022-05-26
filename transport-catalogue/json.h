#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final
    : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>
{
public:
    // Делаем доступными все конструкторы родительского класса variant
    using variant::variant;

    struct OstreamValuePrinter {
        std::ostream& out;
        void operator()(std::nullptr_t) const;
        void operator()(Array array) const;
        void operator()(Dict map) const;
        void operator()(std::string value) const;

        template <typename Number>
        void operator() (Number value) const { out << std::boolalpha << value; }
    };

    const variant& GetValue() const { return *this; }

    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json

