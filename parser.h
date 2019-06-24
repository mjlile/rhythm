#include "Ast.h"
#include "gen_iterator.h"
// Parses a range of tokens (e.g. Lexer (generator), vector<Token>)
// Generates statements as abstract syntax trees (ASTs)
template<typename InputIterator>
struct Parser {
    Parser(InputIterator tokens_begin, InputerIterator tokens_end)
    : tokens_begin(tokens_begin), tokens_end(tokens_end) {
        curr_statement = get();
    }

    Statement get();
    Statement peek() const { return curr_statement; }

    // iterator information
    using value_type = Statement;
    using iterator = gen_iterator<Parser>;
    friend iterator;
    iterator begin() { return iterator(*this); }
    iterator end() { return iterator(); }
    bool at_end() const;

private:
    InputIterator tokens_begin;
    InputerIterator tokens_end;
    Statement curr_statement;

    std::unique_ptr<Expression> get_expr();
};

template<typename InputIterator>
Statement Parser<InputIterator>::get() {
    Token t = *tokens_begin++;
}