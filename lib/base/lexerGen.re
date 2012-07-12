// This file is adapted from one of the re2c samples (push.re)

// if first non-comment line is a #include, QtCreator recognizes this as a C/C++ file for syntax highlighting.
#include <typeinfo>

#if !defined(_WIN32)
#pragma GCC diagnostic ignored "-pedantic" // disable "comma at end of enum list" warning in generated code
#endif

// this causes the conditon enum to be generated
/*!types:re2c */

#define NEXT() { _start = _cursor; goto yy0; }

void z::Lexer::Impl::lex(ParserContext& pctx) {
/*!re2c
re2c:define:YYCTYPE          = "inputChar_t";
re2c:define:YYCURSOR         = _cursor;
re2c:define:YYLIMIT          = _limit;
re2c:define:YYMARKER         = _marker;
re2c:define:YYFILL           = "return;";
re2c:define:YYFILL:naked     = 1;

re2c:define:YYGETSTATE       = "_state";
re2c:define:YYGETSTATE:naked = 1;
re2c:define:YYSETSTATE@state = #;
re2c:define:YYSETSTATE           = "_state = #;";

re2c:define:YYCONDTYPE           = ScanCondition;
re2c:define:YYSETCONDITION       = "_cond = #;";
re2c:define:YYSETCONDITION@cond  = #;
re2c:define:YYGETCONDITION       = "_cond";
re2c:define:YYGETCONDITION:naked = 1;
re2c:cond:goto                   = "NEXT();";
re2c:condenumprefix              = EState;

re2c:variable:yyaccept       = _yyaccept;
re2c:variable:yych           = _yych;
re2c:yych:emit               = 0;
re2c:indent:top              = 2;

<Normal>   "<<="   := send(pctx, ZENTOK_SHIFTLEFTEQUAL); NEXT();
<Normal>   ">>="   := send(pctx, ZENTOK_SHIFTRIGHTEQUAL); NEXT();
<Normal>   ":="    := send(pctx, ZENTOK_DEFINEEQUAL); NEXT();
<Normal>   "*="    := send(pctx, ZENTOK_TIMESEQUAL); NEXT();
<Normal>   "/="    := send(pctx, ZENTOK_DIVIDEEQUAL); NEXT();
<Normal>   "-="    := send(pctx, ZENTOK_MINUSEQUAL); NEXT();
<Normal>   "+="    := send(pctx, ZENTOK_PLUSEQUAL); NEXT();
<Normal>   "%="    := send(pctx, ZENTOK_MODEQUAL); NEXT();
<Normal>   "&="    := send(pctx, ZENTOK_BITWISEANDEQUAL); NEXT();
<Normal>   "^="    := send(pctx, ZENTOK_BITWISEXOREQUAL); NEXT();
<Normal>   "|="    := send(pctx, ZENTOK_BITWISEOREQUAL); NEXT();
<Normal>   "!="    := send(pctx, ZENTOK_NOTEQUAL); NEXT();
<Normal>   "=="    := send(pctx, ZENTOK_EQUAL); NEXT();
<Normal>   "<="    := send(pctx, ZENTOK_LTE); NEXT();
<Normal>   ">="    := send(pctx, ZENTOK_GTE); NEXT();
<Normal>   ">>"    := send(pctx, ZENTOK_SHR); NEXT();
<Normal>   "<<"    := send(pctx, ZENTOK_SHL); NEXT();
<Normal>   "++"    := send(pctx, ZENTOK_INC); NEXT();
<Normal>   "--"    := send(pctx, ZENTOK_DEC); NEXT();
<Normal>   "=>"    := send(pctx, ZENTOK_LINK); NEXT();
<Normal>   "->"    := send(pctx, ZENTOK_RESERVED); NEXT();
<Normal>   "&&"    := send(pctx, ZENTOK_AND); NEXT();
<Normal>   "||"    := send(pctx, ZENTOK_OR); NEXT();
<Normal>   ";"     := send(pctx, ZENTOK_SEMI); NEXT();
<Normal>   "!"     := send(pctx, ZENTOK_NOT); NEXT();
<Normal>   "="     := send(pctx, ZENTOK_ASSIGNEQUAL); NEXT();
<Normal>   "~"     := send(pctx, ZENTOK_BITWISENOT); NEXT();
<Normal>   "^"     := send(pctx, ZENTOK_BITWISEXOR); NEXT();
<Normal>   "|"     := send(pctx, ZENTOK_BITWISEOR); NEXT();
<Normal>   "&"     := send(pctx, ZENTOK_BITWISEAND); NEXT();
<Normal>   "?"     := send(pctx, ZENTOK_QUESTION); NEXT();
<Normal>   ":"     := send(pctx, ZENTOK_COLON); NEXT();
<Normal>   "::"    := send(pctx, ZENTOK_SCOPE); NEXT();
<Normal>   "."     := send(pctx, ZENTOK_DOT); NEXT();
<Normal>   ","     := send(pctx, ZENTOK_COMMA); NEXT();
<Normal>   "@"     := send(pctx, ZENTOK_AMP); NEXT();
<Normal>   "("     := send(pctx, ZENTOK_LBRACKET); NEXT();
<Normal>   ")"     := send(pctx, ZENTOK_RBRACKET); NEXT();
<Normal>   "["     := send(pctx, ZENTOK_LSQUARE); NEXT();
<Normal>   "]"     := send(pctx, ZENTOK_RSQUARE); NEXT();
<Normal>   "{"     := sendOpenCurly(pctx); NEXT();
<Normal>   "}"     := send(pctx, ZENTOK_RCURLY); NEXT();
<Normal>   "%"     := send(pctx, ZENTOK_MOD); NEXT();
<Normal>   "<"     := sendLessThan(pctx); NEXT();
<Normal>   ">"     := send(pctx, ZENTOK_GT); NEXT();
<Normal>   "+"     := send(pctx, ZENTOK_PLUS); NEXT();
<Normal>   "-"     := send(pctx, ZENTOK_MINUS); NEXT();
<Normal>   "*"     := send(pctx, ZENTOK_STAR); NEXT();
<Normal>   "/"     := send(pctx, ZENTOK_DIVIDE); NEXT();
<Normal>   "..."   := send(pctx, ZENTOK_ELIPSIS); NEXT();

<Normal>   "import"    := send(pctx, ZENTOK_IMPORT); NEXT();
<Normal>   "include"   := send(pctx, ZENTOK_INCLUDE); NEXT();
<Normal>   "namespace" := send(pctx, ZENTOK_NAMESPACE); NEXT();

<Normal>   "private"   := send(pctx, ZENTOK_PRIVATE); NEXT();
<Normal>   "protected" := send(pctx, ZENTOK_PROTECTED); NEXT();
<Normal>   "public"    := send(pctx, ZENTOK_PUBLIC); NEXT();
<Normal>   "internal"  := send(pctx, ZENTOK_INTERNAL); NEXT();
<Normal>   "external"  := send(pctx, ZENTOK_EXTERNAL); NEXT();

<Normal>   "typedef"   := send(pctx, ZENTOK_TYPEDEF); NEXT();
<Normal>   "template"  := send(pctx, ZENTOK_TEMPLATE); NEXT();
<Normal>   "enum"      := send(pctx, ZENTOK_ENUM); NEXT();
<Normal>   "struct"    := send(pctx, ZENTOK_STRUCT); NEXT();
<Normal>   "routine"   := send(pctx, ZENTOK_ROUTINE); NEXT();
<Normal>   "function"  := send(pctx, ZENTOK_FUNCTION); NEXT();
<Normal>   "interface" := send(pctx, ZENTOK_INTERFACE); NEXT();
<Normal>   "event"     := send(pctx, ZENTOK_EVENT); NEXT();

<Normal>   "property"  := send(pctx, ZENTOK_PROPERTY); NEXT();
<Normal>   "get"       := send(pctx, ZENTOK_GET); NEXT();
<Normal>   "set"       := send(pctx, ZENTOK_SET); NEXT();

<Normal>   "native"    := send(pctx, ZENTOK_NATIVE); NEXT();
<Normal>   "abstract"  := send(pctx, ZENTOK_ABSTRACT); NEXT();
<Normal>   "final"     := send(pctx, ZENTOK_FINAL); NEXT();
<Normal>   "nocopy"    := send(pctx, ZENTOK_NOCOPY); NEXT();
<Normal>   "const"     := send(pctx, ZENTOK_CONST); NEXT();

<Normal>   "coerce"    := send(pctx, ZENTOK_COERCE); NEXT();
<Normal>   "default"   := send(pctx, ZENTOK_DEFAULT); NEXT();

<Normal>   "sizeof"    := send(pctx, ZENTOK_SIZEOF); NEXT();
<Normal>   "typeof"    := send(pctx, ZENTOK_TYPEOF); NEXT();

<Normal>   "auto"      := send(pctx, ZENTOK_AUTO); NEXT();
<Normal>   "print"     := send(pctx, ZENTOK_PRINT); NEXT();
<Normal>   "if"        := send(pctx, ZENTOK_IF); NEXT();
<Normal>   "else"      := send(pctx, ZENTOK_ELSE); NEXT();
<Normal>   "while"     := send(pctx, ZENTOK_WHILE); NEXT();
<Normal>   "do"        := send(pctx, ZENTOK_DO); NEXT();
<Normal>   "for"       := send(pctx, ZENTOK_FOR); NEXT();
<Normal>   "foreach"   := send(pctx, ZENTOK_FOREACH); NEXT();
<Normal>   "in"        := send(pctx, ZENTOK_IN); NEXT();
<Normal>   "has"       := send(pctx, ZENTOK_HAS); NEXT();
<Normal>   "switch"    := send(pctx, ZENTOK_SWITCH); NEXT();
<Normal>   "case"      := send(pctx, ZENTOK_CASE); NEXT();
<Normal>   "break"     := send(pctx, ZENTOK_BREAK); NEXT();
<Normal>   "continue"  := send(pctx, ZENTOK_CONTINUE); NEXT();
<Normal>   "run"       := send(pctx, ZENTOK_RUN); NEXT();
<Normal>   "raise"     := send(pctx, ZENTOK_RAISE); NEXT();
<Normal>   "exit"      := send(pctx, ZENTOK_EXIT); NEXT();

<Normal>   "return"    := sendReturn(pctx); NEXT();

<Normal>   "null"      := send(pctx, ZENTOK_NULL_CONST); NEXT();

<Normal>   "false"     := send(pctx, ZENTOK_FALSE_CONST); NEXT();
<Normal>   "true"      := send(pctx, ZENTOK_TRUE_CONST); NEXT();

id_seq = [a-zA-Z][a-zA-Z0-9_]*;
<Normal>   "@" id_seq := send(pctx, ZENTOK_KEY_CONST); NEXT();
<Normal>       id_seq := sendId(pctx); NEXT();


dec_digit = [0-9];
dec_seq = dec_digit+;
oct_seq = "0" [0-7]+;
hex_seq = "0" [xX] [A-Za-z0-9]+;

<Normal>   oct_seq [lL] := send(pctx, ZENTOK_LOCTINT_CONST); NEXT();
<Normal>   dec_seq [lL] := send(pctx, ZENTOK_LDECINT_CONST); NEXT();
<Normal>   hex_seq [lL] := send(pctx, ZENTOK_LHEXINT_CONST); NEXT();

<Normal>   oct_seq      := send(pctx, ZENTOK_OCTINT_CONST); NEXT();
<Normal>   dec_seq      := send(pctx, ZENTOK_DECINT_CONST); NEXT();
<Normal>   hex_seq      := send(pctx, ZENTOK_HEXINT_CONST); NEXT();

<Normal>   oct_seq [sS] := send(pctx, ZENTOK_SOCTINT_CONST); NEXT();
<Normal>   dec_seq [sS] := send(pctx, ZENTOK_SDECINT_CONST); NEXT();
<Normal>   hex_seq [sS] := send(pctx, ZENTOK_SHEXINT_CONST); NEXT();

<Normal>   oct_seq [yY] := send(pctx, ZENTOK_BOCTINT_CONST); NEXT();
<Normal>   dec_seq [yY] := send(pctx, ZENTOK_BDECINT_CONST); NEXT();
<Normal>   hex_seq [yY] := send(pctx, ZENTOK_BHEXINT_CONST); NEXT();

<Normal>   oct_seq [uU] [lL] := send(pctx, ZENTOK_ULOCTINT_CONST); NEXT();
<Normal>   dec_seq [uU] [lL] := send(pctx, ZENTOK_ULDECINT_CONST); NEXT();
<Normal>   hex_seq [uU] [lL] := send(pctx, ZENTOK_ULHEXINT_CONST); NEXT();

<Normal>   oct_seq [uU]      := send(pctx, ZENTOK_UOCTINT_CONST); NEXT();
<Normal>   dec_seq [uU]      := send(pctx, ZENTOK_UDECINT_CONST); NEXT();
<Normal>   hex_seq [uU]      := send(pctx, ZENTOK_UHEXINT_CONST); NEXT();

<Normal>   oct_seq [uU] [sS] := send(pctx, ZENTOK_USOCTINT_CONST); NEXT();
<Normal>   dec_seq [uU] [sS] := send(pctx, ZENTOK_USDECINT_CONST); NEXT();
<Normal>   hex_seq [uU] [sS] := send(pctx, ZENTOK_USHEXINT_CONST); NEXT();

<Normal>   oct_seq [uU] [yY] := send(pctx, ZENTOK_UBOCTINT_CONST); NEXT();
<Normal>   dec_seq [uU] [yY] := send(pctx, ZENTOK_UBDECINT_CONST); NEXT();
<Normal>   hex_seq [uU] [yY] := send(pctx, ZENTOK_UBHEXINT_CONST); NEXT();

exp_seq = [Ee] [+-]? dec_seq;
<Normal>   dec_digit* "." dec_seq (exp_seq)? [fF]  := send(pctx, ZENTOK_FLOAT_CONST); NEXT();
<Normal>                  dec_seq  exp_seq   [fF]  := send(pctx, ZENTOK_FLOAT_CONST); NEXT();
<Normal>   dec_digit* "." dec_seq (exp_seq)? [dD]  := send(pctx, ZENTOK_DOUBLE_CONST); NEXT();
<Normal>                  dec_seq  exp_seq   [dD]  := send(pctx, ZENTOK_DOUBLE_CONST); NEXT();

<Normal>   '\''   => Char   := _text = _cursor; assert(_text); NEXT(); // lexer code has a bug that will get triggered when _text == 0. \todo fix bug and remove this assert()
<Char>     '\''   => Normal := send(pctx, ZENTOK_CHAR_CONST); NEXT();
<Char>     '\\' .           := NEXT();
<Char>     [^]              := NEXT();

<Normal>   '"'    => String := _text = _cursor; assert(_text); NEXT(); // as above.
<String>   '"'    => Normal := send(pctx, ZENTOK_STRING_CONST); NEXT();
<String>   '\\' .           := NEXT();
<String>   [^]              := NEXT();

<Normal>        "/*"            :=> MultiComment
<MultiComment>  "*" "/"         :=> Normal
<MultiComment>  "\r" "\n"        => MultiComment :=  newLine(); NEXT();
<MultiComment>  "\n"             => MultiComment :=  newLine(); NEXT();
<MultiComment>  [^]             :=> MultiComment

<Normal>        "//"            :=> SingleComment
<SingleComment> "\r" "\n"        => Normal      :=  newLine(); NEXT();
<SingleComment> "\n"             => Normal      :=  newLine(); NEXT();
<SingleComment> "\\" "\r"? "\n" :=> SingleComment
<SingleComment> [^]             :=> SingleComment

<Normal>   "\r" "\n" := newLine(); NEXT();
<Normal>   "\n"      := newLine(); NEXT();
<Normal>   " "       := NEXT();
<Normal>   "\t"      := NEXT();

<Normal>   "\000" := send(pctx, ZENTOK_EOF); NEXT();
<Normal>   [^]    := send(pctx, ZENTOK_ERR); NEXT();
*/
}
