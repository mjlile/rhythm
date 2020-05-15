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
        : name_m(name), parameters_m(parameters) {}
    const auto& name()       const { return name_m; }
    const auto& parameters() const { return parameters_m; }

private:
    std::string name_m;
    // if parameters_m is nonempty, this is a type constructor
    // parameters are types of natural numbers e.g. Array(Int, 8)
    // must start with a Type
    std::vector<std::variant<Type, size_t>> parameters_m;
};


        /*------------.
        | Expressions |
        `------------*/
struct Expression;

struct Literal {
    enum Type { string, integer, rational };
    Literal(const std::string& value, Type t) : value_m(value), type_m(t) {
        if (t == Type::string) {
            // TODO: add other escape sequences
            size_t i = value_m.find("\\n"); 
            while (i != std::string::npos) {
                value_m.replace(i, 2, "\n");
                i = value_m.find("\\n"); 
            }
        }
    }
    const auto& value() const { return value_m; }
    Type        type()  const { return type_m; }
private:
    std::string value_m;
    Type type_m;
};

struct Variable {
    Variable(const std::string& name) : name_m(name) {}
    const auto& name() const { return name_m; }
private:
    std::string name_m;
};

struct Invocation {
    Invocation(const std::string& name, const std::vector<Expression> args)
             : name_m(name), args_m(args) {}
    const std::string& name()             const { return name_m; }
    const std::vector<Expression>& args() const { return args_m; }
private:
    std::string name_m;
    std::vector<Expression> args_m;
};

struct Expression {
    Expression(const Literal& expr)    : value_m(expr) {}
    Expression(const Variable& expr)   : value_m(expr) {}
    Expression(const Invocation& expr) : value_m(expr) {}

    const auto& value() const { return value_m; }
private:
    std::variant<Literal, Variable, Invocation> value_m;
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
              : type_m(type), variable_m(variable), initializer_m(std::nullopt) {}
    Declaration(const Type& type, const Variable& variable,
                const Expression& initializer)
              : type_m(type), variable_m(variable), initializer_m(initializer) {}
              
    const auto& type()        const { return type_m; }
    const auto& variable()    const { return variable_m; }
    const auto& initializer() const { return initializer_m; }
private:
    Type type_m;
    Variable variable_m;
    std::optional<Expression> initializer_m;
};

struct Import {
    Import(const std::string& identifier) : identifier_m(identifier) {}
    const auto& identifier() const { return identifier_m; }
private:
    std::string identifier_m;
};

struct Conditional {
    // todo: move constructor for vector
    Conditional(const Expression& condition, const Block& then_block)
              : Conditional(condition, then_block, Block()) {}
    // TODO: else block parsing
    Conditional(const Expression& condition, const Block& then_block,
                const Block& else_block)
              : condition_m(condition), then_m(then_block), else_m(else_block) {}
    const auto& condition()  const { return condition_m; }
    const auto& then_block() const { return then_m; }
    const auto& else_block() const { return else_m; }
private:
    Expression condition_m;
    Block then_m;
    Block else_m;
};

struct WhileLoop {
    WhileLoop(const Expression& condition, const Block& block)
            : condition_m(condition), block_m(block) {}
    const auto& condition() const { return condition_m; }
    const auto& block()   const { return block_m; }
private:
    Expression condition_m;
    Block block_m;
};

struct Procedure {
    Procedure(const std::string& name,
              const std::vector<Declaration>& parameters,
              const Type& return_type,
              const Block& block)
            : name_m(name), return_type_m(return_type),
              parameters_m(parameters), block_m(block) {}

    const auto& name()        const { return name_m; }
    const auto& return_type() const { return return_type_m; }
    const auto& parameters()  const { return parameters_m; }
    const auto& block()       const { return block_m; }

private:
    std::string name_m;
    Type return_type_m;
    std::vector<Declaration> parameters_m;
    Block block_m;
};

struct Return {
    Return(const Expression& value) : value_m(value) {}
    Return()                        : value_m(std::nullopt) {}
    const auto& value() const { return value_m; }
private:
    std::optional<Expression> value_m;
};

struct Statement {
    Statement(const Expression&  value) : value_m(value) {}
    Statement(const Declaration& value) : value_m(value) {}
    Statement(const Import&      value) : value_m(value) {}
    Statement(const Conditional& value) : value_m(value) {}
    Statement(const WhileLoop&   value) : value_m(value) {}
    Statement(const Procedure&   value) : value_m(value) {}
    Statement(const Return&      value) : value_m(value) {}

    const auto& value() const { return value_m; }
private:
    std::variant<Expression,
                 Declaration,
                 Import,
                 Conditional,
                 WhileLoop,
                 Procedure,
                 Return      > value_m;
};
