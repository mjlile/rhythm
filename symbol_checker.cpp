#include "symbol_checker.hpp"
#include "scope_checker.hpp"
#include "generic_utility.hpp"
#include <vector>
#include <iostream>

// add variable from declaration node to symbol_table
void declare_var(const std::unique_ptr<ParseTree>& node, std::vector<std::string>& symbol_table) {
    // second child of a declaration is the name
    symbol_table.push_back(node->get_children()[1]->get_value());
}

// add variable from declaration node to symbol_table
void import_type(const std::unique_ptr<ParseTree>& node, std::vector<std::string>& symbol_table) {
    // first child of a declaration is the type
    symbol_table.push_back(node->get_children()[0]->get_value());
}

// add an identifier node to undefined if not in symbol_table
void check_definition(const std::unique_ptr<ParseTree>& node, std::vector<std::string>& symbol_table,
        std::vector<std::string>& undefined)
{
    if (!contains(symbol_table, node->get_value())) {
        undefined.push_back(node->get_value());
    }
}

void manage_symbols(const std::unique_ptr<ParseTree>& node,
        std::vector<std::string>& undefined,
        std::vector<std::string>& symbol_table)
{
    using PTT = ParseTree::Type;
    switch (node->get_type()) {
        case PTT::Identifier:
        case PTT::Type:
            check_definition(node, symbol_table, undefined);
            break;
        case PTT::Declaration:
            declare_var(node, symbol_table);
            break;
        case PTT::Import:
            import_type(node, symbol_table);
            break;
        
    }
}

void check_symbols(const std::unique_ptr<ParseTree>& node, std::vector<std::string>& undefined) {
    std::string block_sentinel = "";
    check_scope(node, undefined, block_sentinel, manage_symbols);
}