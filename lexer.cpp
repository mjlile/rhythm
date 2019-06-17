#include "lexer.h"
#include <cctype>

using namespace std;

struct Source
{
    explicit Source(const string& old_source) : source(old_source) {}

    bool isAtEnd() const {
        return start + length >= source.size();
    }
    bool is_single_char_op(char c) { return single_char_ops.find(c) != single_char_ops.end(); }
    const string_view view() const { return string_view(source).substr(start, length); }
    void ignore() { ++start; }
    char advance() { char c = peak(); ++length; return c; }
    char peak() const {
        if (isAtEnd()) {
            return '\0';
        }
        return source.at(start + length);
    }
    int get_line_num() const { return line_num; }
    int new_line() { line_start = start + length; ++line_num; }



    Token advance_keyword_id_token() {
        auto keyword_it = keywords.find(view());
        if (keyword_it != keywords.end()) {
            return advance_token(keywords.find(view())->second);
        }

        // if it's not a keyword, it's an identifier
        return advance_token(Token::Type::Identifier);
    }

    Token advance_single_char_token() {
        assert(view().size() == 1);
        return advance_token(single_char_ops.find(view().at(0))->second);
    }

    Token peak_token(Token::Type type) const {
        return Token(type, view(), get_line_num());
    }

    Token advance_token(Token::Type type) {
        Token tok = peak_token(type);
        // TODO: += length + 1 ?
        start += length;
        // TODO: length = 1 ?
        length = 0;
        return tok;
    }

private:
    int start = 0;
    int length = 0;
    int line_num = 1;
    int line_start = 0;
    const string& source;

    const map<string_view, Token::Type, less<> > keywords = 
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
            {"let",    Token::Type::Let}
        };

    const map<char, Token::Type, less<> > single_char_ops = 
        {
            {'(', Token::Type::LeftParen},
            {')', Token::Type::RightParen},
            {'[', Token::Type::LeftBracket},
            {']', Token::Type::RightBracket},
            {'{', Token::Type::LeftBrace},
            {'}', Token::Type::RightBrace},
            {',', Token::Type::Comma},
            {'.', Token::Type::Dot},
            {':', Token::Type::Colon},
            {';', Token::Type::Semicolon},
            {'+', Token::Type::Plus},
            {'-', Token::Type::Minus},
            {'*', Token::Type::Star},
            {'/', Token::Type::Slash},
            {'=', Token::Type::Equal},
            {'>', Token::Type::Greater},
            {'<', Token::Type::Less}
        };
};

ostream& operator<< (ostream& os, const Token& obj) {
    os << obj.get_lexeme() << " (" << (int)obj.get_type() << ") at line " << obj.get_line();
    return os;
}

Token get_number(Source& source) {
    while (isdigit(source.peak())) {
        source.advance();
    }
    return source.advance_token(Token::Type::Number);
}

Token get_string(Source& source) {
    while (source.peak() != '\"') {
        source.advance();
    }
    // consume closing quote
    source.advance(); 
    return source.advance_token(Token::Type::String);
}

Token get_id_or_keyword(Source& source) {
    while (isalnum(source.peak()) || source.peak() == '_') {
        source.advance();
    }
    return source.advance_keyword_id_token();
}


Token get_symbol_token(char c, Source& source) {
    // multi char symbols
    if (c == '<' && source.peak() == '-') {
        source.advance();
        return source.advance_token(Token::Type::Assign);
    }
    if (c == '<' && source.peak() == '=') {
        source.advance();
        return source.advance_token(Token::Type::LessEqual);
    }
    if (c == '>' && source.peak() == '=') {
        source.advance();
        return source.advance_token(Token::Type::GreaterEqual);
    }

    // single character symbols
    if (source.is_single_char_op(c)) {
        return source.advance_single_char_token();
    }

    // no match found
    return source.advance_token(Token::Type::Unknown);
}


Token get_token(Source& source) {
    string lexeme;
    char c = source.advance();
    
    if (isdigit(c)) {
        return get_number(source);
    }
    if (c == '\"') {
        return get_string(source);
    }
    if (isalpha(c)) {
        return get_id_or_keyword(source);
    }

    return get_symbol_token(c, source);
}


vector<Token> lex(string source_content) {
    vector<Token> tokens;
    Source source(source_content);

    while (!source.isAtEnd()) {
        while (isspace(source.peak()) && !source.isAtEnd()) {
            if (source.peak() == '\n') {
                source.new_line();
            }
            source.ignore();
        }
        tokens.push_back(get_token(source));
    }

    return tokens;
}
