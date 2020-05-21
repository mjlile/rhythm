#include <map>
#include <iostream>
#include "llvm_intrinsics.hpp"
#include "type_system.hpp"


// convinient fold expression
template<typename First, typename ... T>
bool is_in(First&& first, T&& ... t)
{
    return ((first == t) || ...);
}


static llvm::Value *error(std::string_view str) {
    std::cerr << str << std::endl;
    return nullptr;
}

llvm::Value* intrinsic_op(const Invocation& invoc, llvm::IRBuilder<>& builder, llvm::Value* v) {
    assert(invoc.args.size() == 1);
    using namespace TypeSystem;
    if (invoc.name == "-") {
        if (is_integral(type_of(invoc))) {
            return builder.CreateNeg(v);
        }
        else if (is_floating_point(type_of(invoc))) {
            return builder.CreateFNeg(v);
        }
        else {
            assert(false);
        }
    }

    return error("unknown unary intrinsic op `" + invoc.name + "` or invalid parameter types");
}

llvm::Value* intrinsic_op(const Invocation& invoc, llvm::IRBuilder<>& builder, llvm::Value* lhs, llvm::Value* rhs) {
    assert(invoc.args.size() == 2);
    using namespace TypeSystem;
    if (invoc.name == "+") {
        if (is_integral(type_of(invoc))) {
            return builder.CreateAdd(lhs, rhs);
        }
        else if (is_floating_point(type_of(invoc))) {
            return builder.CreateFAdd(lhs, rhs);
        }
        else {
            assert(false);
        }
    }
    else if (invoc.name == "-") {
        if (is_integral(type_of(invoc))) {
            return builder.CreateSub(lhs, rhs);
        }
        else if (is_floating_point(type_of(invoc))) {
            return builder.CreateFSub(lhs, rhs);
        }
        else {
            assert(false);
        }
    }
    else if (invoc.name == "*") {
        if (is_integral(type_of(invoc))) {
            return builder.CreateMul(lhs, rhs);
        }
        else if (is_floating_point(type_of(invoc))) {
            return builder.CreateFMul(lhs, rhs);
        }
        else {
            assert(false);
        }
    }
    else if (invoc.name == "/") {
        if (is_unsigned_integral(type_of(invoc))) {
            return builder.CreateUDiv(lhs, rhs);
        }
        else if (is_signed_integral(type_of(invoc))) {
            return builder.CreateSDiv(lhs, rhs);
        }
        else if (is_floating_point(type_of(invoc))) {
            return builder.CreateFDiv(lhs, rhs);
        }
        else {
            assert(false);
        }
    }
    else if (invoc.name == "%") {
        // TODO: remainder vs modulo
        if (is_unsigned_integral(type_of(invoc))) {
            return builder.CreateURem(lhs, rhs);
        }
        else if (is_signed_integral(type_of(invoc))) {
            return builder.CreateSRem(lhs, rhs);
        }
        else if (is_floating_point(type_of(invoc))) {
            return builder.CreateFRem(lhs, rhs);
        }
        else {
            assert(false);
        }
    }
    else if (invoc.name == "=") {
        if (is_integral(type_of(invoc))) {
            return builder.CreateICmpEQ(lhs, rhs);
        }
        else if (is_floating_point(type_of(invoc))) {
            return builder.CreateFCmpUEQ(lhs, rhs);
        }
        else {
            assert(false);
        }
    }
    else if (invoc.name == "!=") {
        if (is_integral(type_of(invoc))) {
            return builder.CreateICmpNE(lhs, rhs);
        }
        else if (is_floating_point(type_of(invoc))) {
            return builder.CreateFCmpUNE(lhs, rhs);
        }
        else {
            assert(false);
        }
    }
    else if (invoc.name == "<") {
        if (is_unsigned_integral(type_of(invoc))) {
            return builder.CreateICmpULT(lhs, rhs);
        }
        else if (is_signed_integral(type_of(invoc))) {
            return builder.CreateICmpSLT(lhs, rhs);
        }
        else if (is_floating_point(type_of(invoc))) {
            return builder.CreateFCmpULT(lhs, rhs);
        }
        else {
            assert(false);
        }
    }
    else if (invoc.name == "<=") {
        if (is_unsigned_integral(type_of(invoc))) {
            return builder.CreateICmpULE(lhs, rhs);
        }
        else if (is_signed_integral(type_of(invoc))) {
            return builder.CreateICmpSLE(lhs, rhs);
        }
        else if (is_floating_point(type_of(invoc))) {
            return builder.CreateFCmpULE(lhs, rhs);
        }
        else {
            assert(false);
        }
    }
    else if (invoc.name == ">") {
        if (is_unsigned_integral(type_of(invoc))) {
            return builder.CreateICmpUGT(lhs, rhs);
        }
        else if (is_signed_integral(type_of(invoc))) {
            return builder.CreateICmpSGT(lhs, rhs);
        }
        else if (is_floating_point(type_of(invoc))) {
            return builder.CreateFCmpUGT(lhs, rhs);
        }
        else {
            assert(false);
        }
    }
    else if (invoc.name == ">=") {
        if (is_unsigned_integral(type_of(invoc))) {
            return builder.CreateICmpUGE(lhs, rhs);
        }
        else if (is_signed_integral(type_of(invoc))) {
            return builder.CreateICmpSGE(lhs, rhs);
        }
        else if (is_floating_point(type_of(invoc))) {
            return builder.CreateFCmpUGE(lhs, rhs);
        }
        else {
            assert(false);
        }
    }
    else if (invoc.name == "&&") {
        // TODO: short circuitating
        if (type_of(invoc) == Intrinsics::boolean) {
            return builder.CreateAnd(lhs, rhs);
        }
        else {
            assert(false);
        }
    }
    else if (invoc.name == "||") {
        // TODO: short circuitating
        if (type_of(invoc) == Intrinsics::boolean) {
            return builder.CreateOr(lhs, rhs);
        }
        else {
            assert(false);
        }
    }

    return nullptr;
}