#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    char c;

    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (c != ']') {
        throw ParsingError("Invalid array");
    }

    return Node(move(result));
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(s);
}

Node LoadDict(istream& input) {
    Dict result;
    char c;

    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if (c != '}') {
        throw ParsingError("Invalid map");
    }

    return Node(move(result));
}

Node LoadBool(istream& input, char c) {
    string s;

    switch (c) {
        case 'n':
            s = "ull";
            for (int i = 0; i < 3; ++i) {
                if (input.peek() != s[i]) {
                    throw ParsingError("Invalid stream");
                }
                input.get();
            }
            return Node();

        case 'f':
            s = "alse";
            for (int i = 0; i < 4; ++i) {
                if (input.peek() != s[i]) {
                    throw ParsingError("Invalid stream");
                }
                input.get();
            }
            return Node(false);

        case 't':
            s = "rue";
            for (int i = 0; i < 3; ++i) {
                if (input.peek() != s[i]) {
                    throw ParsingError("Invalid stream");
                }
                input.get();
            }
            return Node(true);
    }

    return Node();
}

Node LoadNode(istream& input) {
    char c;
    input >> c;
    if ((c == ']') || (c == '}')) {
        throw ParsingError("Unexpected symbol: " + string(c, 1));
    }
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 'n' || c == 'f' || c == 't') {
        return LoadBool(input, c);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

void Node::OstreamValuePrinter::operator()(std::nullptr_t) const {
    out << "null"sv;
}

void Node::OstreamValuePrinter::operator()(Array array) const {
    bool is_first = false;
    out << "["sv;
    for (const auto& elem : array) {
        if (is_first) {
            out << ","sv;
        } else {
            is_first = true;
        }
        std::visit(Node::OstreamValuePrinter{out}, elem.GetValue());
    }
    out << "]";
}

void Node::OstreamValuePrinter::operator()(Dict map) const {
    bool is_first = false;
    out << "{";
    for (const auto& [key, value] : map) {
        if (is_first) {
            out << ","sv;
        } else {
            is_first = true;
        }
        out << "\""sv << key << "\": "sv;
        std::visit(Node::OstreamValuePrinter{out}, value.GetValue());
    }
    out << "}"sv;
}

void Node::OstreamValuePrinter::operator()(std::string value) const {
    out << "\"";
    for (char c : value) {
        switch (c) {
            case '\n':
                out << "\\n"sv;
                break;
            case '\"':
                out << "\\\""sv;
                break;
            case '\r':
                out << "\\r"sv;
                break;
            case '\\':
                out << "\\\\"sv;
                break;
            default:
                out << c;
        }
    }
    out << "\"";
}

Node::Node(nullptr_t) {
}

Node::Node(Array array)
    : value_(move(array)) {
}

Node::Node(Dict map)
    : value_(move(map)) {
}

Node::Node(string value)
    : value_(move(value)) {
}

bool Node::operator==(const Node& other) const {
    return GetValue() == other.GetValue();
}

bool Node::operator!=(const Node& other) const {
    return GetValue() != other.GetValue();
}

bool Node::IsInt() const {
    if (holds_alternative<int>(GetValue())) {
        return true;
    }
    return false;
}

bool Node::IsDouble() const {
    if (holds_alternative<int>(GetValue()) || holds_alternative<double>(GetValue())) {
        return true;
    }
    return false;
}

bool Node::IsPureDouble() const {
    if (holds_alternative<double>(GetValue())) {
        return true;
    }
    return false;
}

bool Node::IsBool() const {
    if (holds_alternative<bool>(GetValue())) {
        return true;
    }
    return false;
}

bool Node::IsString() const {
    if (holds_alternative<string>(GetValue())) {
        return true;
    }
    return false;
}

bool Node::IsNull() const {
    if (holds_alternative<nullptr_t>(GetValue())) {
        return true;
    }
    return false;
}

bool Node::IsArray() const {
    if (holds_alternative<Array>(GetValue())) {
        return true;
    }
    return false;
}

bool Node::IsMap() const {
    if (holds_alternative<Dict>(GetValue())) {
        return true;
    }
    return false;
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw logic_error("Invalid type"s);
    }
    return get<int>(value_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw logic_error("Invalid type"s);
    }
    return get<bool>(value_);
}

double Node::AsDouble() const {
    if (!IsDouble()) {
        throw logic_error("Invalid type"s);
    }
    if (IsPureDouble()) {
        return get<double>(value_);
    }
    return get<int>(value_) * 1.0;
}

const string& Node::AsString() const {
    if (!IsString()) {
        throw logic_error("Invalid type"s);
    }
    return get<string>(value_);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw logic_error("Invalid type"s);
    }
    return get<Array>(value_);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw logic_error("Invalid type"s);
    }
    return get<Dict>(value_);
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document& other) const {
    return GetRoot() == other.GetRoot();
}

bool Document::operator!=(const Document& other) const {
    return GetRoot() != other.GetRoot();
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    std::visit(Node::OstreamValuePrinter{output}, doc.GetRoot().GetValue());
}

}  // namespace json
