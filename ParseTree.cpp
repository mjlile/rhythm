#include "ParseTree.hpp"
#include <iomanip>
#include <map>

int level = 0;
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
    {PTT::Token, "token"},
    {PTT::Literal, "literal"},
    {PTT::Identifier, "ident"},
    {PTT::Type, "type"}
};

std::ostream& operator<<(std::ostream& os, const ParseTree& pt) {
    os << std::setw(level);
    os << '(' << std::flush << type_names.find(pt.type)->second;
    if (!pt.value.empty()) {
        os << ", " << '\'' << pt.value << "\'";
    }
    if (pt.token != -1) {
        os << ", " << pt.token;
    }
    level += 2;
    for (auto&& child : pt.children) {
        os << std::endl << *child;
    }
    level -= 2;

    if (!pt.children.empty() ) { os << std::setw(level) << std::endl; }
    os << ')';
}