#pragma once

#if defined(UN_AMALGAMATED)
#include "base/parser.hpp"
#endif

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
