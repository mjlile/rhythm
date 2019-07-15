#include "ParseTree.hpp"
#include <iomanip>
#include <map>

int level = 0;
const int space_per_level = 2;
using PTT = ParseTree::Type;
const std::map<PTT, const char*> type_names {
    {PTT::Block, "block"},
    {PTT::Expression, "expr"},
    {PTT::Declaration, "decl"},
    {PTT::Assignment, "assign"},
    {PTT::Conditional, "if"},
    {PTT::While, "while"},
    {PTT::Return, "return"},
    {PTT::Group, "group"},
    {PTT::Operator, "op"},
    {PTT::Literal, "literal"},
    {PTT::Identifier, "ident"},
    {PTT::Type, "type"}
};

void print_level(std::ostream& os) {
    for (int i = 0; i < level; ++i) {
        if (i % space_per_level == 0) {
            os << '.';
        }
        else {
            os << ' ';
        }
    }
}

std::ostream& operator<<(std::ostream& os, const ParseTree& pt) {
    //os << std::setw(level);
    print_level(os);

    os << type_names.find(pt.type)->second;
    if (!pt.value.empty()) {
        os << ", " << '\'' << pt.value << "\'";
    }
    if (pt.token != -1) {
        os << ", " << pt.token;
    }
    level += space_per_level;
    for (auto&& child : pt.children) {
        os << std::endl << *child;
    }
    level -= space_per_level;

    return os;
}