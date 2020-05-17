%{
#include <iostream>
#include <memory>
#include <cstring>
// required because bison does not include headers in parser.hpp
#include "parse_tree.hpp"
#include "parser.hpp"

bool savable_token(int t) {
    return t == TOKEN_IDENT || t == TOKEN_INT || t == TOKEN_TYPE
        || t == TOKEN_REAL || t == TOKEN_STR;
}

int make_token(int t) {
    if (savable_token(t)) {
        // lex / yacc c interface, shouldn't new a std::string
        if (t == TOKEN_STR) {
            // remove quotes
            yylval.string = new std::string(yytext + 1, yyleng - 2);
        }
        else {
            yylval.string = new std::string(yytext, yyleng);
        }
    }
    else {
        yylval.token = t;
    }
    return t;
}

extern "C" int yywrap() { return true; }
%}

%%

[ \t\r]                   ;
"import"                return make_token(TOKEN_IMPORT);
"let"                   return make_token(TOKEN_LET);
"return"                return make_token(TOKEN_RETURN);
"if"                    return make_token(TOKEN_IF);
"while"                 return make_token(TOKEN_WHILE);
"proc"                  return make_token(TOKEN_PROC);
"and"                   return make_token(TOKEN_AND);
"or"                    return make_token(TOKEN_OR);
"typedef"               return make_token(TOKEN_TYPEDEF);
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
":"                     return make_token(TOKEN_COLON);
"<-"                    return make_token(TOKEN_LARROW);
"->"                    return make_token(TOKEN_RARROW);
"."                     return make_token(TOKEN_DOT);
","                     return make_token(TOKEN_COMMA);
"!"                     return make_token(TOKEN_BANG);
"+"                     return make_token(TOKEN_PLUS);
"-"                     return make_token(TOKEN_MINUS);
"*"                     return make_token(TOKEN_STAR);
"/"                     return make_token(TOKEN_SLASH);
"%"                     return make_token(TOKEN_PERCENT);
"\n"                    return make_token(TOKEN_EOL);
.                       std::cerr << "Unknown token: " << std::string(yytext, yyleng) << std::endl;

%%