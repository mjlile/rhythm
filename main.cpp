#include <iostream>
#include <map>
#include <unistd.h>
#include "parse_tree.hpp"
#include "print_tree.hpp"
#include "type_system.hpp"
#include "ir_emitter.hpp"

// bison (yacc) setup requires pointers, will change in the future
extern Block* program;
extern int yyparse();



int main(int argc, char **argv)
{
    // why in the world is the initializer list not working
    llvm_types[TypeSystem::Intrinsics::boolean] = llvm::Type::getInt8Ty   (context);
    llvm_types[TypeSystem::Intrinsics::boolean] = llvm::Type::getInt8Ty   (context);
    llvm_types[TypeSystem::Intrinsics::integer] = llvm::Type::getInt32Ty  (context);
    llvm_types[TypeSystem::Intrinsics::int8] =    llvm::Type::getInt8Ty   (context);
    llvm_types[TypeSystem::Intrinsics::int16] =   llvm::Type::getInt16Ty  (context);
    llvm_types[TypeSystem::Intrinsics::int32] =   llvm::Type::getInt32Ty  (context);
    llvm_types[TypeSystem::Intrinsics::int64] =   llvm::Type::getInt64Ty  (context);
    llvm_types[TypeSystem::Intrinsics::natural] = llvm::Type::getInt32Ty  (context);
    llvm_types[TypeSystem::Intrinsics::nat8] =    llvm::Type::getInt8Ty   (context);
    llvm_types[TypeSystem::Intrinsics::nat16] =   llvm::Type::getInt16Ty  (context);
    llvm_types[TypeSystem::Intrinsics::nat32] =   llvm::Type::getInt32Ty  (context);
    llvm_types[TypeSystem::Intrinsics::nat64] =   llvm::Type::getInt64Ty  (context);
    llvm_types[TypeSystem::Intrinsics::float32] = llvm::Type::getFloatTy  (context);
    llvm_types[TypeSystem::Intrinsics::float64] = llvm::Type::getDoubleTy (context);
    llvm_types[TypeSystem::Intrinsics::void0] =   llvm::Type::getVoidTy   (context);
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