#include <iostream>
#include "parse_tree.hpp"
#include "print_tree.hpp"
#include "code_gen.hpp"

// bison (yacc) setup requires pointers, will change in the future
extern std::vector<Statement>* program;
extern int yyparse();


int main(int argc, char **argv)
{
    // parse with bison (yacc)
    yyparse();
    // print program tree
    //std::cout << *program << std::endl;
    // TODO: symbol and type checking
    // run program with (very inefficient) abstract syntax tree walker
    //interpret(*program);

    cstdlib();
    llvm::Value* ir = code_gen(*program);
    if (!ir) {
        std::cerr << "failed to generate code" << std::endl;
        return 1;
    }
    llvm::outs() << *module;
    llvm::verifyModule(*module, &llvm::errs());
}