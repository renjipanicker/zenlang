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

    if (!s->eof && s->lim - s->tok < len)
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

<Normal>	"&"        := _parser.feed(token(s, ZENTOK_BITWISEAND)); continue;

<Normal>	"("        := _parser.feed(token(s, ZENTOK_LBRACKET)); continue;
<Normal>	")"        := _parser.feed(token(s, ZENTOK_RBRACKET)); continue;
<Normal>	"{"        := _parser.feed(token(s, ZENTOK_LCURLY)); continue;
<Normal>	"}"        := _parser.feed(token(s, ZENTOK_RCURLY)); continue;
<Normal>	","        := _parser.feed(token(s, ZENTOK_COMMA)); continue;
<Normal>	";"        := _parser.feed(token(s, ZENTOK_SEMI)); continue;
<Normal>	"::"       := _parser.feed(token(s, ZENTOK_SCOPE)); continue;

<Normal>	"import"    := _parser.feed(token(s, ZENTOK_IMPORT)); continue;
<Normal>	"include"   := _parser.feed(token(s, ZENTOK_INCLUDE)); continue;
<Normal>	"namespace" := _parser.feed(token(s, ZENTOK_NAMESPACE)); continue;

<Normal>	"private"   := _parser.feed(token(s, ZENTOK_PRIVATE)); continue;
<Normal>	"internal"  := _parser.feed(token(s, ZENTOK_INTERNAL)); continue;
<Normal>	"protected" := _parser.feed(token(s, ZENTOK_PROTECTED)); continue;
<Normal>	"public"    := _parser.feed(token(s, ZENTOK_PUBLIC)); continue;
<Normal>	"export"    := _parser.feed(token(s, ZENTOK_EXPORT)); continue;

<Normal>	"typedef"  := _parser.feed(token(s, ZENTOK_TYPEDEF)); continue;
<Normal>	"struct"   := _parser.feed(token(s, ZENTOK_STRUCT)); continue;
<Normal>	"function" := _parser.feed(token(s, ZENTOK_FUNCTION)); continue;

<Normal>	"native"   := _parser.feed(token(s, ZENTOK_NATIVE)); continue;
<Normal>	"const"    := _parser.feed(token(s, ZENTOK_CONST)); continue;

<Normal>	"switch"   := _parser.feed(token(s, ZENTOK_STRUCT)); continue;

<Normal>	[a-zA-Z][a-zA-Z0-9_]* := _parser.feed(token(s, ZENTOK_ID)); continue;

<Normal>	"/*" :=> Comment
<Normal>	"//" :=> Skiptoeol

<Normal>	'"' => String := s->text = s->cur; continue;

<Normal>	((" ")|("\r"? "\n")) := continue;

<Normal>	"\000" := _parser.feed(token(s, ZENTOK_EOF)); continue;

<Normal>	[^] := _parser.feed(token(s, ZENTOK_ERR)); continue;

<Comment>	"*" "/" :=> Normal
<Comment>	[^] :=> Comment

<Skiptoeol>	"\\" "\r"? "\n" :=> Skiptoeol

<Skiptoeol>	"\r" "\n" => Normal := continue;

<Skiptoeol>	"\n" => Normal := continue;

<Skiptoeol>	[^] :=> Skiptoeol

<String>	'\\' . := continue;

<String>	'"' => Normal := _parser.feed(token(s, ZENTOK_STRING_CONST)); continue;

<String>	[^] := continue;
*/

/*
<!*>					:= fprintf(stderr, "Normal\n");
<!Comment,Skiptoeol>	:= fprintf(stderr, "Comment\n");
*/
    }
    if(feof(s->fp)) {
        _parser.feed(token(s, ZENTOK_EOF));
    }
}
