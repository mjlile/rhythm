#pragma once
// iterator class template for a generator (e.g. lexer, parser)
template<typename Generator>
// Generator must have peek, get, and at_end functions
// and must have value_type typedef
struct gen_iterator {
    using iterator_category = std::input_iterator_tag;
    using difference_type = ptrdiff_t;
    using size_type = size_t;
    using value_type = typename Generator::value_type;
    // reference/pointer not actually a reference/pointer, doesn't store values
    using pointer = typename Generator::value_type*;
    using reference = typename Generator::value_type&;

    // construct begin itetator
    gen_iterator(Generator& gen) : gen_ptr(&gen) {}
    // default constructor creates end iterator
    gen_iterator() : gen_ptr(nullptr) {}
    gen_iterator(const gen_iterator& other) = default;
    gen_iterator& operator=(const gen_iterator& other) = default;

    // friend functions defined inside according to Dan Saks'
    // "Making New Friends" idiom
    friend void swap(gen_iterator& lhs, gen_iterator& rhs) {
        using std::swap;
        swap(lhs.gen_ptr, rhs.gen_ptr);
    }
    friend bool operator==(const gen_iterator& lhs, const gen_iterator& rhs) {
        return lhs.gen_ptr == rhs.gen_ptr;
    }
    friend bool operator!=(const gen_iterator& lhs, const gen_iterator& rhs) {
        return !(lhs == rhs);
    }

    // input iterator operations
    gen_iterator& operator++() {
        gen_ptr->next();

        if (gen_ptr->at_end()) {
            *this = gen_ptr->end();
        }
        return *this;
    }

    gen_iterator operator++(int) {
        gen_iterator prev = *this;
        ++(*this);
        return prev;
    }

    // cannot write to an input iterator, so it doesn't yield a reference (TODO)
    value_type operator*() const {
        return gen_ptr->peek();
    }

    pointer operator->() const {
        return gen_ptr->peek();
    }

private:
    // non-owning pointer
    Generator* gen_ptr;
};