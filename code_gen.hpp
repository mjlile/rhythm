#include <iostream>
#include <algorithm>
#include "llvm/ADT/APFloat.h"
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
#include "parser.hpp"

static llvm::LLVMContext context;
static llvm::IRBuilder<> builder(context);
static std::unique_ptr<llvm::Module> module;
static std::map<std::string, llvm::Value *> named_values;
constexpr auto num_int_bits = 64;
constexpr auto number_base = 10;


void log_error(const std::sting& str) {
    std::cerr << str << std::endl;
}

llvm::Value *log_error_v(const char *Str) {
    log_error(Str);
    return nullptr;
}

llvm::Value* code_gen(std::nullptr_t) {
    return nullptr;
}

llvm::Value* code_gen(const Literal& literal) {
    switch (literal.type()) {
    case Literal::integer:
        // TODO: construct directly from string for arbitrary precision
        llvm::APFloat val(std::stof(literal.value()))
        return llvm::ConstantFP::get(context, val);
    case Literal::rational:
        llvm::APInt val(num_int_bits, StringRef(literal.value()), number_base);
        return llvm::ConstantInt::get(context, val);
    case Literal::string:
        return log_error_v("unimplemented string literals")
    default:
        return log_error_v("unknown literal type "s + literal.type());
    } 
}

llvm::Value* code_gen_add(llvm::Value* lhs, llvm::Value* rhs) {
    if (!lhs || !rhs) {
        return nullptr;
    }
    return llvm::BinaryOperator::Create(llvm::Instruction::Add, lhs, 
                                        rhs, "", context.currentBlock());
}

std::vector<llvm::Value*> code_gen_args(const std::vector<Expression>& args) {
    std::vector<llvm::Value*> val_args;
    std::transform(args.cbegin(), args.cend(),
                   std::back_insertor(val_args),
                   [](const auto& e){ return code_gen(e); });
    return val_args;
}


llvm::Value* code_gen(const Invocation& invoc) {
    Function *callee = module->getFunction(invoc.name());
    if (!callee) {
        return log_error_v("Call to unknown procedure "s + invoc.name());
    }
    if (callee->arg_size() != invoc.args().size()) {
        return log_error_v(invoc.name() + " requires "s + callee->arg_size()
                           + " arguments, "s + invoc.args().size() + " given"s);
    }

    auto arg_vals = code_gen_args(invoc.args());
    if (std::count(eval_args.begin(), eval_args.end(), nullptr)) {
        // error in code_gen for expressions, error already logged
        return nullptr;
    }

    return builder.CreateCall(callee, arg_vals, "calltmp");
}

llvm::Value* code_gen(const std::string& variable) {
    return log_error_v();
}

llvm::Value* code_gen(const Expression& expr) {
    llvm::Value* result;
    std::visit([&result](auto const& value){ result = code_gen(value); }, expr.value());
    return result;
}

llvm::Value* code_gen(const Declaration& stmt) {
 
}

llvm::Value* code_gen(const Statement& stmt) {
    llvm::Value* result;
    std::visit([&result](auto const& value){ result = code_gen(value); }, stmt.value());
    return result;
}

std::optional<llvm::Instruction::BinaryOps>
get_binary_instr(int op) {
    llvm::Instruction::BinaryOps instr;
    switch (op) {
    case TOKEN_PLUS:
        instr = llvm::Instruction::Add;
    case TOKEN_MINUS:
        instr = llvm::Instruction::Sub;
    case TOKEN_SLASH:
        instr = llvm::Instruction::SDiv;
    case TOKEN_STAR:
        instr = llvm::Instruction::Mul;
    default:
        return std::nullopt;
    }
}

llvm::Value* code_gen_binary_op(ParseTree* node) {
    auto L = code_gen(node->observe_child(0));
    auto R = code_gen(node->observe_child(1));
    int op = node->get_token();
    if (!L || !R)
        return nullptr;

    auto instr = get_binary_instr(op);
    if (!instr) {
        return log_error_value("invalid binary operator");
    }

    return llvm::BinaryOperator::Create(instr, L, 
        R, "", context.currentBlock());
}

llvm::Value* code_gen_op(ParseTree* node) {
    
}

llvm::Value *CallExprAST::codegen() {
  // Look up the name in the global module table.
  llvm::Function *CalleeF = module->getllvm::Function(Callee);
  if (!CalleeF)
    return log_error_value("Unknown function referenced");

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return log_error_value("Incorrect # arguments passed");

  std::vector<llvm::Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }

  return builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Function *PrototypeAST::codegen() {
  // Make the function type:  double(double,double) etc.
  std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(context));
  llvm::FunctionType *FT =
      llvm::FunctionType::get(Type::getDoubleTy(context), Doubles, false);

  llvm::Function *F =
      llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, module.get());

  // Set names for all arguments.
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  return F;
}

llvm::Function *llvm::FunctionAST::codegen() {
  // First, check for an existing function from a previous 'extern' declaration.
  llvm::Function *Thellvm::Function = module->getllvm::Function(Proto->getName());

  if (!Thellvm::Function)
    Thellvm::Function = Proto->codegen();

  if (!Thellvm::Function)
    return nullptr;

  // Create a new basic block to start insertion into.
  BasicBlock *BB = BasicBlock::Create(context, "entry", Thellvm::Function);
  builder.SetInsertPoint(BB);

  // Record the function arguments in the named_values map.
  named_values.clear();
  for (auto &Arg : Thellvm::Function->args())
    named_values[Arg.getName()] = &Arg;

  if (llvm::Value *RetVal = Body->codegen()) {
    // Finish off the function.
    builder.CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    verifyllvm::Function(*Thellvm::Function);

    return Thellvm::Function;
  }

  // Error reading body, remove function.
  Thellvm::Function->eraseFromParent();
  return nullptr;
}