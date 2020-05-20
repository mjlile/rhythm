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
extern llvm::LLVMContext context;
extern std::map<Type, llvm::Type*> llvm_types;


void cstdlib();
llvm::Type*  llvm_type(const Type& type);

llvm::Value* emit_expr(const Literal    & lit,   bool addr = false);
llvm::Value* emit_expr(const Variable   & var,   bool addr = false);
llvm::Value* emit_expr(const Invocation & invoc, bool addr = false);
llvm::Value* emit_expr(const Expression & expr,  bool addr = false);

bool emit_stmt(const Expression  & expr );
bool emit_stmt(const Block       & stmts);
bool emit_stmt(const Declaration & decl );
bool emit_stmt(const Return      & ret  );
bool emit_stmt(const Conditional & cond );
bool emit_stmt(const Procedure   & proc );
bool emit_stmt(const Typedef     & def  );
bool emit_stmt(const Statement   & stmt );

#endif