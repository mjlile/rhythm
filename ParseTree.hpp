#pragma once
#include <iostream>
#include <memory>
#include <vector>

struct ParseTree {
    enum class Type {
        // node_type, // children
        Block, // stmts

        Expression, // expr
        Declaration, // type, identifier, [initialization]
        Assignment, // identifier, value
        Conditional, // condition, block
        While, // condition, block
        Return, // [expression]

        Group, // expr
        Token, // no children, token
        Literal, // no children, value
        Identifier, // no children, value
        Type // no children, value
    };

    ParseTree(Type type) : type(type), token(-1) {}
    ParseTree(Type type, const std::string& value)
        : type(type), value(value), token(-1) {}
    ParseTree(int token)
        : type(Type::Token), token(token) {}
    
    void add_child(std::unique_ptr<ParseTree> child) {
        children.push_back(std::move(child));
    }

    // takes ownership of dynamically allocated child raw ptr
    void adopt_child(ParseTree* child) {
        add_child(std::unique_ptr<ParseTree>(child));
    }

    void make_child(Type type, const std::string& val) {
        children.push_back(std::make_unique<ParseTree>(type, val));
    }
    void make_child(int token) {
        children.push_back(std::make_unique<ParseTree>(token));
    }


    Type get_type() {
        return type;
    }

    const std::string& get_value() {
        return value;
    }

    friend std::ostream& operator<<(std::ostream& os, const ParseTree& pt);

private:
    Type type;
    // terminals (leaf nodes) have either value or token;
    std::string value;
    int token;
    std::vector<std::unique_ptr<ParseTree>> children;    
};
