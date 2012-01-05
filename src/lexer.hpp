#pragma once
#include "parser.hpp"

class Lexer {
public:
    Lexer(Ast::NodeFactory& context, Parser& parser);
    ~Lexer();
    bool push(const char* buffer, const size_t& len, const bool& isEof);
private:
    class Impl;
    Impl* _impl;
};
