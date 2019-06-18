#pragma once

#include <memory>
#include <iostream>
#include <vector>
#include <utility>
#include "token.h"

struct Expr {
    enum class Type {
        Literal, // number, string, true, false
        Unary, // op expr
        Binary, // expr op expr
        Grouping // ( expr )
    };

    Expr(Type type) : type(type) {}
    Expr(Type type, Token val) : type(type), val(val) {}

    void addChild(std::unique_ptr<Expr>&& child) {
        children.push_back(std::move(child));
    }


    void print(std::ostream& os) const {
        if (type == Type::Literal) {
            os << val.get_lexeme();
            return;
        }
        
        os << "(" << (type == Type::Grouping? "group" : val.get_lexeme());
        for (const auto& child_ptr : children) {
            os << " ";
            child_ptr->print(os);
        }
        os << ")";
    }
private:
    std::vector<std::unique_ptr<Expr>> children;
    Token val;

    Type type;
};


std::ostream& operator<< (std::ostream& os, const Expr& obj);