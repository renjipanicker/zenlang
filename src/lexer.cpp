#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "lexer.hpp"
#include "NodeFactory.hpp"
#include "parserGen.h"

// the char width could be 8 bit, 16, or 32 in future.
typedef char inputChar_t;

class Lexer::Impl {
public:
    inline Impl(Parser& parser);
    void push(Ast::NodeFactory& factory, const char* _buffer, size_t len, const bool& isEof);

private:
    TokenData token(const int& id);
    void send(Ast::NodeFactory& factory, const int& id);
    bool trySendId(Ast::NodeFactory& factory, const Ast::TypeSpec* typeSpec);
    void sendId(Ast::NodeFactory& factory);
    void sendLessThan(Ast::NodeFactory& factory);
    void sendOpenCurly(Ast::NodeFactory& factory);
    void sendReturn(Ast::NodeFactory& factory);
    void newLine();
    void dump(const std::string& x) const;

private:
    // this function is generated by re2c
    void lex(Ast::NodeFactory& factory);

private:
    Parser& _parser;
    int _lastToken;
    static const char* reservedWords[];

private:
    int _cond;
    int _state;

    char* _buffer;
    char* _sol;
    char* _start;
    char* _text;
    char* _marker;
    char* _cursor;
    char* _limit;
    char* _bufferEnd;

    int _row;
    char  _yych;
    unsigned int _yyaccept;
};

void Lexer::Impl::newLine() {
    _row++;
    _sol = _cursor;
}

TokenData Lexer::Impl::token(const int& id) {
    if(_text > 0) {
        char* t = _text;
        _text = 0;
        return TokenData::createT(id, _row, _cursor-_sol, t, _cursor-1);
    }
    return TokenData::createT(id, _row, _cursor-_sol, _start, _cursor);
}

void Lexer::Impl::send(Ast::NodeFactory& factory, const int& id) {
    TokenData td = token(id);
    _parser.feed(factory, td);
    _lastToken = td.id();
}

inline bool Lexer::Impl::trySendId(Ast::NodeFactory& factory, const Ast::TypeSpec* typeSpec) {
    if(!typeSpec)
        return false;

    if(dynamic_cast<const Ast::TemplateDecl*>(typeSpec) != 0) {
        send(factory, ZENTOK_TEMPLATE_TYPE);
        return true;
    }

    if(dynamic_cast<const Ast::StructDefn*>(typeSpec) != 0) {
        send(factory, ZENTOK_STRUCT_TYPE);
        return true;
    }
    if(dynamic_cast<const Ast::Routine*>(typeSpec) != 0) {
        send(factory, ZENTOK_ROUTINE_TYPE);
        return true;
    }
    if(dynamic_cast<const Ast::Function*>(typeSpec) != 0) {
        send(factory, ZENTOK_FUNCTION_TYPE);
        return true;
    }
    if(dynamic_cast<const Ast::EventDecl*>(typeSpec) != 0) {
        send(factory, ZENTOK_EVENT_TYPE);
        return true;
    }
    if(dynamic_cast<const Ast::TypeSpec*>(typeSpec) != 0) {
        send(factory, ZENTOK_OTHER_TYPE);
        return true;
    }
    return false;
}

inline void Lexer::Impl::sendId(Ast::NodeFactory& factory) {
    Ast::Token td = token(0);

    if(_lastToken == ZENTOK_SCOPE) {
        const Ast::TypeSpec* child = factory.ctx().currentTypeRefHasChild(td);
        if(child) {
            if(trySendId(factory, child))
                return;
        }
    }

    if(trySendId(factory, factory.ctx().hasRootTypeSpec(td)))
        return;

    for(int i = 0; reservedWords[i][0] != 0; ++i) {
        const char* res = reservedWords[i];
        if(td.string() == res) {
            return send(factory, ZENTOK_RESERVED);
        }
    }

    send(factory, ZENTOK_ID);
}

inline void Lexer::Impl::sendLessThan(Ast::NodeFactory& factory) {
    if(_lastToken == ZENTOK_TEMPLATE_TYPE) {
        return send(factory, ZENTOK_TLT);
    }
    send(factory, ZENTOK_LT);
}

inline void Lexer::Impl::sendOpenCurly(Ast::NodeFactory& factory) {
    if(factory.isStructExpected() || factory.isPointerToStructExpected() || factory.isListOfStructExpected() || factory.isListOfPointerToStructExpected()) {
        if((_lastToken != ZENTOK_STRUCT_TYPE) && (_lastToken != ZENTOK_STRUCT)) {
            send(factory, ZENTOK_STRUCT);
        }
    }

    if(factory.isFunctionExpected()) {
        if((_lastToken != ZENTOK_FUNCTION_TYPE) && (_lastToken != ZENTOK_FUNCTION)) {
            send(factory, ZENTOK_FUNCTION);
        }
    }
    send(factory, ZENTOK_LCURLY);
}

inline void Lexer::Impl::sendReturn(Ast::NodeFactory& factory) {
    const Ast::RoutineDefn* rd = 0;
    const Ast::RootFunctionDefn* rfd = 0;
    const Ast::ChildFunctionDefn* cfd = 0;
    for(Ast::Context::TypeSpecStack::const_reverse_iterator it = factory.ctx().typeSpecStack().rbegin(); it != factory.ctx().typeSpecStack().rend(); ++it) {
        const Ast::TypeSpec* ts = *it;
        if((rd = dynamic_cast<const Ast::RoutineDefn*>(ts)) != 0) {
            return send(factory, ZENTOK_RRETURN);
        }
        if((rfd = dynamic_cast<const Ast::RootFunctionDefn*>(ts)) != 0) {
            return send(factory, ZENTOK_FRETURN);
        }
        if((cfd = dynamic_cast<const Ast::ChildFunctionDefn*>(ts)) != 0) {
            return send(factory, ZENTOK_FRETURN);
        }
    }
    throw z::Exception("Invalid return in lexer\n");
}

inline Lexer::Impl::Impl(Parser& parser) : _parser(parser), _lastToken(0) {
    _cond = 0;
    _state = -1;

    _buffer = 0;
    _sol = 0;
    _start = 0;
    _text = 0;
    _marker = 0;
    _cursor = 0;
    _limit = 0;
    _bufferEnd = 0;

    _row = 1;
    _yych = 0;
    _yyaccept = 0;
}

// the lex() function
#include "lexerGen.hpp"

inline void Lexer::Impl::dump(const std::string& x) const {
//    printf("%s: buffer %lu, start %lu, limit %lu, text '%s'\n",
//           x.c_str(), (unsigned long)_buffer, (unsigned long)_start, (unsigned long)_limit, _buffer);
}

void Lexer::Impl::push(Ast::NodeFactory& factory, const char* input, size_t inputSize, const bool& isEof) {
//    printf("push(0): '%s'\n", input);

    /*
     * We need a small overhang after EOF on the stream which is
     * equal to the length of the largest keyword (maxFill). This is
     * slightly annoying because maxFill can only be known after re2c
     * does its thing. Practically though, maxFill is never bigger than
     * the longest keyword. A good way to get this value is to run:
     * grep "cursor - start" lexerGen.hpp
     * where lexerGen.hpp is the generated file. You will get lines like:
     * if ((limit - cursor) < 2) goto do_fill;
     * if ((limit - cursor) < 10) goto do_fill;
     * Now set maxFill to the largest RHS value among all the lines matched.
     * Currently it is 10.
     */
    const size_t maxFill = isEof?10:0;

    /*
     * When we get here, we have a partially
     * consumed buffer which is in the following state:
     *                                                                  last valid char        last valid buffer spot
     *                                                                  v                      v
     * +---------+----------+-----+--------+---------------+-------------+----------------------+
     * ^         ^          ^     ^        ^               ^             ^                      ^
     * buffer    sol        start text     marker          cursor        limit                  bufferEnd
     *
     * We need to stretch the buffer (if required) and concatenate the new chunk of input to it
     *
     */
    size_t used      = _limit - _buffer;
    size_t needed    = used + inputSize + maxFill;
    size_t allocated = _bufferEnd - _buffer;

    dump("push(1)");
//    printf("allocated %lu, needed %lu, maxFill %lu\n", allocated, needed, maxFill);
    if(allocated < needed) {
        size_t solOffset    = _sol    - _buffer;
        size_t startOffset  = _start  - _buffer;
        size_t textOffset   = _text?(_text - _buffer):0;
        size_t markerOffset = _marker - _buffer;
        size_t cursorOffset = _cursor - _buffer;
        size_t limitOffset  = _limit  - _buffer;

        _buffer = (inputChar_t*)realloc(_buffer, needed);
        _bufferEnd = _buffer + needed;

        _sol    = _buffer + solOffset;
        _start  = _buffer + startOffset;
        _text   = textOffset?(_buffer + textOffset):0;
        _marker = _buffer + markerOffset;
        _cursor = _buffer + cursorOffset;
        _limit  = _buffer + limitOffset;
    }

    dump("push(2)");

    size_t cpcnt = inputSize + maxFill;
    memcpy(_limit, input, cpcnt);
    _limit += cpcnt;

    dump("push(3)");

    lex(factory);

    dump("push(4)");

    //  Once we get here, we can get rid of everything before start and after limit.
    if(_buffer < _start) {
        size_t startOffset = _start-_buffer;
        memmove(_buffer, _start, _limit-_start);
        _marker -= startOffset;
        _cursor -= startOffset;
        _limit -= startOffset;
        _start -= startOffset;
    }
    dump("push(5)");
}

Lexer::Lexer(Parser& parser) : _impl(0) {_impl = new Impl(parser);}
Lexer::~Lexer() {delete _impl;}
void Lexer::push(Ast::NodeFactory& factory, const char* buffer, const size_t& len, const bool& isEof) {return z::ref(_impl).push(factory, buffer, len, isEof);}

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
