#include <iostream>
#include "ParseTree.hpp"
extern std::unique_ptr<ParseTree> program_tree;
extern int yyparse();

int main(int argc, char **argv)
{
    yyparse();
    std::cout << program_tree.get() << std::endl;
    std::cout << *program_tree << std::endl;
    std::cout << "done" << std::endl;
    return 0;
}