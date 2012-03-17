#pragma once
#include "base/parser.hpp"

class Lexer {
public:
    Lexer(Parser& parser);
    ~Lexer();
    void push(ParserContext& pctx, const char* buffer, const std::streamsize& len, const bool& isEof);
    void reset();
private:
    class Impl;
    Impl* _impl;
};
