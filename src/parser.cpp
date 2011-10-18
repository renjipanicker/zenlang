#include "pch.hpp"
#include "common.hpp"
#include "exception.hpp"
#include "parser.hpp"
#include "parserGen.c"

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
    ZenParser(_parser, 0, td, ptr(_context));
}
