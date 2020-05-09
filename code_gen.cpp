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
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "parse_tree.hpp"

llvm::LLVMContext context;
llvm::IRBuilder<> builder(context);
std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("rhythm", context);
// TODO: add stack frames
std::map<std::string, llvm::AllocaInst*> named_values;

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

// create_entry_alloca - Create an alloca instruction in the entry block of
// the function.  This is used for mutable variables etc.
// TODO: type of var
llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function* f, const std::string& var_name) {
    llvm::IRBuilder<> tmp_b(&f->getEntryBlock(), f->getEntryBlock().begin());
    return tmp_b.CreateAlloca(llvm::Type::getInt64Ty(context), 0, var_name.c_str());
}


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
                   std::back_inserter(val_args),
                   [] (auto& x) { return code_gen(x); } );	
    return val_args;
}

llvm::Value* code_gen(const std::string& var_name) {
    // Look this variable up in the function.
    llvm::Value *v = named_values[var_name];
    if (!v) {
        return error("unknown variable \"" + var_name + "\"");
    }

    // Load the value.
    return builder.CreateLoad(v, var_name.c_str());
}

llvm::Value* code_gen(const Invocation& invoc) {	
    // TODO: general invocations. currently binary built in ops
    std::cerr << "invocation of " << invoc.name() << std::endl;

    if (invoc.name() == "<-") {
        if (!std::holds_alternative<std::string>(invoc.args()[0].value())) {
            return error("left-hand side of assignment must be a variable");
        }
        std::string var_name = std::get<std::string>(invoc.args()[0].value());
        llvm::AllocaInst* var = named_values[var_name];
        if (!var) {
            return error("unknown variable " + var_name);
        }
        llvm::Value *r = code_gen(invoc.args()[1]);
        builder.CreateStore(r, var);
        // forbid assignment as expression
        return var;
    }

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

llvm::Value* code_gen(const Declaration& decl) {
    std::cerr << "decl" << std::endl;
    // TODO: allow shadowed variables from higher scopes
    if (named_values.find(decl.variable()) != named_values.end()) {
        return error("variable \"" + decl.variable() + "\" is already declared");
    }
    llvm::Function* f = builder.GetInsertBlock()->getParent();

    llvm::AllocaInst* alloc = CreateEntryBlockAlloca(f, decl.variable());

    std::cerr << "initializing" << std::endl;
    // store initializer, if applicable. otherwise, the value is undefined
    if (decl.initializer()) {
        llvm::Value* init_val = code_gen(*decl.initializer());
        builder.CreateStore(init_val, alloc);
    }
    std::cerr << "initialized" << std::endl;

    // update symbol table, saving old value to restore later
    // TODO: restore old value when this variable goes out of scope (e.g. shadowing)
    llvm::AllocaInst* old = std::exchange(named_values[decl.variable()], alloc);
    (void) old;
    return named_values[decl.variable()];
}

llvm::Value* code_gen(const Return& ret) {
    std::cerr << "return" <<std::endl;
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
// TODO: should statements return a value at all?
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

llvm::Value* code_gen(const Conditional& cond) {
    llvm::Value* condition = code_gen(cond.condition());
    if (!condition) {
        return nullptr;
    }

    llvm::Function* f = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* then_block = llvm::BasicBlock::Create(context, "then", f);
    llvm::BasicBlock* else_block = llvm::BasicBlock::Create(context, "else");
    llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(context, "ifcont");

    // TODO: what if there's no else block? replace else with merge?
    builder.CreateCondBr(condition, then_block, else_block);
    builder.SetInsertPoint(then_block);

    if (!code_gen(cond.then_block())) {
        return nullptr;
    }

    builder.CreateBr(merge_block);

    // code_gen for then block may have changed insertion block (e.g. nested if)
    then_block = builder.GetInsertBlock();

    // emit else block
    f->getBasicBlockList().push_back(else_block);
    builder.SetInsertPoint(else_block);
    
    if (!code_gen(cond.else_block())) {
        return nullptr;
    }

    builder.CreateBr(merge_block);
    // code_gen for else block may have changed insertion block
    else_block = builder.GetInsertBlock();

    // emit merge block
    f->getBasicBlockList().push_back(merge_block);
    builder.SetInsertPoint(merge_block);

    // TODO: ?
    return merge_block;
}

llvm::Value* code_gen(const ConditionalLoop& loop) {
    llvm::Function* f = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* cond_block = llvm::BasicBlock::Create(context, "cond", f);
    llvm::BasicBlock* loop_block = llvm::BasicBlock::Create(context, "loop");
    llvm::BasicBlock* cont_block = llvm::BasicBlock::Create(context, "loopcont");

    builder.CreateBr(cond_block);
    builder.SetInsertPoint(cond_block);

    // emit condition block
    llvm::Value* condition = code_gen(loop.conditional().condition());
    if (!condition) {
        return nullptr;
    }

    builder.CreateCondBr(condition, loop_block, cont_block);

    // emit loop block
    f->getBasicBlockList().push_back(loop_block);
    builder.SetInsertPoint(loop_block);

    if (!code_gen(loop.conditional().then_block())) {
        return nullptr;
    }

    // always branch back to check the condition
    builder.CreateBr(cond_block);

    // continue code after while loop
    f->getBasicBlockList().push_back(cont_block);
    builder.SetInsertPoint(cont_block);

    // TODO: ?
    return cont_block;

}

llvm::Value* code_gen(const Procedure& proc) {
    std::cerr << "proc" << std::endl;
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
    std::cerr << "created function " << proc.name() << std::endl;

    // Set names for all arguments
    for_each_together(
        f->args().begin(), f->args().end(),
        proc.parameters().begin(),
        [](auto& llvm_arg, const Declaration& arg) { 	
            std::cerr << arg.variable() << std::endl;
            llvm_arg.setName(arg.variable());	
        }
    );
    std::cerr << "args done" << std::endl;

    // Create a new basic block to start insertion into.	
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(context, "entry", f);	
    builder.SetInsertPoint(bb);

    // allocate memory for parameters
    for (auto& arg : f->args()) {
        llvm::AllocaInst* alloc = CreateEntryBlockAlloca(f, arg.getName());
        builder.CreateStore(&arg, alloc);
        if (named_values.find(arg.getName()) != named_values.end()) {
            return error("variable " + std::string(arg.getName().str()) + " already defined");
        }
        named_values[arg.getName()] = alloc;
    }

    if (!code_gen(proc.block())) {
        // Error reading body, remove function.	
        f->eraseFromParent();	
        return error("could not generate procedure " + proc.name());	
    }
	
    // add implicit return at the end of void function
    if (proc.return_type() == "void") {
        builder.CreateRetVoid();
    }

    // Validate the generated code, checking for consistency.	
    llvm::verifyFunction(*f);	

    return f;	
}

llvm::Value* code_gen(const Statement& stmt) {
    std::cerr << "stmt" << std::endl;
    return std::visit([] (auto& x) { return code_gen(x); }, stmt.value());
}