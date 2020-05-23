#include "parse_tree.hpp"

std::map<std::string, Declaration> variable_definitions;
std::map<std::string, std::vector<Procedure>> procedure_definitions;

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

bool operator==(const TypeCast& lhs, const TypeCast& rhs) {
    return lhs.type == rhs.type &&
          *lhs.expr == *rhs.expr;
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

bool operator==(const Typedef& lhs, const Typedef& rhs) {
    return lhs.name == rhs.name &&
           lhs.type == rhs.type;
}

bool operator==(const Statement& lhs, const Statement& rhs) {
    return lhs.value == rhs.value;
}

// inequality
bool operator!=(const Type& lhs, const Type& rhs) {
    return !(lhs == rhs);
}

// ordering for std::map
bool operator<(const Type& lhs, const Type& rhs) {
    return lhs.name < rhs.name || (lhs.name == rhs.name && lhs.parameters < rhs.parameters);
}

bool operator<(const Declaration& lhs, const Declaration& rhs) {
    return lhs.variable < rhs.variable || (lhs.variable == rhs.variable && lhs.type < rhs.type);
}

bool operator<(const Variable& lhs, const Variable& rhs) {
    return lhs.name < rhs.name;
}

// others
std::string to_string(const Type& t) {
    if (t.parameters.empty()) {
        return t.name;
    }

    std::string name = t.name + ".";
    for (const auto& p : t.parameters) {
        name += "_";
        if (std::holds_alternative<Type>(p)) {
            name += to_string(std::get<Type>(p));
        }
        else if (std::holds_alternative<Declaration>(p)) {
            name += to_string(std::get<Declaration>(p).type);
        }
        else {
            name += std::to_string(std::get<size_t>(p));
        }
    }

    name += ".";
    return name;
}