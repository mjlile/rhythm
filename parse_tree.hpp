#ifndef PARSE_TREE_HPP
#define PARSE_TREE_HPP

#include <cstdint>
#include <variant>
#include <optional>
#include <vector>
#include <map>
#include <memory>

struct Declaration;

struct Type {
    std::string name;
    std::vector<std::variant<Type, size_t, Declaration>> parameters;
};

     /*------------.
     | Expressions |
     `------------*/
struct Expression;

struct Literal {
    std::string value;
    enum Type { string, integer, rational } type;
};

struct Variable {
    std::string name;
};

struct Invocation {
    std::string name;
    std::vector<Expression> args;
};

struct TypeCast {
    Type type;
    std::shared_ptr<Expression> expr;
};

struct Expression {
    std::variant<Literal, Variable, Invocation, TypeCast> value;
};

     /*-----------.
     | Statements |
     `-----------*/
struct Statement;

struct Block {
    std::vector<Statement> statements;
};

struct Declaration {
    Variable variable;
    Type type;
    std::optional<Expression> initializer;
};

struct Import {
    std::string identifier;
};

struct Conditional {
    Expression condition;
    Block then_block;
    Block else_block;
};

struct WhileLoop {
    Expression condition;
    Block block;
};

struct Procedure {
    std::string name;
    std::vector<Declaration> parameters;
    Type return_type;
    Block block;
};

struct Return {
    std::optional<Expression> value;
};

struct Typedef {
    std::string name;
    Type type;
};

struct Statement {
    std::variant<Expression, Declaration, Import, 
        Conditional, WhileLoop, Procedure, Return, Typedef> value;
};

extern std::map<std::string, Declaration> variable_definitions;
extern std::map<std::string, std::vector<Procedure>>  procedure_definitions;


// provide equality operator to make parse tree types regular
// --------------------------------------------------
bool operator==(const Type        & lhs, const Type        & rhs);
bool operator==(const Literal     & lhs, const Literal     & rhs);
bool operator==(const Variable    & lhs, const Variable    & rhs);
bool operator==(const Invocation  & lhs, const Invocation  & rhs);
bool operator==(const TypeCast    & lhs, const TypeCast    & rhs);
bool operator==(const Expression  & lhs, const Expression  & rhs);
bool operator==(const Block       & lhs, const Block       & rhs);
bool operator==(const Declaration & lhs, const Declaration & rhs);
bool operator==(const Import      & lhs, const Import      & rhs);
bool operator==(const Conditional & lhs, const Conditional & rhs);
bool operator==(const WhileLoop   & lhs, const WhileLoop   & rhs);
bool operator==(const Procedure   & lhs, const Procedure   & rhs);
bool operator==(const Return      & lhs, const Return      & rhs);
bool operator==(const Typedef     & lhs, const Typedef     & rhs);
bool operator==(const Statement   & lhs, const Statement   & rhs);

bool operator!=(const Type        & lhs, const Type        & rhs);
 
// ordering for std::map
bool operator<(const Type        & lhs, const Type        & rhs);
bool operator<(const Declaration & lhs, const Declaration & rhs);
bool operator<(const Variable    & lhs, const Variable    & rhs);

// convert type to string for name decoration for function overloading
std::string to_string(const Type& t);

#endif