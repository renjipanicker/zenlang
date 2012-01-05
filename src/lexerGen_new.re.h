// if first line is a #include, QtCreator recognizes this as a C/C++ file for syntax highlighting.
#include <typeinfo>

#define	BSIZE	8192

/*!types:re2c */

struct Scanner {
    std::istream* is;
    char* cur;
    char* tok;
    char* lim;
    char buffer[BSIZE];
    char yych;
    enum ScanCondition cond;
    int state;
    int yyaccept;

    char* sol; // start of line
    int row;
    char* text;
    char* mar;
};

inline void newLine(Scanner *s) {
    s->row++;
    s->sol = s->cur;
}

static size_t fill(Scanner *s, size_t len) {
    size_t got = 0;
    size_t count = 0;

    if ((z::ref(s->is).eof() == false) && ((s->lim - s->cur) < (int)len)) {
        if (s->tok > s->buffer) {
            count = s->tok - s->buffer;
            memcpy(s->buffer, s->tok, s->lim - s->tok);
            s->tok -= count;
            s->cur -= count;
            s->lim -= count;
            s->sol -= count;
            s->mar -= count;
            if(s->text > 0) {
                assert(s->text >= (s->buffer + count));
                s->text -= count;
            }
            count = &(s->buffer[BSIZE]) - s->lim;
        } else {
            count = BSIZE;
        }

        z::ref(z::ref(s).is).read(s->lim, count);
        got = z::ref(z::ref(s).is).gcount();
        s->lim += got;
    }
    return got;
}

size_t Lexer::Impl::init(Scanner *s) {
    s->sol = s->cur = s->tok = s->lim = s->buffer;
    s->text = 0;
    s->mar = 0;
    s->cond = EStateNormal;
    s->state = -1;
    s->yyaccept = 0;
    s->row = 1;

    return fill(s, 1);
}

inline TokenData createToken(const int& id, const int& row, const int& col, const char* start, const char* end) {
    return TokenData::createT(id, row, col, start, end);
}

inline TokenData Lexer::Impl::token(Scanner* s, const int& id) {
    if(s->text > 0) {
        char* t = s->text;
        s->text = 0;
        return createToken(id, s->row, s->cur-s->sol, t, s->cur - 1);
    }
    printf("scan: createToken mar %lu, tok %lu, cur %lu\n", (unsigned long)s->mar, (unsigned long)s->tok, (unsigned long)s->cur);
    return createToken(id, s->row, s->cur-s->sol, s->mar, s->cur);
}

inline void Lexer::Impl::feedToken(const TokenData& t) {
    _parser.feed(t);
    _lastToken = t.id();
}

inline bool Lexer::Impl::trySendId(Scanner* s, const Ast::TypeSpec* typeSpec) {
    if(!typeSpec)
        return false;

    if(dynamic_cast<const Ast::TemplateDecl*>(typeSpec) != 0) {
        feedToken(token(s, ZENTOK_TEMPLATE_TYPE));
        return true;
    }

    if(dynamic_cast<const Ast::StructDefn*>(typeSpec) != 0) {
        feedToken(token(s, ZENTOK_STRUCT_TYPE));
        return true;
    }
    if(dynamic_cast<const Ast::Routine*>(typeSpec) != 0) {
        feedToken(token(s, ZENTOK_ROUTINE_TYPE));
        return true;
    }
    if(dynamic_cast<const Ast::Function*>(typeSpec) != 0) {
        feedToken(token(s, ZENTOK_FUNCTION_TYPE));
        return true;
    }
    if(dynamic_cast<const Ast::EventDecl*>(typeSpec) != 0) {
        feedToken(token(s, ZENTOK_EVENT_TYPE));
        return true;
    }
    if(dynamic_cast<const Ast::TypeSpec*>(typeSpec) != 0) {
        feedToken(token(s, ZENTOK_OTHER_TYPE));
        return true;
    }
    return false;
}

inline void Lexer::Impl::sendId(Scanner* s) {
    Ast::Token td = token(s, 0);

    if(_lastToken == ZENTOK_SCOPE) {
        const Ast::TypeSpec* child = _context.currentTypeRefHasChild(td);
        if(child) {
            if(trySendId(s, child))
                return;
        }
    }

    if(trySendId(s, _context.hasRootTypeSpec(td)))
        return;

    for(int i = 0; reservedWords[i][0] != 0; ++i) {
        const char* res = reservedWords[i];
        if(td.string() == res) {
            feedToken(token(s, ZENTOK_RESERVED));
        }
    }

    feedToken(token(s, ZENTOK_ID));
}

inline void Lexer::Impl::sendLessThan(Scanner* s) {
    if(_lastToken == ZENTOK_TEMPLATE_TYPE) {
        feedToken(token(s, ZENTOK_TLT));
        return;
    }
    feedToken(token(s, ZENTOK_LT));
}

inline void Lexer::Impl::sendOpenCurly(Scanner* s) {
    if(_context.isStructExpected() || _context.isPointerToStructExpected() || _context.isListOfStructExpected() || _context.isListOfPointerToStructExpected()) {
        if((_lastToken != ZENTOK_STRUCT_TYPE) && (_lastToken != ZENTOK_STRUCT)) {
            feedToken(token(s, ZENTOK_STRUCT));
        }
    }

    if(_context.isFunctionExpected()) {
        if((_lastToken != ZENTOK_FUNCTION_TYPE) && (_lastToken != ZENTOK_FUNCTION)) {
            feedToken(token(s, ZENTOK_FUNCTION));
        }
    }

    feedToken(token(s, ZENTOK_LCURLY));
}

inline void Lexer::Impl::sendReturn(Scanner* s) {
    const Ast::RoutineDefn* rd = 0;
    const Ast::RootFunctionDefn* rfd = 0;
    const Ast::ChildFunctionDefn* cfd = 0;
    for(Ast::NodeFactory::TypeSpecStack::const_reverse_iterator it = _context.typeSpecStack().rbegin(); it != _context.typeSpecStack().rend(); ++it) {
        const Ast::TypeSpec* ts = *it;
        if((rd = dynamic_cast<const Ast::RoutineDefn*>(ts)) != 0) {
            feedToken(token(s, ZENTOK_RRETURN));
            return;
        }
        if((rfd = dynamic_cast<const Ast::RootFunctionDefn*>(ts)) != 0) {
            feedToken(token(s, ZENTOK_FRETURN));
            return;
        }
        if((cfd = dynamic_cast<const Ast::ChildFunctionDefn*>(ts)) != 0) {
            feedToken(token(s, ZENTOK_FRETURN));
            return;
        }
    }
    throw z::Exception("Invalid return in lexer\n");
}

#define NEXT_TOK goto do_start

void Lexer::Impl::scan(Scanner *s) {
}

void Lexer::Impl::send() {
}

bool Lexer::Impl::push(Scanner* /*s*/, const char* buffer, const size_t& len) {
    //printf("Lexer::Impl::push: mar %lu, tok %lu, cur %lu, buffer %s, len %lu, state %d\n", (unsigned long)s->mar, (unsigned long)s->tok, (unsigned long)s->cur, buffer, len, s->state);
    printf("Lexer::Impl::push: limit %lu, start %lu, cursor %lu, marker %lu, state %d, buffer %s, len %lu\n", (unsigned long)limit, (unsigned long)start, (unsigned long)cursor, (unsigned long)marker, state, buffer, len);
    /*
     * Data source is signaling end of file when batch size
     * is less than maxFill. This is slightly annoying because
     * maxFill is a value that can only be known after re2c does
     * its thing. Practically though, maxFill is never bigger than
     * the longest keyword, so given our grammar, 32 is a safe bet.
     */
    uint8_t null[64];
    const ssize_t maxFill = 32;
    if(inputSize<maxFill)
    {
        eof = true;
        input = null;
        inputSize = sizeof(null);
        memset(null, 0, sizeof(null));
    }

    /*
     * When we get here, we have a partially
     * consumed buffer which is in the following state:
     *                                                                last valid char        last valid buffer spot
     *                                                                v                      v
     * +-------------------+-------------+---------------+-------------+----------------------+
     * ^                   ^             ^               ^             ^                      ^
     * buffer              start         marker          cursor        limit                  bufferEnd
     *
     * We need to stretch the buffer and concatenate the new chunk of input to it
     *
     */
    size_t used = limit-buffer;
    size_t needed = used+inputSize;
    size_t allocated = bufferEnd-buffer;
    if(allocated<needed)
    {
        size_t limitOffset = limit-buffer;
        size_t startOffset = start-buffer;
        size_t markerOffset = marker-buffer;
        size_t cursorOffset = cursor-buffer;

            buffer = (uint8_t*)realloc(buffer, needed);
            bufferEnd = needed+buffer;

        marker = markerOffset + buffer;
        cursor = cursorOffset + buffer;
        start = buffer + startOffset;
        limit = limitOffset + buffer;
    }
    memcpy(limit, input, inputSize);
    limit += inputSize;

    // The scanner starts here
//    #define YYLIMIT         limit
//    #define YYCURSOR        cursor
//    #define YYMARKER        marker
//    #define YYCTYPE         uint8_t

//    #define SKIP(x)         { start = cursor; goto yy0; }
//    #define SEND(x)         { send(x); SKIP();          }
//    #define YYFILL(n)       { goto fill;                }

//    #define YYGETSTATE()    state
//    #define YYSETSTATE(x)   { state = (x);  }
//re2c:define:YYGETSTATE       = "state";
//re2c:define:YYGETSTATE:naked = 1;

start:

    /*!getstate:re2c */

    /*!re2c
    re2c:define:YYCONDTYPE       = ScanCondition;
    re2c:indent:top              = 1;
    re2c:cond:goto               = "NEXT_TOK;";

    re2c:define:YYCTYPE          = "char";
    re2c:define:YYCURSOR         = cursor;
    re2c:define:YYLIMIT          = limit;
    re2c:define:YYMARKER         = marker;
    re2c:define:YYFILL@len       = #;
    re2c:define:YYFILL:naked     = 1;
    re2c:define:YYFILL           = "goto do_fill;";
    re2c:define:YYSETSTATE@state = #;
    re2c:define:YYSETSTATE           = "state = #;";
    re2c:define:YYSETCONDITION       = "cond = #;";
    re2c:define:YYSETCONDITION@cond  = #;
    re2c:define:YYGETCONDITION       = cond;
    re2c:define:YYGETCONDITION:naked = 1;
    re2c:variable:yych           = yych;
    re2c:variable:yyaccept       = yyaccept;
    re2c:yych:emit               = 0;
    re2c:condenumprefix          = EState;
    */

    /*!re2c

<Normal> "if" := send(ZENTOK_IF); NEXT_TOK;

    */

fill:
    ssize_t unfinishedSize = cursor-start;
    printf(
        "scanner needs a refill. Exiting for now with:\n"
        "    saved fill state = %d\n"
        "    unfinished token size = %d\n",
        state,
        unfinishedSize
    );

    if(0<unfinishedSize && start<limit)
    {
        printf("    unfinished token is :");
        fwrite(start, 1, cursor-start, stdout);
        putchar('\n');
    }
    putchar('\n');

    /*
     * Once we get here, we can get rid of
     * everything before start and after limit.
     */
    if(eof==true) goto start;
    if(buffer<start)
    {
        size_t startOffset = start-buffer;
        memmove(buffer, start, limit-start);
        marker -= startOffset;
        cursor -= startOffset;
        limit -= startOffset;
        start -= startOffset;
    }
    return true;
}
