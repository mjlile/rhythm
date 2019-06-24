#pragma once

#include "token.h"
#include "gen_iterator.h"
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <cassert>
#include <utility>

// Lexes a range of characters (e.g. istream, string)
// Generates tokens
template<typename InputIterator>
struct Lexer {
    Lexer(InputIterator chars_begin, InputIterator chars_end)
            : chars_begin(chars_begin), chars_end(chars_end), line_num(0) {
        curr_token = get();
    }
    // get the current Token and advance the state of the lexer
    Token get();
    // peek at the current Token without advancing
    Token peek() const;
    
    // iterator info
    using value_type = Token;
    using iterator = gen_iterator<Lexer>;
    friend iterator;
    iterator begin() { return iterator(*this); }
    iterator end() { return iterator(); }

    bool at_end() {
        return curr_token.get_type() == Token::Type::EndOfFile;    
    }


private:
    InputIterator chars_begin;
    InputIterator chars_end;
    int line_num;
    Token curr_token;


    // returns the type if it's a keyword, otherwise identifier
    Token::Type get_keyword_type(const std::string& lexeme);
    // returns the type if it's a valid symbol, otherwise unknown
    Token::Type get_symbol_type(const std::string& lexeme);
};


// helper functions
template<typename InputIterator>
Token::Type Lexer<InputIterator>::get_keyword_type(const std::string& lexeme) {
    static const std::map<std::string, Token::Type, std::less<>> keywords = 
    {
        {"not",    Token::Type::Not},
        {"and",    Token::Type::And},
        {"or",     Token::Type::Or},
        {"true",   Token::Type::True},
        {"false",  Token::Type::False},
        {"fun",    Token::Type::Fun},
        {"while",  Token::Type::While},
        {"for",    Token::Type::For},
        {"if",     Token::Type::If},
        {"return", Token::Type::Return},
        {"end",    Token::Type::End},
        {"let",    Token::Type::Let},
        {"print",  Token::Type::Print}
    };

    auto it = keywords.find(lexeme);
    if (it != keywords.end()) {
        return it->second;
    }
    return Token::Type::Identifier;
}

template<typename InputIterator>
Token::Type Lexer<InputIterator>::get_symbol_type(const std::string& lexeme) {
    static const std::map<std::string, Token::Type, std::less<>> symbols = 
    {
        {"(",  Token::Type::LeftParen},
        {")",  Token::Type::RightParen},
        {"[",  Token::Type::LeftBracket},
        {"]",  Token::Type::RightBracket},
        {"{",  Token::Type::LeftBrace},
        {"}",  Token::Type::RightBrace},
        {",",  Token::Type::Comma},
        {".",  Token::Type::Dot},
        {":",  Token::Type::Colon},
        {";",  Token::Type::Semicolon},
        {"+",  Token::Type::Plus},
        {"-",  Token::Type::Minus},
        {"*",  Token::Type::Star},
        {"/",  Token::Type::Slash},
        {"=",  Token::Type::Equal},
        {">",  Token::Type::Greater},
        {"<",  Token::Type::Less},
        {"<=", Token::Type::LessEqual},
        {">=", Token::Type::GreaterEqual},
        {"!=", Token::Type::Assign},
        {"<-", Token::Type::Assign},
    };

    auto it = symbols.find(lexeme);
    if (it != symbols.end()) {
        return it->second;
    }
    return Token::Type::Unknown;
}

bool ignorable(char c) {
    return std::isspace(c) && c != '\n';
}
// TODO: and error handling for end of stream before finishing a token
template<typename InputIterator>
Token Lexer<InputIterator>::get() {
    if (chars_begin == chars_end) {
        curr_token = Token(Token::Type::EndOfFile, line_num);
        return curr_token;
    }
    while (ignorable(*chars_begin)) { ++chars_begin; }
    char c = *chars_begin++;
    if (c == '\n') {
        // new line
        ++line_num;
        curr_token = Token(Token::Type::Newline, line_num - 1);
    }
    else if (std::isdigit(c)) {
        // numeric literal
        std::string lexeme(1, c);
        while (std::isdigit(*chars_begin)) {
            // TODO: more efficient? stream? int (num = num * 10 + ctoi(c))and store variant?
            lexeme.push_back(*chars_begin++);
        }
        if (*chars_begin != '.') {
            curr_token = Token(std::move(lexeme), Token::Type::Integer, line_num);
        }
        else {
            lexeme.push_back(*chars_begin++);
            while (std::isdigit(*chars_begin)) {
                // TODO: more efficient? stream? int (num = num * 10 + ctoi(c))and store variant?
                lexeme.push_back(*chars_begin++);
            }
            curr_token = Token(std::move(lexeme), Token::Type::Float, line_num);
        }
    }
    else if (c == '"') {
        // string literal
        std::string lexeme;
        while (*chars_begin != '"') { 
            lexeme.push_back(*chars_begin++);
        }
        curr_token = Token(std::move(lexeme), Token::Type::String, line_num);
    }
    else if (std::isalpha(c)) {
        // identifier or keyword
        std::string lexeme(1, c);
        while (std::isalnum(*chars_begin) || *chars_begin == '_') {
            lexeme.push_back(*chars_begin++);
        }
        auto type = Lexer<InputIterator>::get_keyword_type(lexeme);
        if (type == Token::Type::Identifier) {
            curr_token = Token(std::move(lexeme), type, line_num);
        }
        else {
            curr_token = Token(type, line_num);
        }
    }
    else {
        // symbolic token
        std::string lexeme(1, c);
        auto one_char_type = Lexer::get_symbol_type(lexeme);
        lexeme += *chars_begin;
        auto two_char_type = Lexer::get_symbol_type(lexeme);

        // check if it's a two char symbol, otherwise one char symbol
        if (two_char_type != Token::Type::Unknown) {
            ++chars_begin;
            curr_token = Token(two_char_type, line_num);
        }
        else {
            // TODO: returns unknown if not a symbol
            // should it throw instead? store lexeme to show later?
            curr_token = Token(one_char_type, line_num);
        }
    }

    return curr_token;
}


template<typename InputIterator>
Token Lexer<InputIterator>::peek() const {
    return curr_token;
}
