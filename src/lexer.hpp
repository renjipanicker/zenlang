#pragma once
#include "parser.hpp"

class Lexer {
public:
    Lexer(Parser& parser);
    ~Lexer();
    void push(Ast::NodeFactory& factory, const char* buffer, const size_t& len, const bool& isEof);
private:
    class Impl;
    Impl* _impl;
};
