#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "parser.hpp"
#include "parserGen.hpp"

Parser::Parser() : _factory(0), _parser(0) {
    _parser = ZenParserAlloc(malloc);
    //ZenParserTrace(stdout, "TP: ");
}

Parser::~Parser() {
    ZenParserFree(_parser, free);
    _parser = 0;
}

void Parser::feed(Ast::NodeFactory& factory, const TokenData& td) {
//    printf("Parser::feed: %d %s\n", td.id(), td.text());
    ZenParser(_parser, td.id(), td, z::ptr(factory));
}

void Parser::done(Ast::NodeFactory& factory) {
    TokenData td;
    td.init();
    ZenParser(_parser, 0, td, z::ptr(factory));
}

void Parser::reset() {
    ZenParserFree(_parser, free);
    _parser = ZenParserAlloc(malloc);
}
