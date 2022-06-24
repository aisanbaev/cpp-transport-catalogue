#pragma once

#include "json.h"

#include <optional>

namespace json {

class ArrayItemContext;
class DictItemContext;
class ValueItemContext;
class ValueKeyItemContext;
class KeyItemContext;

class Builder {
public:
    Builder() = default;

    KeyItemContext Key(std::string key);
    DictItemContext StartDict();
    ArrayItemContext StartArray();

    Builder& Value(Node::Value node);
    Builder& EndArray();
    Builder& EndDict();

    Node Build();

private:
    Node root_;
    std::vector<Node*> nodes_stack_{&root_};
    std::optional<std::string> key_;

    void StartContainer(Node container);
};

class KeyItemContext : private Builder {
public:
    KeyItemContext(Builder& builder) : builder_(builder) { }
    ValueKeyItemContext Value(Node::Value node);
    ArrayItemContext StartArray();
    DictItemContext StartDict();

private:
    Builder& builder_;
};

class ValueKeyItemContext : private Builder {
public:
    ValueKeyItemContext(Builder& builder) : builder_(builder) { }
    KeyItemContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class ValueItemContext : private Builder {
public:
    ValueItemContext(Builder& builder) : builder_(builder) { }
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
    ValueItemContext Value(Node::Value node);

private:
    Builder& builder_;
};

class DictItemContext : private Builder {
public:
    DictItemContext(Builder& builder) : builder_(builder) { }
    KeyItemContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class ArrayItemContext : private Builder {
public:
    ArrayItemContext(Builder& builder) : builder_(builder) { }
    ValueItemContext Value(Node::Value node);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
private:
    Builder& builder_;
};

} // namespace json


