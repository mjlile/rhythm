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
#include "type_system.hpp"
#include "llvm_intrinsics.hpp"
#include "symbol_table.hpp"

llvm::LLVMContext context;
llvm::IRBuilder<> builder(context);
std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("rhythm", context);
// TODO: add stack frames
SymbolTable<Variable, llvm::AllocaInst> variable_table;
SymbolTable<Type, llvm::Type> type_table;
std::map<std::string, std::map<std::string, llvm::ConstantInt*>> struct_field_indices;

std::map<Type, llvm::Type*> llvm_types = {
    { TypeSystem::Intrinsics::boolean, llvm::Type::getInt8Ty   (context) },
    { TypeSystem::Intrinsics::integer, llvm::Type::getInt32Ty  (context) },
    { TypeSystem::Intrinsics::int8,    llvm::Type::getInt8Ty   (context) },
    { TypeSystem::Intrinsics::int16,   llvm::Type::getInt16Ty  (context) },
    { TypeSystem::Intrinsics::int32,   llvm::Type::getInt32Ty  (context) },
    { TypeSystem::Intrinsics::int64,   llvm::Type::getInt64Ty  (context) },
    { TypeSystem::Intrinsics::natural, llvm::Type::getInt32Ty  (context) },
    { TypeSystem::Intrinsics::nat8,    llvm::Type::getInt8Ty   (context) },
    { TypeSystem::Intrinsics::nat16,   llvm::Type::getInt16Ty  (context) },
    { TypeSystem::Intrinsics::nat32,   llvm::Type::getInt32Ty  (context) },
    { TypeSystem::Intrinsics::nat64,   llvm::Type::getInt64Ty  (context) },
    { TypeSystem::Intrinsics::float32, llvm::Type::getFloatTy  (context) },
    { TypeSystem::Intrinsics::float64, llvm::Type::getDoubleTy (context) },
    { TypeSystem::Intrinsics::void0,   llvm::Type::getVoidTy   (context) },
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

std::string decorate_name(const Invocation& invoc) {
    std::string name = invoc.name;
    for (const auto& expr : invoc.args) {
        name += "_" + to_string(TypeSystem::type_of(expr));
    }
    return name;
}

std::string decorate_name(const Procedure& proc) {
    std::string name = proc.name;
    for (const auto& decl : proc.parameters) {
        name += "_" + to_string(decl.type);
    }
    return name;
}

llvm::Type* llvm_type(const Type& type) {
    if (type.parameters.empty()) {
        llvm::Type* t = type_table.find(type);
        if (t) {
            return t;
        }
        if (auto it = llvm_types.find(type); it != llvm_types.end()) {
            return it->second;
        }
        
        return (llvm::Type*) error("unknown type " + type.name);
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

        llvm::StructType* st = llvm::StructType::create(context, types);

        // TODO: default name for anonymous struct
        for (size_t i = 0; i < type.parameters.size(); ++i) {
            if (!std::holds_alternative<Declaration>(type.parameters[i])) {
                return nullptr;
            }
            const std::string& name = std::get<Declaration>(type.parameters[i]).variable.name;
            struct_field_indices[st->getName().str()][name] = builder.getInt32(i);
        }

        return st;
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
        return llvm::ConstantInt::get(static_cast<llvm::IntegerType*>(llvm_types.at(TypeSystem::Intrinsics::integer)),
            lit.value, 10);
    case Literal::Type::rational:
        return llvm::ConstantFP::get(llvm_types.at(TypeSystem::Intrinsics::float64), lit.value);
    }
    return error("invalid literal type");
}

llvm::Value* emit_expr(const Variable& variable, bool addr) {
    // Look this variable up in the function.
    llvm::AllocaInst *v = variable_table.find(variable);
    if (!v) {
        return error("unknown variable `" + variable.name + "`");
    }
    if (addr) {
        return v;
    }

    // Load the value.
    return builder.CreateLoad(v, variable.name.c_str());
}

llvm::Value* emit_expr(const Invocation& invoc, bool addr) {	
    // assignment must be handled uniquely
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
    else if (invoc.name == ".") {
        if (invoc.args.size() != 2) {
            return error("too many arguments to field access");
        }
        if (!std::holds_alternative<Variable>(invoc.args[1].value)) {
            return error("invalid field name");
        }
        const std::string& field_name = std::get<Variable>(invoc.args[1].value).name;
        llvm::Value* struct_ptr = emit_expr(invoc.args[0], true);
        if (!struct_ptr) {
            return error("bad struct");
        }
        //return error("field access not supported");
        // TODO access other fields besides first
        auto at = llvm::cast<llvm::ArrayType>(struct_ptr->getType());
        auto st = llvm::cast<llvm::StructType>(at->getElementType());

        auto it1 = struct_field_indices.find(st->getName().str());
        if (it1 == struct_field_indices.end()) {
            return error("no such struct type");
        }
        const auto& field_indices = it1->second;
        auto it2 = field_indices.find(field_name);
        if (it2 == field_indices.end()) {
            return error("no such field");
        }
        llvm::ConstantInt* field_index = it2->second;
        std::vector<llvm::Value*> indices = {
            builder.getInt64(0), field_index
        };
        llvm::Value* field_ptr = builder.CreateGEP(struct_ptr, llvm::ArrayRef(indices));
        if (addr) {
            return field_ptr;
        }

        return builder.CreateLoad(field_ptr);
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
        if (invoc.args.size() != 1) {
            return error("`begin` expects 1 parameter: (range)");
        }
        llvm::Value* arr = emit_expr(invoc.args[0], true);
        llvm::Type* value_type = llvm::cast<llvm::ArrayType>(arr->getType())->getElementType()->getArrayElementType();
        // TODO: generalize for arrays of any type
        return builder.CreateBitCast(arr, llvm::PointerType::getUnqual(value_type));
    }
    else if (invoc.name == "limit") {
        if (invoc.args.size() != 1) {
            return error("`limit` expects 1 parameter: (range)");
        }
        llvm::Value* arr = emit_expr(invoc.args[0], true);
        llvm::Type* value_type = llvm::cast<llvm::ArrayType>(arr->getType())->getElementType()->getArrayElementType();

        return builder.CreateBitCast(builder.CreateGEP(arr, builder.getInt64(1)), llvm::PointerType::getUnqual(value_type));
    }
    else if (invoc.name == "successor") {
        if (invoc.args.size() != 1) {
            return error("`successor` expects 1 parameter: (iterator)");
        }
        // TODO: integral and floating point types
        llvm::Value* v = emit_expr(invoc.args[0]);
        if (TypeSystem::is_pointer(TypeSystem::type_of(invoc.args[0]))) {
            return builder.CreateGEP(v, builder.getInt32(1));
        }
        if (TypeSystem::is_integral(TypeSystem::type_of(invoc.args[0]))) {
            return builder.CreateAdd(v, builder.getInt32(1));
        }

        return error("bad type to `successor`");
    }

    // built-in op
    if (TypeSystem::is_intrinsic_op(invoc)) {
        if (invoc.args.size() == 1) {
            llvm::Value* v = emit_expr(invoc.args[0]);
            if (!v) {
                return error("bad arguments to `" + invoc.name + "`");
            }
            return intrinsic_op(invoc, builder, v);
        }
        if (invoc.args.size() != 2) {
            return error("too many arguments to intrinsic operation");
        }
        llvm::Value *lhs = emit_expr(invoc.args[0]);
        llvm::Value *rhs = emit_expr(invoc.args[1]);
        if (!lhs || !rhs) { return error("bad input"); }
        return intrinsic_op(invoc, builder, lhs, rhs);
    }


    // other defined function

    // TODO: extern c
    std::string name = decorate_name(invoc);
    if (TypeSystem::is_intrinsic_op(invoc) || invoc.name == "printf" || invoc.name == "scanf") {
        name = invoc.name;
    }
    llvm::Function *callee = module->getFunction(name);
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

llvm::Value* emit_expr(const TypeCast& cast, bool addr) {
    Type from = TypeSystem::type_of(*cast.expr);
    const Type& to = cast.type;
    llvm::Value* v = emit_expr(*cast.expr, addr);
    llvm::Type* t = llvm_type(to);

    if (TypeSystem::is_unsigned_integral(from)) {
        if (TypeSystem::is_unsigned_integral(to)) {
            return builder.CreateZExtOrTrunc(v, t);
        }
        else if (TypeSystem::is_signed_integral(to)) {
            return builder.CreateZExtOrTrunc(v, t);
        }
        else if (TypeSystem::is_floating_point(to)) {
            return builder.CreateUIToFP(v, t);
        }
    }
    else if (TypeSystem::is_signed_integral(from)) {
        if (TypeSystem::is_unsigned_integral(to)) {
            return builder.CreateSExtOrTrunc(v, t); // TODO
        }
        else if (TypeSystem::is_signed_integral(to)) {
            return builder.CreateSExtOrTrunc(v, t);
        }
        else if (TypeSystem::is_floating_point(to)) {
            return builder.CreateSIToFP(v, t);
        }
    }
    else if (TypeSystem::is_floating_point(from)) {
        if (TypeSystem::is_unsigned_integral(to)) {
            return builder.CreateFPToUI(v, t);
        }
        else if (TypeSystem::is_signed_integral(to)) {
            return builder.CreateFPToSI(v, t);
        }
        else if (TypeSystem::is_floating_point(to)) {
            if (TypeSystem::size_of(from) < TypeSystem::size_of(to)) {
                return builder.CreateFPTrunc(v, t);
            }
            else {
                return builder.CreateFPExt(v, t);
            }
        }
    }

    // pointers
    if (TypeSystem::is_pointer(from)) {
        if (TypeSystem::is_pointer(to)) {
            return builder.CreateBitCast(v, t);
        }
        else if (TypeSystem::is_integral(to)) {
            return builder.CreatePtrToInt(v, t);
        }
    }
    else if (TypeSystem::is_integral(from)) {
        if (TypeSystem::is_pointer(to)) {
            return builder.CreateIntToPtr(v, t);
        }
    }

    return error("unknown conversion");
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
    if (variable_table.find_current_frame(decl.variable)) {
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

    variable_table.add(decl.variable, alloc);
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
    variable_table.push_frame();
    type_table.push_frame();
    bool success = emit_stmt_current_frame(block);
    type_table.pop_frame();
    variable_table.pop_frame();
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

    if (TypeSystem::type_of(cond.condition) == TypeSystem::Intrinsics::boolean) {
        condition = builder.CreateICmpNE(condition, builder.getInt8(0)); // convert boolean i8 to i1
    }
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
    llvm::Function *redef_check = module->getFunction(decorate_name(proc));	
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
    if (std::find(param_types.begin(), param_types.end(), nullptr) != param_types.end()) {
        return error("bas parameter type");
    }
    
    llvm::Type* ret_type = llvm_type(proc.return_type);
    if (!ret_type) {
        return error("bad return type");
    }
    llvm::FunctionType* ft = llvm::FunctionType::get(ret_type, param_types, false);

    llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, decorate_name(proc), module.get());	

    // Create a new basic block to start insertion into.	
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(context, "entry", f);	
    builder.SetInsertPoint(bb);

    // include parameters in stack frame
    variable_table.push_frame();
    type_table.push_frame();
    // Set names for all arguments
    
    for_each_together(
        f->args().begin(), f->args().end(),
        proc.parameters.begin(),
        [f](auto& llvm_arg, const Declaration& formal_param) { 	
            llvm_arg.setName(formal_param.variable.name);	
            llvm::AllocaInst* alloc = create_entry_block_alloca(f, formal_param);
            builder.CreateStore(&llvm_arg, alloc);
            if (variable_table.find_current_frame(formal_param.variable)) {
                // TODO
                // return error("variable " + formal_param.variable.name + " already defined in this scope");
            }
            variable_table.add(formal_param.variable.name, alloc);
        }
    );

    if (!emit_stmt_current_frame(proc.block)) {
        // Error reading body, remove function.	
        f->eraseFromParent();	
        error("could not generate procedure " + proc.name);	
        return false;
    }
    type_table.pop_frame();
    variable_table.pop_frame();

    // add implicit return at the end of void function
    if (proc.return_type == TypeSystem::Intrinsics::void0) {
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

bool emit_stmt(const Typedef& def) {
    if (def.type.name != "Struct") {
        error("only Structs can be typedefed currently");
        return false;
    }

    llvm::Type* t = llvm_type(def.type);
    if (!t) {
        error("could not emit type");
        return false;
    }
    llvm::StructType* st = llvm::cast<llvm::StructType>(t);
    if (!st) {
        error("could not emit type");
        return false;
    }
    st->setName(def.name);
    for (size_t i = 0; i < def.type.parameters.size(); ++i) {
            if (!std::holds_alternative<Declaration>(def.type.parameters[i])) {
                error("non-declaration in struct parameters");
                return false;
            }
            const std::string& name = std::get<Declaration>(def.type.parameters[i]).variable.name;
            struct_field_indices[st->getName().str()][name] = builder.getInt32(i);
        }
    type_table.add(Type{def.name}, t);
    return true;
}


bool emit_stmt(const Statement& stmt) {
    return std::visit([] (auto& x) { return emit_stmt(x); }, stmt.value);
}