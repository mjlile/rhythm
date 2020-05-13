#ifndef CODE_GEN_HPP
#define CODE_GEN_HPP

#include <memory>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "parse_tree.hpp"

extern std::unique_ptr<llvm::Module> module;

void cstdlib();

llvm::Value* code_gen(const Literal& lit);
llvm::Value* code_gen(const Variable& var);
llvm::Type*  code_gen(const Type& type);
llvm::Value* code_gen(const Invocation& invoc);
llvm::Value* code_gen(const Expression& expr);
llvm::Value* code_gen(const Block& stmts);

llvm::Value* code_gen(const Declaration& decl);
llvm::Value* code_gen(const Return& ret);
llvm::Value* code_gen(const Conditional& cond);
llvm::Value* code_gen(const Procedure& proc);
llvm::Value* code_gen(const Statement& stmt);

#endif