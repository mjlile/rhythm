#ifndef LEXER_H
#define LEXER_H

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <cassert>

struct Token {
    enum class Type {
        Unknown,
        // Literals
        Identifier, String, Number,
        // Symbols
        LeftParen, RightParen, LeftBrace, RightBrace,
        LeftBracket, RightBracket,
        Comma, Dot, Colon, Semicolon, Minus, Plus, Slash, Star, 
        Equal, Greater, GreaterEqual, Less, LessEqual, Assign,
        // Keywords
        Not, And, Or, True, False, Fun, While, For, If,
        Return, End, Let
    };

    Token(Type type, const std::string_view lexeme, int line)
        : type(type), lexeme(lexeme), line(line) {}

    std::string_view get_lexeme() const { return lexeme; }
    Type get_type() const { return type; }
    int get_line() const { return line; }

private:
    Type type;
    const std::string_view lexeme;
    int line;
};


std::vector<Token> lex(std::string source_content);
std::ostream& operator<< (std::ostream& os, const Token& obj);

#endif