#include "parse_tree3.hpp"
#include <string>
#include <vector>
#include <memory>


void check_symbols(const std::unique_ptr<ParseTree>& node, std::vector<std::string>& undefined);