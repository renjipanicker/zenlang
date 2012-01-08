#pragma once

#include "token.hpp"
#include "NodeFactory.hpp"

class Parser {
public:
    Parser();
    ~Parser();
public:
    void feed(Ast::NodeFactory& factory, const TokenData& td);
    void done(Ast::NodeFactory& factory);
    void reset();
private:
    Ast::NodeFactory* _factory;
    void* _parser;
};
