#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include "llvm/IR/Instructions.h"

struct SymbolTable {
    void push_frame() {
        table.emplace_back();
    }

    void pop_frame() {
        table.pop_back();
    }

    void add(const Variable& variable, llvm::AllocaInst* alloc) {
        add(variable.name, alloc);
    }
    void add(const std::string& variable_name, llvm::AllocaInst* alloc) {
        table.back()[variable_name] = alloc;
    }

    llvm::AllocaInst* find_current_frame(const Variable& variable) {
        if (auto it = table.back().find(variable.name); it != table.back().end()) {
            return it->second;
        }
        return nullptr;
    }

    llvm::AllocaInst* find(const Variable& variable) {
        auto map_it = std::find_if(table.rbegin(), table.rend(),
            [variable](auto& map) {
                return map.find(variable.name) != map.end();
            });

        if (map_it != table.rend()) {
            return map_it->find(variable.name)->second;
        }

        return nullptr;
    }
private:
    std::vector<std::unordered_map<std::string, llvm::AllocaInst*>> table;
};

#endif