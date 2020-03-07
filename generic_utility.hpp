#pragma once
#include <algorithm>

template<typename T, typename V> // V is a variant with possible type T
std::optional<T> get_opt(const V& variant) {
    if (std::holds_alternative<T>(variant)) {
        return std::get<T>(variant);
    }
    return std::nullopt;
}

template<typename I1, typename I2, typename BP> 
// I1, I2 model InputIterator, BP models BinaryProcedure
// pre: the range beginning with first two is no longer than [first1, limit1).
//      proc takes arguments of type value_type(I1), value_type(I2)
I2 for_each_together(I1 first1, I1 limit1, I2 first2, BP proc) {
    while (first1 != limit1) {
        proc(*first1++, *first2++);
    }
    return first2;
}

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