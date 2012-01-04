#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "lexer.hpp"
#include "NodeFactory.hpp"
#include "parserGen.h"

struct Scanner;
class Lexer::Impl {
public:
    inline bool openString(const std::string& data);
    inline bool openFile(const std::string& filename);
    inline bool close();
    inline bool read();

public:
    inline Impl(Ast::NodeFactory& context, Parser& parser);
    inline ~Impl();

private:
    size_t init(Scanner* s);
    void scan(Scanner* s);

private:
    inline TokenData token(Scanner* s, const int& id);
    inline void feedToken(const TokenData& t);
    inline bool trySendId(Scanner* s, const Ast::TypeSpec* typeSpec);
    inline void sendId(Scanner* s);
    inline void sendLessThan(Scanner* s);
    inline void sendOpenCurly(Scanner* s);
    inline void sendReturn(Scanner* s);

private:
    Scanner* _s;
    Parser& _parser;
    Ast::NodeFactory& _context;
    int _lastToken;
    static const char* reservedWords[];
};

#include "lexerGen.hpp"

inline bool Lexer::Impl::openString(const std::string& data) {
    z::ref(_s).is = new std::stringstream(data);
    return true;
}

inline bool Lexer::Impl::openFile(const std::string& filename) {
    z::ref(_s).is = 0;
    std::ifstream* is = new std::ifstream();
    z::ref(is).open(filename.c_str(), std::ifstream::in);
    if(z::ref(is).is_open() == false) {
        delete is;
        return false;
    }
    z::ref(_s).is = is;
    return true;
}

inline bool Lexer::Impl::close() {
    if(z::ref(_s).is != 0) {
        /// \todo check if delete closes the file
        //z::ref(z::ref(_s).is).close();
        delete z::ref(_s).is;
        z::ref(_s).is = 0;
    }
    return true;
}

inline bool Lexer::Impl::read() {
    if(init(_s) > 0) {
        scan(_s);
    }

    close();
    return true;
}

inline Lexer::Impl::Impl(Ast::NodeFactory& context, Parser& parser) : _s(0), _parser(parser), _context(context), _lastToken(0) {
    _s = new Scanner();
}

inline Lexer::Impl::~Impl() {
    close();
    delete _s;
}

Lexer::Lexer(Ast::NodeFactory& context, Parser& parser) : _impl(0) {_impl = new Impl(context, parser);}
Lexer::~Lexer() {delete _impl;}
bool Lexer::openString(const std::string& data) {return z::ref(_impl).openString(data);}
bool Lexer::openFile(const std::string& filename) {return z::ref(_impl).openFile(filename);}
bool Lexer::read() {return z::ref(_impl).read();}

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
    "override"     ,
    "implements"   ,
    "interface"    ,
    "base"         ,
    "parent"       ,
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
    "and"          ,
    "or"           ,
    "xor"          ,
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
