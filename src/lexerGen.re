// if first line is a #include, QtCreator recognizes this as a C/C++ file for syntax highlighting.
#include <typeinfo>

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
        if((_lastToken != ZENTOK_STRUCT_TYPE) && (_lastToken != ZENTOK_AUTO)) {
            feedToken(token(s, ZENTOK_AUTO_STRUCT));
        }
    }

    if(_context.isFunctionExpected()) {
        if((_lastToken != ZENTOK_FUNCTION_TYPE) && (_lastToken != ZENTOK_AUTO)) {
            feedToken(token(s, ZENTOK_AUTO_FUNCTION));
        }
    }

    feedToken(token(s, ZENTOK_LCURLY));
}

inline void Lexer::Impl::sendReturn(Scanner* s) {
    const Ast::RoutineDefn* rd = 0;
    const Ast::RootFunctionDefn* rfd = 0;
    const Ast::ChildFunctionDefn* cfd = 0;
    for(Context::TypeSpecStack::const_reverse_iterator it = _context.typeSpecStack().rbegin(); it != _context.typeSpecStack().rend(); ++it) {
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

<Normal>   "<<="   := feedToken(token(s, ZENTOK_SHIFTLEFTEQUAL)); continue;
<Normal>   ">>="   := feedToken(token(s, ZENTOK_SHIFTRIGHTEQUAL)); continue;
<Normal>   ":="    := feedToken(token(s, ZENTOK_DEFINEEQUAL)); continue;
<Normal>   "*="    := feedToken(token(s, ZENTOK_TIMESEQUAL)); continue;
<Normal>   "/="    := feedToken(token(s, ZENTOK_DIVIDEEQUAL)); continue;
<Normal>   "-="    := feedToken(token(s, ZENTOK_MINUSEQUAL)); continue;
<Normal>   "+="    := feedToken(token(s, ZENTOK_PLUSEQUAL)); continue;
<Normal>   "%="    := feedToken(token(s, ZENTOK_MODEQUAL)); continue;
<Normal>   "&="    := feedToken(token(s, ZENTOK_BITWISEANDEQUAL)); continue;
<Normal>   "^="    := feedToken(token(s, ZENTOK_BITWISEXOREQUAL)); continue;
<Normal>   "|="    := feedToken(token(s, ZENTOK_BITWISEOREQUAL)); continue;
<Normal>   "!="    := feedToken(token(s, ZENTOK_NOTEQUAL)); continue;
<Normal>   "=="    := feedToken(token(s, ZENTOK_EQUAL)); continue;
<Normal>   "<="    := feedToken(token(s, ZENTOK_LTE)); continue;
<Normal>   ">="    := feedToken(token(s, ZENTOK_GTE)); continue;
<Normal>   ">>"    := feedToken(token(s, ZENTOK_SHR)); continue;
<Normal>   "<<"    := feedToken(token(s, ZENTOK_SHL)); continue;
<Normal>   "++"    := feedToken(token(s, ZENTOK_INC)); continue;
<Normal>   "--"    := feedToken(token(s, ZENTOK_DEC)); continue;
<Normal>   "=>"    := feedToken(token(s, ZENTOK_LINK)); continue;
<Normal>   "->"    := feedToken(token(s, ZENTOK_RESERVED)); continue;
<Normal>   "&&"    := feedToken(token(s, ZENTOK_AND)); continue;
<Normal>   "||"    := feedToken(token(s, ZENTOK_OR)); continue;
<Normal>   ";"     := feedToken(token(s, ZENTOK_SEMI)); continue;
<Normal>   "!"     := feedToken(token(s, ZENTOK_NOT)); continue;
<Normal>   "="     := feedToken(token(s, ZENTOK_ASSIGNEQUAL)); continue;
<Normal>   "~"     := feedToken(token(s, ZENTOK_BITWISENOT)); continue;
<Normal>   "^"     := feedToken(token(s, ZENTOK_BITWISEXOR)); continue;
<Normal>   "|"     := feedToken(token(s, ZENTOK_BITWISEOR)); continue;
<Normal>   "&"     := feedToken(token(s, ZENTOK_BITWISEAND)); continue;
<Normal>   "?"     := feedToken(token(s, ZENTOK_QUESTION)); continue;
<Normal>   ":"     := feedToken(token(s, ZENTOK_COLON)); continue;
<Normal>   "::"    := feedToken(token(s, ZENTOK_SCOPE)); continue;
<Normal>   "."     := feedToken(token(s, ZENTOK_DOT)); continue;
<Normal>   ","     := feedToken(token(s, ZENTOK_COMMA)); continue;
<Normal>   "@"     := feedToken(token(s, ZENTOK_AMP)); continue;
<Normal>   "("     := feedToken(token(s, ZENTOK_LBRACKET)); continue;
<Normal>   ")"     := feedToken(token(s, ZENTOK_RBRACKET)); continue;
<Normal>   "["     := feedToken(token(s, ZENTOK_LSQUARE)); continue;
<Normal>   "]"     := feedToken(token(s, ZENTOK_RSQUARE)); continue;
<Normal>   "{"     := sendOpenCurly(s); continue;
<Normal>   "}"     := feedToken(token(s, ZENTOK_RCURLY)); continue;
<Normal>   "%"     := feedToken(token(s, ZENTOK_MOD)); continue;
<Normal>   "<"     := sendLessThan(s); continue;
<Normal>   ">"     := feedToken(token(s, ZENTOK_GT)); continue;
<Normal>   "+"     := feedToken(token(s, ZENTOK_PLUS)); continue;
<Normal>   "-"     := feedToken(token(s, ZENTOK_MINUS)); continue;
<Normal>   "*"     := feedToken(token(s, ZENTOK_STAR)); continue;
<Normal>   "/"     := feedToken(token(s, ZENTOK_DIVIDE)); continue;

<Normal>   "import"    := feedToken(token(s, ZENTOK_IMPORT)); continue;
<Normal>   "include"   := feedToken(token(s, ZENTOK_INCLUDE)); continue;
<Normal>   "namespace" := feedToken(token(s, ZENTOK_NAMESPACE)); continue;

<Normal>   "private"   := feedToken(token(s, ZENTOK_PRIVATE)); continue;
<Normal>   "public"    := feedToken(token(s, ZENTOK_PUBLIC)); continue;
<Normal>   "internal"  := feedToken(token(s, ZENTOK_INTERNAL)); continue;
<Normal>   "external"  := feedToken(token(s, ZENTOK_EXTERNAL)); continue;

<Normal>   "typedef"   := feedToken(token(s, ZENTOK_TYPEDEF)); continue;
<Normal>   "template"  := feedToken(token(s, ZENTOK_TEMPLATE)); continue;
<Normal>   "enum"      := feedToken(token(s, ZENTOK_ENUM)); continue;
<Normal>   "struct"    := feedToken(token(s, ZENTOK_STRUCT)); continue;
<Normal>   "routine"   := feedToken(token(s, ZENTOK_ROUTINE)); continue;
<Normal>   "function"  := feedToken(token(s, ZENTOK_FUNCTION)); continue;
<Normal>   "event"     := feedToken(token(s, ZENTOK_EVENT)); continue;

<Normal>   "property"  := feedToken(token(s, ZENTOK_PROPERTY)); continue;
<Normal>   "get"       := feedToken(token(s, ZENTOK_GET)); continue;
<Normal>   "set"       := feedToken(token(s, ZENTOK_SET)); continue;

<Normal>   "native"    := feedToken(token(s, ZENTOK_NATIVE)); continue;
<Normal>   "abstract"  := feedToken(token(s, ZENTOK_ABSTRACT)); continue;
<Normal>   "const"     := feedToken(token(s, ZENTOK_CONST)); continue;

<Normal>   "coerce"    := feedToken(token(s, ZENTOK_COERCE)); continue;
<Normal>   "default"   := feedToken(token(s, ZENTOK_DEFAULT)); continue;
<Normal>   "typeof"    := feedToken(token(s, ZENTOK_TYPEOF)); continue;

<Normal>   "auto"      := feedToken(token(s, ZENTOK_AUTO)); continue;
<Normal>   "print"     := feedToken(token(s, ZENTOK_PRINT)); continue;
<Normal>   "if"        := feedToken(token(s, ZENTOK_IF)); continue;
<Normal>   "else"      := feedToken(token(s, ZENTOK_ELSE)); continue;
<Normal>   "while"     := feedToken(token(s, ZENTOK_WHILE)); continue;
<Normal>   "do"        := feedToken(token(s, ZENTOK_DO)); continue;
<Normal>   "for"       := feedToken(token(s, ZENTOK_FOR)); continue;
<Normal>   "foreach"   := feedToken(token(s, ZENTOK_FOREACH)); continue;
<Normal>   "in"        := feedToken(token(s, ZENTOK_IN)); continue;
<Normal>   "has"       := feedToken(token(s, ZENTOK_HAS)); continue;
<Normal>   "switch"    := feedToken(token(s, ZENTOK_SWITCH)); continue;
<Normal>   "case"      := feedToken(token(s, ZENTOK_CASE)); continue;
<Normal>   "break"     := feedToken(token(s, ZENTOK_BREAK)); continue;
<Normal>   "continue"  := feedToken(token(s, ZENTOK_CONTINUE)); continue;
<Normal>   "run"       := feedToken(token(s, ZENTOK_RUN)); continue;

<Normal>   "return"    := sendReturn(s); continue;

<Normal>   "false"     := feedToken(token(s, ZENTOK_FALSE_CONST)); continue;
<Normal>   "true"      := feedToken(token(s, ZENTOK_TRUE_CONST)); continue;

<Normal>   "@" [a-zA-Z][a-zA-Z0-9_]* := feedToken(token(s, ZENTOK_KEY_CONST)); continue;

<Normal>       [a-zA-Z][a-zA-Z0-9_]* := sendId(s); continue;

<Normal>   "0" [0-7]+      := feedToken(token(s, ZENTOK_OCTINT_CONST)); continue;
<Normal>   "0" [0-7]+ [lL] := feedToken(token(s, ZENTOK_LOCTINT_CONST)); continue;

<Normal>   [0-9]+      := feedToken(token(s, ZENTOK_DECINT_CONST)); continue;
<Normal>   [0-9]+ [lL] := feedToken(token(s, ZENTOK_LDECINT_CONST)); continue;

<Normal>   "0" [xX] [A-Za-z0-9]+                        := feedToken(token(s, ZENTOK_HEXINT_CONST)); continue;
<Normal>   "0" [xX] [A-Za-z0-9]+ [lL]                   := feedToken(token(s, ZENTOK_LHEXINT_CONST)); continue;

<Normal>   [0-9]* "." [0-9]+ ([Ee] [+-]? [0-9]+)? [fF]  := feedToken(token(s, ZENTOK_FLOAT_CONST)); continue;
<Normal>   [0-9]+ [Ee] [+-]? [0-9]+ [fF]                := feedToken(token(s, ZENTOK_FLOAT_CONST)); continue;

<Normal>   [0-9]* "." [0-9]+ ([Ee] [+-]? [0-9]+)? [dD]? := feedToken(token(s, ZENTOK_DOUBLE_CONST)); continue;
<Normal>   [0-9]+ [Ee] [+-]? [0-9]+ [dD]?               := feedToken(token(s, ZENTOK_DOUBLE_CONST)); continue;

<Normal>   '\''    => Char  := s->text = s->cur; continue;
<Char>     '\\' .           := continue;
<Char>     '\''   => Normal := feedToken(token(s, ZENTOK_CHAR_CONST)); continue;
<Char>     [^]              := continue;

<Normal>   '"'    => String := s->text = s->cur; continue;
<String>   '"'    => Normal := feedToken(token(s, ZENTOK_STRING_CONST)); continue;
<String>   '\\' .           := continue;
<String>   [^]              := continue;

<Normal>   "/*"    :=> Comment
<Comment>  "*" "/" :=> Normal
<Comment>  "\r" "\n"  => Comment :=  newLine(s); continue;
<Comment>  "\n"       => Comment :=  newLine(s); continue;
<Comment>  [^]       :=> Comment

<Normal>      "//"            :=> Skiptoeol
<Skiptoeol>   "\r" "\n"        => Normal      :=  newLine(s); continue;
<Skiptoeol>   "\n"             => Normal      :=  newLine(s); continue;
<Skiptoeol>   "\\" "\r"? "\n" :=> Skiptoeol
<Skiptoeol>   [^]             :=> Skiptoeol

<Normal>   "\r" "\n" := newLine(s); continue;
<Normal>   "\n"      := newLine(s); continue;
<Normal>   " "       := continue;
<Normal>   "\t"      := continue;

<Normal>   "\000" := feedToken(token(s, ZENTOK_EOF)); continue;
<Normal>   [^]    := feedToken(token(s, ZENTOK_ERR)); continue;
*/

/*
<!*>                 := fprintf(stderr, "Normal\n");
<!Comment,Skiptoeol> := fprintf(stderr, "Comment\n");
*/
    }
    feedToken(token(s, ZENTOK_EOF));
}

/*
//ESC = [\\] ([abfnrtv?'"\\] | "x" HEX+ | OCT+);
*/
