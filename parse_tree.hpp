#pragma once
#include <cstdint>
#include <variant>
#include <optional>
#include <vector>
#include <memory>

namespace details {
    template<typename T, typename V> // V is a variant with possible type T
    std::optional<T> get_opt(const V& variant) {
        if (std::holds_alternative<T>(variant)) {
            return std::get<T>(variant);
        }
        return std::nullopt;
    }
}

        /*------------.
        | Expressions |
        `------------*/
struct Expression;

struct Literal {
    Literal(const std::string& value) : value_(value) {}
    const std::string& value() const { return value_; }
private:
    std::string value_;
};

struct Invocation {
    Invocation(const std::string& name) : name_(name) {}
    void add_arg(const Expression& arg) { args_.push_back(arg); }
    const std::string& name() const { return name_; }
    const std::vector<Expression>& args() const { return args_; }
private:
    std::string name_;
    std::vector<Expression> args_;
    
};

struct Expression {
    using Variable = std::string;
    Expression(const Literal& expr) : value_(expr) {}
    Expression(const Variable& expr) : value_(expr) {}
    Expression(const Invocation& expr) : value_(expr) {}

    const auto& value() const { return value_; }
private:
    std::variant<Literal, Variable, Invocation> value_;
};


// get an optional value from the literal
/*
template<typename T, typename Node>
T* value(const Node& node) {
    if (std::holds_alternative<T*>(node.value)) {
        return std::get<T*>(node.value);
    }
    return nullptr;
}args.push_back


        /*-----------.
        | Statements |
        `-----------*/
struct Statement;

struct Declaration {
    Declaration(const std::string& type, const std::string& variable)
              : type_(type), variable_(variable) {}
    Declaration(const std::string& type, const std::string& variable,
                const Expression& initializer)
              : type_(type), variable_(variable), initializer_(initializer) {}
              
    const auto& type() const { return type_; }
    const auto& variable() const { return variable_; }
    const auto& initializer() const { return initializer_; }
private:
    std::string type_;
    std::string variable_;
    std::optional<Expression> initializer_;
};

struct Import {
    Import(const std::string& identifier) : identifier_(identifier) {}
    const std::string& identifier() const { return identifier_; }
private:
    std::string identifier_;
};

struct Conditional {
    // todo: move constructor for vector
    Conditional(const Expression& condition, std::vector<Statement> block)
              : condition_(condition), block_(block) {}
    const auto& condition() const { return condition_; }
    const auto& block() const { return block_; }
private:
    Expression condition_;
    std::vector<Statement> block_;
};

struct ConditionalLoop {
    ConditionalLoop(const Conditional& conditional)
                  : conditional_(conditional) {}
    const auto& conditional() const { return conditional_; }
private:
    Conditional conditional_;
};

struct Procedure {
    Procedure(const std::string& name,
              const std::vector<Declaration>& parameters,
              const std::string& return_type,
              const std::vector<Statement>& block)
            : name_(name), return_type_(return_type),
              parameters_(parameters), block_(block) {}

    const std::string& name() const { return name_; }
    const std::string& return_type() const { return return_type_; }
    const std::vector<Declaration>& parameters() const { return parameters_; }
    const std::vector<Statement>& block() const { return block_; }

private:
    std::string name_;
    std::string return_type_;
    std::vector<Declaration> parameters_;
    std::vector<Statement> block_;
};

struct Return {
    Return(const Expression& value) : value_(value) {}
    Return() : value_(std::nullopt) {}
    const auto& value() const { return value_; }
private:
    std::optional<Expression> value_;
};

struct Statement {
    Statement(const Expression& value) : value_(value) {}
    Statement(const Declaration& value) : value_(value) {}
    Statement(const Import& value) : value_(value) {}
    Statement(const Conditional& value) : value_(value) {}
    Statement(const ConditionalLoop& value) : value_(value) {}
    Statement(const Procedure& value) : value_(value) {}
    Statement(const Return& value) : value_(value) {}

    const auto& value() const { return value_; }
private:
    std::variant<Expression, Declaration, Import,
                 Conditional, ConditionalLoop, Procedure, Return> value_;
};