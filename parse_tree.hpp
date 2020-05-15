#pragma once
#include <cstdint>
#include <variant>
#include <optional>
#include <vector>
#include <memory>

// Others
// ======
struct Type {
    Type(const std::string& name) : Type(name, {}) {}
    Type(const std::string& name, const std::vector<std::variant<Type, size_t>>& parameters)
        : name(name), parameters(parameters) {}


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
    enum Type { string, integer, rational };
    Literal(const std::string& val, Type t) : value(val), type(t) {
        if (t == Type::string) {
            // TODO: add other escape sequences
            size_t i = value.find("\\n"); 
            while (i != std::string::npos) {
                value.replace(i, 2, "\n");
                i = value.find("\\n"); 
            }
        }
    }

    std::string value;
    Type type;
};

struct Variable {
    Variable(const std::string& name) : name(name) {}
    
    std::string name;
};

struct Invocation {
    Invocation(const std::string& name, const std::vector<Expression> args)
             : name(name), args(args) {}

    std::string name;
    std::vector<Expression> args;
};

struct Expression {
    Expression(const Literal& expr)    : value(expr) {}
    Expression(const Variable& expr)   : value(expr) {}
    Expression(const Invocation& expr) : value(expr) {}

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
    Declaration(const Type& type, const Variable& variable)
              : type(type), variable(variable), initializer(std::nullopt) {}
    Declaration(const Type& type, const Variable& variable,
                const Expression& initializer)
              : type(type), variable(variable), initializer(initializer) {}
              
    Type type;
    Variable variable;
    std::optional<Expression> initializer;
};

struct Import {
    Import(const std::string& identifier) : identifier(identifier) {}

    std::string identifier;
};

struct Conditional {
    // todo: move constructor for vector
    Conditional(const Expression& condition, const Block& then_block)
              : Conditional(condition, then_block, Block()) {}
    // TODO: else block parsing
    Conditional(const Expression& condition, const Block& then_block,
                const Block& else_block)
              : condition(condition), then_block(then_block), else_block(else_block) {}

    Expression condition;
    Block then_block;
    Block else_block;
};

struct WhileLoop {
    WhileLoop(const Expression& condition, const Block& block)
            : condition(condition), block(block) {}

    Expression condition;
    Block block;
};

struct Procedure {
    Procedure(const std::string& name,
              const std::vector<Declaration>& parameters,
              const Type& return_type,
              const Block& block)
            : name(name), return_type(return_type),
              parameters(parameters), block(block) {}

    std::string name;
    Type return_type;
    std::vector<Declaration> parameters;
    Block block;
};

struct Return {
    Return(const Expression& value) : value(value) {}
    Return()                        : value(std::nullopt) {}

    std::optional<Expression> value;
};

struct Statement {
    Statement(const Expression&  value) : value(value) {}
    Statement(const Declaration& value) : value(value) {}
    Statement(const Import&      value) : value(value) {}
    Statement(const Conditional& value) : value(value) {}
    Statement(const WhileLoop&   value) : value(value) {}
    Statement(const Procedure&   value) : value(value) {}
    Statement(const Return&      value) : value(value) {}

    std::variant<Expression,
                 Declaration,
                 Import,
                 Conditional,
                 WhileLoop,
                 Procedure,
                 Return      > value;
};
