#pragma once

#include "base/factory.hpp"
#include "token.hpp"

class Compiler;
struct ParserContext {
    inline ParserContext(Ast::Factory& f, Compiler& c) : factory(f), compiler(c) {}
    Ast::Factory& factory;
    Compiler& compiler;
};

inline Ast::Token t2t(TokenData& td) {
    Ast::Token t(td.filename(), td.row(), td.col(), td.text());
    TokenData::deleteT(td);
    return t;
}

inline Ast::Factory& c2f(ParserContext* pctx) {
    return z::ref(pctx).factory;
}

inline Compiler& c2c(ParserContext* pctx) {
    return z::ref(pctx).compiler;
}

class Parser {
public:
    Parser();
    ~Parser();
public:
    void feed(ParserContext& pctx, const TokenData& td);
    void done(ParserContext& pctx);
    void reset();
private:
    void* _parser;
};
