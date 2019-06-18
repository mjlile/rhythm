#include "parser.h"
#include <initializer_list>

using namespace std;
using TT = Token::Type;

std::ostream& operator<< (std::ostream& os, const Expr& obj) {
    obj.print(os);
    return os;
}

struct TokenStream {
    TokenStream(const vector<Token>& tokens)
        : tokens(tokens), index(0) {}
    
    bool check(TT type) const {
        return !isAtEnd() && peek().get_type() == type;
    }

    Token advance() {
        if(!isAtEnd()) {
            ++index;
        }
        return previous();
    }

    Token peek() const {
        return tokens.at(index);
    }

    Token previous() const {
        return tokens.at(index - 1);
    }

    Token consume(TT type, string error_message) {
        if (check(type)) { return advance(); }
        throw error_message;
    }

    bool isAtEnd() const {
        return index >= tokens.size();
    }

private:
    const vector<Token>& tokens;
    size_t index;
};


bool match(initializer_list<TT> types, TokenStream& tstream) {
    for (auto type : types) {
        if (tstream.check(type)) {
            tstream.advance();
            return true;
        }
    }

    return false;
}

bool match(TT type, TokenStream& tstream) { match({type}, tstream); }


// grammar pattern matching
unique_ptr<Expr> expression(TokenStream& tstream);


unique_ptr<Expr> primary(TokenStream& tstream) {
    if (match({TT::Number, TT::String, TT::False, TT::True}, tstream)) {
        return make_unique<Expr>(Expr::Type::Literal, tstream.previous());
    }

    if (match(TT::LeftParen, tstream)) {
        auto expr = expression(tstream);
        tstream.consume(TT::RightParen, "Expected ')' after expression");
        auto new_expr = make_unique<Expr>(Expr::Type::Grouping);
        new_expr->addChild(move(expr));
        return new_expr;
    }

    throw "Expected expression";
}

    
unique_ptr<Expr> unary(TokenStream& tstream) {
    if (match({TT::Not, TT::Minus}, tstream)) {
        Token op = tstream.previous();
        auto right = unary(tstream);
        auto new_expr = make_unique<Expr>(Expr::Type::Unary, op);
        new_expr->addChild(move(right));
        return new_expr;
    }

    return primary(tstream);
}


unique_ptr<Expr> multiplication(TokenStream& tstream) {
    auto expr = unary(tstream);

    while (match({TT::Slash, TT::Star}, tstream)) {
        Token op = tstream.previous();
        auto right = unary(tstream);
        auto new_expr = make_unique<Expr>(Expr::Type::Binary, op);
        new_expr->addChild(move(expr));
        new_expr->addChild(move(right));
        expr = move(new_expr);
    }

    return expr;
}


unique_ptr<Expr> addition(TokenStream& tstream) {
    auto expr = multiplication(tstream);

    while (match({TT::Minus, TT::Plus}, tstream)) {
        Token op = tstream.previous();
        
        auto right = multiplication(tstream);
        auto new_expr = make_unique<Expr>(Expr::Type::Binary, op);
        new_expr->addChild(move(expr));
        new_expr->addChild(move(right));
        expr = move(new_expr);
    }

    return expr;
}


unique_ptr<Expr> comparison(TokenStream& tstream) {
    auto expr = addition(tstream);

    while (match({TT::Greater, 
                  TT::GreaterEqual,
                  TT::Less,
                  TT::LessEqual}, tstream)) {
        Token op = tstream.previous();
        auto right = addition(tstream);
        auto new_expr = make_unique<Expr>(Expr::Type::Binary, op);
        new_expr->addChild(move(expr));
        new_expr->addChild(move(right));
        expr = move(new_expr);
    }

    return expr;
}


unique_ptr<Expr> equality(TokenStream& tstream) {
    auto expr = comparison(tstream);

    while (match({TT::Equal, TT::NotEqual}, tstream)) {
        Token op = tstream.previous();
        auto right = comparison(tstream);
        auto new_expr = make_unique<Expr>(Expr::Type::Binary, op);
        new_expr->addChild(move(expr));
        new_expr->addChild(move(right));
        expr = move(new_expr);
    }

    return expr;
}

unique_ptr<Expr> expression(TokenStream& tstream) {
    return equality(tstream);
}

// takes in rvalue ref to tokens, consume vector
unique_ptr<Expr> parse(vector<Token> tokens) {
    TokenStream tstream(move(tokens));
    return expression(tstream);
}