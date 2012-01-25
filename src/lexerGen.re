// This file is adapted from one of the re2c samples (push.re)

// if first non-comment line is a #include, QtCreator recognizes this as a C/C++ file for syntax highlighting.
#include <typeinfo>

// this causes the conditon enum to be generated
/*!types:re2c */

#define NEXT() { _start = _cursor; goto yy0; }

void Lexer::Impl::lex(Ast::NodeFactory& factory) {
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

<Normal>   "<<="   := send(factory, ZENTOK_SHIFTLEFTEQUAL); NEXT();
<Normal>   ">>="   := send(factory, ZENTOK_SHIFTRIGHTEQUAL); NEXT();
<Normal>   ":="    := send(factory, ZENTOK_DEFINEEQUAL); NEXT();
<Normal>   "*="    := send(factory, ZENTOK_TIMESEQUAL); NEXT();
<Normal>   "/="    := send(factory, ZENTOK_DIVIDEEQUAL); NEXT();
<Normal>   "-="    := send(factory, ZENTOK_MINUSEQUAL); NEXT();
<Normal>   "+="    := send(factory, ZENTOK_PLUSEQUAL); NEXT();
<Normal>   "%="    := send(factory, ZENTOK_MODEQUAL); NEXT();
<Normal>   "&="    := send(factory, ZENTOK_BITWISEANDEQUAL); NEXT();
<Normal>   "^="    := send(factory, ZENTOK_BITWISEXOREQUAL); NEXT();
<Normal>   "|="    := send(factory, ZENTOK_BITWISEOREQUAL); NEXT();
<Normal>   "!="    := send(factory, ZENTOK_NOTEQUAL); NEXT();
<Normal>   "=="    := send(factory, ZENTOK_EQUAL); NEXT();
<Normal>   "<="    := send(factory, ZENTOK_LTE); NEXT();
<Normal>   ">="    := send(factory, ZENTOK_GTE); NEXT();
<Normal>   ">>"    := send(factory, ZENTOK_SHR); NEXT();
<Normal>   "<<"    := send(factory, ZENTOK_SHL); NEXT();
<Normal>   "++"    := send(factory, ZENTOK_INC); NEXT();
<Normal>   "--"    := send(factory, ZENTOK_DEC); NEXT();
<Normal>   "=>"    := send(factory, ZENTOK_LINK); NEXT();
<Normal>   "->"    := send(factory, ZENTOK_RESERVED); NEXT();
<Normal>   "&&"    := send(factory, ZENTOK_AND); NEXT();
<Normal>   "||"    := send(factory, ZENTOK_OR); NEXT();
<Normal>   ";"     := send(factory, ZENTOK_SEMI); NEXT();
<Normal>   "!"     := send(factory, ZENTOK_NOT); NEXT();
<Normal>   "="     := send(factory, ZENTOK_ASSIGNEQUAL); NEXT();
<Normal>   "~"     := send(factory, ZENTOK_BITWISENOT); NEXT();
<Normal>   "^"     := send(factory, ZENTOK_BITWISEXOR); NEXT();
<Normal>   "|"     := send(factory, ZENTOK_BITWISEOR); NEXT();
<Normal>   "&"     := send(factory, ZENTOK_BITWISEAND); NEXT();
<Normal>   "?"     := send(factory, ZENTOK_QUESTION); NEXT();
<Normal>   ":"     := send(factory, ZENTOK_COLON); NEXT();
<Normal>   "::"    := send(factory, ZENTOK_SCOPE); NEXT();
<Normal>   "."     := send(factory, ZENTOK_DOT); NEXT();
<Normal>   ","     := send(factory, ZENTOK_COMMA); NEXT();
<Normal>   "@"     := send(factory, ZENTOK_AMP); NEXT();
<Normal>   "("     := send(factory, ZENTOK_LBRACKET); NEXT();
<Normal>   ")"     := send(factory, ZENTOK_RBRACKET); NEXT();
<Normal>   "["     := send(factory, ZENTOK_LSQUARE); NEXT();
<Normal>   "]"     := send(factory, ZENTOK_RSQUARE); NEXT();
<Normal>   "{"     := sendOpenCurly(factory); NEXT();
<Normal>   "}"     := send(factory, ZENTOK_RCURLY); NEXT();
<Normal>   "%"     := send(factory, ZENTOK_MOD); NEXT();
<Normal>   "<"     := sendLessThan(factory); NEXT();
<Normal>   ">"     := send(factory, ZENTOK_GT); NEXT();
<Normal>   "+"     := send(factory, ZENTOK_PLUS); NEXT();
<Normal>   "-"     := send(factory, ZENTOK_MINUS); NEXT();
<Normal>   "*"     := send(factory, ZENTOK_STAR); NEXT();
<Normal>   "/"     := send(factory, ZENTOK_DIVIDE); NEXT();
<Normal>   "..."   := send(factory, ZENTOK_ELIPSIS); NEXT();

<Normal>   "import"    := send(factory, ZENTOK_IMPORT); NEXT();
<Normal>   "include"   := send(factory, ZENTOK_INCLUDE); NEXT();
<Normal>   "namespace" := send(factory, ZENTOK_NAMESPACE); NEXT();

<Normal>   "private"   := send(factory, ZENTOK_PRIVATE); NEXT();
<Normal>   "public"    := send(factory, ZENTOK_PUBLIC); NEXT();
<Normal>   "internal"  := send(factory, ZENTOK_INTERNAL); NEXT();
<Normal>   "external"  := send(factory, ZENTOK_EXTERNAL); NEXT();

<Normal>   "typedef"   := send(factory, ZENTOK_TYPEDEF); NEXT();
<Normal>   "template"  := send(factory, ZENTOK_TEMPLATE); NEXT();
<Normal>   "enum"      := send(factory, ZENTOK_ENUM); NEXT();
<Normal>   "struct"    := send(factory, ZENTOK_STRUCT); NEXT();
<Normal>   "routine"   := send(factory, ZENTOK_ROUTINE); NEXT();
<Normal>   "function"  := send(factory, ZENTOK_FUNCTION); NEXT();
<Normal>   "event"     := send(factory, ZENTOK_EVENT); NEXT();

<Normal>   "property"  := send(factory, ZENTOK_PROPERTY); NEXT();
<Normal>   "get"       := send(factory, ZENTOK_GET); NEXT();
<Normal>   "set"       := send(factory, ZENTOK_SET); NEXT();

<Normal>   "native"    := send(factory, ZENTOK_NATIVE); NEXT();
<Normal>   "abstract"  := send(factory, ZENTOK_ABSTRACT); NEXT();
<Normal>   "final"     := send(factory, ZENTOK_FINAL); NEXT();
<Normal>   "const"     := send(factory, ZENTOK_CONST); NEXT();

<Normal>   "coerce"    := send(factory, ZENTOK_COERCE); NEXT();
<Normal>   "default"   := send(factory, ZENTOK_DEFAULT); NEXT();
<Normal>   "typeof"    := send(factory, ZENTOK_TYPEOF); NEXT();

<Normal>   "auto"      := send(factory, ZENTOK_AUTO); NEXT();
<Normal>   "print"     := send(factory, ZENTOK_PRINT); NEXT();
<Normal>   "if"        := send(factory, ZENTOK_IF); NEXT();
<Normal>   "else"      := send(factory, ZENTOK_ELSE); NEXT();
<Normal>   "while"     := send(factory, ZENTOK_WHILE); NEXT();
<Normal>   "do"        := send(factory, ZENTOK_DO); NEXT();
<Normal>   "for"       := send(factory, ZENTOK_FOR); NEXT();
<Normal>   "foreach"   := send(factory, ZENTOK_FOREACH); NEXT();
<Normal>   "in"        := send(factory, ZENTOK_IN); NEXT();
<Normal>   "has"       := send(factory, ZENTOK_HAS); NEXT();
<Normal>   "switch"    := send(factory, ZENTOK_SWITCH); NEXT();
<Normal>   "case"      := send(factory, ZENTOK_CASE); NEXT();
<Normal>   "break"     := send(factory, ZENTOK_BREAK); NEXT();
<Normal>   "continue"  := send(factory, ZENTOK_CONTINUE); NEXT();
<Normal>   "run"       := send(factory, ZENTOK_RUN); NEXT();

<Normal>   "return"    := sendReturn(factory); NEXT();

<Normal>   "false"     := send(factory, ZENTOK_FALSE_CONST); NEXT();
<Normal>   "true"      := send(factory, ZENTOK_TRUE_CONST); NEXT();

id_seq = [a-zA-Z][a-zA-Z0-9_]*;
<Normal>   "@" id_seq := send(factory, ZENTOK_KEY_CONST); NEXT();
<Normal>       id_seq := sendId(factory); NEXT();

<Normal>   "0" [0-7]+ [uU]?      := send(factory, ZENTOK_OCTINT_CONST); NEXT();
<Normal>   "0" [0-7]+ [uU]? [lL] := send(factory, ZENTOK_LOCTINT_CONST); NEXT();

dec_digit = [0-9];
dec_seq = dec_digit+;
<Normal>   dec_seq      := send(factory, ZENTOK_DECINT_CONST); NEXT();
<Normal>   dec_seq [lL] := send(factory, ZENTOK_LDECINT_CONST); NEXT();

<Normal>   "0" [xX] [A-Za-z0-9]+      := send(factory, ZENTOK_HEXINT_CONST); NEXT();
<Normal>   "0" [xX] [A-Za-z0-9]+ [lL] := send(factory, ZENTOK_LHEXINT_CONST); NEXT();

exp_seq = [Ee] [+-]? dec_seq;
<Normal>   dec_digit* "." dec_seq (exp_seq)? [fF]  := send(factory, ZENTOK_FLOAT_CONST); NEXT();
<Normal>                  dec_seq  exp_seq   [fF]  := send(factory, ZENTOK_FLOAT_CONST); NEXT();
<Normal>   dec_digit* "." dec_seq (exp_seq)? [dD]  := send(factory, ZENTOK_DOUBLE_CONST); NEXT();
<Normal>                  dec_seq  exp_seq   [dD]  := send(factory, ZENTOK_DOUBLE_CONST); NEXT();

<Normal>   '\''   => Char   := _text = _cursor; NEXT();
<Char>     '\''   => Normal := send(factory, ZENTOK_CHAR_CONST); NEXT();
<Char>     '\\' .           := NEXT();
<Char>     [^]              := NEXT();

<Normal>   '"'    => String := _text = _cursor; NEXT();
<String>   '"'    => Normal := send(factory, ZENTOK_STRING_CONST); NEXT();
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

<Normal>   "\000" := send(factory, ZENTOK_EOF); NEXT();
<Normal>   [^]    := send(factory, ZENTOK_ERR); NEXT();
*/
}
