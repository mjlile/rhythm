#pragma once
#include <cstdint>
#include <variant>
#include <optional>
#include <vector>
#include <memory>

// Others
// ======
struct Type {
    std::string name;
    // if parameters is nonempty, this is a type constructor
    // parameters are types of natural numbers e.g. Array(Int, 8)
    // must start with a Type
    std::vector<std::variant<Type, size_t>> parameters;
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

struct Expression {
    std::variant<Literal, Variable, Invocation> value;
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

struct Statement {
    std::variant<Expression, Declaration, Import, 
                 Conditional, WhileLoop, Procedure, Return> value;
};
