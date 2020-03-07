#include <iostream>
#include "parse_tree.hpp"
#include "print_tree.hpp"
#include "interpreter.hpp"

// bison (yacc) setup requires pointers, will change in the future
extern std::vector<Statement>* program;
extern int yyparse();


int main(int argc, char **argv)
{
    // parse with bison (yacc)
    yyparse();
    // print program tree
    std::cout << *program << std::endl;
    // TODO: symbol and type checking
    // run program with (very inefficient) abstract syntax tree walker
    interpret(*program);
}