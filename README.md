Rhythm Programming Language
===========================
Rhythm is a very early work-in-progress language intended to promote [generic programming](https://www.youtube.com/watch?v=iwJpxWHuZQY), primarily inspired by the works of [Alex Stepanov](http://stepanovpapers.com/), namely the C++ standard template library (STL) and the book [Elements of Programming](http://elementsofprogramming.com/).

Current Status
--------------
This repo is only a basic implementation at the moment, with control flow, operators, user-defined procedures, and a simple type system. The type system supports integers (signed/unsigned, 8/16/32/64 bit), floating point numbers (32 and 64 bit), pointers, arrays, and C-style structures. User-defined procedures can now be overloaded. IO currently relies on C `printf` and `scanf` calls. Rhythm code is compiled to [LLVM](https://llvm.org/) IR, which can then be passed into `clang` for native compilation.

### Goals
A non-exhaustive list of goals in different areas

#### Basics
* Bug fixes
* Documentation
* Better error messages
* Modules

#### Generic Programming Features
* First-class iterator support
* Generic procedures
* Generic structs
* Type functions
* Concepts
* Pointers to concepts (runtime generics)
* Extensive library generic components

#### Other Nice Features
These are features from other languages that I want in my ideal language, but they aren't at the core of what I want Rhythm to be
* Algebraic data types (e.g. Rust's enum types, optional)
* Concurrency (async/await coroutines + Go-style channel communication)
* Ownership (e.g. Rust)
* Many more

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
#### hello_world.rh
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

proc readInt(ptr Pointer(Int)) {
    printf("n = ")
    scanf("%d", ptr)
}

proc main() Int {
    printf("hello world!\n")

    n Int
    readInt(address(n))
    printf("2^%d = %d\n", n, power(2, n))

    return 0
}

```
##### Output
```
hello world!
n = 11
2^11 = 2048
```

#### alg_example.rh
```c
proc copy(f_i Pointer(Int), l_i Pointer(Int), f_o Pointer(Int)) Pointer(Int) {
    while f_i < l_i {
        deref(f_o) <- deref(f_i)
        f_i <- successor(f_i)
        f_o <- successor(f_o)
    }
    return f_o
}

proc iota(f Pointer(Int), l Pointer(Int), initial Int) {
    while f < l {
        deref(f) <- initial
        f <- successor(f)
        initial <- initial + 1
    }
}

proc fill(f Pointer(Int), l Pointer(Int), val Int) {
    while f < l {
        deref(f) <- val
        f <- successor(f)
    }
}

proc print(f Pointer(Int), l Pointer(Int)) {
    while f < l {
        printf("%d ", deref(f))
        f <- successor(f)
    }
    printf("\n")
}

proc main() Int {
    arr1 Array(Int, 16)
    arr2 Array(Int, 16)

    printf("array 1:\n")
    iota(begin(arr1), limit(arr1), 32)
    print(begin(arr1), limit(arr1))

    printf("\narray 2:\n")
    fill(begin(arr2), limit(arr2), 64)
    print(begin(arr2), limit(arr2))

    printf("\narray 1 copied to array 2:\n")
    copy(begin(arr1), limit(arr1), begin(arr2))
    print(begin(arr2), limit(arr2))

    return 0
}
```
##### Output
```
array 1:
32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 

array 2:
64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64

array 1 copied to array 2:
32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
```
