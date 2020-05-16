#include "parse_tree.hpp"

// equality
bool operator==(const Type& lhs, const Type& rhs) {
    return lhs.name == rhs.name;
}

bool operator==(const Literal& lhs, const Literal& rhs) {
    return lhs.type == rhs.type &&
           lhs.value == rhs.value;
}

bool operator==(const Variable& lhs, const Variable& rhs) {
    return lhs.name == rhs.name;
}

bool operator==(const Invocation& lhs, const Invocation& rhs) {
    return lhs.name == rhs.name &&
           lhs.args == rhs.args;
}

bool operator==(const Expression& lhs, const Expression& rhs) {
    return lhs.value == rhs.value;
}

bool operator==(const Block& lhs, const Block& rhs) {
    return lhs.statements == rhs.statements;
}

bool operator==(const Declaration& lhs, const Declaration& rhs) {
    return lhs.variable == rhs.variable &&
           lhs.type == rhs.type &&
           lhs.initializer == rhs.initializer;
}

bool operator==(const Import& lhs, const Import& rhs) {
    return lhs.identifier == rhs.identifier;
}

bool operator==(const Conditional& lhs, const Conditional& rhs) {
    return lhs.condition == rhs.condition &&
           lhs.then_block == rhs.then_block &&
           lhs.else_block == rhs.else_block;
}

bool operator==(const WhileLoop& lhs, const WhileLoop& rhs) {
    return lhs.condition == rhs.condition &&
           lhs.block == rhs.block;
}

bool operator==(const Procedure& lhs, const Procedure& rhs) {
    return lhs.name == rhs.name &&
           lhs.parameters == rhs.parameters &&
           lhs.return_type == rhs.return_type &&
           lhs.block == rhs.block;
}

bool operator==(const Return& lhs, const Return& rhs) {
    return lhs.value == rhs.value;
}

bool operator==(const Statement& lhs, const Statement& rhs) {
    return lhs.value == rhs.value;
}


// ordering for std::map
bool operator<(const Type& lhs, const Type& rhs) {
    return lhs.name < rhs.name || (lhs.name == rhs.name && lhs.parameters < rhs.parameters);
}