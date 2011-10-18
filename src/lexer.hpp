#pragma once
#include "parser.hpp"

class Lexer {
public:
    Lexer(Context& context, Parser& parser);
    bool readFile(const std::string& filename);
private:
    class Impl;
    Impl* _impl;
};
