#include "interpreter.h"
#include <unordered_map>


std::unordered_map<std::string, int> variables;

int interpret(const Expression& expr);


int interpret(const Literal& lit) {
    // todo: handle doubles and strings
    return stoi(lit.value());
}

int interpret(const std::string& name) {
    return variables[name];
}

int interpret(const Invocation& invoc) {
    if (invoc.name() == "operator+") {
        return interpret(invoc.args().at(0))
             + interpret(invoc.args().at(1));
    }
    else if (invoc.name() == "operator-") {
        return interpret(invoc.args().at(0))
             - interpret(invoc.args().at(1));
    }
    else if (invoc.name() == "operator*") {
        return interpret(invoc.args().at(0))
             * interpret(invoc.args().at(1));
    }
    else if (invoc.name() == "operator/") {
        return interpret(invoc.args().at(0))
             / interpret(invoc.args().at(1));
    }
    else if (invoc.name() == "println") {
        return printf("%d\n", interpret(invoc.args().at(0)));
    }
    else if (invoc.name() == "operator<-") {
        variables[std::get<std::string>(invoc.args().at(0).value())] = interpret(invoc.args().at(1));
    }
}
int interpret(const Expression& expr) {
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

void interpret(const Statement& stmt) {
    std::visit([](auto const& s){ interpret(s); }, stmt.value());
}

void interpret(const std::vector<Statement>& statements) {
    for (auto& stmt : statements) {
        interpret(stmt);
    }
}