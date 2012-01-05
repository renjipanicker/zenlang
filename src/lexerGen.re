// if first line is a #include, QtCreator recognizes this as a C/C++ file for syntax highlighting.
#include <typeinfo>

#define	BSIZE	8192

/*!types:re2c */

void Lexer::Impl::newLine() {
//    s->row++;
//    s->sol = s->cur;
}

void Lexer::Impl::send(const int& id) {
    if(text > 0) {
        char* t = text;
        text = 0;
        printf("send-text: t = %lu, cursor = %lu\n", (unsigned long)t, (unsigned long)cursor);
        size_t tokenSize = cursor-t-1;
        std::string s(t, tokenSize);
        TokenData td = TokenData::createT(id, 0, 0, t, cursor-1);
        printf("send-text: id = %d, s = '%s', td = %s\n", id, s.c_str(), td.text());
        _parser.feed(td);
        _lastToken = td.id();
        return;
    }
    size_t tokenSize = cursor-start;
    std::string s(start, tokenSize);
    printf("send: id = %d, '%s'\n", id, s.c_str());
    TokenData td = TokenData::createT(id, 0, 0, start, cursor);
    _parser.feed(td);
    _lastToken = td.id();
}

inline bool Lexer::Impl::trySendId(const Ast::TypeSpec* typeSpec) {
    if(!typeSpec)
        return false;

    if(dynamic_cast<const Ast::TemplateDecl*>(typeSpec) != 0) {
        send(ZENTOK_TEMPLATE_TYPE);
        return true;
    }

    if(dynamic_cast<const Ast::StructDefn*>(typeSpec) != 0) {
        send(ZENTOK_STRUCT_TYPE);
        return true;
    }
    if(dynamic_cast<const Ast::Routine*>(typeSpec) != 0) {
        send(ZENTOK_ROUTINE_TYPE);
        return true;
    }
    if(dynamic_cast<const Ast::Function*>(typeSpec) != 0) {
        send(ZENTOK_FUNCTION_TYPE);
        return true;
    }
    if(dynamic_cast<const Ast::EventDecl*>(typeSpec) != 0) {
        send(ZENTOK_EVENT_TYPE);
        return true;
    }
    if(dynamic_cast<const Ast::TypeSpec*>(typeSpec) != 0) {
        send(ZENTOK_OTHER_TYPE);
        return true;
    }
    return false;
}

inline void Lexer::Impl::sendId() {
    Ast::Token td = TokenData::createT(0, 0, 0, start, cursor);

    if(_lastToken == ZENTOK_SCOPE) {
        const Ast::TypeSpec* child = _context.currentTypeRefHasChild(td);
        if(child) {
            if(trySendId(child))
                return;
        }
    }

    if(trySendId(_context.hasRootTypeSpec(td)))
        return;

    for(int i = 0; reservedWords[i][0] != 0; ++i) {
        const char* res = reservedWords[i];
        if(td.string() == res) {
            return send(ZENTOK_RESERVED);
        }
    }

    send(ZENTOK_ID);
}

inline void Lexer::Impl::sendLessThan() {
    if(_lastToken == ZENTOK_TEMPLATE_TYPE) {
        return send(ZENTOK_TLT);
    }
    send(ZENTOK_LT);
}

inline void Lexer::Impl::sendOpenCurly() {
    if(_context.isStructExpected() || _context.isPointerToStructExpected() || _context.isListOfStructExpected() || _context.isListOfPointerToStructExpected()) {
        if((_lastToken != ZENTOK_STRUCT_TYPE) && (_lastToken != ZENTOK_STRUCT)) {
            send(ZENTOK_STRUCT);
        }
    }

    if(_context.isFunctionExpected()) {
        if((_lastToken != ZENTOK_FUNCTION_TYPE) && (_lastToken != ZENTOK_FUNCTION)) {
            send(ZENTOK_FUNCTION);
        }
    }
    send(ZENTOK_LCURLY);
}

inline void Lexer::Impl::sendReturn() {
    const Ast::RoutineDefn* rd = 0;
    const Ast::RootFunctionDefn* rfd = 0;
    const Ast::ChildFunctionDefn* cfd = 0;
    for(Ast::NodeFactory::TypeSpecStack::const_reverse_iterator it = _context.typeSpecStack().rbegin(); it != _context.typeSpecStack().rend(); ++it) {
        const Ast::TypeSpec* ts = *it;
        if((rd = dynamic_cast<const Ast::RoutineDefn*>(ts)) != 0) {
            return send(ZENTOK_RRETURN);
        }
        if((rfd = dynamic_cast<const Ast::RootFunctionDefn*>(ts)) != 0) {
            return send(ZENTOK_FRETURN);
        }
        if((cfd = dynamic_cast<const Ast::ChildFunctionDefn*>(ts)) != 0) {
            return send(ZENTOK_FRETURN);
        }
    }
    throw z::Exception("Invalid return in lexer\n");
}

typedef char uint8_t;

bool Lexer::Impl::push(const char* input, size_t inputSize, const bool& isEof) {
    printf("Lexer::Impl::push: buffer %lu, marker %lu, start %lu, cursor %lu, limit %lu, state %d, input %lu, inputSize %lu, isEof %d\n",
           (unsigned long)buffer, (unsigned long)marker, (unsigned long)start, (unsigned long)cursor, (unsigned long)limit, state, (unsigned long)input, inputSize, isEof);

    /*
     * Data source is signaling end of file when batch size
     * is less than maxFill. This is slightly annoying because
     * maxFill is a value that can only be known after re2c does
     * its thing. Practically though, maxFill is never bigger than
     * the longest keyword, so given our grammar, 32 is a safe bet.
     */
//    uint8_t nullstr[64];
    const size_t maxFill = isEof?10:0;
//    if(inputSize<maxFill)
//    {
//        printf("eof = true\n");
//        eof = true;
////        input = nullstr;
////        inputSize = sizeof(nullstr);
////        memset(nullstr, 0, sizeof(nullstr));
//    }

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
    size_t needed = used+inputSize+maxFill;
    size_t allocated = bufferEnd-buffer;
    if(allocated<needed)
    {
        size_t limitOffset = limit-buffer;
        size_t startOffset = start-buffer;
        size_t markerOffset = marker-buffer;
        size_t cursorOffset = cursor-buffer;
        printf("** re-allocating, limitOffset %lu\n", limitOffset);

        buffer = (uint8_t*)realloc(buffer, needed);
        bufferEnd = buffer + needed;

        marker = buffer + markerOffset;
        cursor = buffer + cursorOffset;
        start = buffer + startOffset;
        limit = buffer + limitOffset;
    }

    size_t cpcnt = inputSize + maxFill;
    memcpy(limit, input, cpcnt);
    limit += cpcnt;

    /////////////
    printf("cursor %lu, *cursor %d, *cursor %c, buffer: '%s'\n", (unsigned long)cursor, *cursor, *cursor, buffer);
    std::string ns(cursor, 9);
    if(ns == "namespace") {
        printf("** namespace\n");
    }

#define SKIP(x)         { start = cursor; goto yy0; }
#define SEND(x)         { send(x); SKIP();          }
#define YYFILL(n)       { goto do_fill;             }

//do_start:
/*!re2c
re2c:define:YYGETSTATE       = "state";
re2c:define:YYGETSTATE:naked = 1;
re2c:define:YYCONDTYPE       = ScanCondition;
re2c:indent:top              = 1;
re2c:cond:goto               = "SKIP();";
*/

/*!getstate:re2c */

/*!re2c

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
re2c:yych:emit               = 0;
re2c:indent:top              = 2;
re2c:condenumprefix          = EState;

<Normal>   "<<="   := send(ZENTOK_SHIFTLEFTEQUAL); SKIP();
<Normal>   ">>="   := send(ZENTOK_SHIFTRIGHTEQUAL); SKIP();
<Normal>   ":="    := send(ZENTOK_DEFINEEQUAL); SKIP();
<Normal>   "*="    := send(ZENTOK_TIMESEQUAL); SKIP();
<Normal>   "/="    := send(ZENTOK_DIVIDEEQUAL); SKIP();
<Normal>   "-="    := send(ZENTOK_MINUSEQUAL); SKIP();
<Normal>   "+="    := send(ZENTOK_PLUSEQUAL); SKIP();
<Normal>   "%="    := send(ZENTOK_MODEQUAL); SKIP();
<Normal>   "&="    := send(ZENTOK_BITWISEANDEQUAL); SKIP();
<Normal>   "^="    := send(ZENTOK_BITWISEXOREQUAL); SKIP();
<Normal>   "|="    := send(ZENTOK_BITWISEOREQUAL); SKIP();
<Normal>   "!="    := send(ZENTOK_NOTEQUAL); SKIP();
<Normal>   "=="    := send(ZENTOK_EQUAL); SKIP();
<Normal>   "<="    := send(ZENTOK_LTE); SKIP();
<Normal>   ">="    := send(ZENTOK_GTE); SKIP();
<Normal>   ">>"    := send(ZENTOK_SHR); SKIP();
<Normal>   "<<"    := send(ZENTOK_SHL); SKIP();
<Normal>   "++"    := send(ZENTOK_INC); SKIP();
<Normal>   "--"    := send(ZENTOK_DEC); SKIP();
<Normal>   "=>"    := send(ZENTOK_LINK); SKIP();
<Normal>   "->"    := send(ZENTOK_RESERVED); SKIP();
<Normal>   "&&"    := send(ZENTOK_AND); SKIP();
<Normal>   "||"    := send(ZENTOK_OR); SKIP();
<Normal>   ";"     := send(ZENTOK_SEMI); SKIP();
<Normal>   "!"     := send(ZENTOK_NOT); SKIP();
<Normal>   "="     := send(ZENTOK_ASSIGNEQUAL); SKIP();
<Normal>   "~"     := send(ZENTOK_BITWISENOT); SKIP();
<Normal>   "^"     := send(ZENTOK_BITWISEXOR); SKIP();
<Normal>   "|"     := send(ZENTOK_BITWISEOR); SKIP();
<Normal>   "&"     := send(ZENTOK_BITWISEAND); SKIP();
<Normal>   "?"     := send(ZENTOK_QUESTION); SKIP();
<Normal>   ":"     := send(ZENTOK_COLON); SKIP();
<Normal>   "::"    := send(ZENTOK_SCOPE); SKIP();
<Normal>   "."     := send(ZENTOK_DOT); SKIP();
<Normal>   ","     := send(ZENTOK_COMMA); SKIP();
<Normal>   "@"     := send(ZENTOK_AMP); SKIP();
<Normal>   "("     := send(ZENTOK_LBRACKET); SKIP();
<Normal>   ")"     := send(ZENTOK_RBRACKET); SKIP();
<Normal>   "["     := send(ZENTOK_LSQUARE); SKIP();
<Normal>   "]"     := send(ZENTOK_RSQUARE); SKIP();
<Normal>   "{"     := sendOpenCurly(); SKIP();
<Normal>   "}"     := send(ZENTOK_RCURLY); SKIP();
<Normal>   "%"     := send(ZENTOK_MOD); SKIP();
<Normal>   "<"     := sendLessThan(); SKIP();
<Normal>   ">"     := send(ZENTOK_GT); SKIP();
<Normal>   "+"     := send(ZENTOK_PLUS); SKIP();
<Normal>   "-"     := send(ZENTOK_MINUS); SKIP();
<Normal>   "*"     := send(ZENTOK_STAR); SKIP();
<Normal>   "/"     := send(ZENTOK_DIVIDE); SKIP();
<Normal>   "..."   := send(ZENTOK_ELIPSIS); SKIP();

<Normal>   "import"    := send(ZENTOK_IMPORT); SKIP();
<Normal>   "include"   := send(ZENTOK_INCLUDE); SKIP();
<Normal>   "namespace" := send(ZENTOK_NAMESPACE); SKIP();

<Normal>   "private"   := send(ZENTOK_PRIVATE); SKIP();
<Normal>   "public"    := send(ZENTOK_PUBLIC); SKIP();
<Normal>   "internal"  := send(ZENTOK_INTERNAL); SKIP();
<Normal>   "external"  := send(ZENTOK_EXTERNAL); SKIP();

<Normal>   "typedef"   := send(ZENTOK_TYPEDEF); SKIP();
<Normal>   "template"  := send(ZENTOK_TEMPLATE); SKIP();
<Normal>   "enum"      := send(ZENTOK_ENUM); SKIP();
<Normal>   "struct"    := send(ZENTOK_STRUCT); SKIP();
<Normal>   "routine"   := send(ZENTOK_ROUTINE); SKIP();
<Normal>   "function"  := send(ZENTOK_FUNCTION); SKIP();
<Normal>   "event"     := send(ZENTOK_EVENT); SKIP();

<Normal>   "property"  := send(ZENTOK_PROPERTY); SKIP();
<Normal>   "get"       := send(ZENTOK_GET); SKIP();
<Normal>   "set"       := send(ZENTOK_SET); SKIP();

<Normal>   "native"    := send(ZENTOK_NATIVE); SKIP();
<Normal>   "abstract"  := send(ZENTOK_ABSTRACT); SKIP();
<Normal>   "final"     := send(ZENTOK_FINAL); SKIP();
<Normal>   "const"     := send(ZENTOK_CONST); SKIP();

<Normal>   "coerce"    := send(ZENTOK_COERCE); SKIP();
<Normal>   "default"   := send(ZENTOK_DEFAULT); SKIP();
<Normal>   "typeof"    := send(ZENTOK_TYPEOF); SKIP();

<Normal>   "auto"      := send(ZENTOK_AUTO); SKIP();
<Normal>   "print"     := send(ZENTOK_PRINT); SKIP();
<Normal>   "if"        := send(ZENTOK_IF); SKIP();
<Normal>   "else"      := send(ZENTOK_ELSE); SKIP();
<Normal>   "while"     := send(ZENTOK_WHILE); SKIP();
<Normal>   "do"        := send(ZENTOK_DO); SKIP();
<Normal>   "for"       := send(ZENTOK_FOR); SKIP();
<Normal>   "foreach"   := send(ZENTOK_FOREACH); SKIP();
<Normal>   "in"        := send(ZENTOK_IN); SKIP();
<Normal>   "has"       := send(ZENTOK_HAS); SKIP();
<Normal>   "switch"    := send(ZENTOK_SWITCH); SKIP();
<Normal>   "case"      := send(ZENTOK_CASE); SKIP();
<Normal>   "break"     := send(ZENTOK_BREAK); SKIP();
<Normal>   "continue"  := send(ZENTOK_CONTINUE); SKIP();
<Normal>   "run"       := send(ZENTOK_RUN); SKIP();

<Normal>   "return"    := sendReturn(); SKIP();

<Normal>   "false"     := send(ZENTOK_FALSE_CONST); SKIP();
<Normal>   "true"      := send(ZENTOK_TRUE_CONST); SKIP();

<Normal>   "@" [a-zA-Z][a-zA-Z0-9_]* := send(ZENTOK_KEY_CONST); SKIP();

<Normal>       [a-zA-Z][a-zA-Z0-9_]* := sendId(); SKIP();

<Normal>   "0" [0-7]+                                   := send(ZENTOK_OCTINT_CONST); SKIP();
<Normal>   "0" [0-7]+ [uU]                              := send(ZENTOK_OCTINT_CONST); SKIP();
<Normal>   "0" [0-7]+ [lL]                              := send(ZENTOK_LOCTINT_CONST); SKIP();
<Normal>   "0" [0-7]+ [uU] [lL]                         := send(ZENTOK_LOCTINT_CONST); SKIP();

<Normal>   [0-9]+                                       := send(ZENTOK_DECINT_CONST); SKIP();
<Normal>   [0-9]+ [lL]                                  := send(ZENTOK_LDECINT_CONST); SKIP();

<Normal>   "0" [xX] [A-Za-z0-9]+                        := send(ZENTOK_HEXINT_CONST); SKIP();
<Normal>   "0" [xX] [A-Za-z0-9]+ [lL]                   := send(ZENTOK_LHEXINT_CONST); SKIP();

<Normal>   [0-9]* "." [0-9]+ ([Ee] [+-]? [0-9]+)? [fF]  := send(ZENTOK_FLOAT_CONST); SKIP();
<Normal>   [0-9]+ [Ee] [+-]? [0-9]+ [fF]                := send(ZENTOK_FLOAT_CONST); SKIP();

<Normal>   [0-9]* "." [0-9]+ ([Ee] [+-]? [0-9]+)? [dD]? := send(ZENTOK_DOUBLE_CONST); SKIP();
<Normal>   [0-9]+ [Ee] [+-]? [0-9]+ [dD]?               := send(ZENTOK_DOUBLE_CONST); SKIP();

<Normal>   '\''    => Char  := text = cursor; SKIP();
<Char>     '\\' .           := SKIP();
<Char>     '\''   => Normal := send(ZENTOK_CHAR_CONST); SKIP();
<Char>     [^]              := SKIP();

<Normal>   '"'    => String := text = cursor; SKIP();
<String>   '"'    => Normal := send(ZENTOK_STRING_CONST); SKIP();
<String>   '\\' .           := SKIP();
<String>   [^]              := SKIP();

<Normal>   "/*"    :=> Comment
<Comment>  "*" "/" :=> Normal
<Comment>  "\r" "\n"  => Comment :=  newLine(); SKIP();
<Comment>  "\n"       => Comment :=  newLine(); SKIP();
<Comment>  [^]       :=> Comment

<Normal>      "//"            :=> Skiptoeol
<Skiptoeol>   "\r" "\n"        => Normal      :=  newLine(); SKIP();
<Skiptoeol>   "\n"             => Normal      :=  newLine(); SKIP();
<Skiptoeol>   "\\" "\r"? "\n" :=> Skiptoeol
<Skiptoeol>   [^]             :=> Skiptoeol

<Normal>   "\r" "\n" := newLine(); SKIP();
<Normal>   "\n"      := newLine(); SKIP();
<Normal>   " "       := SKIP();
<Normal>   "\t"      := SKIP();

<Normal>   "\000" := send(ZENTOK_EOF); SKIP();
<Normal>   [^]    := send(ZENTOK_ERR); SKIP();
*/

do_fill:
    printf("Lexer::Impl::push/do_fill: buffer %lu, marker %lu, start %lu, cursor %lu, limit %lu, state %d, input %lu, inputSize %lu, isEof %d\n",
           (unsigned long)buffer, (unsigned long)marker, (unsigned long)start, (unsigned long)cursor, (unsigned long)limit, state, (unsigned long)input, inputSize, isEof);
    printf("do_fill: limit - cursor %ld\n", limit - cursor);
    size_t unfinishedSize = cursor-start;
    printf(
        "scanner needs a refill. Exiting for now with:\n"
        "    saved fill state = %d\n"
        "    unfinished token size = %ld\n",
        state,
        unfinishedSize
    );

    if(0<unfinishedSize && start<limit)
    {
        printf("    unfinished token is :");
        fwrite(start, 1, cursor-start, stdout);
        putchar('\n');
    }

    //    if(eof==true) goto do_start;

    /*
     * Once we get here, we can get rid of
     * everything before start and after limit.
     */
    if(buffer<start)
    {
        size_t startOffset = start-buffer;
        printf("** cleanup, startOffset = %lu\n", startOffset);
        memmove(buffer, start, limit-start);
        marker -= startOffset;
        cursor -= startOffset;
        limit -= startOffset;
        start -= startOffset;
    }
    return true;
}
