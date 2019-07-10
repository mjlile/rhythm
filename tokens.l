%{
#include <iostream>
#include <memory>
#include "parser.hpp"

bool savable_token(int t) {
    return t == TOKEN_IDENT || t == TOKEN_INT || t == TOKEN_TYPE
        || t == TOKEN_REAL || t == TOKEN_STR;
}

std::string* get_string() {
    // this hurts me
    return new std::string(yytext, yyleng);
}

int make_token(int t) {
    if (savable_token(t)) {
        yylval.string = get_string();
    }
    else {
        yylval.token = t;
    }
    return t;
}

extern "C" int yywrap() { }
%}

%%

[ \t\r]                   ;
"return"                return make_token(TOKEN_RETURN);
"if"                    return make_token(TOKEN_IF);
"then"                  return make_token(TOKEN_THEN);
"while"                 return make_token(TOKEN_WHILE);
"do"                    return make_token(TOKEN_DO);
"pro"                   return make_token(TOKEN_PROC);
"end"                   return make_token(TOKEN_END);
"and"                   return make_token(TOKEN_AND);
"or"                    return make_token(TOKEN_OR);
[a-z][a-zA-Z0-9_]*      return make_token(TOKEN_IDENT);
[A-Z][a-zA-Z0-9_]*      return make_token(TOKEN_TYPE);
[0-9]+\.[0-9]*          return make_token(TOKEN_REAL);
[0-9]+                  return make_token(TOKEN_INT);
\".*\"                  return make_token(TOKEN_STR);
"="                     return make_token(TOKEN_EQ);
"!="                    return make_token(TOKEN_NE);
"<"                     return make_token(TOKEN_LT);
"<="                    return make_token(TOKEN_LE);
">"                     return make_token(TOKEN_GT);
">="                    return make_token(TOKEN_GE);
"("                     return make_token(TOKEN_LPAREN);
")"                     return make_token(TOKEN_RPAREN);
"{"                     return make_token(TOKEN_LBRACE);
"}"                     return make_token(TOKEN_RBRACE);
"["                     return make_token(TOKEN_LBRACK);
"]"                     return make_token(TOKEN_RBRACK);
"<-"                    return make_token(TOKEN_LARROW);
"->"                    return make_token(TOKEN_RARROW);
"."                     return make_token(TOKEN_DOT);
","                     return make_token(TOKEN_COMMA);
"!"                     return make_token(TOKEN_BANG);
"+"                     return make_token(TOKEN_PLUS);
"-"                     return make_token(TOKEN_MINUS);
"*"                     return make_token(TOKEN_STAR);
"/"                     return make_token(TOKEN_SLASH);
"\n"                    return make_token(TOKEN_EOL);
.                       std::cerr << "Unknown token: " << std::string(yytext, yyleng); yyterminate();

%%