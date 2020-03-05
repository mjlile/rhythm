all: parser

clean:
	rm parser.cpp parser.hpp parser tokens.cpp

force: clean all

parser.cpp: parser.y
	bison -dv -o $@ $^
	
parser.h: parser.cpp

tokens.cpp: tokens.l parser.hpp
	lex -o $@ $^

parser: parser.cpp main.cpp tokens.cpp
	g++-7 -o $@ *.cpp -std=c++17 -g3
