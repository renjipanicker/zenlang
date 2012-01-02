#pragma once

#include "token.hpp"
#include "context.hpp"

class Parser {
public:
    Parser(Ast::NodeFactory& context);
    ~Parser();
public:
    void feed(const TokenData& td);
    void done();
    inline Ast::NodeFactory& context() {return _context;}
private:
    Ast::NodeFactory& _context;
    void* _parser;
};
