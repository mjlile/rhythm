#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <map>
#include "parse_tree.hpp"
#include "llvm/IR/Type.h"

extern std::map<Type, llvm::Type*> types;

namespace TypeSystem {

namespace Intrinsics {

extern const Type void0;

extern const Type boolean;

extern const Type integer; 
extern const Type int8; 
extern const Type int16;
extern const Type int32;
extern const Type int64;

extern const Type natural;
extern const Type nat8;
extern const Type nat16;
extern const Type nat32;
extern const Type nat64;

extern const Type float32;
extern const Type float64;

// names of type constructors
extern const std::string pointer;
extern const std::string array;
extern const std::string structure;

Type make_pointer  (const Type& value_type);
Type make_array    (const Type& value_type, size_t sz);
Type make_structure(const std::vector<Declaration> fields);

} // Intrinsics

Type type_of(const Expression& expr);
Type type_of(const Variable& var);
Type type_of(const Invocation& invoc);
Type type_of(const TypeCast& cast);
Type type_of(const Literal& lit);

size_t size_of(const Type& t);
Type value_type(const Type& t);
// precondition: is_structure(struct_type)
std::vector<Type> field_types(const Type& struct_type);
// precondition: is_array(array_type)
size_t num_elements(const Type& array_type);

// returns true if the procedure is intrinsic and its parameters are all intrinsic
bool is_intrinsic_op(const Invocation& invoc);

bool is_intrinsic        (const Type& t);
bool is_signed_integral  (const Type& t);
bool is_unsigned_integral(const Type& t);
bool is_integral         (const Type& t);
bool is_floating_point   (const Type& t);
bool is_pointer          (const Type& t);
bool is_array            (const Type& t);
bool is_structure        (const Type& t);
bool is_aggregate        (const Type& t);


} // TypeSystem

#endif