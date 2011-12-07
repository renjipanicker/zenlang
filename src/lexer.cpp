#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "lexer.hpp"
#include "context.hpp"
#include "parserGen.h"

struct Scanner;
class Lexer::Impl {
public:
    inline Impl(Context& context, Parser& parser);
    inline ~Impl();
    inline bool openFile(const std::string& filename);
    inline bool readFile();

private:
    size_t init(Scanner* s);
    void scan(Scanner* s);

private:
    inline TokenData token(Scanner* s, const int& id);
    inline void feedToken(const TokenData& t);
    inline bool trySendId(Scanner* s, const Ast::TypeSpec* typeSpec);
    inline void sendId(Scanner* s);
    inline void sendLessThan(Scanner* s);
    inline void sendReturn(Scanner* s);

private:
    Scanner* _s;
    Parser& _parser;
    Context& _context;
    int _lastToken;
    static const char* reservedWords[];
};

#include "lexerGen.hpp"

inline Lexer::Impl::Impl(Context& context, Parser& parser) : _s(0), _parser(parser), _context(context), _lastToken(0) {
    _s = new Scanner();
}

inline Lexer::Impl::~Impl() {
    if(ref(_s).fp != 0) {
        fclose(ref(_s).fp);
        ref(_s).fp = 0;
    }
    delete _s;
}

inline bool Lexer::Impl::openFile(const std::string& filename) {
    if ((ref(_s).fp = fopen(filename.c_str(), "r")) == NULL) {
        ref(_s).fp = 0;
        return false;
    }
    return true;
}

inline bool Lexer::Impl::readFile() {
    if(init(_s) > 0) {
        scan(_s);
    }

    fclose(ref(_s).fp);
    ref(_s).fp = 0;
    return true;
}

Lexer::Lexer(Context& context, Parser& parser) : _impl(0) {_impl = new Impl(context, parser);}
Lexer::~Lexer() {delete _impl;}
bool Lexer::openFile(const std::string& filename) {return ref(_impl).openFile(filename);}
bool Lexer::readFile() {return ref(_impl).readFile();}

//-------------------------------------------------
// All keywords that are not used by zen, but are reserved because
// 1. they may have a meaning in the generated language.
// 2. zen might use it later
const char* Lexer::Impl::reservedWords[] = {
    "protected"    ,
    "new"          ,
    "delete"       ,
    "create"       ,
    "insert"       ,
    "remove"       ,
    "class"        ,
    "each"         ,
    "throw"        ,
    "catch"        ,
    "try"          ,
    "raise"        ,
    "lambda"       ,
    "api"          ,
    "inline"       ,
    "static"       ,
    "virtual"      ,
    "pure"         ,
    "final"        ,
    "override"     ,
    "implements"   ,
    "interface"    ,
    "base"         ,
    "parent"       ,
    "child"        ,
    "extends"      ,
    "union"        ,
    "system"       ,
    "plain"        ,
    "sequence"     ,
    "continuation" ,
    "closure"      ,
    "iterate"      ,
    "mutable"      ,
    "local"        ,
    "shared"       ,
    "any"          ,
    "def"          ,
    "grammar"      ,
    "parser"       ,
    "lexer"        ,
    "not"          ,
    "export"       ,
    "import"       ,
    "owner"        ,
    "log"          ,
    "debug"        ,
    "write"        ,
    "exit"         ,
    "quit"         ,
    "link"         ,
    "join"         ,
    "id"           ,
    "assign"       ,
    "query"        ,
    "scope"        ,
    "\0"           // End of list marker. Must be here.
};
