#include "code_gen.hpp"
#include <iostream>
#include <algorithm>
#include <string_view>
#include <string>
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
#include "types.hpp"
#include "symbol_table.hpp"

llvm::LLVMContext context;
llvm::IRBuilder<> builder(context);
std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("rhythm", context);
// TODO: add stack frames
SymbolTable symbol_table;

// built in binary operations
std::map<const std::string, std::function<llvm::Value* (llvm::Value*, llvm::Value*)>> binary_ops = {
    {"+",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateAdd    (l, r); } },
    {"-",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateSub    (l, r); } },
    {"*",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateMul    (l, r); } },
    {"/",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateSDiv   (l, r); } },
    {"%",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateSRem   (l, r); } },
    {"=",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateICmpEQ (l, r); } },
    {"!=", [](llvm::Value* l, llvm::Value* r) { return builder.CreateICmpNE (l, r); } },
    {"<",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateICmpSLT(l, r); } },
    {"<=", [](llvm::Value* l, llvm::Value* r) { return builder.CreateICmpSLE(l, r); } },
    {">",  [](llvm::Value* l, llvm::Value* r) { return builder.CreateICmpSGT(l, r); } },
    {">=", [](llvm::Value* l, llvm::Value* r) { return builder.CreateICmpSGE(l, r); } }
};

std::map<Type, llvm::Type*> types = {
    // boolean is 1 byte, but llvm uses int1 TODO?
    { type::boolean, llvm::Type::getInt8Ty   (context) },
    // integer types
    { type::integer, llvm::Type::getInt32Ty  (context) },
    { type::int8,    llvm::Type::getInt8Ty   (context) },
    { type::int16,   llvm::Type::getInt16Ty  (context) },
    { type::int32,   llvm::Type::getInt32Ty  (context) },
    { type::int64,   llvm::Type::getInt64Ty  (context) },
    // natural number types
    // llvm only distinguishes signed/unsigned for relevant operations
    { type::natural, llvm::Type::getInt32Ty  (context) },
    { type::nat8,    llvm::Type::getInt8Ty   (context) },
    { type::nat16,   llvm::Type::getInt16Ty  (context) },
    { type::nat32,   llvm::Type::getInt32Ty  (context) },
    { type::nat64,   llvm::Type::getInt64Ty  (context) },
    // floating point types
    { type::float32, llvm::Type::getFloatTy  (context) },
    { type::float64, llvm::Type::getDoubleTy (context) },

    { type::void0,   llvm::Type::getVoidTy   (context) }
};



// create_entry_alloca - Create an alloca instruction in the entry block of
// the function.  This is used for mutable variables etc.
// TODO: type of var
llvm::AllocaInst *create_entry_block_alloca(llvm::Function* f, const Declaration& decl) {
    llvm::IRBuilder<> tmp_b(&f->getEntryBlock(), f->getEntryBlock().begin());
    return tmp_b.CreateAlloca(code_gen(decl.type), 0, decl.variable.name.c_str());
}

void cstdlib() {
    std::vector<llvm::Type*> param_types;
    llvm::FunctionType* ft;
    llvm::Function* f;

    // scanf
    param_types = { llvm::Type::getInt8PtrTy(context) };
    ft = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), param_types, true);
    f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "scanf", module.get());	
    f->setCallingConv(llvm::CallingConv::C);

    // printf
    param_types = { llvm::Type::getInt8PtrTy(context) };
    ft = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), param_types, true);
    f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "printf", module.get());	
    f->setCallingConv(llvm::CallingConv::C);
}


llvm::Value *error(std::string_view str) {
    std::cerr << str << std::endl;
    return nullptr;
}

llvm::Value* constant_num(int n) {
    return code_gen(Literal{std::to_string(n), Literal::Type::integer});
}

llvm::Value* code_gen(const Literal& lit) {
    // TODO: typeless literals from Go 
    switch (lit.type) {
    case Literal::Type::string:
        return llvm::ConstantExpr::getBitCast(builder.CreateGlobalString(lit.value, ".str"),
            llvm::Type::getInt8PtrTy(context));
    case Literal::Type::integer:
        // TODO: hex literals
        return llvm::ConstantInt::get(static_cast<llvm::IntegerType*>(types[type::integer]),
            lit.value, 10);
    case Literal::Type::rational:
        return llvm::ConstantFP::get(types[type::float64], lit.value);
    }
    return error("invalid literal type");
}

std::vector<llvm::Value*> code_gen_args(const std::vector<Expression>& args) {	
    std::vector<llvm::Value*> val_args;	
    std::transform(args.cbegin(), args.cend(),	
                   std::back_inserter(val_args),
                   [] (auto& x) { return code_gen(x); } );	
    return val_args;
}

llvm::Value* code_gen(const Variable& variable) {
    // Look this variable up in the function.
    llvm::Value *v = symbol_table.find(variable);
    if (!v) {
        return error("unknown variable \"" + variable.name + "\"");
    }

    // Load the value.
    return builder.CreateLoad(v, variable.name.c_str());
}

llvm::Value* address(const Variable& variable) {
    // Look this variable up in the function.
    llvm::Value *v = symbol_table.find(variable);
    if (!v) {
        return error("unknown variable `" + variable.name + "`");
    }

    return v;
}

llvm::Type* code_gen(const Type& type) {
    if (type.parameters.empty()) {
        return types[type];
    }

    if (type.name == "Pointer") {
        // TODO: segfaulting when params is empty
        if (type.parameters.size() != 1 || !std::holds_alternative<Type>(type.parameters[0])) {
            return (llvm::Type*) error("`Pointer` expects 1 parameter: (value type)");
        }
        // TODO: address space?
        return llvm::PointerType::getUnqual(code_gen(std::get<Type>(type.parameters[0])));
    }
    
    else if (type.name == "Array") {
        if (type.parameters.size() != 2 
            || !std::holds_alternative<Type>(type.parameters[0])
            || !std::holds_alternative<size_t>(type.parameters[1]))
        {
            std::cerr << type.parameters.size() << std::flush << std::get<Type>(type.parameters[0]).name
                << std::flush << std::get<size_t>(type.parameters[1])<< std::endl;
            return (llvm::Type*) error("`Array` expects 2 parameters: (value type, size)");
        }
        // TODO: address space?
        const Type& t = std::get<Type>(type.parameters[0]);
        size_t sz = std::get<size_t>(type.parameters[1]);
        return llvm::ArrayType::get(code_gen(t), sz);
    }
    

    return (llvm::Type*) error("bad type");
}

llvm::Value* code_gen(const Invocation& invoc) {	
    // assignment must be handles uniquely
    if (invoc.name == "<-") {
        if (invoc.args.size() != 2) {
            return error("too many arguments to assignment");
        }
        if (!std::holds_alternative<Variable>(invoc.args[0].value)) {
            return error("left-hand side of assignment must be a variable");
        }
        const Variable& var = std::get<Variable>(invoc.args[0].value);
        llvm::AllocaInst* alloca = symbol_table.find(var);
        if (!alloca) {
            return error("unknown variable " + var.name);
        }
        llvm::Value* r = code_gen(invoc.args[1]);
        builder.CreateStore(r, alloca);
        // TODO(?): forbid assignment as expression
        return alloca;
    }
    else if (invoc.name == "address") {
        if (invoc.args.size() != 1
        // TODO: address of other lvalues
            || !std::holds_alternative<Variable>(invoc.args[0].value))
        {
            return error("`address` expects 1 parameter: (variable)");
        }

        const Variable& var = std::get<Variable>(invoc.args[0].value);
        return address(var);
    }
    else if (invoc.name == "deref") {
        if (invoc.args.size() != 1) {
            return error("`deref` expects 1 parameter");
        }
        llvm::Value* v = code_gen(invoc.args[0]);

        return builder.CreateLoad(v);
    }
    else if (invoc.name == "begin") {
        if (invoc.args.size() != 1 || !std::holds_alternative<Variable>(invoc.args[0].value)) {
            return error("`begin` expects 1 parameter: (range). Only variables are currently supported, not expressions");
        }
        llvm::Value* arr = address(std::get<Variable>(invoc.args[0].value));
        
        /*
        if (!arr->getType()->isArrayTy()) {
            return error("`begin` expects 1 parameter: (range). Only array types are currently supported");
        }*/
        // returning pointer to arr still?
        //return builder.CreateGEP(arr, constant_num(0));
        // TODO: generalize for arrays of any type
        return builder.CreateBitCast(arr, llvm::Type::getInt32PtrTy(context));
    }
    else if (invoc.name == "limit") {
        if (invoc.args.size() != 1) {
            return error("`limit` expects 1 parameter: (range)");
        }
        //llvm::Value* arr = code_gen(invoc.args[0]);
        llvm::Value* arr = address(std::get<Variable>(invoc.args[0].value));
        /*
        if (!llvm::isa<llvm::ArrayType>(arr->getType())) {
            return error("`limit` expects 1 parameter: (range). Only array types are currently supported");
        }
        */
        llvm::Value* sz = constant_num(code_gen(invoc.args[0])->getType()->getArrayNumElements());
        return builder.CreateBitCast(builder.CreateGEP(arr, sz), llvm::Type::getInt32PtrTy(context));
    }
    else if (invoc.name == "successor") {
        if (invoc.args.size() != 1) {
            return error("`successor` expects 1 parameter: (iterator)");
        }
        llvm::Value* ptr = code_gen(invoc.args[0]);
        return builder.CreateGEP(ptr, constant_num(1));
    }

    if (auto it = binary_ops.find(invoc.name); it != binary_ops.end()) {
        llvm::Value *l = code_gen(invoc.args[0]);
        llvm::Value *r = code_gen(invoc.args[1]);
        if (!l || !r) { return error("bad input"); }
        return it->second(l, r);
    }

    llvm::Function *callee = module->getFunction(invoc.name);	
    if (!callee) {	
        return error("call to unknown procedure " + invoc.name);	
    }	
    // correct number of parameters
    if (!callee->isVarArg() && callee->arg_size() != invoc.args.size()) {	
        return error(invoc.name + " requires " + std::to_string(callee->arg_size())
            + " parameters, " + std::to_string(invoc.args.size()) + " given");	
    }	
    if (callee->isVarArg() && callee->arg_size() > invoc.args.size()) {
        return error(invoc.name + " requires at least" + std::to_string(callee->arg_size())
            + " parameters, " + std::to_string(invoc.args.size()) + " given");	
    }


    auto arg_vals = code_gen_args(invoc.args);	
    if (auto it = std::find(arg_vals.begin(), arg_vals.end(), nullptr);
        it != arg_vals.end()) {	
        return error("bad argument in position " + std::to_string(it - arg_vals.begin())
             + " to procedure " + invoc.name);	
    }

    return builder.CreateCall(callee, arg_vals);	
}

llvm::Value* code_gen(const Declaration& decl) {
    if (symbol_table.find_current_frame(decl.variable)) {
        return error("variable \"" + decl.variable.name + "\" is already declared in this scope");
    }
    llvm::Function* f = builder.GetInsertBlock()->getParent();

    llvm::AllocaInst* alloc = create_entry_block_alloca(f, decl);

    // store initializer, if applicable. otherwise, the value is undefined
    if (decl.initializer) {
        llvm::Value* init_val = code_gen(*decl.initializer);
        builder.CreateStore(init_val, alloc);
    }

    symbol_table.add(decl.variable, alloc);

    return alloc;
}

llvm::Value* code_gen(const Return& ret) {
    if (ret.value) {
        return builder.CreateRet(code_gen(*ret.value));
    }

    return builder.CreateRetVoid();
}

llvm::Value* code_gen(const Expression& expr) {
    return std::visit([] (auto& x) { return code_gen(x); }, expr.value);
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
llvm::Value* code_gen_current_frame(const Block& block) {
    // TODO
    llvm::Value* val = constant_num(0);
    for (const Statement& stmt : block.statements) {
        val = code_gen(stmt);
        if (!val) {
            return error("bad statement in body");	
        }
    }
    return val;
}

llvm::Value* code_gen(const Block& block) {
    symbol_table.push_frame();
    // TODO
    auto val = code_gen_current_frame(block);
    symbol_table.pop_frame();
    return val;
}

llvm::Value* code_gen(const Conditional& cond) {
    llvm::Value* condition = code_gen(cond.condition);
    if (!condition) {
        return error("bad condition");
    }

    llvm::Function* f = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* then_block = llvm::BasicBlock::Create(context, "then", f);
    llvm::BasicBlock* else_block = llvm::BasicBlock::Create(context, "else");
    llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(context, "ifcont");

    // TODO: what if there's no else block? replace else with merge?
    builder.CreateCondBr(condition, then_block, else_block);
    builder.SetInsertPoint(then_block);

    if (!code_gen(cond.then_block)) {
        return error("bad then block");
    }

    builder.CreateBr(merge_block);

    // code_gen for then block may have changed insertion block (e.g. nested if)
    then_block = builder.GetInsertBlock();

    // emit else block
    f->getBasicBlockList().push_back(else_block);
    builder.SetInsertPoint(else_block);
    
    if (!code_gen(cond.else_block)) {
        return error("bad else block");
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

llvm::Value* code_gen(const WhileLoop& loop) {
    llvm::Function* f = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* cond_block = llvm::BasicBlock::Create(context, "cond", f);
    llvm::BasicBlock* loop_block = llvm::BasicBlock::Create(context, "loop");
    llvm::BasicBlock* cont_block = llvm::BasicBlock::Create(context, "loopcont");

    builder.CreateBr(cond_block);
    builder.SetInsertPoint(cond_block);

    // emit condition block
    llvm::Value* condition = code_gen(loop.condition);
    if (!condition) {
        return error("bad condition");
    }

    builder.CreateCondBr(condition, loop_block, cont_block);

    // emit loop block
    f->getBasicBlockList().push_back(loop_block);
    builder.SetInsertPoint(loop_block);

    if (!code_gen(loop.block)) {
        return error("bad while loop body");
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
    // check for function redefinition	
    // TODO: function hoisting?
    llvm::Function *redef_check = module->getFunction(proc.name);	
    if(redef_check && !redef_check->empty()) {	
        return error("function " + proc.name + " redefined");	
    }	
    // Make the function type:  int(int...) etc.
    std::vector<llvm::Type*> param_types(proc.parameters.size());
    std::transform(proc.parameters.begin(), proc.parameters.end(),
                   param_types.begin(),
                   [](const Declaration& t) {
                       return code_gen(t.type);
                   });

    llvm::Type* ret_type = code_gen(proc.return_type);
    llvm::FunctionType* ft = llvm::FunctionType::get(ret_type, param_types, false);

    llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, proc.name, module.get());	

    // Create a new basic block to start insertion into.	
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(context, "entry", f);	
    builder.SetInsertPoint(bb);

    // include parameters in stack frame
    symbol_table.push_frame();
    // Set names for all arguments
    
    for_each_together(
        f->args().begin(), f->args().end(),
        proc.parameters.begin(),
        [f](auto& llvm_arg, const Declaration& formal_param) { 	
            llvm_arg.setName(formal_param.variable.name);	
            llvm::AllocaInst* alloc = create_entry_block_alloca(f, formal_param);
            builder.CreateStore(&llvm_arg, alloc);
            if (symbol_table.find_current_frame(formal_param.variable)) {
                // TODO
                // return error("variable " + formal_param.variable.name + " already defined in this scope");
            }
            symbol_table.add(formal_param.variable.name, alloc);
        }
    );

    if (!code_gen_current_frame(proc.block)) {
        // Error reading body, remove function.	
        f->eraseFromParent();	
        return error("could not generate procedure " + proc.name);	
    }
    symbol_table.pop_frame();

    // add implicit return at the end of void function
    if (proc.return_type == type::void0) {
        builder.CreateRetVoid();
    }

    // Validate the generated code, checking for consistency.	
    llvm::verifyFunction(*f, &llvm::errs());	

    return f;	
}

llvm::Value* code_gen(const Import& import) {
    return error("import not yet supported");
}

llvm::Value* code_gen(const Statement& stmt) {
    return std::visit([] (auto& x) { return code_gen(x); }, stmt.value);
}