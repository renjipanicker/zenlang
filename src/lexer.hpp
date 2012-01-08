#pragma once
#include "parser.hpp"

class Lexer {
public:
    enum Mode {
        lmCompiler,
        lmInterpreter
    };

public:
    Lexer(Parser& parser, const Mode& mode);
    ~Lexer();
    void push(Ast::NodeFactory& factory, const char* buffer, const size_t& len, const bool& isEof);
    void reset();
private:
    class Impl;
    Impl* _impl;
};
