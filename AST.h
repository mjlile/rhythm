#pragma once

#include <memory>
#include <iostream>
#include <vector>
#include <utility>
#include <variant>
#include <string>
#include "token.h"


using Value = std::variant<int, double, std::string, bool>;

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
    
    Value evaluate() const {
        switch (type) {
            case Type::Literal:
                switch (val.get_type()) {
                    case Token::Type::String:
                        return Value(std::string(val.get_lexeme()));
                        break;
                    case Token::Type::Integer:
                        return Value(std::stoi(std::string(val.get_lexeme())));
                        break;
                    case Token::Type::Float:
                        return Value(std::stod(std::string(val.get_lexeme())));
                        break;
                    case Token::Type::False:
                        return Value(false);
                        break;
                    case Token::Type::True:
                        return Value(true);
                        break;
                }
                break;

            case Type::Grouping:
                return children.at(0)->evaluate();
                break;
            
            case Type::Unary:
            {
                Value right = children.at(0)->evaluate();
                switch (val.get_type()) {
                    case Token::Type::Minus:
                        if (std::holds_alternative<int>(right)) {
                            return Value(-std::get<int>(right));
                        }
                        if (std::holds_alternative<double>(right)) {
                            return Value(-std::get<double>(right));
                        }
                        break;
                    case Token::Type::Not:
                        return Value(!std::get<bool>(right));
                    default:
                        throw "invalid unary";
                        break;
                }
            }
            case Type::Binary:
                Value left = children.at(0)->evaluate();
                Value right = children.at(1)->evaluate();
                switch (val.get_type()) {
                    case Token::Type::Plus :
                        return std::get<double>(left) + std::get<double>(right);
                        break;
                    case Token::Type::Minus :
                        return std::get<double>(left) - std::get<double>(right);
                        break;
                    case Token::Type::Star :
                        return std::get<double>(left) * std::get<double>(right);
                        break;
                    case Token::Type::Slash :
                        return std::get<double>(left) / std::get<double>(right);
                        break;
                    case Token::Type::And :
                        return std::get<bool>(left) && std::get<bool>(right);
                        break;
                    case Token::Type::Or :
                        return std::get<bool>(left) || std::get<bool>(right);
                        break;
                    
                    default:
                        break;
                }
        }
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


struct Stmt {
    enum class Type {
        Expression,
        Print
    };
    Stmt(Type type, std::unique_ptr<Expr>&& expr)
            : type(type), expr(move(expr)) {}

    void interpret() const {
        if (type == Type::Print) {
            std::visit([](const auto& v) { std::cout << v << std::endl; }, expr->evaluate());
        }
    }

    std::unique_ptr<Expr> expr;
    Type type;
};



std::ostream& operator<< (std::ostream& os, const Stmt& obj);