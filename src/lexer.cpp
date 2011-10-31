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
    TokenData token(Scanner* s, const int& id);
    void sendReturn(Scanner* s) {
        const Ast::RoutineDefn* rd = 0;
        const Ast::FunctionDefn* fd = 0;
        const Ast::FunctionImpl* fi = 0;
        for(Context::TypeSpecStack::const_reverse_iterator it = _context.typeSpecStack().rbegin(); it != _context.typeSpecStack().rend(); ++it) {
            const Ast::TypeSpec* ts = *it;
            if((rd = dynamic_cast<const Ast::RoutineDefn*>(ts)) != 0) {
                _parser.feed(token(s, ZENTOK_RRETURN));
                return;
            }
            if((fd = dynamic_cast<const Ast::FunctionDefn*>(ts)) != 0) {
                _parser.feed(token(s, ZENTOK_FRETURN));
                return;
            }
            if((fi = dynamic_cast<const Ast::FunctionImpl*>(ts)) != 0) {
                _parser.feed(token(s, ZENTOK_FRETURN));
                return;
            }
        }
        throw Exception("Invalid return in lexer\n");
    }

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
