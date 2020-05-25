%{
    #include <iostream>
    #include <memory>
    #include <string>
    #include <variant>
    #include <map>
    #include "parse_tree.hpp"
    #include "parser.hpp"
    #include "type_system.hpp"
    extern int yylex();
    int line_num = 1;
    void yyerror(const char *s) { printf("ERROR: %s (line %i)\n", s, line_num); }
    Block* program;

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
        {TOKEN_PERCENT, "%"},
        {TOKEN_DOT, "."},
        {TOKEN_AND, "&&"},
        {TOKEN_OR, "||"}
    };

    Expression* operator_to_invocation(int op_token, Expression* expr1, Expression* expr2 = nullptr) {
        std::string name = op_to_string[op_token];
        std::vector<Expression> args;
        args.push_back(*expr1);
        if (expr2) {
            args.push_back(*expr2);
        }
        return new Expression{Invocation{name, args}};
    }
%}

%union {
    Literal* literal;
    Invocation* invocation;
    Expression* expression;
    Declaration* declaration;
    std::vector<Declaration>* decl_list;
    std::vector<Expression>* input;
    std::variant<Type, size_t, Declaration>* type_param;
    std::vector<std::variant<Type, size_t, Declaration>>* type_param_list;
    Import* import;
    Conditional* conditional;
    WhileLoop* while_loop;
    Procedure* procedure;
    Return* return_stmt;
    Typedef* type_def;
    Statement* statement;
    Block* block;
    Type* type;
    std::string* string;
    int token;
}

%token <string> TOKEN_IDENT TOKEN_TYPE TOKEN_INT TOKEN_REAL TOKEN_STR

%token <token> TOKEN_EOL TOKEN_EQ TOKEN_NE TOKEN_LT TOKEN_LE TOKEN_GT TOKEN_AND TOKEN_OR
%token <token> TOKEN_GE TOKEN_LPAREN TOKEN_RPAREN TOKEN_LBRACE TOKEN_RBRACE
%token <token> TOKEN_LBRACK TOKEN_RBRACK TOKEN_LARROW TOKEN_RARROW TOKEN_DOT TOKEN_COMMA
%token <token> TOKEN_COLON TOKEN_BANG TOKEN_PLUS TOKEN_MINUS TOKEN_STAR TOKEN_SLASH TOKEN_PERCENT
/* keywords */
%token <token> TOKEN_RETURN TOKEN_IF TOKEN_WHILE TOKEN_DO TOKEN_TYPEDEF
%token <token> TOKEN_PROC TOKEN_IMPORT TOKEN_LET TOKEN_TRUE TOKEN_FALSE

%type <type> type
%type <type_param_list> type_param_list
%type <type_param> type_param

%type <token> or and eq relate add multiply pre

%type <input> expr_list
%type <literal> literal
%type <expression> primary prefix multiplicative additive relational 
%type <expression> equality conjunction disjunction
%type <expression> expression assignment

%type <import> import
%type <invocation> invocation
%type <declaration> declaration
%type <conditional> conditional
%type <while_loop> while_stmt
%type <procedure> procedure
%type <decl_list> parameters decl_list
%type <return_stmt> return_stmt
%type <type_def> type_def
%type <statement> statement control

%type <block> block statement_list

/* Operator precedence for mathematical operators */
%left TOKEN_PLUS TOKEN_MINUS
%left TOKEN_STAR TOKEN_SLASH TOKEN_PERCENT

%start program

%%

program         : block
                    {
                        program = $1;
                    }
                ;

block           : statement_list
                | statement
                    {
                        $$ = new Block{};
                        $$->statements.push_back(*$1);
                    }
                ;

statement_list  : statement eol
                    {
                        $$ = new Block{};
                        $$->statements.push_back(*$1);
                    }
                | statement_list statement eol
                    {
                        $$ = $1;
                        $$->statements.push_back(*$2);
                    }
                | eol { $$ = new Block(); }
                ;

statement       : expression  { $$ = new Statement{*$1}; }
                | declaration { $$ = new Statement{*$1}; }
                | assignment  { $$ = new Statement{*$1}; }
                | import      { $$ = new Statement{*$1}; }
                | type_def    { $$ = new Statement{*$1}; delete $1; }
                | control
                ;

expr_list       : expression
                    {
                        $$ = new std::vector<Expression>({*$1});
                        delete $1;
                    }
                | expr_list TOKEN_COMMA expression
                    {
                        $$ = $1;
                        $$->push_back(*$3);
                        delete $3;
                    }
                ;

invocation      : TOKEN_IDENT TOKEN_LPAREN expr_list TOKEN_RPAREN
                    {
                        $$ = new Invocation{*$1, std::move(*$3)};
                        delete $1;
                        delete $3;
                    }
                | TOKEN_IDENT TOKEN_LPAREN TOKEN_RPAREN
                    {
                        $$ = new Invocation{*$1, {}};
                        delete $1;
                    }
                ;

declaration     : TOKEN_IDENT type
                    {
                        $$ = new Declaration{*$1, Type{*$2}};
                        variable_definitions[*$1] = *$$;
                        delete $1;
                        delete $2;
                    }
                | TOKEN_IDENT type TOKEN_LARROW expression
                    {
                        $$ = new Declaration{*$1, Type{*$2}, *$4};
                        variable_definitions[*$1] = *$$;
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

control         : return_stmt { $$ = new Statement{*$1}; delete $1; }
                | conditional { $$ = new Statement{*$1}; delete $1; }
                | while_stmt { $$ = new Statement{*$1}; delete $1; }
                | procedure { $$ = new Statement{*$1}; delete $1; }
                ;

return_stmt     : TOKEN_RETURN { $$ = new Return{}; }
                | TOKEN_RETURN expression { $$ = new Return{*$2}; delete $2; }
                ;

conditional     : TOKEN_IF expression TOKEN_LBRACE block TOKEN_RBRACE
                    {
                        $$ = new Conditional{*$2, *$4};
                        delete $2;
                        delete $4;
                    }
                ;

while_stmt      : TOKEN_WHILE expression TOKEN_LBRACE block TOKEN_RBRACE
                    {
                        $$ = new WhileLoop{*$2, std::move(*$4)};
                        delete $2;
                        delete $4;
                    }
                ;

procedure       : TOKEN_PROC TOKEN_IDENT parameters type TOKEN_LBRACE block TOKEN_RBRACE
                    {
                        $$ = new Procedure{*$2, std::move(*$3), *$4, std::move(*$6)};
                        procedure_definitions[*$2].emplace_back(*$$);
                        delete $2;
                        delete $3;
                        delete $4;
                        delete $6;
                    }
                | TOKEN_PROC TOKEN_IDENT parameters TOKEN_LBRACE block TOKEN_RBRACE
                    {
                        // void procedure
                        $$ = new Procedure{*$2, std::move(*$3), TypeSystem::Intrinsics::void0, std::move(*$5)};
                        procedure_definitions[*$2].emplace_back(*$$);
                        delete $2;
                        delete $3;
                        delete $5;
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
                    { $$ = new Import{*$2}; delete $2; }
                | TOKEN_IMPORT TOKEN_IDENT
                    { $$ = new Import{*$2}; delete $2; }




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

prefix          : primary
                | pre prefix
                    {
                        $$ = operator_to_invocation($1, $2);
                    }
                ;

primary         : literal { $$ = new Expression{*$1}; delete $1; }
                | TOKEN_IDENT { $$ = new Expression{Variable{*$1}}; delete $1; }
                | invocation { $$ = new Expression{*$1}; delete $1; }
                | TOKEN_LPAREN expression TOKEN_RPAREN {
                    $$ = $2;
                }
                | TOKEN_TYPE TOKEN_BANG expression {
                    // type cast takes ownership of expression
                    $$ = new Expression{TypeCast{Type{*$1}, std::shared_ptr<Expression>($3)}};
                    delete $1;
                }
                ;

type_param      : type {
                    $$ = new std::variant<Type, size_t, Declaration>(Type{*$1});
                    delete $1;
                }
                | TOKEN_INT {
                    $$ = new std::variant<Type, size_t, Declaration>(size_t{(size_t) atoll($1->c_str())});
                    delete $1;
                }
                | declaration {
                    $$ = new std::variant<Type, size_t, Declaration>(Declaration{*$1});
                    delete $1;
                }
                ;

type_param_list : type_param {
                    $$ = new std::vector<std::variant<Type, size_t, Declaration>>{*$1};
                    delete $1;
                }
                | type_param_list TOKEN_COMMA type_param {
                    $$ = $1;
                    $1->push_back(*$3);
                    delete $3;
                }
                ;

type            : TOKEN_TYPE { 
                    $$ = new Type{*$1};
                    delete $1; 
                }
                | TOKEN_TYPE TOKEN_LPAREN type_param_list TOKEN_RPAREN {
                    $$ = new Type{*$1, std::move(*$3)};
                    delete $1;
                    delete $3;
                }
                | TOKEN_TYPE TOKEN_LPAREN TOKEN_RPAREN {
                    $$ = new Type{*$1, {}};
                    delete $1;
                }
                ;

type_def        : TOKEN_TYPEDEF TOKEN_TYPE type {
                    $$ = new Typedef{*$2, *$3};
                    delete $2;
                    delete $3;
                }
                ;

or : TOKEN_OR;
and : TOKEN_AND;
eq : TOKEN_EQ | TOKEN_NE;
relate : TOKEN_LT | TOKEN_LE | TOKEN_GT | TOKEN_GE;
add : TOKEN_PLUS | TOKEN_MINUS;
multiply : TOKEN_STAR | TOKEN_SLASH | TOKEN_PERCENT | TOKEN_DOT; // TODO: make dot op before prefix
pre : TOKEN_BANG | TOKEN_MINUS;


literal : TOKEN_INT
            {
                $$ = new Literal{*$1, Literal::integer};
                delete $1;
            }
        | TOKEN_REAL
            {
                $$ = new Literal{*$1, Literal::rational};
                delete $1;
            }
        | TOKEN_STR
            {
                $$ = new Literal{*$1, Literal::string};
                // TODO: add other escape sequences
                size_t i = $$->value.find("\\n"); 
                while (i != std::string::npos) {
                    $$->value.replace(i, 2, "\n");
                    i = $$->value.find("\\n"); 
                }
                delete $1;
            }
        ;
eol : TOKEN_EOL { ++line_num; } | eol TOKEN_EOL { ++line_num; };
