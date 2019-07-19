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
        Import, // type
        Procedure, // ident, (return) type, block (params at start of block)
        DeclList, // declaration+

        Group, // expr
        Operator, // operand+
        Literal, // no children
        Identifier, // no children
        Type // no children
    };

    ParseTree(Type type) : type(type), token(-1) {}
    ParseTree(Type type, const std::string& value)
        : type(type), value(value), token(-1) {}
    ParseTree(int token)
        : type(Type::Operator), token(token) {}
    
    void add_child(std::unique_ptr<ParseTree> child) {
        children.push_back(std::move(child));
    }

    // takes ownership of dynamically allocated child raw ptr without existing parent
    void adopt_orphan(ParseTree* child) {
        add_child(std::unique_ptr<ParseTree>(child));
    }

    // adopt orphan and insert at front of children
    void adopt_orphan_front(ParseTree* child) {
        children.insert(children.begin(), std::unique_ptr<ParseTree>(child));
    }

    void make_child(Type type, const std::string& val) {
        children.push_back(std::make_unique<ParseTree>(type, val));
    }
    void make_child(int token) {
        children.push_back(std::make_unique<ParseTree>(token));
    }


    Type get_type() const { return type; }

    const std::string& get_value() const { return value; }

    const std::vector<std::unique_ptr<ParseTree>>&
    get_children() const { return children; }
    const std::unique_ptr<ParseTree>&
    get_child(std::size_t i) const { return children[i]; }

    friend std::ostream& operator<<(std::ostream& os, const ParseTree& pt);

private:
    Type type;
    // terminals (leaf nodes) have either value or token;
    std::string value;
    int token;
    std::vector<std::unique_ptr<ParseTree>> children;    
};
