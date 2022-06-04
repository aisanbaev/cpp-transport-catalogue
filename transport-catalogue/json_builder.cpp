#include "json_builder.h"

namespace json {
using namespace std::literals;

    KeyItemContext Builder::Key(std::string key) {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Dictionary not declarated"s);
        }
        key_ = key;

        return *this;
    }

    Builder& Builder::Value(Node::Value node) {
        if (root_.IsNull()) {
            root_.SetValue() = std::move(node);
            nodes_stack_.pop_back();

        } else if (nodes_stack_.back()->IsArray()) {
            Node tmp;
            tmp.SetValue() = std::move(node);
            std::get<Array>(nodes_stack_.back()->SetValue()).push_back(std::move(tmp));

        } else if (nodes_stack_.back()->IsDict() && key_) {
            Node tmp;
            tmp.SetValue() = std::move(node);
            std::get<Dict>(nodes_stack_.back()->SetValue())[*key_] = std::move(tmp);
            key_.reset();

        } else {
            throw std::logic_error("Node initialization error"s);
        }

        return *this;
    }

    DictItemContext Builder::StartDict() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Dictionary creating error"s);
        }

        if (root_.IsNull()) {
            root_ = std::move(Node(Dict{}));

        } else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.push_back(&std::get<Array>(nodes_stack_.back()->SetValue()).emplace_back(Node(Dict{})));

        } else if (nodes_stack_.back()->IsDict() && key_) {
            auto [it, b] = std::get<Dict>(nodes_stack_.back()->SetValue()).emplace(*key_, Node(Dict{}));
            nodes_stack_.push_back(&it->second);
            key_.reset();
        }

        return *this;
    }

    ArrayItemContext Builder::StartArray() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Array creating error"s);
        }

        if (root_.IsNull()) {
            root_ = std::move(Node(Array{}));

        } else if (nodes_stack_.back()->IsDict() && key_) {
            auto [it, b] = std::get<Dict>(nodes_stack_.back()->SetValue()).emplace(*key_, Node(Array{}));
            nodes_stack_.push_back(&it->second);
            key_.reset();

        } else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.push_back(&std::get<Array>(nodes_stack_.back()->SetValue()).emplace_back(Node(Array{})));

        }

        return *this;
    }

    Builder& Builder::EndArray() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Array not created"s);
        }
        nodes_stack_.pop_back();

        return *this;
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Dictionary not created"s);
        }
        nodes_stack_.pop_back();

        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Node is not completed");
        }

        return root_;
    }

    ValueKeyItemContext KeyItemContext::Value(Node::Value node) {
        return builder_.Value(node);
    }
    ArrayItemContext KeyItemContext::StartArray() {
        return builder_.StartArray();
    }
    DictItemContext KeyItemContext::StartDict() {
        return builder_.StartDict();
    }

    KeyItemContext ValueKeyItemContext::Key(std::string key) {
        return builder_.Key(key);
    }
    Builder& ValueKeyItemContext::EndDict() {
        return builder_.EndDict();
    }

    DictItemContext ValueItemContext::StartDict() {
        return builder_.StartDict();
    }
    ArrayItemContext ValueItemContext::StartArray() {
        return builder_.StartArray();
    }
    Builder& ValueItemContext::EndArray() {
        return builder_.EndArray();
    }
    ValueItemContext ValueItemContext::Value(Node::Value node) {
        return builder_.Value(node);
    }

    KeyItemContext DictItemContext::Key(std::string key) {
        return builder_.Key(key);
    }
    Builder& DictItemContext::EndDict() {
        return builder_.EndDict();
    }

    ValueItemContext ArrayItemContext::Value(Node::Value node) {
        return builder_.Value(node);
    }
    DictItemContext ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }
    ArrayItemContext ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }
    Builder& ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }

} // namespace json
