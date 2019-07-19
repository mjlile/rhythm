#include <iostream>
#include "parse_tree.hpp"
#include "symbol_checker.hpp"
//#include "code_gen.hpp"
extern std::unique_ptr<ParseTree> program_tree;
extern int yyparse();

int main(int argc, char **argv)
{
    yyparse();
    std::cout << *program_tree << std::endl;

    std::vector<std::string> undefined_symbols;
    check_symbols(program_tree, undefined_symbols);
    std::cout << "undefined symbols:" << std::endl;
    for (const auto& undefined : undefined_symbols) {
        std::cout << undefined << std::endl;
    }
    return 0;
}