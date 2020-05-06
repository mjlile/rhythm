#include "code_gen.hpp"
#include <iostream>
#include <algorithm>
#include <string_view>
#include <map>
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "parse_tree.hpp"

llvm::LLVMContext context;
llvm::IRBuilder<> builder(context);
std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("rhythm", context);
std::map<std::string, llvm::Value*> named_values;

// built in binary operations
const std::map<std::string, std::function<llvm::Value* (llvm::Value*, llvm::Value*)>> binary_ops = {
    {"+",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateAdd    (l, r); } },
    {"-",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateSub    (l, r); } },
    {"*",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateMul    (l, r); } },
    {"/",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateSDiv   (l, r); } },
    {"=",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateICmpEQ (l, r); } },
    {"!=", [](llvm::Value* l, llvm::Value* r) { return builder.CreateICmpNE (l, r); } },
    {"<",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateICmpSLT(l, r); } },
    {"<=", [](llvm::Value* l, llvm::Value* r) { return builder.CreateICmpSLE(l, r); } },
    {">",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateICmpSGT(l, r); } },
    {">=", [](llvm::Value* l, llvm::Value* r) { return builder.CreateICmpSGE(l, r); } }
};


llvm::Value *error(std::string_view str) {
    std::cerr << str << std::endl;
    return nullptr;
}


llvm::Value* code_gen(const Literal& lit) {
    // TODO: doubles and strings
    uint64_t val = atoi(lit.value().c_str());
    llvm::Type* t = llvm::IntegerType::get (context, 64);
    return llvm::ConstantInt::get(t, val, true);
}

std::vector<llvm::Value*> code_gen_args(const std::vector<Expression>& args) {	
    std::vector<llvm::Value*> val_args;	
    std::transform(args.cbegin(), args.cend(),	
                   std::back_inserter(val_args), [] (auto& x) { return code_gen(x); } );	
    return val_args;	
}

llvm::Value* code_gen(const std::string& var_name) {
    return named_values[var_name];
}

llvm::Value* code_gen(const Invocation& invoc) {	
    // TODO: general invocations. currently binary built in ops
    std::cout << "invocation of " << invoc.name() << std::endl;

    if (auto it = binary_ops.find(invoc.name()); it != binary_ops.end()) {
        llvm::Value *l = code_gen(invoc.args()[0]);
        llvm::Value *r = code_gen(invoc.args()[1]);
        if (!l || !r) { return error("bad input"); }
        return it->second(l, r);
    }

    llvm::Function *callee = module->getFunction(invoc.name());	
    if (!callee) {	
        return error("call to unknown procedure " + invoc.name());	
    }	
    if (callee->arg_size() != invoc.args().size()) {	
        return error(invoc.name() + " requires " + std::to_string(callee->arg_size())
                           + " arguments, " + std::to_string(invoc.args().size()) + " given");	
    }	

    auto arg_vals = code_gen_args(invoc.args());	
    if (auto it = std::find(arg_vals.begin(), arg_vals.end(), nullptr);
        it != arg_vals.end()) {	
        return error("bad argument in position " + std::to_string(it - arg_vals.begin())
             + " to procedure " + invoc.name());	
    }	

    return builder.CreateCall(callee, arg_vals, "calltmp");	
}

llvm::Value* code_gen(const Return& ret) {
    std::cout << "return" <<std::endl;
    if (ret.value()) {
        return builder.CreateRet(code_gen(*ret.value()));
    }

    return builder.CreateRetVoid();
}

llvm::Value* code_gen(const Expression& expr) {
    return std::visit([] (auto& x) { return code_gen(x); }, expr.value());
}


template<typename I1, typename I2, typename F>
// requires I1, I2 model iterator, F models binary function
I2 for_each_together(I1 first1, I1 limit1, I2 first2, F f) {
    // precondition: the range starting at first2 is at least as long as [first1, limit1)
    while (first1 != limit1) {
        f(*first1++, *first2++);
    }
    
    return first2;
}

// returns last statement value TODO: change?
llvm::Value* code_gen(const std::vector<Statement>& stmts) {
    llvm::Value* val;
    for (const Statement& stmt : stmts) {
        val = code_gen(stmt);
        if (!val) {
            return nullptr;	
        }
    }
    return val;
}

llvm::Value* code_gen(const Procedure& proc) {	
    std::cout << "proc" << std::endl;
    // check for function redefinition	
    // TODO: function hoisting?
    llvm::Function *redef_check = module->getFunction(proc.name());	
    if(redef_check && !redef_check->empty()) {	
        return error("function " + proc.name() + " redefined");	
    }	
    // Make the function type:  int(int...) etc.
    std::vector<llvm::Type*> param_types(proc.parameters().size(), llvm::Type::getInt64Ty(context)); // TODO: types	
    llvm::Type* ret_type = proc.return_type() == "void" ? 
        llvm::Type::getVoidTy(context) : llvm::Type::getInt64Ty(context);
    llvm::FunctionType* ft = llvm::FunctionType::get(ret_type, param_types, false); //TODO: types	

    llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, proc.name(), module.get());	
    std::cout << "created function " << proc.name() << std::endl;
    // Set names for all arguments.	
    // Record the function arguments in the named_values map.	
    named_values.clear();	
    for_each_together(
        f->args().begin(), f->args().end(),
        proc.parameters().begin(),
        [](auto& llvm_arg, const Declaration& arg) { 	
            std::cout << arg.variable() << std::endl;
            llvm_arg.setName(arg.variable());	
            named_values[arg.variable()] = &llvm_arg;
        }
    );	
    std::cout << "args done" << std::endl;

    // Create a new basic block to start insertion into.	
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(context, "entry", f);	
    builder.SetInsertPoint(bb);	

    if (!code_gen(proc.block())) {
        // Error reading body, remove function.	
        f->eraseFromParent();	
        return error("could not generate procedure " + proc.name());	
    }

    // Validate the generated code, checking for consistency.	
    llvm::verifyFunction(*f);	

    return f;	
}

llvm::Value* code_gen(const Statement& stmt) {
    std::cout << "stmt" << std::endl;
    return std::visit([] (auto& x) { return code_gen(x); }, stmt.value());
}