#pragma once

#include "token.hpp"
#include "NodeFactory.hpp"

class Parser {
public:
    Parser(Ast::NodeFactory& nodeFactory);
    ~Parser();
public:
    void feed(const TokenData& td);
    void done();
    inline Ast::NodeFactory& nodeFactory() {return _nodeFactory;}
private:
    Ast::NodeFactory& _nodeFactory;
    void* _parser;
};
