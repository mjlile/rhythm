#pragma once
#include <algorithm>

template<typename Container, typename Value>
bool contains(const Container& c, const Value& v) {
    return std::find(std::begin(c), std::end(c), v) != std::end(c);
}

// removes all elements after and including the last appearence of v
// if v does not appear, removes all elements
template<typename Container, typename Value>
bool remove_end(Container& c, const Value& v) {
    while (!c.empty() && c.back() != v) { c.pop_back(); }
    if (!c.empty()) { c.pop_back(); }
}