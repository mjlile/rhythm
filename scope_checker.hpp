#include <memory>
#include <vector>
#include "parse_tree.hpp"
#include"generic_utility.hpp"


namespace details {

    bool is_block(const std::unique_ptr<ParseTree>& node) {
        return node->get_type() == ParseTree::Type::Block;
    }
    
    template<typename T, typename ItemManager>
    void check_scope_impl(const std::unique_ptr<ParseTree>& node, // ...
            std::vector<T>& undefined, const T& block_sentinel, // ...
            ItemManager manage_items, std::vector<T>& in_scope_items)
    {
        manage_items(node, undefined, in_scope_items);

        // mark the start of a block so we can remove out-of-scope symbols
        if (is_block(node)) {
            in_scope_items.push_back(block_sentinel);
        }

        // check if all children are defined
        for (const auto& child : node->get_children()) {
            check_scope_impl(child, undefined, block_sentinel, manage_items, in_scope_items);
        }

        // remove out of scope symbols
        if (is_block(node)) {
            remove_end(in_scope_items, block_sentinel);
        }
    }
}


template<typename T, typename ItemManager>
void check_scope(const std::unique_ptr<ParseTree>& node, // ...
        std::vector<T>& undefined, const T& block_sentinel, // ...
        const ItemManager& manage_items)
{
    std::vector<T> in_scope_items;
    details::check_scope_impl(node, undefined, block_sentinel, // ...
            manage_items, in_scope_items);
}