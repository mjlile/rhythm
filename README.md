Rhythm Programming Language
===========================
Rhythm is a very early work-in-progress language intended to promote [generic programming](https://www.youtube.com/watch?v=iwJpxWHuZQY), primarily inspired by the works of [Alex Stepanov](http://stepanovpapers.com/), namely the C++ standard template library (STL) and the book [Elements of Programming](http://elementsofprogramming.com/).

Current Status
--------------
This repo is only a very basic implementation at the moment. The only supported types are 32 bit signed integers and string literals. Rhythm code is compiled to [LLVM](https://llvm.org/) IR, which can then be passed into `clang` for native compilation.

### Goals
A non-exhaustive list of goals in different areas

#### Basics
* Documentation
* Better error messages
* Other primitive types (e.g. floating point numbers)
* C-style structs
* Modules

#### Generic Programming Features
* First-class iterator support
* Generic procedures
* Generic structs
* Type functions
* Concepts
* Pointers to concepts (runtime generics)

#### Other Nice Features
These are interesting features from other languages that I would want in my ideal language, but they aren't at the core of what I want Rhythm to be
* Algebraic data types (e.g. Rust's enum types, optional)
* Concurrency (async/await coroutines + Go-style channel communication)
* Ownership (e.g. Rust)

Getting Started
---------------

### Prerequisites
The current Rhythm implementation is written in [Flex](https://github.com/westes/flex/), [Bison](https://www.gnu.org/software/bison/), and [C++17](https://en.cppreference.com/w/cpp/17) for Linux systems. Flex is the GNU implementation of Lex, a lexer generator, while Bison comes from Yacc and is a parser generator. [Clang](https://clang.llvm.org/) is required to compile the LLVM IR to machine code.

### How to use
Clone the repo and build with the provided Makefile. `rhythmc` reads from standard input and writes LLVM IR to standard output. This can be piped into the LLVM interpreter (`lli`) or `clang` with IR input mode. `rhythmc.sh` reads from the file in the first parameter and compiles a native binary (optionally to the file specificed after `-o`).
```
git clone https://github.com/mjlile/Rhythm.git
cd Rhythm
make
./rhythmc.sh hello_world.rh -o hello
./hello
```
### Example
#### Input
```c
proc power (a Int, n Int) Int {
    acc Int <- 1

    while n > 0 {
        if n % 2 = 1 {
            acc <- a * acc
        }

        a <- a * a
        n <- n / 2
    }

    return acc
}

proc main() Int {
    printf("hello world!\n")
    printf("2^11 = %d\n", power(2, 11))

    return 0
}

```
#### Output
```
hello world!
2^11 = 2048
```
