#define	BSIZE	8192

/*!types:re2c */

struct Scanner {
    FILE* fp;
    char* cur;
    char* tok;
    char* lim;
    char* eof;
    char buffer[BSIZE];
    char yych;
    enum ScanContition  cond;
    int state;

    int row;
    int col;
    char* text;
};

static size_t fill(Scanner *s, size_t len)
{
    size_t got = ~0, cnt;

    if ((!(s->eof)) && ((s->lim - s->tok) < (int)len))
    {
        if (s->tok > s->buffer)
        {
            cnt = s->tok - s->buffer;
            memcpy(s->buffer, s->tok, s->lim - s->tok);
            s->tok -= cnt;
            s->cur -= cnt;
            s->lim -= cnt;
            if(s->text > 0) {
                assert(s->text >= (s->buffer + cnt));
                s->text -= cnt;
            }
            cnt = &s->buffer[BSIZE] - s->lim;
        }
        else
        {
            cnt = BSIZE;
        }
        if ((got = fread(s->lim, 1, cnt, s->fp)) != cnt)
        {
            s->eof = &s->lim[got];
        }
        s->lim += got;
    }
    if (s->eof && s->cur + len > s->eof)
    {
        return ~0; /* not enough input data */
    }
    return got;
}

size_t Lexer::Impl::init(Scanner *s)
{
    s->cur = s->tok = s->lim = s->buffer;
    s->text = 0;
    s->eof = 0;
    s->cond = EStateNormal;
    s->state = -1;

    return fill(s, 0);
}

static inline TokenData token(Scanner* s, const int& id) {
    if(s->text > 0) {
        char* t = s->text;
        s->text = 0;
        return TokenData::createT(id, s->row, s->col, t, s->cur - 1);
    }
    return TokenData::createT(id, s->row, s->col, s->tok, s->cur);
}

void Lexer::Impl::scan(Scanner *s)
{
    s->tok = s->cur;
/*!re2c
re2c:define:YYGETSTATE       = "s->state";
re2c:define:YYGETSTATE:naked = 1;
re2c:define:YYCONDTYPE       = ScanContition;
re2c:indent:top              = 1;
re2c:cond:goto               = "continue;";
*/

/*!getstate:re2c */

    for(;;)
    {
        s->tok = s->cur;
/*!re2c

re2c:define:YYCTYPE          = "char";
re2c:define:YYCURSOR         = s->cur;
re2c:define:YYLIMIT          = s->lim;
re2c:define:YYMARKER         = s->tok;
re2c:define:YYFILL@len       = #;
re2c:define:YYFILL:naked     = 1;
re2c:define:YYFILL           = "if (fill(s, #) == ~0) break;";
re2c:define:YYSETSTATE@state = #;
re2c:define:YYSETSTATE       = "s->state = #;";
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

<Normal>   "typedef"  := _parser.feed(token(s, ZENTOK_TYPEDEF)); continue;
<Normal>   "enum"     := _parser.feed(token(s, ZENTOK_ENUM)); continue;
<Normal>   "struct"   := _parser.feed(token(s, ZENTOK_STRUCT)); continue;
<Normal>   "routine"  := _parser.feed(token(s, ZENTOK_ROUTINE)); continue;
<Normal>   "function" := _parser.feed(token(s, ZENTOK_FUNCTION)); continue;
<Normal>   "event"    := _parser.feed(token(s, ZENTOK_EVENT)); continue;

<Normal>   "native"   := _parser.feed(token(s, ZENTOK_NATIVE)); continue;
<Normal>   "const"    := _parser.feed(token(s, ZENTOK_CONST)); continue;

<Normal>   "return"   := _parser.feed(token(s, ZENTOK_RETURN)); continue;

<Normal>   "@" [a-zA-Z][a-zA-Z0-9_]* := _parser.feed(token(s, ZENTOK_KEY)); continue;
<Normal>   [a-zA-Z][a-zA-Z0-9_]* := _parser.feed(token(s, ZENTOK_ID)); continue;

<Normal>   "0" [0-7]+      := _parser.feed(token(s, ZENTOK_OCTINT_CONST)); continue;
<Normal>   "0" [0-7]+ [lL] := _parser.feed(token(s, ZENTOK_LOCTINT_CONST)); continue;

<Normal>   [0-9]+      := _parser.feed(token(s, ZENTOK_DECINT_CONST)); continue;
<Normal>   [0-9]+ [lL] := _parser.feed(token(s, ZENTOK_LDECINT_CONST)); continue;

<Normal>   "0" [xX] [A-Za-z0-9]+      := _parser.feed(token(s, ZENTOK_HEXINT_CONST)); continue;
<Normal>   "0" [xX] [A-Za-z0-9]+ [lL] := _parser.feed(token(s, ZENTOK_LHEXINT_CONST)); continue;

<Normal>   [0-9]* "." [0-9]+ ([Ee] [+-]? [0-9]+)? [fF] := _parser.feed(token(s, ZENTOK_FLOAT_CONST)); continue;
<Normal>   [0-9]+ [Ee] [+-]? [0-9]+ [fF] := _parser.feed(token(s, ZENTOK_FLOAT_CONST)); continue;

<Normal>   [0-9]* "." [0-9]+ ([Ee] [+-]? [0-9]+)? [dD]? := _parser.feed(token(s, ZENTOK_DOUBLE_CONST)); continue;
<Normal>   [0-9]+ [Ee] [+-]? [0-9]+ [dD]? := _parser.feed(token(s, ZENTOK_DOUBLE_CONST)); continue;

<Normal>   '"' => String := s->text = s->cur; continue;
<String>   '\\' . := continue;
<String>   '"' => Normal := _parser.feed(token(s, ZENTOK_STRING_CONST)); continue;
<String>   [^] := continue;

<Normal>   "/*" :=> Comment
<Comment>   "*" "/" :=> Normal
<Comment>   [^] :=> Comment

<Normal>   "//" :=> Skiptoeol
<Skiptoeol>   "\\" "\r"? "\n" :=> Skiptoeol
<Skiptoeol>   "\r" "\n" => Normal := continue;
<Skiptoeol>   "\n" => Normal := continue;
<Skiptoeol>   [^] :=> Skiptoeol

<Normal>   ((" ")|("\r"? "\n")) := continue;
<Normal>   "\000" := _parser.feed(token(s, ZENTOK_EOF)); continue;
<Normal>   [^] := _parser.feed(token(s, ZENTOK_ERR)); continue;
*/

/*
<!*>                 := fprintf(stderr, "Normal\n");
<!Comment,Skiptoeol> := fprintf(stderr, "Comment\n");
*/
    }
    if(feof(s->fp)) {
        _parser.feed(token(s, ZENTOK_EOF));
    }
}

/*
//ESC = [\\] ([abfnrtv?'"\\] | "x" HEX+ | OCT+);

*/
