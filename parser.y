%{
    #include <iostream>
    #include <memory>
    #include "ParseTree.hpp"
    extern int yylex();
    void yyerror(const char *s) { printf("ERROR: %s\n", s); }
    std::unique_ptr<ParseTree> program_tree;
    using PTT = ParseTree::Type;
    // constructs std::string from new[]ed cstr and delete[]s cstr
    std::string dyn_cstr_to_str(char* cstr) {
        // anonymous unique_ptr delete[]s cstr after string construction.
        // notice char[] tells unique_ptr to delete[] instead of delete
        return std::string(std::unique_ptr<char[]>(cstr).get());
    }
%}

%union {
    ParseTree* parse_tree;
    char* string;
    int token;
}

%token <string> TOKEN_IDENT TOKEN_TYPE TOKEN_INT TOKEN_REAL TOKEN_STR

%token <token> TOKEN_EOL TOKEN_EQ TOKEN_NE TOKEN_LT TOKEN_LE TOKEN_GT
%token <token> TOKEN_GE TOKEN_LPAREN TOKEN_RPAREN TOKEN_LBRACE TOKEN_RBRACE
%token <token> TOKEN_LBRACK TOKEN_RBRACK TOKEN_LARROW TOKEN_RARROW TOKEN_DOT TOKEN_COMMA
%token <token> TOKEN_BANG TOKEN_PLUS TOKEN_MINUS TOKEN_STAR TOKEN_SLASH
/* keywords */
%token <token> TOKEN_RETURN TOKEN_IF TOKEN_THEN TOKEN_WHILE TOKEN_DO TOKEN_END
%token <token> TOKEN_AND TOKEN_OR TOKEN_PROC

%type <string> literal 
%type <token> or and eq relate add multiply pre post

%type <parse_tree> primary postfix prefix multiplicative additive relational 
%type <parse_tree> equality conjunction disjunction

%type <parse_tree> block statements statement expression declaration assignment
%type <parse_tree> control return_stmt conditional while_stmt procedure

/* Operator precedence for mathematical operators */
%left TOKEN_PLUS TOKEN_MINUS
%left TOKEN_STAR TOKEN_SLASH

%start program

%%

program         : block
                    {
                        program_tree = std::unique_ptr<ParseTree>($1);
                        std::cout << "success" << std::endl;
                    }
                ;

block           : statements
                | statement
                    {
                        $$ = new ParseTree(PTT::Block);
                        $$->adopt_child($1);
                    }
                ;

statements      : statement eol
                    {
                        $$ = new ParseTree(PTT::Block);
                        $$->adopt_child($1);
                    }
                | statements statement eol
                    {
                        $$ = $1;
                        $$->adopt_child($2);
                    }
                | eol { $$ = new ParseTree(PTT::Block); }
                ;

statement       : expression
                | declaration
                | assignment
                | control
                ;

declaration     : TOKEN_TYPE TOKEN_IDENT
                    {
                        $$ = new ParseTree(PTT::Declaration);
                        $$->make_child(PTT::Type, $1);
                        $$->make_child(PTT::Identifier, $2);
                    }
                | TOKEN_TYPE TOKEN_IDENT TOKEN_LARROW expression
                    {
                        $$ = new ParseTree(PTT::Declaration);
                        $$->make_child(PTT::Type, $1);
                        $$->make_child(PTT::Identifier, $2);
                        $$->adopt_child($4);
                    }
                ;

assignment      : TOKEN_IDENT TOKEN_LARROW expression
                    {
                        $$ = new ParseTree(PTT::Assignment);
                        $$->make_child(PTT::Identifier, $1);
                        $$->adopt_child($3);
                    }
                ;

control         : return_stmt
                | conditional
                | while_stmt
                | procedure
                ;

return_stmt     : TOKEN_RETURN { $$ = new ParseTree(PTT::Return); }
                | TOKEN_RETURN expression
                    {
                        $$ = new ParseTree(PTT::Return);
                        $$->adopt_child($2);
                    }
                ;

conditional     : TOKEN_IF expression TOKEN_THEN block TOKEN_END
                    {
                        $$ = new ParseTree(PTT::Conditional);
                        $$->adopt_child($2);
                        $$->adopt_child($4);
                    }
                ;

while_stmt      : TOKEN_WHILE expression TOKEN_DO block TOKEN_END
                    {
                        $$ = new ParseTree(PTT::While);
                        $$->adopt_child($2);
                        $$->adopt_child($4);
                    }
                ;

procedure       : TOKEN_PROC TOKEN_IDENT parameters TOKEN_RARROW TOKEN_TYPE block TOKEN_END
                    {
                        $$ = nullptr; // TODO
                    }
                ;

parameters      : TOKEN_LPAREN TOKEN_RPAREN
                | TOKEN_LPAREN decl_list TOKEN_RPAREN
                ;

decl_list       : TOKEN_TYPE TOKEN_IDENT
                | decl_list TOKEN_COMMA TOKEN_TYPE TOKEN_IDENT
                ;




expression      : disjunction
                ;

disjunction     : conjunction
                | disjunction or conjunction
                    {
                        $$ = new ParseTree(PTT::Expression);
                        $$->adopt_child($1);
                        $$->make_child($2);
                        $$->adopt_child($3);
                    }
                ;

conjunction     : equality
                | conjunction and equality
                    {
                        $$ = new ParseTree(PTT::Expression);
                        $$->adopt_child($1);
                        $$->make_child($2);
                        $$->adopt_child($3);
                    }
                ;

equality        : relational
                | equality eq relational
                    {
                        $$ = new ParseTree(PTT::Expression);
                        $$->adopt_child($1);
                        $$->make_child($2);
                        $$->adopt_child($3);
                    }
                ;

relational      : additive
                | relational relate additive
                    {
                        $$ = new ParseTree(PTT::Expression);
                        $$->adopt_child($1);
                        $$->make_child($2);
                        $$->adopt_child($3);
                    }
                ;

additive        : multiplicative
                | additive add multiplicative
                    {
                        $$ = new ParseTree(PTT::Expression);
                        $$->adopt_child($1);
                        $$->make_child($2);
                        $$->adopt_child($3);
                    }
                ;

multiplicative  : prefix
                | multiplicative multiply prefix
                    {
                        $$ = new ParseTree(PTT::Expression);
                        $$->adopt_child($1);
                        $$->make_child($2);
                        $$->adopt_child($3);
                    }
                ;

prefix          : postfix
                | pre prefix
                    {
                        $$ = new ParseTree(PTT::Expression);
                        $$->make_child($1);
                        $$->adopt_child($2);
                    }
                ;

postfix         : primary
                | postfix post
                    {
                        $$ = new ParseTree(PTT::Expression);
                        $$->adopt_child($1);
                        $$->make_child($2);
                    }
                ;

primary         : literal { std::cout << "literal: " << $1 << std::endl; $$ = new ParseTree(PTT::Literal, dyn_cstr_to_str($1)); }
                | TOKEN_IDENT { $$ = new ParseTree(PTT::Identifier, dyn_cstr_to_str($1)); }
                | TOKEN_LPAREN expression TOKEN_RPAREN
                    {
                        $$ = new ParseTree(PTT::Group);
                        $$->adopt_child($2);
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


literal : TOKEN_INT | TOKEN_REAL | TOKEN_STR;
eol : TOKEN_EOL | eol TOKEN_EOL { std::cout << "eol" << std::endl; };
