#include <iostream>
#include <fstream>
#include <vector>
#include "lexer.h"

using namespace std;

void compile(string source_file_name) {
    ifstream source_stream(source_file_name);
    string source_content( (std::istreambuf_iterator<char>(source_stream) ),
                           (std::istreambuf_iterator<char>()    ) );

    vector<Token> tokens = lex(source_content);

    for(auto token : tokens) {
        cout << token << endl;
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


