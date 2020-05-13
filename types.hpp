#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include "llvm/IR/Type.h"

extern std::map<const std::string, llvm::Type*> types;

namespace type {

const std::string boolean = "Bool";

const std::string integer = "Int";
const std::string int8 = "I8";
const std::string int16 = "I16";
const std::string int32 = "I32";
const std::string int64 = "I64";

const std::string natural = "Nat"; // TODO: unsigned vs. natural
const std::string nat8 = "N8";
const std::string nat16 = "N16";
const std::string nat32 = "N32";
const std::string nat64 = "N64";

const std::string float32 = "F32";
const std::string float64 = "F64";

const std::string void0 = "Void";


// map intinsics to LLVM types

}

#endif