#include <iostream>
#include <unistd.h>
#include "parse_tree.hpp"
#include "print_tree.hpp"
#include "ir_emitter.hpp"

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
    bool success = emit_stmt(*program);

    if (!success) {
        std::cerr << "failed to generate code" << std::endl;
        return 1;
    }

    llvm::outs() << *module;

    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "module failed to verify. compilation terminated" << std::endl;
        return 1;
    }

}