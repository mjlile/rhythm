#include <iostream>
#include "parse_tree.hpp"
#include "print_tree.hpp"
//#include "symbol_checker.hpp"
//#include "code_gen.hpp"
extern std::vector<Statement>* program;
extern int yyparse();


int main(int argc, char **argv)
{
    yyparse();
    std::cout << program->size() << std::endl;
    std::cout << *program << std::endl;
    std::cout << sizeof(program->at(0)) << std::endl;

/*
    std::vector<std::string> undefined_symbols;
    check_symbols(program_tree, undefined_symbols);
    std::cout << "undefined symbols:" << std::endl;
    for (const auto& undefined : undefined_symbols) {
        std::cout << undefined << std::endl;
    }
    */
    return 0;
}