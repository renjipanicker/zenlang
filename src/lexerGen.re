#define	BSIZE	8192

/*!types:re2c */

struct Scanner {
    FILE* fp;
    char* cur;
    char* tok;
    char* lim;
    char buffer[BSIZE];
    char yych;
    enum ScanCondition cond;
    int state;

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
    size_t got = 0, cnt;

    if ((!(feof(s->fp))) && ((s->lim - s->tok) < (int)len)) {
        if (s->tok > s->buffer) {
            cnt = s->tok - s->buffer;
            memcpy(s->buffer, s->tok, s->lim - s->tok);
            s->tok -= cnt;
            s->cur -= cnt;
            s->lim -= cnt;
            s->sol -= cnt;
            s->mar -= cnt;
            if(s->text > 0) {
                assert(s->text >= (s->buffer + cnt));
                s->text -= cnt;
            }
            cnt = &s->buffer[BSIZE] - s->lim;
        } else {
            cnt = BSIZE;
        }

        got = fread(s->lim, 1, cnt, s->fp);
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
    return createToken(id, s->row, s->cur-s->sol, s->mar, s->cur);
}

inline bool Lexer::Impl::trySendId(Scanner* s, const Ast::TypeSpec* typeSpec) {
    if(dynamic_cast<const Ast::StructDefn*>(typeSpec) != 0) {
        _parser.feed(token(s, ZENTOK_STRUCT_TYPE));
        return true;
    }
    if(dynamic_cast<const Ast::Routine*>(typeSpec) != 0) {
        _parser.feed(token(s, ZENTOK_ROUTINE_TYPE));
        return true;
    }
    if(dynamic_cast<const Ast::Function*>(typeSpec) != 0) {
        _parser.feed(token(s, ZENTOK_FUNCTION_TYPE));
        return true;
    }
    if(dynamic_cast<const Ast::TypeSpec*>(typeSpec) != 0) {
        _parser.feed(token(s, ZENTOK_OTHER_TYPE));
        return true;
    }
    return false;
}

inline void Lexer::Impl::sendId(Scanner* s) {
    Ast::Token td = token(s, 0);
    if(trySendId(s, _context.currentTypeRef()))
        return;
    if(trySendId(s, _context.hasRootTypeSpec(td)))
        return;
    _parser.feed(token(s, ZENTOK_ID));
}

inline void Lexer::Impl::sendReturn(Scanner* s) {
    const Ast::RoutineDefn* rd = 0;
    const Ast::RootFunctionDefn* rfd = 0;
    const Ast::ChildFunctionDefn* cfd = 0;
    for(Context::TypeSpecStack::const_reverse_iterator it = _context.typeSpecStack().rbegin(); it != _context.typeSpecStack().rend(); ++it) {
        const Ast::TypeSpec* ts = *it;
        if((rd = dynamic_cast<const Ast::RoutineDefn*>(ts)) != 0) {
            _parser.feed(token(s, ZENTOK_RRETURN));
            return;
        }
        if((rfd = dynamic_cast<const Ast::RootFunctionDefn*>(ts)) != 0) {
            _parser.feed(token(s, ZENTOK_FRETURN));
            return;
        }
        if((cfd = dynamic_cast<const Ast::ChildFunctionDefn*>(ts)) != 0) {
            _parser.feed(token(s, ZENTOK_FRETURN));
            return;
        }
    }
    throw Exception("Invalid return in lexer\n");
}

void Lexer::Impl::scan(Scanner *s) {
    s->tok = s->cur;

/*!re2c
re2c:define:YYGETSTATE       = "s->state";
re2c:define:YYGETSTATE:naked = 1;
re2c:define:YYCONDTYPE       = ScanCondition;
re2c:indent:top              = 1;
re2c:cond:goto               = "continue;";
*/

/*!getstate:re2c */

    while(s->cur < s->lim) {
        s->mar = s->tok = s->cur;

/*!re2c

re2c:define:YYCTYPE          = "char";
re2c:define:YYCURSOR         = s->cur;
re2c:define:YYLIMIT          = s->lim;
re2c:define:YYMARKER         = s->tok;
re2c:define:YYFILL@len       = #;
re2c:define:YYFILL:naked     = 1;
re2c:define:YYFILL           = "fill(s, #);";
re2c:define:YYSETSTATE@state = #;
re2c:define:YYSETSTATE           = "s->state = #;";
re2c:define:YYSETCONDITION       = "s->cond = #;";
re2c:define:YYSETCONDITION@cond  = #;
re2c:define:YYGETCONDITION       = s->cond;
re2c:define:YYGETCONDITION:naked = 1;
re2c:variable:yych           = s->yych;
re2c:yych:emit               = 0;
re2c:indent:top              = 2;
re2c:condenumprefix          = EState;

<Normal>   "<<="   := _parser.feed(token(s, ZENTOK_SHIFTLEFTEQUAL)); continue;
<Normal>   ">>="   := _parser.feed(token(s, ZENTOK_SHIFTRIGHTEQUAL)); continue;
<Normal>   ":="    := _parser.feed(token(s, ZENTOK_DEFINEEQUAL)); continue;
<Normal>   "*="    := _parser.feed(token(s, ZENTOK_TIMESEQUAL)); continue;
<Normal>   "/="    := _parser.feed(token(s, ZENTOK_DIVIDEEQUAL)); continue;
<Normal>   "-="    := _parser.feed(token(s, ZENTOK_MINUSEQUAL)); continue;
<Normal>   "+="    := _parser.feed(token(s, ZENTOK_PLUSEQUAL)); continue;
<Normal>   "%="    := _parser.feed(token(s, ZENTOK_MODEQUAL)); continue;
<Normal>   "&="    := _parser.feed(token(s, ZENTOK_BITWISEANDEQUAL)); continue;
<Normal>   "^="    := _parser.feed(token(s, ZENTOK_BITWISEXOREQUAL)); continue;
<Normal>   "|="    := _parser.feed(token(s, ZENTOK_BITWISEOREQUAL)); continue;
<Normal>   "!="    := _parser.feed(token(s, ZENTOK_NOTEQUAL)); continue;
<Normal>   "=="    := _parser.feed(token(s, ZENTOK_EQUAL)); continue;
<Normal>   "<="    := _parser.feed(token(s, ZENTOK_LTE)); continue;
<Normal>   ">="    := _parser.feed(token(s, ZENTOK_GTE)); continue;
<Normal>   ">>"    := _parser.feed(token(s, ZENTOK_SHR)); continue;
<Normal>   "<<"    := _parser.feed(token(s, ZENTOK_SHL)); continue;
<Normal>   "++"    := _parser.feed(token(s, ZENTOK_INC)); continue;
<Normal>   "--"    := _parser.feed(token(s, ZENTOK_DEC)); continue;
<Normal>   "=>"    := _parser.feed(token(s, ZENTOK_LINK)); continue;
<Normal>   "->"    := _parser.feed(token(s, ZENTOK_JOIN)); continue;
<Normal>   "&&"    := _parser.feed(token(s, ZENTOK_AND)); continue;
<Normal>   "||"    := _parser.feed(token(s, ZENTOK_OR)); continue;
<Normal>   "}"     := _parser.feed(token(s, ZENTOK_RCURLY)); continue;
<Normal>   ";"     := _parser.feed(token(s, ZENTOK_SEMI)); continue;
<Normal>   "!"     := _parser.feed(token(s, ZENTOK_NOT)); continue;
<Normal>   "="     := _parser.feed(token(s, ZENTOK_ASSIGNEQUAL)); continue;
<Normal>   "~"     := _parser.feed(token(s, ZENTOK_BITWISENOT)); continue;
<Normal>   "^"     := _parser.feed(token(s, ZENTOK_BITWISEXOR)); continue;
<Normal>   "|"     := _parser.feed(token(s, ZENTOK_BITWISEOR)); continue;
<Normal>   "&"     := _parser.feed(token(s, ZENTOK_BITWISEAND)); continue;
<Normal>   "?"     := _parser.feed(token(s, ZENTOK_QUESTION)); continue;
<Normal>   ":"     := _parser.feed(token(s, ZENTOK_COLON)); continue;
<Normal>   "::"    := _parser.feed(token(s, ZENTOK_SCOPE)); continue;
<Normal>   "."     := _parser.feed(token(s, ZENTOK_DOT)); continue;
<Normal>   ","     := _parser.feed(token(s, ZENTOK_COMMA)); continue;
<Normal>   "@"     := _parser.feed(token(s, ZENTOK_AMP)); continue;
<Normal>   "("     := _parser.feed(token(s, ZENTOK_LBRACKET)); continue;
<Normal>   ")"     := _parser.feed(token(s, ZENTOK_RBRACKET)); continue;
<Normal>   "["     := _parser.feed(token(s, ZENTOK_LSQUARE)); continue;
<Normal>   "]"     := _parser.feed(token(s, ZENTOK_RSQUARE)); continue;
<Normal>   "{"     := _parser.feed(token(s, ZENTOK_LCURLY)); continue;
<Normal>   "}"     := _parser.feed(token(s, ZENTOK_RCURLY)); continue;
<Normal>   "%"     := _parser.feed(token(s, ZENTOK_MOD)); continue;
<Normal>   "<"     := _parser.feed(token(s, ZENTOK_LT)); continue;
<Normal>   ">"     := _parser.feed(token(s, ZENTOK_GT)); continue;
<Normal>   "+"     := _parser.feed(token(s, ZENTOK_PLUS)); continue;
<Normal>   "-"     := _parser.feed(token(s, ZENTOK_MINUS)); continue;
<Normal>   "*"     := _parser.feed(token(s, ZENTOK_STAR)); continue;
<Normal>   "/"     := _parser.feed(token(s, ZENTOK_DIVIDE)); continue;

<Normal>   "import"    := _parser.feed(token(s, ZENTOK_IMPORT)); continue;
<Normal>   "include"   := _parser.feed(token(s, ZENTOK_INCLUDE)); continue;
<Normal>   "namespace" := _parser.feed(token(s, ZENTOK_NAMESPACE)); continue;

<Normal>   "private"   := _parser.feed(token(s, ZENTOK_PRIVATE)); continue;
<Normal>   "internal"  := _parser.feed(token(s, ZENTOK_INTERNAL)); continue;
<Normal>   "protected" := _parser.feed(token(s, ZENTOK_PROTECTED)); continue;
<Normal>   "public"    := _parser.feed(token(s, ZENTOK_PUBLIC)); continue;
<Normal>   "export"    := _parser.feed(token(s, ZENTOK_EXPORT)); continue;

<Normal>   "typedef"   := _parser.feed(token(s, ZENTOK_TYPEDEF)); continue;
<Normal>   "template"  := _parser.feed(token(s, ZENTOK_TEMPLATE)); continue;
<Normal>   "enum"      := _parser.feed(token(s, ZENTOK_ENUM)); continue;
<Normal>   "struct"    := _parser.feed(token(s, ZENTOK_STRUCT)); continue;
<Normal>   "routine"   := _parser.feed(token(s, ZENTOK_ROUTINE)); continue;
<Normal>   "function"  := _parser.feed(token(s, ZENTOK_FUNCTION)); continue;
<Normal>   "event"     := _parser.feed(token(s, ZENTOK_EVENT)); continue;

<Normal>   "native"    := _parser.feed(token(s, ZENTOK_NATIVE)); continue;
<Normal>   "const"     := _parser.feed(token(s, ZENTOK_CONST)); continue;

<Normal>   "coerce"    := _parser.feed(token(s, ZENTOK_COERCE)); continue;
<Normal>   "default"   := _parser.feed(token(s, ZENTOK_DEFAULT)); continue;
<Normal>   "typeof"    := _parser.feed(token(s, ZENTOK_TYPEOF)); continue;

<Normal>   "local"     := _parser.feed(token(s, ZENTOK_LOCAL)); continue;
<Normal>   "print"     := _parser.feed(token(s, ZENTOK_PRINT)); continue;
<Normal>   "if"        := _parser.feed(token(s, ZENTOK_IF)); continue;
<Normal>   "else"      := _parser.feed(token(s, ZENTOK_ELSE)); continue;
<Normal>   "while"     := _parser.feed(token(s, ZENTOK_WHILE)); continue;
<Normal>   "do"        := _parser.feed(token(s, ZENTOK_DO)); continue;
<Normal>   "for"       := _parser.feed(token(s, ZENTOK_FOR)); continue;
<Normal>   "foreach"   := _parser.feed(token(s, ZENTOK_FOREACH)); continue;
<Normal>   "in"        := _parser.feed(token(s, ZENTOK_IN)); continue;
<Normal>   "has"       := _parser.feed(token(s, ZENTOK_HAS)); continue;
<Normal>   "switch"    := _parser.feed(token(s, ZENTOK_SWITCH)); continue;
<Normal>   "case"      := _parser.feed(token(s, ZENTOK_CASE)); continue;
<Normal>   "break"     := _parser.feed(token(s, ZENTOK_BREAK)); continue;
<Normal>   "continue"  := _parser.feed(token(s, ZENTOK_CONTINUE)); continue;
<Normal>   "run"       := _parser.feed(token(s, ZENTOK_RUN)); continue;

<Normal>   "return"    := sendReturn(s); continue;

<Normal>   "false"     := _parser.feed(token(s, ZENTOK_FALSE_CONST)); continue;
<Normal>   "true"      := _parser.feed(token(s, ZENTOK_TRUE_CONST)); continue;

<Normal>   "@" [a-zA-Z][a-zA-Z0-9_]* := _parser.feed(token(s, ZENTOK_KEY)); continue;

<Normal>       [a-zA-Z][a-zA-Z0-9_]* := sendId(s); continue;

<Normal>   "0" [0-7]+      := _parser.feed(token(s, ZENTOK_OCTINT_CONST)); continue;
<Normal>   "0" [0-7]+ [lL] := _parser.feed(token(s, ZENTOK_LOCTINT_CONST)); continue;

<Normal>   [0-9]+      := _parser.feed(token(s, ZENTOK_DECINT_CONST)); continue;
<Normal>   [0-9]+ [lL] := _parser.feed(token(s, ZENTOK_LDECINT_CONST)); continue;

<Normal>   "0" [xX] [A-Za-z0-9]+                        := _parser.feed(token(s, ZENTOK_HEXINT_CONST)); continue;
<Normal>   "0" [xX] [A-Za-z0-9]+ [lL]                   := _parser.feed(token(s, ZENTOK_LHEXINT_CONST)); continue;

<Normal>   [0-9]* "." [0-9]+ ([Ee] [+-]? [0-9]+)? [fF]  := _parser.feed(token(s, ZENTOK_FLOAT_CONST)); continue;
<Normal>   [0-9]+ [Ee] [+-]? [0-9]+ [fF]                := _parser.feed(token(s, ZENTOK_FLOAT_CONST)); continue;

<Normal>   [0-9]* "." [0-9]+ ([Ee] [+-]? [0-9]+)? [dD]? := _parser.feed(token(s, ZENTOK_DOUBLE_CONST)); continue;
<Normal>   [0-9]+ [Ee] [+-]? [0-9]+ [dD]?               := _parser.feed(token(s, ZENTOK_DOUBLE_CONST)); continue;

<Normal>   '\''    => Char  := s->text = s->cur; continue;
<Char>     '\\' .           := continue;
<Char>     '\''   => Normal := _parser.feed(token(s, ZENTOK_CHAR_CONST)); continue;
<Char>     [^]              := continue;

<Normal>   '"'    => String := s->text = s->cur; continue;
<String>   '\\' .           := continue;
<String>   '"'    => Normal := _parser.feed(token(s, ZENTOK_STRING_CONST)); continue;
<String>   [^]              := continue;

<Normal>    "/*"    :=> Comment
<Comment>   "*" "/" :=> Normal
<Comment>   [^]     :=> Comment

<Normal>      "//"            :=> Skiptoeol
<Skiptoeol>   "\\" "\r"? "\n" :=> Skiptoeol
<Skiptoeol>   "\r" "\n"        => Normal      :=  newLine(s); continue;
<Skiptoeol>   "\n"             => Normal      :=  newLine(s); continue;
<Skiptoeol>   [^]             :=> Skiptoeol

<Normal>   "\r" "\n" := newLine(s); continue;
<Normal>   "\n"      := newLine(s); continue;
<Normal>   " "       := continue;

<Normal>   "\000" := _parser.feed(token(s, ZENTOK_EOF)); continue;
<Normal>   [^]    := _parser.feed(token(s, ZENTOK_ERR)); continue;
*/

/*
<!*>                 := fprintf(stderr, "Normal\n");
<!Comment,Skiptoeol> := fprintf(stderr, "Comment\n");
*/
    }
    _parser.feed(token(s, ZENTOK_EOF));
}

/*
//ESC = [\\] ([abfnrtv?'"\\] | "x" HEX+ | OCT+);

*/
