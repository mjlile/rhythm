#include "token.h"
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <cctype>

using namespace std;
using TT = Token::Type;

const unordered_map<string, TT> token_map = 
    {
        // keywords
        {"not",    TT::Not},
        {"and",    TT::And},
        {"or",     TT::Or},
        {"true",   TT::True},
        {"false",  TT::False},
        {"fun",    TT::Fun},
        {"while",  TT::While},
        {"for",    TT::For},
        {"if",     TT::If},
        {"return", TT::Return},
        {"end",    TT::End},
        {"let",    TT::Let},
        {"print",  TT::Print},
        // symbols
        {"(", TT::LeftParen},
        {")", TT::RightParen},
        {"[", TT::LeftBracket},
        {"]", TT::RightBracket},
        {"{", TT::LeftBrace},
        {"}", TT::RightBrace},
        {",", TT::Comma},
        {".", TT::Dot},
        {":", TT::Colon},
        {";", TT::Semicolon},
        {"+", TT::Plus},
        {"-", TT::Minus},
        {"*", TT::Star},
        {"/", TT::Slash},
        {"=", TT::Equal},
        {">", TT::Greater},
        {"<", TT::Less},
        // multi char symbols
        {"<=", TT::LessEqual},
        {">=", TT::GreaterEqual},
        {"!=", TT::NotEqual},
        {"<-", TT::Assign},
    };

const unordered_map<TT, string> token_print_map = 
    {
        {TT::Unknown, "Unknown"},
        {TT::Not, "not"},
        {TT::And, "and"},
        {TT::Or, "or"},
        {TT::True, "true"},
        {TT::False, "false"},
        {TT::Fun, "fun"},
        {TT::While, "while"},
        {TT::For, "for"},
        {TT::If, "if"},
        {TT::Return, "return"},
        {TT::End, "end"},
        {TT::Let, "let"},
        {TT::Print, "print"},
        {TT::LeftParen, "("},
        {TT::RightParen, ")"},
        {TT::LeftBracket, "["},
        {TT::RightBracket, "]"},
        {TT::LeftBrace, "{"},
        {TT::RightBrace, "}"},
        {TT::Comma, ","},
        {TT::Dot, "."},
        {TT::Colon, ":"},
        {TT::Semicolon, ";"},
        {TT::Plus, "+"},
        {TT::Minus, "-"},
        {TT::Star, "*"},
        {TT::Slash, "/"},
        {TT::Equal, "="},
        {TT::Greater, ">"},
        {TT::Less, "<"},
        {TT::LessEqual, "<="},
        {TT::GreaterEqual, ">="},
        {TT::NotEqual, "!="},
        {TT::Assign, "<-"},
        {TT::Newline, "\\n"}
    };

ostream& operator<< (ostream& os, const Token& obj) {
    if (!obj.value.empty()) {
        os << obj.value;
        return os;
    }
    auto print_it = token_print_map.find(obj.get_type());
    if (print_it != token_print_map.end()) {
        os << print_it->second;
    }
    else {
        os << "(token printing not implemented: "
           << (int)obj.get_type() << ")";
    }

    return os;
}