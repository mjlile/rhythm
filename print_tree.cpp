#include "print_tree.hpp"
#include <iostream>

struct PrintLevel {
    friend std::ostream& operator<<(std::ostream& os, PrintLevel obj);
    int depth = 0;
};
std::ostream& operator<<(std::ostream& os, const Procedure& proc);

std::ostream& operator<<(std::ostream& os, PrintLevel obj) {
    for (int i = 0; i < obj.depth; ++i) {
        os << ".   ";
    }
    return os;
}

PrintLevel print_level;


std::ostream& operator<<(std::ostream& os, const Expression& expr);

std::ostream& operator<<(std::ostream& os, const Literal& obj) {
    os << "literal: " << obj.value();
    return os;
}

std::ostream& operator<<(std::ostream& os, const Invocation& obj) {
    os << "invocation: " << obj.name();
    ++print_level.depth;
    for (const auto& arg : obj.args()) {
        os << std::endl << print_level << arg;
    }
    --print_level.depth;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Expression& expr) {
    std::visit([&os](auto const& e){ os << e; }, expr.value());
    return os;
}




std::ostream& operator<<(std::ostream& os, const Declaration& obj) {
    os << obj.type() << " " << obj.variable();
    if (obj.initializer()) {
        ++print_level.depth;
        os << std::endl << print_level << *obj.initializer();
        --print_level.depth;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Return& obj) {
    os << "return";
    if (obj.value()) {
        ++print_level.depth;
        os << std::endl << print_level << *obj.value();
        --print_level.depth;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Import& obj) {
    os << "import " << obj.identifier();
    return os;
}

std::ostream& operator<<(std::ostream& os, const Conditional& obj) {
    os << "condition";
    ++print_level.depth;
    os << std::endl << print_level << obj.condition();
    os << std::endl << print_level << obj.then_block();
    os << std::endl << print_level << obj.else_block();
    --print_level.depth;
    return os;
}

std::ostream& operator<<(std::ostream& os, const ConditionalLoop& obj) {
    os << "loop " << obj.conditional();
    return os;
}

std::ostream& operator<<(std::ostream& os, const Statement& stmt) {
    std::visit([&os](auto const& e){ os << e; }, stmt.value());
    return os;
}


std::ostream& operator<<(std::ostream& os, const std::vector<Statement>& block) {
    std::cout << "block";
    ++print_level.depth;
    for (const auto& stmt : block) {
        os << std::endl << print_level << stmt;
    }
    --print_level.depth;

    return os;
}


std::ostream& operator<<(std::ostream& os, const Procedure& proc) {
    std::cout << "procedure: " << proc.name() << std::endl;
    ++print_level.depth;
    for (const auto& param : proc.parameters()) {
        std::cout << print_level << param << std::endl;
    }
    std::cout << print_level << "returns: " <<  proc.return_type() << std::endl;
    std::cout << print_level << proc.block() << std::endl;
    --print_level.depth;

    return os;
}