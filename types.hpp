#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <map>
#include "parse_tree.hpp"
#include "llvm/IR/Type.h"

extern std::map<Type, llvm::Type*> types;

namespace type {

const Type boolean  = Type{"Bool"};

const Type integer  = Type{"Int"};
const Type int8     = Type{"I8"};
const Type int16    = Type{"I16"};
const Type int32    = Type{"I32"};
const Type int64    = Type{"I64"};

const Type natural  = Type{"Nat"}; // TODO: unsigned vs. natural
const Type nat8     = Type{"N8"};
const Type nat16    = Type{"N16"};
const Type nat32    = Type{"N32"};
const Type nat64    = Type{"N64"};

const Type float32  = Type{"F32"};
const Type float64  = Type{"F64"};

const Type void0    = Type{"Void"};



}

#endif