/** \file
  \brief Zen langauge grammar definition.

  This file is intended to be parsed with the lemon parser.
*/

%token_prefix ZENTOK_

%syntax_error {
    throw Exception("(%d, %d) Syntax error at token: %d (%s)\n", TOKEN.row(), TOKEN.col(), TOKEN.id(), TOKEN.text());
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
%nonassoc ERR EOF.
%nonassoc OCTINT_CONST DECINT_CONST HEXINT_CONST DOUBLE_CONST FLOAT_CONST STRING_CONST.
%nonassoc JOIN LINK.

//-------------------------------------------------
// All operators, in increasing order of precedence
%left DEFINEEQUAL.
%left ASSIGNEQUAL TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL.
%left LSQUARE RSQUARE.
%left LBRACKET RBRACKET.
%left QUESTION.
%left AND OR.
%left BITWISEAND BITWISEXOR BITWISEOR.
%left EQUAL NOTEQUAL.
%left LT GT LTE GTE HAS.
%left SHL SHR.
%left PLUS MINUS MOD.
%left DIVIDE STAR.
%left INC DEC BITWISENOT NOT.
%left AMP.
%left DOT.
%left KEY.
%left QUERY_SCOPE.
%left TYPE_SCOPE.
%left COLON.

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
global_statement ::= global_typespec_statement(S). {ref(pctx).addGlobalStatement(ref(S));}

//-------------------------------------------------
%type global_typespec_statement {Ast::Statement*}
global_typespec_statement(S) ::= access_type(A) typespec_def(T). {ref(T).accessType(A); S = ptr(ref(pctx).addUserDefinedTypeSpecStatement(ref(T)));}

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
typespec_def(L) ::= typedef_def(R).  {L = R;}
typespec_def(L) ::= enum_def(R).     {L = R;}
typespec_def(L) ::= struct_def(R).   {L = R;}
typespec_def(L) ::= routine_def(R).  {L = R;}
typespec_def(L) ::= function_def(R). {L = R;}
typespec_def(L) ::= event_def(R).    {L = R;}

//-------------------------------------------------
// typedef declarations
%type typedef_def {Ast::TypeDef*}
typedef_def(T) ::= TYPEDEF ID(N) NATIVE SEMI. {T = ptr(ref(pctx).addTypeDefSpec(N, Ast::DefinitionType::Native));}

//-------------------------------------------------
// enum declarations
%type enum_def {Ast::EnumDef*}
enum_def(T) ::= ENUM ID(N) NATIVE SEMI. {T = ptr(ref(pctx).addEnumDefSpecEmpty(N, Ast::DefinitionType::Native));}
enum_def(T) ::= ENUM ID(N) LCURLY enum_member_def_list(L) RCURLY SEMI. {T = ptr(ref(pctx).addEnumDefSpec(N, Ast::DefinitionType::Direct, ref(L)));}

//-------------------------------------------------
%type enum_member_def_list {Ast::EnumMemberDefList*}
enum_member_def_list(L) ::= enum_member_def_list(R) enum_member_def(D). {L = R; ref(L).addEnumMemberDef(ref(D));}
enum_member_def_list(L) ::= enum_member_def(D). {L = ptr(ref(pctx).addEnumMemberDefList()); ref(L).addEnumMemberDef(ref(D));}

//-------------------------------------------------
%type enum_member_def {Ast::EnumMemberDef*}
enum_member_def(L) ::= ID(N) SEMI. {L = ptr(ref(pctx).addEnumMemberDef(N));}

//-------------------------------------------------
// struct declarations
%type struct_def {Ast::StructDef*}
struct_def(T) ::= STRUCT ID(N) NATIVE SEMI.                                 {T = ptr(ref(pctx).addStructDefSpecEmpty(N, Ast::DefinitionType::Native));}
struct_def(T) ::= STRUCT ID(N) LCURLY variabledef_list_semi(D) RCURLY SEMI. {T = ptr(ref(pctx).addStructDefSpec(N, Ast::DefinitionType::Direct, ref(D)));}
struct_def(T) ::= STRUCT ID(N) LCURLY                          RCURLY SEMI. {T = ptr(ref(pctx).addStructDefSpecEmpty(N, Ast::DefinitionType::Direct));}

//-------------------------------------------------
// routine declarations
%type routine_def {Ast::RoutineDef*}
routine_def(L) ::= ROUTINE qtyperef(O) ID(N) params_list(I) NATIVE SEMI. {L = ptr(ref(pctx).addRoutineDefSpec(ref(O), N, ref(I), Ast::DefinitionType::Native));}
routine_def(L) ::= ROUTINE qtyperef(O) ID(N) params_list(I) compound_statement(S). {L = ptr(ref(pctx).addRoutineDefSpec(ref(O), N, ref(I), Ast::DefinitionType::Direct));S;}

//-------------------------------------------------
// function declarations
%type function_def {Ast::FunctionDef*}
function_def(L) ::= function_sig(R) definition_type(D) SEMI. {L = ptr(ref(pctx).addFunctionDefSpec(ref(R), D));}
function_def(L) ::= function_sig(R) definition_type(D) compound_statement(S). {L = ptr(ref(pctx).addFunctionDefSpec(ref(R), D)); ref(pctx).addFunctionImpl(ref(L), ref(S));}

//-------------------------------------------------
// event declarations
%type event_def {Ast::EventDef*}
event_def(L) ::= EVENT LBRACKET variable_def(I) RBRACKET LINK function_sig(F) SEMI. {L = ptr(ref(pctx).addEventDefSpec(ref(I), ref(F), Ast::DefinitionType::Direct));}

//-------------------------------------------------
// function signature.
%type function_sig {Ast::FunctionSig*}
function_sig(T) ::= FUNCTION params_list(O) ID(N) params_list(I). {T = ptr(ref(pctx).addFunctionSig(ref(O), N, ref(I)));}

//-------------------------------------------------
// parameter lists
%type params_list {const Ast::VariableDefList*}
params_list(L) ::= LBRACKET variabledef_list_comma(R) RBRACKET. {L = R;}

//-------------------------------------------------
// variable lists
%type variabledef_list_semi {Ast::VariableDefList*}
variabledef_list_semi(L) ::= variabledef_list_semi(P) variable_def(D) SEMI. {L = P; ref(L).addVariableDef(ref(D));}
variabledef_list_semi(L) ::= variable_def(D) SEMI.                          {L = ptr(ref(pctx).addVariableDefList()); ref(L).addVariableDef(ref(D));}

%type variabledef_list_comma {Ast::VariableDefList*}
variabledef_list_comma(L) ::= variabledef_list_comma(P) COMMA variable_def(D). {L = P; ref(L).addVariableDef(ref(D));}
variabledef_list_comma(L) ::= variable_def(D).                                 {L = ptr(ref(pctx).addVariableDefList()); ref(L).addVariableDef(ref(D));}
variabledef_list_comma(L) ::= .                                                {L = ptr(ref(pctx).addVariableDefList());}

//-------------------------------------------------
// variable def
%type variable_def {const Ast::VariableDef*}
variable_def(L) ::= qtyperef(Q) ID(N).  {L = ptr(ref(pctx).addVariableDef(ref(Q), N));}
variable_def(L) ::= qtyperef(Q) ID(N) ASSIGNEQUAL expr.  {L = ptr(ref(pctx).addVariableDef(ref(Q), N));}

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

//-------------------------------------------------
// statements
%type local_statement {Ast::Statement*}
local_statement(R) ::= local_typespec_statement(S). {R = S;}
local_statement(R) ::= expr(E) SEMI. {R = ptr(ref(pctx).addExprStatement(ref(E)));}
local_statement(R) ::= compound_statement(S). {R = S;}
local_statement(R) ::= RETURN expr_list(S). {R = ptr(ref(pctx).addReturnStatement(ref(S)));}
local_statement(R) ::= RETURN expr(S) SEMI. {R;S;}

//-------------------------------------------------
%type local_typespec_statement {Ast::Statement*}
local_typespec_statement(S) ::= typespec_def(T). {S = ptr(ref(pctx).addUserDefinedTypeSpecStatement(ref(T)));}

%type compound_statement {Ast::CompoundStatement*}
compound_statement ::= enter_compound_statement statement_list leave_compound_statement.

enter_compound_statement ::= LCURLY.
leave_compound_statement ::= RCURLY.

%type statement_list {Ast::CompoundStatement*}
statement_list(L) ::= statement_list(R) local_statement(S). {L = R; ref(L).addStatement(ref(S));}
statement_list(L) ::= . {L = ptr(ref(pctx).addCompoundStatement());}

//-------------------------------------------------
// expression list in brackets
%type expr_list {Ast::ExprList*}
expr_list(R) ::= LBRACKET expr_list_comma(L) RBRACKET. {R = L;}

//-------------------------------------------------
// comma-separated list of expressions
%type expr_list_comma {Ast::ExprList*}
expr_list_comma(R) ::= expr_list_comma(L) COMMA expr(E). {R = L;                      ref(R).addExpr(ref(E));}
expr_list_comma(R) ::=                    expr(E). {R = ptr(ref(pctx).addExprList()); ref(R).addExpr(ref(E));}
expr_list_comma(R) ::= .                           {R = ptr(ref(pctx).addExprList());}

//-------------------------------------------------
// expressions
%type expr {const Ast::Expr*}

//-------------------------------------------------
// binary operators

// It could be possible to implement creating local variables inline within expressions.
// Not sure how to implement it in the generated code. Not a priority, so on hold for now.
//expr(E) ::= ID(L) DEFINEEQUAL       expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(ref(L), z::string("="), ref(R)));}

expr(E) ::= expr(L) ASSIGNEQUAL(O)     expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) TIMESEQUAL(O)      expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) DIVIDEEQUAL(O)     expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) MINUSEQUAL(O)      expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) PLUSEQUAL(O)       expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) MODEQUAL(O)        expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) SHIFTLEFTEQUAL(O)  expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) SHIFTRIGHTEQUAL(O) expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) BITWISEANDEQUAL(O) expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) BITWISEXOREQUAL(O) expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) BITWISEOREQUAL(O)  expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}

expr(E) ::= expr(L) QUESTION(O) expr(T) COLON expr(F). {E = ptr(ref(pctx).addTernaryOpExpr(O, ref(L), ref(T), ref(F)));}

expr(E) ::= expr(L) BITWISEAND(O) expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) BITWISEXOR(O) expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) BITWISEOR(O)  expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) BITWISENOT(O) expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}

expr(E) ::= expr(L) AND(O) expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) OR(O)  expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::=         NOT(O) expr(R). {E = ptr(ref(pctx).addPrefixOpExpr(O, ref(R)));}

expr(E) ::= expr(L) EQUAL(O)    expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) NOTEQUAL(O) expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) LT(O)       expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) GT(O)       expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) LTE(O)      expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) GTE(O)      expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) HAS(O)      expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}

expr(E) ::= expr(L) SHL(O) expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) SHR(O) expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}

expr(E) ::= expr(L) PLUS(O)   expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) MINUS(O)  expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) STAR(O)   expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) DIVIDE(O) expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}
expr(E) ::= expr(L) MOD(O)    expr(R). {E = ptr(ref(pctx).addBinaryOpExpr(O, ref(L), ref(R)));}

expr(E) ::= PLUS(O)       expr(R). {E = ptr(ref(pctx).addPrefixOpExpr(O,  ref(R)));}
expr(E) ::= MINUS(O)      expr(R). {E = ptr(ref(pctx).addPrefixOpExpr(O,  ref(R)));}
expr(E) ::= INC(O)        expr(R). {E = ptr(ref(pctx).addPrefixOpExpr(O, ref(R)));}
expr(E) ::= DEC(O)        expr(R). {E = ptr(ref(pctx).addPrefixOpExpr(O, ref(R)));}
expr(E) ::= BITWISENOT(O) expr(R). {E = ptr(ref(pctx).addPrefixOpExpr(O,  ref(R)));}

expr(E) ::= expr(L) INC(O). {E = ptr(ref(pctx).addPostfixOpExpr(O, ref(L)));}
expr(E) ::= expr(L) DEC(O). {E = ptr(ref(pctx).addPostfixOpExpr(O, ref(L)));}

//-------------------------------------------------
// variable member expressions
expr(L) ::= expr(R) DOT expr(M). {L;R;M;}

//-------------------------------------------------
// type member expressions
expr(L) ::= typeref(R) DOT ID(M). {L;R;M;}

//-------------------------------------------------
// constant expressions
expr(L) ::= constant_expr(R). {L = R;}

//-------------------------------------------------
%type constant_expr {const Ast::ConstantExpr*}
constant_expr(E) ::= FLOAT_CONST(A).   {E = ptr(ref(pctx).addConstantExpr("float", A));}
constant_expr(E) ::= DOUBLE_CONST(A).  {E = ptr(ref(pctx).addConstantExpr("double", A));}
constant_expr(E) ::= TRUE_CONST(A).    {E = ptr(ref(pctx).addConstantExpr("bool", A));}
constant_expr(E) ::= FALSE_CONST(A).   {E = ptr(ref(pctx).addConstantExpr("bool", A));}
constant_expr(E) ::= KEY_CONST(A).     {E = ptr(ref(pctx).addConstantExpr("string", A));}
constant_expr(E) ::= STRING_CONST(A).  {E = ptr(ref(pctx).addConstantExpr("string", A));}
constant_expr(E) ::= CHAR_CONST(A).    {E = ptr(ref(pctx).addConstantExpr("char", A));}
constant_expr(E) ::= HEXINT_CONST(A).  {E = ptr(ref(pctx).addConstantExpr("int", A));}
constant_expr(E) ::= DECINT_CONST(A).  {E = ptr(ref(pctx).addConstantExpr("int", A));}
constant_expr(E) ::= OCTINT_CONST(A).  {E = ptr(ref(pctx).addConstantExpr("int", A));}
constant_expr(E) ::= LHEXINT_CONST(A). {E = ptr(ref(pctx).addConstantExpr("long", A));}
constant_expr(E) ::= LDECINT_CONST(A). {E = ptr(ref(pctx).addConstantExpr("long", A));}
constant_expr(E) ::= LOCTINT_CONST(A). {E = ptr(ref(pctx).addConstantExpr("long", A));}
