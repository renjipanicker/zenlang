#pragma once

#include "token.hpp"
#include "context.hpp"

class Parser {
public:
    Parser(Context& context);
    ~Parser();
public:
    void feed(const TokenData& td);
    void done();
    inline Context& context() {return _context;}
private:
    Context& _context;
    void* _parser;
};
