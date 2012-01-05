#pragma once
#include "parser.hpp"

class Lexer {
public:
    Lexer(Ast::NodeFactory& context, Parser& parser);
    ~Lexer();
//    bool openString(const std::string& data);
//    bool openFile(const std::string& filename);
//    bool read();
//    bool init();
    bool push(const char* buffer, const size_t& len, const bool& isEof);
private:
    class Impl;
    Impl* _impl;
};
