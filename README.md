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

### How to use
Clone the repo and build with the provided Makefile. `rhythmc` reads from standard input and writes the AST and program outout to standard output.
```
git clone https://github.com/mjlile/Rhythm.git
cd Rhythm
make
rhythmc < test.rhy
```
### Example
#### Input
```c
proc power (let a : Int, let b : Int) -> Int {
    let result : Int <- 1
    while b > 0 {
        if b % 2 = 0 {
            result <- result * result
            b <- b / 2
        }
        if b % 2 = 1 {
            result <- result * a
            b <- b - 1
        }
    }
    return result
}

let x : Int <- 10
let y : Str <- x+1*2-8/2
let z : Int <- 0

while x > 0 {
    x <- x / 3
    z <- z + 1
}

println(x)
println(y)
println(z)
println(power(y, z))
```
#### Output
```
success
block
.   procedure: power
.   .   Int a
.   .   Int b
.   .   returns: Int
.   .   block
.   .   .   Int result
.   .   .   .   literal: 1
.   .   .   loop condition
.   .   .   .   invocation: operator>
.   .   .   .   .   b
.   .   .   .   .   literal: 0
.   .   .   .   block
.   .   .   .   .   condition
.   .   .   .   .   .   invocation: operator=
.   .   .   .   .   .   .   invocation: operator%
.   .   .   .   .   .   .   .   b
.   .   .   .   .   .   .   .   literal: 2
.   .   .   .   .   .   .   literal: 0
.   .   .   .   .   .   block
.   .   .   .   .   .   .   invocation: operator<-
.   .   .   .   .   .   .   .   result
.   .   .   .   .   .   .   .   invocation: operator*
.   .   .   .   .   .   .   .   .   result
.   .   .   .   .   .   .   .   .   result
.   .   .   .   .   .   .   invocation: operator<-
.   .   .   .   .   .   .   .   b
.   .   .   .   .   .   .   .   invocation: operator/
.   .   .   .   .   .   .   .   .   b
.   .   .   .   .   .   .   .   .   literal: 2
.   .   .   .   .   condition
.   .   .   .   .   .   invocation: operator=
.   .   .   .   .   .   .   invocation: operator%
.   .   .   .   .   .   .   .   b
.   .   .   .   .   .   .   .   literal: 2
.   .   .   .   .   .   .   literal: 1
.   .   .   .   .   .   block
.   .   .   .   .   .   .   invocation: operator<-
.   .   .   .   .   .   .   .   result
.   .   .   .   .   .   .   .   invocation: operator*
.   .   .   .   .   .   .   .   .   result
.   .   .   .   .   .   .   .   .   a
.   .   .   .   .   .   .   invocation: operator<-
.   .   .   .   .   .   .   .   b
.   .   .   .   .   .   .   .   invocation: operator-
.   .   .   .   .   .   .   .   .   b
.   .   .   .   .   .   .   .   .   literal: 1
.   .   .   return
.   .   .   .   result

.   Int x
.   .   literal: 10
.   Str y
.   .   invocation: operator-
.   .   .   invocation: operator+
.   .   .   .   x
.   .   .   .   invocation: operator*
.   .   .   .   .   literal: 1
.   .   .   .   .   literal: 2
.   .   .   invocation: operator/
.   .   .   .   literal: 8
.   .   .   .   literal: 2
.   Int z
.   .   literal: 0
.   loop condition
.   .   invocation: operator>
.   .   .   x
.   .   .   literal: 0
.   .   block
.   .   .   invocation: operator<-
.   .   .   .   x
.   .   .   .   invocation: operator/
.   .   .   .   .   x
.   .   .   .   .   literal: 3
.   .   .   invocation: operator<-
.   .   .   .   z
.   .   .   .   invocation: operator+
.   .   .   .   .   z
.   .   .   .   .   literal: 1
.   invocation: println
.   .   x
.   invocation: println
.   .   y
.   invocation: println
.   .   z
.   invocation: println
.   .   invocation: power
.   .   .   y
.   .   .   z
0
8
3
512
```
