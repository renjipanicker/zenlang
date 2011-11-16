#pragma once
#include "parser.hpp"

class Lexer {
public:
    Lexer(Context& context, Parser& parser);
    ~Lexer();
    bool openFile(const std::string& filename);
    bool readFile();
private:
    class Impl;
    Impl* _impl;
};
