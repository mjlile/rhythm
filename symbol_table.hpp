#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include "llvm/IR/Instructions.h"

template<typename Domain, typename Range>
// Domain must have a std::string field `name`
struct SymbolTable {
    void push_frame() {
        table.emplace_back();
    }

    void pop_frame() {
        table.pop_back();
    }

    void add(const Domain& symbol, Range* value) {
        add(symbol.name, value);
    }

    void add(const std::string& name, Range* value) {
        table.back()[name] = value;
    }

    Range* find_current_frame(const Domain& symbol) {
        return find_current_frame(symbol.name);
    }
    Range* find(const Domain& symbol) {
        return find(symbol.name);
    }

    Range* find_current_frame(const std::string& name) {
        if (auto it = table.back().find(name); it != table.back().end()) {
            return it->second;
        }
        return nullptr;
    }

    Range* find(const std::string& name) {
        auto map_it = std::find_if(table.rbegin(), table.rend(),
            [name](auto& map) {
                return map.find(name) != map.end();
            });

        if (map_it != table.rend()) {
            return map_it->find(name)->second;
        }

        return nullptr;
    }
private:
    std::vector<std::unordered_map<std::string, Range*>> table;
};

#endif