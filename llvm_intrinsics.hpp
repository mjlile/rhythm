
#ifndef LLVM_INTRINSICS_HPP
#define LLVM_INTRINSICS_HPP

#include "parse_tree.hpp"
#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"

// returns the value of the intrinsic operation (e.g. +, %, <) applied to lhs and rhs (e.g. lhs + rhs)
llvm::Value* intrinsic_op(const Invocation& invoc, llvm::IRBuilder<>& builder, llvm::Value* lhs, llvm::Value* rhs);

#endif