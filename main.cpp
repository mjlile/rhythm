#include <iostream>
#include <unistd.h>
#include "parse_tree.hpp"
#include "print_tree.hpp"
#include "code_gen.hpp"

// bison (yacc) setup requires pointers, will change in the future
extern Block* program;
extern int yyparse();


int main(int argc, char **argv)
{
    // parse with bison (yacc)
    yyparse();

    // declare some c functions (e.g. printf)
    cstdlib();

    // generate LLVM IR
    llvm::Value* ir = code_gen(*program);

    if (!ir) {
        std::cerr << "failed to generate code" << std::endl;
        return 1;
    }
    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "module failed to verify. compilation terminated" << std::endl;
        return 1;
    }

    llvm::outs() << *module;

}