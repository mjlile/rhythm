#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include "lexer.h"
//#include "parser.h"

using namespace std;

void compile(string source_file_name) {
    ifstream source_stream(source_file_name);
    auto stream_begin = std::istreambuf_iterator<char>(source_stream);
    auto stream_end = std::istreambuf_iterator<char>();
    Lexer lexer(stream_begin, stream_end); 
    // example: store tokens in vector
    vector<Token> tokens(lexer.begin(), lexer.end());

    for (auto token : tokens) {
        cout << token << ' ';
    }
    cout << endl;
    /*
    Parser parser(lexer.begin(), lexer.end());
    vector<Statement> statements(parser.begin(), parser.end());

    for (const auto& stmt : statements) {
        cout << stmt << endl;
    }
    */
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        compile(argv[1]);
    }
    else {
        cout << "Usage: rhythmc [source]" << endl;
    }
}


