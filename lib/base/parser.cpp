#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/parser.hpp"
#include "parserGen.hpp"

z::Parser::Parser() : _parser(0) {
    _parser = ZenParserAlloc(malloc);
//    ZenParserTrace(stdout, "TP: ");
}

z::Parser::~Parser() {
    ZenParserFree(_parser, free);
    _parser = 0;
}

void z::Parser::feed(z::ParserContext& pctx, const z::TokenData& td) {
//    printf("Parser::feed: %d %s\n", td.id(), td.text());
    ZenParser(_parser, td.id(), td, z::ptr(pctx));
}

void z::Parser::done(z::ParserContext& pctx) {
    TokenData td;
    td.init();
    ZenParser(_parser, 0, td, z::ptr(pctx));
}

void z::Parser::reset() {
    ZenParserFree(_parser, free);
    _parser = ZenParserAlloc(malloc);
}
