#include <memory>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "llvm/Support/raw_ostream.h"
#include "parse_tree.hpp"

extern std::unique_ptr<llvm::Module> module;
llvm::Value* code_gen(const std::vector<Statement>& stmts);
llvm::Value* code_gen(const Statement& stmt);
llvm::Value* code_gen(const Expression& expr);