#ifndef PRINT_TREE_HPP
#define PRINT_TREE_HPP
#include "parse_tree.hpp"


std::ostream& operator<<(std::ostream& os,
                         const std::vector<Statement>& program);

#endif