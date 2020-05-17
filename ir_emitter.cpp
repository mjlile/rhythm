#include "ir_emitter.hpp"
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
    return tmp_b.CreateAlloca(llvm_type(decl.type), 0, decl.variable.name.c_str());
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

llvm::Type* llvm_type(const Type& type) {
    if (type.parameters.empty()) {
        return types[type];
    }

    if (type.name == "Struct") {
        std::vector<llvm::Type*> types(type.parameters.size());
        std::transform(type.parameters.begin(), type.parameters.end(),
                       types.begin(),
                       [] (const auto& variant) -> llvm::Type* {
                           if (std::holds_alternative<Declaration>(variant)) {
                               return llvm_type(std::get<Declaration>(variant).type);
                           }
                           if (std::holds_alternative<Type>(variant)) {
                               return llvm_type(std::get<Type>(variant));
                           }
                           return nullptr;
                       });
        return llvm::StructType::create(context, types);
    }
    else if (type.name == "Pointer") {
        // TODO: segfaulting when params is empty
        if (type.parameters.size() != 1 || !std::holds_alternative<Type>(type.parameters[0])) {
            return (llvm::Type*) error("`Pointer` expects 1 parameter: (value type)");
        }
        // TODO: address space?
        return llvm::PointerType::getUnqual(llvm_type(std::get<Type>(type.parameters[0])));
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
        return llvm::ArrayType::get(llvm_type(t), sz);
    }
    

    return (llvm::Type*) error("bad type");
}

llvm::Value* emit_expr(const Literal& lit, bool addr) {
    if (addr) {
        return error("Literals have no address");
    }
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

llvm::Value* emit_expr(const Variable& variable, bool addr) {
    // Look this variable up in the function.
    llvm::AllocaInst *v = symbol_table.find(variable);
    if (!v) {
        return error("unknown variable \"" + variable.name + "\"");
    }
    if (addr) {
        return v;
    }

    // Load the value.
    return builder.CreateLoad(v, variable.name.c_str());
}

llvm::Value* emit_expr(const Invocation& invoc, bool addr) {	
    // assignment must be handles uniquely
    if (invoc.name == "<-") {
        if (invoc.args.size() != 2) {
            return error("too many arguments to assignment");
        }
        llvm::Value* ptr = emit_expr(invoc.args[0], true);
        if (!ptr) {
            return error("bad assignee");
        }
        
        llvm::Value* r = emit_expr(invoc.args[1]);
        if (!r) {
            return error("bad rvalue in assignment");
        }
        builder.CreateStore(r, ptr);
        // TODO (?): forbid assignment as expression
        return ptr;
    }
    else if (invoc.name == "address") {
        if (invoc.args.size() != 1) {
            return error("`address` expects 1 parameter: (variable)");
        }
        return emit_expr(invoc.args[0], true);

    }
    else if (invoc.name == "deref") {
        if (invoc.args.size() != 1) {
            return error("`deref` expects 1 parameter");
        }
        // TODO: false?
        llvm::Value* v = emit_expr(invoc.args[0]);
        if (!v) {
            return error("bad deref parameter");
        }
        if (addr) {
            return v;
        }

        return builder.CreateLoad(v);
    }
    else if (invoc.name == "begin") {
        if (invoc.args.size() != 1 || !std::holds_alternative<Variable>(invoc.args[0].value)) {
            return error("`begin` expects 1 parameter: (range). Only variables are currently supported, not expressions");
        }
        llvm::Value* arr = emit_expr(invoc.args[0], true);
        
        /*
        if (!arr->getType()->isArrayTy()) {
            return error("`begin` expects 1 parameter: (range). Only array types are currently supported");
        }*/
        // returning pointer to arr still?
        //return builder.CreateGEP(arr, builder.getInt32(0));
        // TODO: generalize for arrays of any type
        return builder.CreateBitCast(arr, llvm::Type::getInt32PtrTy(context));
    }
    else if (invoc.name == "limit") {
        if (invoc.args.size() != 1) {
            return error("`limit` expects 1 parameter: (range)");
        }
        llvm::Value* arr = emit_expr(invoc.args[0], true);
        /*
        if (!llvm::isa<llvm::ArrayType>(arr->getType())) {
            return error("`limit` expects 1 parameter: (range). Only array types are currently supported");
        }
        */
        return builder.CreateBitCast(builder.CreateGEP(arr, builder.getInt64(1)), llvm::Type::getInt32PtrTy(context));
    }
    else if (invoc.name == "successor") {
        if (invoc.args.size() != 1) {
            return error("`successor` expects 1 parameter: (iterator)");
        }

        llvm::Value* ptr = emit_expr(invoc.args[0]);
        return builder.CreateGEP(ptr, builder.getInt32(1));
    }

    if (auto it = binary_ops.find(invoc.name); it != binary_ops.end()) {
        llvm::Value *l = emit_expr(invoc.args[0]);
        llvm::Value *r = emit_expr(invoc.args[1]);
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


    std::vector<llvm::Value *> llvm_args(invoc.args.size());
    std::transform(invoc.args.cbegin(), invoc.args.cend(),	
                   llvm_args.begin(),
                   [] (auto& x) { return emit_expr(x); } );

    if (auto it = std::find(llvm_args.begin(), llvm_args.end(), nullptr);
        it != llvm_args.end()) {	
        return error("bad argument in position " + std::to_string(it - llvm_args.begin())
             + " to procedure " + invoc.name);	
    }

    return builder.CreateCall(callee, llvm_args);	
}

llvm::Value* emit_expr(const Expression& expr, bool addr) {
    return std::visit([addr] (auto& x) { return emit_expr(x, addr); }, expr.value);
}

// statements
// ----------
bool emit_stmt(const Expression& expr) {
    llvm::Value* v = std::visit([] (auto& x) { return emit_expr(x); }, expr.value);
    return v != nullptr;
}


bool emit_stmt(const Declaration& decl) {
    if (symbol_table.find_current_frame(decl.variable)) {
        error("variable \"" + decl.variable.name + "\" is already declared in this scope");
        return false;
    }
    llvm::Function* f = builder.GetInsertBlock()->getParent();

    llvm::AllocaInst* alloc = create_entry_block_alloca(f, decl);

    // store initializer, if applicable. otherwise, the value is undefined
    if (decl.initializer) {
        llvm::Value* init_val = emit_expr(*decl.initializer);
        builder.CreateStore(init_val, alloc);
    }

    symbol_table.add(decl.variable, alloc);
    return true;
}

bool emit_stmt(const Return& ret) {
    if (ret.value) {
        llvm::Value* v = emit_expr(*ret.value);
        if (!v) {
            return false;
        }
        builder.CreateRet(v);
        return true;
    }

    builder.CreateRetVoid();
    return true;
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
bool emit_stmt_current_frame(const Block& block) {
    for (const Statement& stmt : block.statements) {
        bool success = emit_stmt(stmt);
        if (!success) {
            return false;
        }
    }
    return true;
}

bool emit_stmt(const Block& block) {
    symbol_table.push_frame();
    bool success = emit_stmt_current_frame(block);
    symbol_table.pop_frame();
    return success;
}

bool emit_stmt(const Conditional& cond) {
    llvm::Value* condition = emit_expr(cond.condition);
    if (!condition) {
        error("bad condition");
        return false;
    }

    llvm::Function* f = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* then_block = llvm::BasicBlock::Create(context, "then", f);
    llvm::BasicBlock* else_block = llvm::BasicBlock::Create(context, "else");
    llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(context, "ifcont");

    // TODO: what if there's no else block? replace else with merge?
    builder.CreateCondBr(condition, then_block, else_block);
    builder.SetInsertPoint(then_block);

    if (!emit_stmt(cond.then_block)) {
        error("bad then block");
        return false;
    }

    builder.CreateBr(merge_block);

    // emit_stmt for then block may have changed insertion block (e.g. nested if)
    then_block = builder.GetInsertBlock();

    // emit else block
    f->getBasicBlockList().push_back(else_block);
    builder.SetInsertPoint(else_block);
    
    if (!emit_stmt(cond.else_block)) {
        error("bad else block");
        return false;
    }

    builder.CreateBr(merge_block);
    // emit_stmt for else block may have changed insertion block
    else_block = builder.GetInsertBlock();

    // emit merge block
    f->getBasicBlockList().push_back(merge_block);
    builder.SetInsertPoint(merge_block);

    return true;
}

bool emit_stmt(const WhileLoop& loop) {
    llvm::Function* f = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* cond_block = llvm::BasicBlock::Create(context, "cond", f);
    llvm::BasicBlock* loop_block = llvm::BasicBlock::Create(context, "loop");
    llvm::BasicBlock* cont_block = llvm::BasicBlock::Create(context, "loopcont");

    builder.CreateBr(cond_block);
    builder.SetInsertPoint(cond_block);

    // emit condition block
    llvm::Value* condition = emit_expr(loop.condition);
    if (!condition) {
        error("bad condition");
        return false;
    }

    builder.CreateCondBr(condition, loop_block, cont_block);

    // emit loop block
    f->getBasicBlockList().push_back(loop_block);
    builder.SetInsertPoint(loop_block);

    if (!emit_stmt(loop.block)) {
        error("bad while loop body");
        return false;
    }

    // always branch back to check the condition
    builder.CreateBr(cond_block);

    // continue code after while loop
    f->getBasicBlockList().push_back(cont_block);
    builder.SetInsertPoint(cont_block);

    return true;
}

bool emit_stmt(const Procedure& proc) {
    // check for function redefinition	
    // TODO: function hoisting?
    llvm::Function *redef_check = module->getFunction(proc.name);	
    if(redef_check && !redef_check->empty()) {	
        error("function " + proc.name + " redefined");	
        return false;
    }	
    // Make the function type:  int(int...) etc.
    std::vector<llvm::Type*> param_types(proc.parameters.size());
    std::transform(proc.parameters.begin(), proc.parameters.end(),
                   param_types.begin(),
                   [](const Declaration& t) {
                       return llvm_type(t.type);
                   });

    llvm::Type* ret_type = llvm_type(proc.return_type);
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

    if (!emit_stmt_current_frame(proc.block)) {
        // Error reading body, remove function.	
        f->eraseFromParent();	
        error("could not generate procedure " + proc.name);	
        return false;
    }
    symbol_table.pop_frame();

    // add implicit return at the end of void function
    if (proc.return_type == type::void0) {
        builder.CreateRetVoid();
    }

    // Validate the generated code, checking for consistency.	
    llvm::verifyFunction(*f, &llvm::errs());	

    return true;	
}

bool emit_stmt(const Import& import) {
    error("import not yet supported");
    return false;
}

bool emit_stmt(const Statement& stmt) {
    return std::visit([] (auto& x) { return emit_stmt(x); }, stmt.value);
}