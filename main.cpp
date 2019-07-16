#include <iostream>
#include "ParseTree.hpp"
#include "symbol_checker.hpp"
extern std::unique_ptr<ParseTree> program_tree;
extern int yyparse();

int main(int argc, char **argv)
{
    yyparse();
    std::cout << *program_tree << std::endl;

    std::unordered_set<std::string> undefined_symbols;
    symbols_are_defined(program_tree.get(), undefined_symbols);
    std::cout << "undefined symbols:" << std::endl;
    for (const auto& undefined : undefined_symbols) {
        std::cout << undefined << std::endl;
    }
    return 0;
}