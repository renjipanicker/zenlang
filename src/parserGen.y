/** \file
  \brief Zen langauge grammar definition.

  This file is intended to be parsed with the lemon parser.
*/

%token_prefix ZENTOK_

%syntax_error {
    throw Exception("(%d, %d) Syntax error at token: %d\n", TOKEN.row(), TOKEN.col(), TOKEN.id());
}

%parse_accept {
//    zbl::mlog() << "Parse complete!";
}

%parse_failure {
//    throw z::exception(z::string::creator("%{err} Parse error").arg(z::any("err"), z::any(z::string(ref(pctx).err(TOKEN)))).value());
}

%stack_overflow {
//    throw z::exception(z::string::creator("%{err} Stack overflow error").arg(z::any("err"), z::any(ref(pctx).err())).value());
}

%token_destructor {
    TokenData::deleteT($$);
}

%name ZenParser

%token_type {TokenData}
%extra_argument {Context* pctx}

//-------------------------------------------------
// All keywords, etc
%nonassoc ERR EOF EOL WS COMMENT.
%nonassoc SEMI.
%nonassoc LOCAL SHARED.
%nonassoc LIST DICT TREE.
%nonassoc ENUM.
%nonassoc SUBTYPE.
%nonassoc RUN LOOP.
%nonassoc FLOAT_CONST STRING_CONST.
%nonassoc SWITCH.

%start_symbol start

//-------------------------------------------------
start ::= sub_start EOF.

//-------------------------------------------------
// source file.
sub_start ::= unit_statement_list.

//-------------------------------------------------
unit_statement_list ::= import_statement_list namespace_statement global_statement_list.

//-------------------------------------------------
namespace_statement ::= NAMESPACE unamespace_id SEMI.
namespace_statement ::= .
unamespace_id ::= unamespace_id TYPE_SCOPE ID(A).    {ref(pctx).addNamespace(A);}
unamespace_id ::=                          ID(A).    {ref(pctx).addNamespace(A);}

//-------------------------------------------------
// list of statements to specify imports
import_statement_list ::= import_statement_list import_statement.
import_statement_list ::= .

//-------------------------------------------------
import_statement ::= header_type(P) inamespace_id(I) definition_type(D) SEMI. {ref(I).headerType(P); ref(I).defType(D); ref(pctx).importHeader(ref(I));}

//-------------------------------------------------
// import type
%type header_type {Ast::HeaderType::T}
header_type(A) ::= INCLUDE.   {A = Ast::HeaderType::Include;}
header_type(A) ::= IMPORT.    {A = Ast::HeaderType::Import;}

//-------------------------------------------------
%type inamespace_id {Ast::ImportStatement*}
inamespace_id(I) ::= inamespace_id(P) SCOPE ID(A).  {ref(P).addPart(A); I = P;}
inamespace_id(I) ::=                        ID(A).  {I = ptr(ref(pctx).addImportStatement()); ref(I).addPart(A);}

//-------------------------------------------------
global_statement_list ::= global_statement_list global_statement.
global_statement_list ::= .

//-------------------------------------------------
global_statement ::= statement(S). {ref(pctx).addGlobalStatement(ref(S));}

//-------------------------------------------------
%type statement {Ast::Statement*}
statement(R) ::= typespec_statement(S). {R = S;}

//-------------------------------------------------
%type typespec_statement {Ast::Statement*}
typespec_statement(S) ::= access_type(A) typespec_def(T). {ref(T).accessType(A); S = ptr(ref(pctx).addUserDefinedTypeSpecStatement(ref(T)));}

//-------------------------------------------------
// access specifiers
%type access_type {Ast::AccessType::T}
access_type(A) ::= PRIVATE.   {A = Ast::AccessType::Private;}
access_type(A) ::= INTERNAL.  {A = Ast::AccessType::Internal;}
access_type(A) ::= PROTECTED. {A = Ast::AccessType::Protected;}
access_type(A) ::= PUBLIC.    {A = Ast::AccessType::Public;}
access_type(A) ::= EXPORT.    {A = Ast::AccessType::Export;}
access_type(A) ::= .          {A = Ast::AccessType::Private;}

//-------------------------------------------------
// definition specifiers
%type definition_type {Ast::DefinitionType::T}
definition_type(A) ::= NATIVE. {A = Ast::DefinitionType::Native;}
definition_type(A) ::= .       {A = Ast::DefinitionType::Direct;}

//-------------------------------------------------
%type typespec_def {Ast::UserDefinedTypeSpec*}
typespec_def(L) ::= typedef_def(R).   {L = R;}
typespec_def(L) ::= struct_def(R).    {L = R;}
typespec_def(L) ::= function_def(R). {L = R;}

//-------------------------------------------------
// typedef declarations
%type typedef_def {Ast::TypeDef*}
typedef_def(T) ::= TYPEDEF ID(N) NATIVE SEMI. {T = ptr(ref(pctx).addTypeDefSpec(N, Ast::DefinitionType::Native));}

//-------------------------------------------------
// struct declarations
%type struct_def {Ast::StructDef*}
struct_def(T) ::= STRUCT ID(N) NATIVE SEMI.                                 {T = ptr(ref(pctx).addStructDefSpecEmpty(N, Ast::DefinitionType::Native));}
struct_def(T) ::= STRUCT ID(N) LCURLY variabledef_list_semi(D) RCURLY SEMI. {T = ptr(ref(pctx).addStructDefSpec(N, Ast::DefinitionType::Direct, ref(D)));}
struct_def(T) ::= STRUCT ID(N) LCURLY                          RCURLY SEMI. {T = ptr(ref(pctx).addStructDefSpecEmpty(N, Ast::DefinitionType::Direct));}

//-------------------------------------------------
// function declarations
%type function_def {Ast::FunctionDef*}
function_def(F) ::= function_sig(S) SEMI. {F = S;}

//-------------------------------------------------
// function signature.
%type function_sig {Ast::FunctionDef*}
function_sig(T) ::= FUNCTION params_list(O) ID(N) params_list(I) definition_type(D). {T = ptr(ref(pctx).addFunctionDefSpec(ref(O), N, ref(I), D));}

//-------------------------------------------------
// parameter lists
%type params_list {const Ast::VariableDefList*}
params_list(L) ::= LBRACKET variabledef_list_comma(R) RBRACKET. {L = R;}

//-------------------------------------------------
// variable lists
%type variabledef_list_semi {Ast::VariableDefList*}
variabledef_list_semi(L) ::= variabledef_list_semi(P) variable_def(D). {L = P; ref(L).addVariableDef(ref(D));}
variabledef_list_semi(L) ::= variable_def(D) SEMI.                     {L = ptr(ref(pctx).addVariableDefList()); ref(L).addVariableDef(ref(D));}

%type variabledef_list_comma {Ast::VariableDefList*}
variabledef_list_comma(L) ::= variabledef_list_comma(P) COMMA variable_def(D). {L = P; ref(L).addVariableDef(ref(D));}
variabledef_list_comma(L) ::= variable_def(D).                                 {L = ptr(ref(pctx).addVariableDefList()); ref(L).addVariableDef(ref(D));}
variabledef_list_comma(L) ::= .                                                {L = ptr(ref(pctx).addVariableDefList());}

//-------------------------------------------------
// variable def
%type variable_def {const Ast::VariableDef*}
variable_def(L) ::= qtyperef(Q) ID(N).  {L = ptr(ref(pctx).addVariableDef(ref(Q), N));}

//-------------------------------------------------
// qualified types
%type qtyperef {const Ast::QualifiedTypeSpec*}
qtyperef(L) ::=       typeref(T).               {L = ptr(ref(pctx).addQualifiedTypeSpec(false, ref(T), false));}
qtyperef(L) ::=       typeref(T) BITWISEAND.    {L = ptr(ref(pctx).addQualifiedTypeSpec(false, ref(T), true));}
qtyperef(L) ::= CONST typeref(T).               {L = ptr(ref(pctx).addQualifiedTypeSpec(true, ref(T), false));}
qtyperef(L) ::= CONST typeref(T) BITWISEAND.    {L = ptr(ref(pctx).addQualifiedTypeSpec(true, ref(T), true));}

//-------------------------------------------------
// type references
%type typeref {const Ast::TypeSpec*}
typeref(R) ::= typeref(T) SCOPE ID(N). {R = ptr(ref(pctx).getChildTypeSpec(ref(T), N));}
typeref(R) ::= ID(N).                  {R = ptr(ref(pctx).getRootTypeSpec(N));}
