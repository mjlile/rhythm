#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <cassert>



std::vector<Token> lex(std::string source_content);

#endif