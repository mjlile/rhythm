#include "parse_tree.hpp"
#include <string>
#include <vector>
#include <memory>
#include <algorithm>


void check_symbols(const std::unique_ptr<ParseTree>& node, std::vector<std::string>& undefined);