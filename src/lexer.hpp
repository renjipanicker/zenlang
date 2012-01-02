#pragma once
#include "parser.hpp"

class Lexer {
public:
    Lexer(Context& context, Parser& parser);
    ~Lexer();
    bool openString(const std::string& data);
    bool openFile(const std::string& filename);
    bool read();
private:
    class Impl;
    Impl* _impl;
};
