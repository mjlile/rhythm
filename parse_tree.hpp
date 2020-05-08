#pragma once
#include <cstdint>
#include <variant>
#include <optional>
#include <vector>
#include <memory>

        /*------------.
        | Expressions |
        `------------*/
struct Expression;

struct Literal {
    enum Type { string, integer, rational };
    Literal(const std::string& value, Type t) : value_m(value) {}
    const auto& value() const { return value_m; }
    Type        type()  const { return type_m; }
private:
    std::string value_m;
    Type type_m;
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
    using Variable = std::string;
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

struct Declaration {
    Declaration(const std::string& type, const std::string& variable)
              : type_m(type), variable_m(variable), initializer_m(std::nullopt) {}
    Declaration(const std::string& type, const std::string& variable,
                const Expression& initializer)
              : type_m(type), variable_m(variable), initializer_m(initializer) {}
              
    const auto& type()        const { return type_m; }
    const auto& variable()    const { return variable_m; }
    const auto& initializer() const { return initializer_m; }
private:
    std::string type_m;
    std::string variable_m;
    std::optional<Expression> initializer_m;
};

struct Import {
    Import(const std::string& identifier) : identifier_m(identifier) {}
    const std::string& identifier() const { return identifier_m; }
private:
    std::string identifier_m;
};

struct Conditional {
    // todo: move constructor for vector
    Conditional(const Expression& condition, const std::vector<Statement>& then_block)
              : Conditional(condition, then_block, std::vector<Statement>()) {}
    // TODO: else block parsing
    Conditional(const Expression& condition, const std::vector<Statement>& then_block,
                const std::vector<Statement>& else_block)
              : condition_m(condition), then_m(then_block), else_m(else_block) {}
    const auto& condition() const { return condition_m; }
    const auto& then_block()     const { return then_m; }
    const auto& else_block()     const { return else_m; }
private:
    Expression condition_m;
    std::vector<Statement> then_m;
    std::vector<Statement> else_m;
};

struct ConditionalLoop {
    ConditionalLoop(const Conditional& conditional)
                  : conditional_m(conditional) {}
    const auto& conditional() const { return conditional_m; }
private:
    Conditional conditional_m;
};

struct Procedure {
    Procedure(const std::string& name,
              const std::vector<Declaration>& parameters,
              const std::string& return_type,
              const std::vector<Statement>& block)
            : name_m(name), return_type_m(return_type),
              parameters_m(parameters), block_m(block) {}

    const auto& name()        const { return name_m; }
    const auto& return_type() const { return return_type_m; }
    const auto& parameters()  const { return parameters_m; }
    const auto& block()       const { return block_m; }

private:
    std::string name_m;
    std::string return_type_m;
    std::vector<Declaration> parameters_m;
    std::vector<Statement> block_m;
};

struct Return {
    Return(const Expression& value) : value_m(value) {}
    Return()                        : value_m(std::nullopt) {}
    const auto& value() const { return value_m; }
private:
    std::optional<Expression> value_m;
};

struct Statement {
    using variant = 
    std::variant<Expression,
                 Declaration,
                 Import,
                 Conditional,
                 ConditionalLoop,
                 Procedure,
                 Return>;
    //             /*
    Statement(const Expression&      value) : value_m(value) {}
    Statement(const Declaration&     value) : value_m(value) {}
    Statement(const Import&          value) : value_m(value) {}
    Statement(const Conditional&     value) : value_m(value) {}
    Statement(const ConditionalLoop& value) : value_m(value) {}
    Statement(const Procedure&       value) : value_m(value) {}
    Statement(const Return&          value) : value_m(value) {}
    //*/
    //Statement(const variant& value) : value_(value) {}

    const auto& value() const { return value_m; }
private:
    variant value_m;
};