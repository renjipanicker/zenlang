#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "lexer.hpp"
#include "context.hpp"
#include "parserGen.h"

struct Scanner;
class Lexer::Impl {
public:
    inline Impl(Context& context, Parser& parser) : _parser(parser), _context(context) {}
    size_t init(Scanner* s);
    void scan(Scanner* s);

private:
    inline TokenData token(Scanner* s, const int& id);
    inline bool trySendId(Scanner* s, const Ast::TypeSpec* typeSpec);
    inline void sendId(Scanner* s);
    inline void sendReturn(Scanner* s);

private:
    Parser& _parser;
    Context& _context;
    std::string _ss;
};

#include "lexerGen.c"

Lexer::Lexer(Context& context, Parser& parser) : _impl(0) {
    _impl = new Impl(context, parser);
}

bool Lexer::readFile(const std::string& filename) {
    Scanner in;
    memset((char*) &in, 0, sizeof(in));

    if ((in.fp = fopen(filename.c_str(), "r")) == NULL) {
        return false;
    }

    if(_impl->init(&in) > 0) {
        _impl->scan(&in);
    }

    fclose(in.fp);
    return true;
}
