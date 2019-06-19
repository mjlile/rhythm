#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include "lexer.h"
#include "parser.h"

using namespace std;

void compile(string source_file_name) {
    ifstream source_stream(source_file_name);
    string source_content( (std::istreambuf_iterator<char>(source_stream) ),
                           (std::istreambuf_iterator<char>()    ) );

    vector<Token> tokens = lex(source_content);

    for (auto token : tokens) {
        cout << token << endl;
    }
    cout << endl;

    vector<Stmt> statements = parse(tokens);
    for (const auto& stmt : statements) {
        cout << stmt << endl;
    }
    for (const auto& stmt : statements) {
        stmt.interpret();
    }
}


int main(int argc, char* argv[]) {
    if (argc == 2) {
        compile(argv[1]);
    }
    else {
        cout << "Usage: rhythmc [source]" << endl;
    }

}


