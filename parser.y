%{
    #include <iostream>
    #include <memory>
    extern int yylex();
    void yyerror(const char *s) { printf("ERROR: %s\n", s); }
%}

%union {
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
%token <token> TOKEN_AND TOKEN_OR TOKEN_PROC


/* Operator precedence for mathematical operators */
%left TOKEN_PLUS TOKEN_MINUS
rleft TOKEN_STAR TOKEN_SLASH

%start program

%%

program         : block { std::cout << "success" << std::endl; }
                ;

block           : statements
                | statement
                ;

statements      : statement eol
                | statements statement eol
                | eol
                ;

statement       : expression
                | assignment
                | control
                ;

assignment      : type expression gets expression { std::cout << "assign" << std::endl; }
                ;

control         : return_stmt
                | conditional
                | while_stmt
                | procedure
                ;

return_stmt     : return
                | return expression
                ;

conditional     : if expression then block end
                ;

while_stmt      : while expression do block end
                ;

procedure       : proc identifier group_begin maybe_params group_end to type block end
                ;

maybe_params    :
                | parameters
                ;

parameters      : type identifier
                | parameters comma type identifier
                ;




expression      : disjunction
                ;

disjunction     : conjunction
                | disjunction or conjunction
                ;

conjunction     : equality
                | conjunction and equality
                ;

equality        : relational
                | equality eq relational
                ;

relational      : additive
                | relational relate additive
                ;

additive        : multiplicative
                | additive add multiplicative
                ;

multiplicative  : prefix
                | multiplicative multiply prefix
                ;

prefix          : postfix
                | pre prefix
                ;

postfix         : primary
                | postfix post
                ;

primary         : literal
                | identifier
                | group_begin expression group_end
                ;



or : TOKEN_OR;
and : TOKEN_AND;
eq : TOKEN_EQ | TOKEN_NE;
relate : TOKEN_LT | TOKEN_LE | TOKEN_GT | TOKEN_GE;
add : TOKEN_PLUS | TOKEN_MINUS;
multiply : TOKEN_STAR | TOKEN_SLASH;
pre : TOKEN_BANG | TOKEN_MINUS | TOKEN_STAR;
post : index;
index : TOKEN_LBRACK expression TOKEN_RBRACK;
literal : TOKEN_INT | TOKEN_REAL | TOKEN_STR;
identifier : TOKEN_IDENT;
group_begin : TOKEN_LPAREN;
group_end : TOKEN_RPAREN;

type : TOKEN_TYPE;
eol : TOKEN_EOL | eol TOKEN_EOL { std::cout << "eol" << std::endl; };
gets : TOKEN_LARROW;
to : TOKEN_RARROW;
return : TOKEN_RETURN;
if : TOKEN_IF;
then : TOKEN_THEN;
while : TOKEN_WHILE;
do : TOKEN_DO;
proc : TOKEN_PROC;
end : TOKEN_END;
comma : TOKEN_COMMA;
