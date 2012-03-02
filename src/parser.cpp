#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "parser.hpp"
#include "parserGen.hpp"

Parser::Parser() : _parser(0) {
    _parser = ZenParserAlloc(malloc);
    //ZenParserTrace(stdout, "TP: ");
}

Parser::~Parser() {
    ZenParserFree(_parser, free);
    _parser = 0;
}

void Parser::feed(ParserContext& pctx, const TokenData& td) {
//    printf("Parser::feed: %d %s\n", td.id(), td.text());
    ZenParser(_parser, td.id(), td, z::ptr(pctx));
}

void Parser::done(ParserContext& pctx) {
    TokenData td;
    td.init();
    ZenParser(_parser, 0, td, z::ptr(pctx));
}

void Parser::reset() {
    ZenParserFree(_parser, free);
    _parser = ZenParserAlloc(malloc);
}
