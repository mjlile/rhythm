#include "interpreter.hpp"
#include "generic_utility.hpp"
#include <cassert>
#include <unordered_map>


std::unordered_map<std::string, int> variables;
const std::unordered_map<std::string, int(*)(int, int)> binary_ops =
    {
        {"operator+", [](int a, int b) { return a + b; } },
        {"operator-", [](int a, int b) { return a - b; } },
        {"operator*", [](int a, int b) { return a * b; } },
        {"operator/", [](int a, int b) { return a / b; } }
    };
std::unordered_map<std::string, const Procedure*> procedures;
std::optional<int> return_register = std::nullopt;

int interpret(const Expression& expr);


int interpret(const Literal& lit) {
    // todo: handle doubles and strings
    return stoi(lit.value());
}

int interpret(const std::string& name) {
    return variables[name];
}

int interpret(const Invocation& invoc) {
    // apply predefined binary operations (e.g. +, -, etc.)
    if (auto it = binary_ops.find(invoc.name()); it != binary_ops.end()) {
        return it->second(interpret(invoc.args().at(0)), interpret(invoc.args().at(1)));
    }
    // print statement
    if (invoc.name() == "println") {
        return printf("%d\n", interpret(invoc.args().at(0)));
    }
    // assignment as invocation (for now)
    if (invoc.name() == "operator<-") {
        return variables[std::get<std::string>(invoc.args().at(0).value())] = interpret(invoc.args().at(1));
    }
    // search user defined functions (TODO: proper scoping, function hoisting)
    else if(auto it = procedures.find(invoc.name()); it != procedures.end()) {
        assert(invoc.name() == it->second->name());
        assert(invoc.args().size() == it->second->parameters().size());
        // set input variables as invocation expressions (TODO: implement scoping and shadowing)
        for_each_together(invoc.args().begin(), invoc.args().end(),
                          it->second->parameters().begin(),
                          [](const auto& a, const auto& b) {
                              variables[b.variable()] = interpret(a); //breaks scoping and shadowing
                          });
        // run procedure block
        interpret(it->second->block());
        // return value
        int ret = *return_register;
        return_register = std::nullopt;
        return ret;
    }
}

int interpret(const Expression& expr) {
    // interpret whatever is inside the expression
    return std::visit([](auto const& e){ return interpret(e); }, expr.value());
}

void interpret(const Declaration& decl) {
    if (decl.initializer()) {
        variables[decl.variable()] = interpret(*decl.initializer());
    }
}

bool interpret(const Conditional& cond) {
    if (interpret(cond.condition())) {
        interpret(cond.block());
        return true;
    }
    return false;
}

void interpret(const ConditionalLoop& cond) {
    while (interpret(cond.conditional()));
}

// define user procedure
void interpret(const Procedure& proc) {
    procedures[proc.name()] = &proc;
}

void interpret(const Return& ret) {
    *return_register = interpret(*ret.value());
}

void interpret(const Statement& stmt) {
    // interpret whatever is inside of the function
    std::visit([](auto const& s){ interpret(s); }, stmt.value());
}

void interpret(const std::vector<Statement>& statements) {
    for (auto& stmt : statements) {
        interpret(stmt);
        if (return_register) {
            return;
        }
    }
}