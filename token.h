#pragma once

#include <string>
#include <variant>
#include <iostream>


struct Token {
    enum class Type {
        Unknown,
        // Operands
        Identifier, String, Integer, Float,
        // Symbols
        LeftParen, RightParen, LeftBrace, RightBrace,
        LeftBracket, RightBracket,
        Comma, Dot, Colon, Semicolon, Minus, Plus, Slash, Star, 
        Equal, NotEqual, Greater, GreaterEqual, Less, LessEqual, Assign,
        // Keywords
        Not, And, Or, True, False, Fun, While, For, If, Then,
        Return, End, Let, Print,
        // Other
        Newline,
        EndOfFile
    };
    //using LiteralVariant = std::variant<std::monostate, int, double, std::string>;

    Token() : type(Type::Unknown), line(-1) {}
    // constructs a token from a lexeme, inferring the type
    Token(std::string&& lexeme, Type type, int line)
        : Token(type, line) { value = lexeme; }
    // constructs a token with a given type
    Token(Type type, int line) : type(type), line(line) {}

    Type get_type() const { return type; }
    int get_line() const { return line; }
    friend std::ostream& operator<< (std::ostream& os, const Token& obj);

private:
    Type type;
    // identifier name or literal value (if applicable)
    std::string value;
    // line number in source file for debug info
    // TODO: column position and source file name too?
    int line;
};

std::ostream& operator<< (std::ostream& os, const Token& obj);
