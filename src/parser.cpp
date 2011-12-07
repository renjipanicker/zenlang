#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "parser.hpp"
#include "parserGen.hpp"

Parser::Parser(Context& context) : _context(context), _parser(0) {
    _parser = ZenParserAlloc(malloc);
    //ZenParserTrace(stdout, "TP: ");
}

Parser::~Parser() {
    ZenParserFree(_parser, free);
    _parser = 0;
}

void Parser::feed(const TokenData& td) {
    //trace("Parser::feed: %d %s\n", td.id(), td.text());
    ZenParser(_parser, td.id(), td, ptr(_context));
}

void Parser::done() {
    TokenData td;
    td.init();
    ZenParser(_parser, 0, td, ptr(_context));
}
