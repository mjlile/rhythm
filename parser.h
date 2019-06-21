#include "AST.h"
#include <vector>

// Parses a range of tokens (e.g. Lexer (generator), vector<Token>)
// Generates statements as abstract syntax trees (ASTs)
template<typename InputIterator>
struct Parser {
    Parser(InputIterator tokens_begin, InputerIterator tokens_end)
     : tokens_begin(tokens_begin), tokens_end(tokens_end) {}


private:
    InputIterator tokens_begin;
    InputerIterator tokens_end;
};

//std::vector<Stmt> parse(std::vector<Token> tokens);