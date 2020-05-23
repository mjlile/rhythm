#include "type_system.hpp"
#include <iostream>
#include <cassert>
#include <numeric>

constexpr size_t pointer_size = sizeof(int*);

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

    // TODO: improve modularity
    // c functions
    if (invoc.name == "printf" || invoc.name == "scanf") {
        return TypeSystem::Intrinsics::integer;
    }
    if (invoc.name == "deref") {
        assert(invoc.args.size() == 1);
        return TypeSystem::value_type(TypeSystem::type_of(invoc.args[0]));
    }
    if (invoc.name == "begin" || invoc.name == "limit") {
        assert(invoc.args.size() == 1);
        return TypeSystem::Intrinsics::make_pointer(TypeSystem::value_type(TypeSystem::type_of(invoc.args[0])));
    }
    if (invoc.name == "successor" || invoc.name == "predecessor") {
        return TypeSystem::type_of(invoc.args[0]);
    }

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
    assert(!it->second.empty());

    const Type* return_type_ptr = nullptr;
    for (const auto& proc : it->second) {
        if (proc.parameters.size() != input_types.size()) {
            continue;
        }
        for (size_t i = 0; i < input_types.size(); ++i) {
            if (input_types[i] != proc.parameters[i].type) {
                goto next;
            }
        }
        return_type_ptr = &proc.return_type;
        break;
    next:
    ;
    }
    if (!return_type_ptr) {
        std::cerr << "could not find matching overload for `" << invoc.name << "`" << std::endl;
        return Intrinsics::void0;
    }
    return *return_type_ptr;
}

Type type_of(const TypeCast& cast) {
    return cast.type;
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

std::map<Type, size_t> type_sizes = {
    {Intrinsics::boolean, 1},
    {Intrinsics::integer, 4},
    {Intrinsics::int8   , 1},
    {Intrinsics::int16  , 2},
    {Intrinsics::int32  , 4},
    {Intrinsics::int64  , 8},
    {Intrinsics::natural, 4},
    {Intrinsics::nat8   , 1},
    {Intrinsics::nat16  , 2},
    {Intrinsics::nat32  , 4},
    {Intrinsics::nat64  , 8},
    {Intrinsics::float32, 4},
    {Intrinsics::float64, 8}
};

size_t size_of(const Type& t) {
    if (auto it = type_sizes.find(t); it != type_sizes.end()) {
        return it->second;
    }

    if (is_pointer(t)) {
        return pointer_size;
    }

    if (is_array(t)) {
        return num_elements(t) * size_of(value_type(t));
    }

    if (is_structure(t)) {
        std::vector<Type> types = field_types(t);
        return std::accumulate(types.begin(), types.end(), 0,
                    [](size_t sz, const Type& t2) {
                        return sz + size_of(t2);
                    });
    }

    assert(false);
    return -1;
}

Type value_type(const Type& t) {
    if (is_array(t) || is_pointer(t)) {
        return std::get<Type>(t.parameters[0]);
    }

    return t;
}

std::vector<Type> field_types(const Type& struct_type) {
    assert(is_structure(struct_type));

    std::vector<Type> types(struct_type.parameters.size());
    std::transform(struct_type.parameters.begin(), struct_type.parameters.end(),
                   types.begin(),
                   [](const auto& var) {
                       return std::get<Type>(var);
                   });
    return types;
}

size_t num_elements(const Type& array_type) {
    assert(is_array(array_type));
    return std::get<size_t>(array_type.parameters[1]);
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
    return is_integral(t) || is_floating_point(t) || t == Intrinsics::boolean
        || is_pointer(t) || is_array(t);
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

bool is_unsigned_or_ptr(const Type& t) {
    return is_unsigned_integral(t) || is_pointer(t);
}

bool is_integral_or_ptr(const Type& t) {
    return is_integral(t) || is_pointer(t);
}

bool is_floating_point(const Type& t) {
    using namespace Intrinsics;
    return is_in(t, float32, float64);
}

bool is_pointer(const Type& t) { return t.name == Intrinsics::pointer; }

bool is_array(const Type& t) { return t.name == Intrinsics::array; }
bool is_structure(const Type& t) { return t.name == Intrinsics::structure; }

bool is_aggregate(const Type& t) { return is_array(t) || is_structure(t); }

}