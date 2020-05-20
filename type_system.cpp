#include "type_system.hpp"
#include <iostream>
#include <cassert>

namespace TypeSystem {

namespace Intrinsics {
const Type void0    = Type{"Void"};

const Type boolean  = Type{"Bool"};

const Type integer  = Type{"Int"};
const Type int8     = Type{"Int8"};
const Type int16    = Type{"Int16"};
const Type int32    = Type{"Int32"};
const Type int64    = Type{"Int64"};

const Type natural  = Type{"Nat"};
const Type nat8     = Type{"Nat8"};
const Type nat16    = Type{"Nat16"};
const Type nat32    = Type{"Nat32"};
const Type nat64    = Type{"Nat64"};

const Type float32  = Type{"Flt32"};
const Type float64  = Type{"Flt64"};

// names of type constructors
const std::string pointer = "Pointer";
const std::string array = "Array";
const std::string structure = "Struct";

Type make_pointer(const Type& value_type) {
    return Type{pointer, {value_type}};
}

Type make_array(const Type& value_type, size_t sz) {
    return Type{array, {value_type, sz}};
}

Type make_structure(const std::vector<Declaration>& fields) {
    std::vector<std::variant<Type, size_t, Declaration>> f(fields.size());
    std::transform(fields.begin(), fields.end(), f.begin(),
        [](const auto& decl) -> std::variant<Type, size_t, Declaration> {
            return decl;
        });
    return Type{structure, std::move(f)};
}

}


Type type_of(const Expression& expr) {
    return std::visit([](auto& v) { return type_of(v); }, expr.value);
}

Type type_of(const Variable& var) {
    auto it = variable_definitions.find(var.name);
    if (it == variable_definitions.end()) {
        std::cerr << "no such variable `" << var.name << "`" << std::endl;
        return Intrinsics::void0;
    }
    assert(it != variable_definitions.end());
    return it->second.type;
}

Type type_of(const Invocation& invoc) {
    // TODO: use input types for overloading
    std::vector<Type> input_types(invoc.args.size());
    std::transform(invoc.args.begin(), invoc.args.end(),
                   input_types.begin(),
                   [](const Expression& expr) {
                       return type_of(expr);
                   });
    if (is_intrinsic_op(invoc)) {
        return input_types.front();
    }

    auto it = procedure_definitions.find(invoc.name);
    if (it == procedure_definitions.end()) {
        std::cerr << "no such procedure `" << invoc.name << "`" << std::endl;
        return Intrinsics::void0;
    }
    assert(it != procedure_definitions.end());
    return it->second.return_type;
}

// TODO: typeless literals
Type type_of(const Literal& lit) {
    switch (lit.type) {
    case Literal::Type::integer:
        return Intrinsics::integer;
    case Literal::Type::rational:
        return Intrinsics::float64;
    case Literal::Type::string:
        // TODO: real string type
        return Intrinsics::make_pointer(Intrinsics::nat8);
    default:
        assert(false);
    }
    return Type{};
}

template<typename First, typename ... T>
bool is_in(First&& first, T&& ... t)
{
    return ((first == t) || ...);
}

bool is_intrinsic_op(const Invocation& invoc) {
    if (!is_in(invoc.name, "+", "-", "*", "/", "%", "=", "!=", "<", "<=", ">", ">=", "&&", "||")) {
        return false;
    }
    for (const auto& expr : invoc.args) {
        if (!TypeSystem::is_intrinsic(TypeSystem::type_of(expr))) {
            return false;
        }
    }
    return true;
}


bool is_intrinsic(const Type& t) {
    return is_integral(t) || is_floating_point(t) || t == Intrinsics::boolean;
}

bool is_signed_integral(const Type& t) {
    using namespace Intrinsics;
    return is_in(t, integer, int8, int16, int32, int64);
}

bool is_unsigned_integral(const Type& t) {
    using namespace Intrinsics;
    return is_in(t, natural, nat8, nat16, nat32, nat64);
}

bool is_integral(const Type& t) {
    return is_signed_integral  (t)
        || is_unsigned_integral(t);
}

bool is_floating_point(const Type& t) {
    using namespace Intrinsics;
    return is_in(t, float32, float64);
}

bool is_array(const Type& t) { return t.name == Intrinsics::array; }
bool is_structure(const Type& t) { return t.name == Intrinsics::structure; }
bool is_aggregate(const Type& t) { return is_array(t) || is_structure(t); }

}