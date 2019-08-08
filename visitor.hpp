#pragma once
#include "parse_tree2.hpp"

struct Visitor {
    virtual void visit(ParseTree& node) = 0;
};