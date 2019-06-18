#include "token.h"

std::ostream& operator<< (std::ostream& os, const Token& obj) {
    os << obj.get_lexeme() << " (" << (int)obj.get_type() << ") at line " << obj.get_line();
    return os;
}