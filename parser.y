%{
    #include <iostream>
    #include <memory>
    #include <string>
    #include <map>
    #include "parse_tree.hpp"
    #include "parser.hpp"
    extern int yylex();
    int line_num = 1;
    void yyerror(const char *s) { printf("ERROR: %s (line %i)\n", s, line_num); }
    std::vector<Statement> program;

    std::map<int, std::string> op_to_string = {
        {TOKEN_PLUS, "+"},
        {TOKEN_MINUS, "-"},
        {TOKEN_STAR, "*"},
        {TOKEN_SLASH, "/"},
        {TOKEN_EQ, "="},
        {TOKEN_LT, "<"},
        {TOKEN_LE, "<="},
        {TOKEN_GT, ">"},
        {TOKEN_GE, ">="},
        {TOKEN_LARROW, "<-"},
    };

    Expression* operator_to_invocation(int op_token, Expression* expr1, Expression* expr2 = nullptr) {
        // C++ style operator__ e.g. operator+, operator()
        std::string name = "operator" + op_to_string[op_token];
        std::vector<Expression> args;
        args.push_back(*expr1);
        if (expr2) {
            args.push_back(*expr2);
        }
        return new Expression(Invocation(name, args));
    }
%}

%union {
    Literal* literal;
    Invocation* Invocation;
    Expression* expression;
    Declaration* declaration;
    std::vector<Declaration>* parameters;
    Import* import;
    Conditional* conditional;
    ConditionalLoop* conditional_loop;
    Procedure* procedure;
    Return* return_stmt;
    Statement* statement;
    std::vector<Statement>* block;
    std::string* string;
    int token;
}

%token <string> TOKEN_IDENT TOKEN_TYPE TOKEN_INT TOKEN_REAL TOKEN_STR

%token <token> TOKEN_EOL TOKEN_EQ TOKEN_NE TOKEN_LT TOKEN_LE TOKEN_GT
%token <token> TOKEN_GE TOKEN_LPAREN TOKEN_RPAREN TOKEN_LBRACE TOKEN_RBRACE
%token <token> TOKEN_LBRACK TOKEN_RBRACK TOKEN_LARROW TOKEN_RARROW TOKEN_DOT TOKEN_COMMA
%token <token> TOKEN_BANG TOKEN_PLUS TOKEN_MINUS TOKEN_STAR TOKEN_SLASH
/* keywords */
%token <token> TOKEN_RETURN TOKEN_IF TOKEN_THEN TOKEN_WHILE TOKEN_DO TOKEN_END
%token <token> TOKEN_AND TOKEN_OR TOKEN_PROC TOKEN_IMPORT

%type <token> or and eq relate add multiply pre post

%type <literal> literal 
%type <expression> primary postfix prefix multiplicative additive relational 
%type <expression> equality conjunction disjunction
%type <expression> expression assignment

%type <import> import
%type <declaration> declaration
%type <conditional> conditional
%type <conditional_loop> while_stmt
%type <procedure> procedure
%type <parameters> parameters decl_list
%type <return_stmt> return_stmt
%type <statement> statement control

%type <block> block statements

/* Operator precedence for mathematical operators */
%left TOKEN_PLUS TOKEN_MINUS
%left TOKEN_STAR TOKEN_SLASH

%start program

%%

program         : block
                    {
                        program = *$1;
                        std::cout << "success" << std::endl;
                    }
                ;

block           : statements
                | statement
                    {
                        $$ = new std::vector<Statement>();
                        $$->push_back(*$1);
                    }
                ;

statements      : statement eol
                    {
                        $$ = new std::vector<Statement>();
                        $$->push_back(*$1);
                    }
                | statements statement eol
                    {
                        $$ = $1;
                        $$->push_back(*$2);
                    }
                | eol { $$ = new std::vector<Statement>(); }
                ;

statement       : expression { $$ = new Statement(*$1); }
                | declaration { $$ = new Statement(*$1); }
                | assignment { $$ = new Statement(*$1); }
                | import { $$ = new Statement(*$1); }
                | control
                ;

declaration     : TOKEN_TYPE TOKEN_IDENT
                    {
                        $$ = new Declaration(*$1, *$2);
                        delete $1;
                        delete $2;
                    }
                | TOKEN_TYPE TOKEN_IDENT TOKEN_LARROW expression
                    {
                        $$ = new Declaration(*$1, *$2, *$4);
                        delete $1;
                        delete $2;
                        delete $4;
                    }
                ;

assignment      : expression TOKEN_LARROW expression
                    {
                        $$ = operator_to_invocation($2, $1, $3);
                    }
                ;

control         : return_stmt { $$ = new Statement(*$1); delete $1; }
                | conditional { $$ = new Statement(*$1); delete $1; }
                | while_stmt { $$ = new Statement(*$1); delete $1; }
                | procedure { $$ = new Statement(*$1); delete $1; }
                ;

return_stmt     : TOKEN_RETURN { $$ = new Return(); }
                | TOKEN_RETURN expression { $$ = new Return(*$2); delete $2; }
                ;

conditional     : TOKEN_IF expression TOKEN_THEN block TOKEN_END
                    {
                        $$ = new Conditional(*$2, *$4);
                        delete $2;
                        delete $4;
                    }
                ;

while_stmt      : TOKEN_WHILE expression TOKEN_DO block TOKEN_END
                    {
                        $$ = new ConditionalLoop(Conditional(*$2, *$4));
                        delete $2;
                        delete $4;
                    }
                ;

procedure       : TOKEN_PROC TOKEN_IDENT parameters TOKEN_RARROW TOKEN_TYPE block TOKEN_END
                    {
                        $$ = new Procedure(*$2, *$3, *$5, *$6);
                        delete $2;
                        delete $3;
                        delete $5;
                        delete $6;
                    }
                ;

parameters      : TOKEN_LPAREN TOKEN_RPAREN { $$ = new std::vector<Declaration>(); }
                | TOKEN_LPAREN decl_list TOKEN_RPAREN
                    {
                        $$ = $2;
                    }
                ;

decl_list       : declaration
                    {
                        $$ = new std::vector<Declaration>();
                        $$->push_back(*$1);
                        delete $1;
                    }
                | decl_list TOKEN_COMMA declaration
                    {
                        $$ = $1;
                        $$->push_back(*$3);
                        delete $3;
                    }
                ;

import          : TOKEN_IMPORT TOKEN_TYPE
                    { $$ = new Import(*$2); delete $2; }
                | TOKEN_IMPORT TOKEN_IDENT
                    { $$ = new Import(*$2); delete $2; }




expression      : disjunction
                ;

disjunction     : conjunction
                | disjunction or conjunction
                    {
                        $$ = operator_to_invocation($2, $1, $3);
                    }
                ;

conjunction     : equality
                | conjunction and equality
                    {
                        $$ = operator_to_invocation($2, $1, $3);
                    }
                ;

equality        : relational
                | equality eq relational
                    {
                        $$ = operator_to_invocation($2, $1, $3);
                    }
                ;

relational      : additive
                | relational relate additive
                    {
                        $$ = operator_to_invocation($2, $1, $3);
                    }
                ;

additive        : multiplicative
                | additive add multiplicative
                    {
                        $$ = operator_to_invocation($2, $1, $3);
                    }
                ;

multiplicative  : prefix
                | multiplicative multiply prefix
                    {
                        $$ = operator_to_invocation($2, $1, $3);
                    }
                ;

prefix          : postfix
                | pre prefix
                    {
                        $$ = operator_to_invocation($1, $2);
                    }
                ;

postfix         : primary
                | postfix post
                    {
                        $$ = operator_to_invocation($2, $1);
                    }
                ;

primary         : literal { $$ = new Expression(*$1); delete $1; }
                | TOKEN_IDENT { $$ = new Expression(*$1); delete $1; }
                | TOKEN_LPAREN expression TOKEN_RPAREN
                    {
                        $$ = operator_to_invocation($1, $2);
                    }
                ;



or : TOKEN_OR;
and : TOKEN_AND;
eq : TOKEN_EQ | TOKEN_NE;
relate : TOKEN_LT | TOKEN_LE | TOKEN_GT | TOKEN_GE;
add : TOKEN_PLUS | TOKEN_MINUS;
multiply : TOKEN_STAR | TOKEN_SLASH;
pre : TOKEN_BANG | TOKEN_MINUS | TOKEN_STAR;
post : TOKEN_DOT;


literal : TOKEN_INT
            {
                $$ = new Literal(*$1, Literal::integer);
                delete $1;
            }
        | TOKEN_REAL
            {
                $$ = new Literal(*$1, Literal::rational);
                delete $1;
            }
        | TOKEN_STR
            {
                $$ = new Literal(*$1, Literal::string);
                delete $1;
            }
        ;
eol : TOKEN_EOL { ++line_num; } | eol TOKEN_EOL { ++line_num; };
