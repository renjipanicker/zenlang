#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "lexer.hpp"
#include "NodeFactory.hpp"
#include "parserGen.h"

class Lexer::Impl {
public:
    inline bool openString(const std::string& data);
    inline bool openFile(const std::string& filename);
    inline bool close();
    inline bool read();
    inline bool init();
    bool push(const char* buffer, size_t len, const bool& isEof);

public:
    inline Impl(Ast::NodeFactory& context, Parser& parser);
    inline ~Impl();

private:
//    bool push(const char* buffer, size_t len, const bool& isEof);
    void send(const int& id);
    bool trySendId(const Ast::TypeSpec* typeSpec);
    void sendId();
    void sendLessThan();
    void sendOpenCurly();
    void sendReturn();
    void newLine();

private:
    Parser& _parser;
    Ast::NodeFactory& _context;
    int _lastToken;
    static const char* reservedWords[];

private:
    bool  eof;
    int   cond;
    int   state;

    char* limit;
    char* start;
    char* cursor;
    char* marker;
    char* text;

    char* buffer;
    char* bufferEnd;

    char  yych;
    unsigned int yyaccept;
};

#include "lexerGen.hpp"

//inline bool Lexer::Impl::openString(const std::string& data) {
//    z::ref(_s).is = new std::stringstream(data);
//    return true;
//}

//inline bool Lexer::Impl::openFile(const std::string& filename) {
//    z::ref(_s).is = 0;
//    std::ifstream* is = new std::ifstream();
//    z::ref(is).open(filename.c_str(), std::ifstream::in);
//    if(z::ref(is).is_open() == false) {
//        delete is;
//        return false;
//    }
//    z::ref(_s).is = is;
//    return true;
//}

//inline bool Lexer::Impl::close() {
//    if(z::ref(_s).is != 0) {
//        /// \todo check if delete closes the file
//        //z::ref(z::ref(_s).is).close();
//        delete z::ref(_s).is;
//        z::ref(_s).is = 0;
//    }
//    return true;
//}

//inline bool Lexer::Impl::read() {
//    if(init(_s) > 0) {
//        scan(_s);
//    }

//    close();
//    return true;
//}

//inline bool Lexer::Impl::init() {
//    return init(_s) > 0;
//    return true;
//}

//inline bool Lexer::Impl::push(const char* buffer, const size_t& len, const bool& isEof) {
//    return push(_s, buffer, len, isEof);
//}

inline Lexer::Impl::Impl(Ast::NodeFactory& context, Parser& parser) : /*_s(0), */_parser(parser), _context(context), _lastToken(0) {
//    _s = new Scanner();

    limit = 0;
    start = 0;
    state = -1;
    cursor = 0;
    marker = 0;
    buffer = 0;
    eof = false;
    bufferEnd = 0;
    text = 0;
    cond = 0;
}

inline Lexer::Impl::~Impl() {
//    close();
//    delete _s;
}

Lexer::Lexer(Ast::NodeFactory& context, Parser& parser) : _impl(0) {_impl = new Impl(context, parser);}
Lexer::~Lexer() {delete _impl;}
//bool Lexer::openString(const std::string& data) {return z::ref(_impl).openString(data);}
//bool Lexer::openFile(const std::string& filename) {return z::ref(_impl).openFile(filename);}
//bool Lexer::read() {return z::ref(_impl).read();}
//bool Lexer::init() {return z::ref(_impl).init();}
bool Lexer::push(const char* buffer, const size_t& len, const bool& isEof) {return z::ref(_impl).push(buffer, len, isEof);}

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
