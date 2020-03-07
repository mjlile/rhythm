Rhythm Programming Language
===========================
Rhythm is a very early work-in-progress language intended to promote [generic programming](https://www.youtube.com/watch?v=iwJpxWHuZQY), primarily inspired by the works of [Alex Stepanov](http://stepanovpapers.com/), namely the C++ standard template library (STL) and the book [Elements of Programming](http://elementsofprogramming.com/)

Current Status
--------------
This repo is only a very basic implementation of a simple programming language, with support for variables, procedures, and basic math operations. Rhythm code is broken down into an abstract syntax tree (AST) and interpreted with a basic tree walker, although Rhythm is intended to be a compiled language.

### Goals
A non-exhaustive list of goals:
* Structures / classes
* Generics & concepts
* Strong type checking
* Standard library
* Algebraic data types
* Native compilation using [LLVM](https://llvm.org/)
* Descriptive yet concise error messages
* Documentation for the language itself

Getting Started
---------------

### Prerequisites
The current Rhythm implementation is written in [Flex](https://github.com/westes/flex/), [Bison](https://www.gnu.org/software/bison/), and C++17 for Linux systems. Flex is the GNU implementation of Lex, a lexer generator, while Bison comes from Yacc and is a parser generator.

### Example
Clone the repo and build with the provided Makefile. `rhythmc` reads from standard input and writes the AST and program outout to standard output.
```
git clone https://github.com/mjlile/Rhythm.git
cd Rhythm
make
rhythmc < input.rhy > output.txt