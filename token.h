#pragma once

#include <string_view>
#include <iostream>


struct Token {
    enum class Type {
        Unknown,
        // Literals
        Identifier, String, Number,
        // Symbols
        LeftParen, RightParen, LeftBrace, RightBrace,
        LeftBracket, RightBracket,
        Comma, Dot, Colon, Semicolon, Minus, Plus, Slash, Star, 
        Equal, NotEqual, Greater, GreaterEqual, Less, LessEqual, Assign,
        // Keywords
        Not, And, Or, True, False, Fun, While, For, If,
        Return, End, Let
    };
    Token() : type(Type::Unknown), lexeme(""), line(-1) {}
    Token(Type type, const std::string_view lexeme, int line)
        : type(type), lexeme(lexeme), line(line) {}
    Token(Type type) : type(type) {}

    std::string_view get_lexeme() const { return lexeme; }
    Type get_type() const { return type; }
    int get_line() const { return line; }

private:
    Type type;
    std::string_view lexeme;
    int line;
};

std::ostream& operator<< (std::ostream& os, const Token& obj);
