/** \file
  \brief Zen langauge grammar definition.

  This file is intended to be parsed with the lemon parser.
  Every rule (rXXX) has a corresponding function (aXXX) with the same name in the
  class Context that contains the action for the rule, except in the following 4 cases:
  - The rule has no corresponding action
  - The rule does nothing other than to assign the RHS to the LHS
  - The rule does nothing other than to select an enumerated value.
  - All rExpr rules.

  All LHS variables are called L.
  The functions in Context are defined in the same order as the rules in this file.
*/

%token_prefix ZENTOK_

%syntax_error {
    throw Exception("%s Syntax error at token: %d (%s)\n", err(ref(pctx).filename(), TOKEN).c_str(), TOKEN.id(), TOKEN.text());
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
    unused(pctx);
    TokenData::deleteT($$);
}

%name ZenParser

%token_type {TokenData}
%extra_argument {Context* pctx}

%include {
#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "error.hpp"
#include "context.hpp"
}

//-------------------------------------------------
// basic tokens
%nonassoc ERR EOF RESERVED.

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

%start_symbol rstart

//-------------------------------------------------
rstart ::= rSubStart EOF.

//-------------------------------------------------
// source file.
rSubStart ::= rUnitStatementList.

//-------------------------------------------------
rUnitStatementList ::= rImportStatementList rNamespaceStatement(N) rGlobalStatementList. {ref(pctx).aUnitStatementList(ref(N));}

//-------------------------------------------------
// list of statements to specify imports
rImportStatementList ::= rImportStatementList rImportStatement.
rImportStatementList ::= .

//-------------------------------------------------
rImportStatement ::= rHeaderType(headerType) rImportNamespaceList(L) rDefinitionType(defType) rOptionalAccessType(A) SEMI. {ref(pctx).aImportStatement(A, headerType, defType, ref(L));}

//-------------------------------------------------
// import namespace list
%type rImportNamespaceList {Ast::NamespaceList*}
rImportNamespaceList(L) ::= rImportNamespaceList(R) SCOPE rAnyId(name). {L = ref(pctx).aImportNamespaceList(ref(R), name);}
rImportNamespaceList(L) ::=                               rAnyId(name). {L = ref(pctx).aImportNamespaceList(name);}

//-------------------------------------------------
// access specifiers
%type rOptionalAccessType {Ast::AccessType::T}
rOptionalAccessType(L) ::= rAccessType(R).    {L = R;}
rOptionalAccessType(L) ::=               .    {L = Ast::AccessType::Private;}

//-------------------------------------------------
// import type
%type rHeaderType {Ast::HeaderType::T}
rHeaderType(L) ::= INCLUDE.   {L = Ast::HeaderType::Include;}
rHeaderType(L) ::= IMPORT.    {L = Ast::HeaderType::Import;}

//-------------------------------------------------
// namespace statement
%type rNamespaceStatement {Ast::EnterNamespaceStatement*}
rNamespaceStatement(L) ::= NAMESPACE rUnitNamespaceList(R) SEMI. {L = ref(pctx).aNamespaceStatement(ref(R));}
rNamespaceStatement(L) ::=                                     . {L = ref(pctx).aNamespaceStatement();}

//-------------------------------------------------
// namespace list
%type rUnitNamespaceList {Ast::NamespaceList*}
rUnitNamespaceList(L) ::= rUnitNamespaceList(R) SCOPE rAnyId(name). {L = ref(pctx).aUnitNamespaceList(ref(R), name);}
rUnitNamespaceList(L) ::=                             rAnyId(name). {L = ref(pctx).aUnitNamespaceList(name);}

//-------------------------------------------------
rAnyId(L) ::= ID(R).            {L = R;}
rAnyId(L) ::= TEMPLATE_TYPE(R). {L = R;}
rAnyId(L) ::= STRUCT_TYPE(R).   {L = R;}
rAnyId(L) ::= ROUTINE_TYPE(R).  {L = R;}
rAnyId(L) ::= FUNCTION_TYPE(R). {L = R;}
rAnyId(L) ::= EVENT_TYPE(R).    {L = R;}
rAnyId(L) ::= OTHER_TYPE(R).    {L = R;}

//-------------------------------------------------
rGlobalStatementList ::= rGlobalStatementList rGlobalStatement.
rGlobalStatementList ::= .

//-------------------------------------------------
rGlobalStatement ::= rGlobalTypeSpecStatement.
rGlobalStatement ::= rGlobalCoerceStatement.
rGlobalStatement ::= rGlobalDefaultStatement.

//-------------------------------------------------
%type rGlobalTypeSpecStatement {Ast::Statement*}
rGlobalTypeSpecStatement(L) ::= rAccessType(accessType) rTypeSpecDef(typeSpec). {L = ref(pctx).aGlobalTypeSpecStatement(accessType, ref(typeSpec));}
rGlobalTypeSpecStatement(L) ::= rInnerStatement(R).                             {L = ref(pctx).aGlobalStatement(ref(R));}

//-------------------------------------------------
// access specifiers
%type rAccessType {Ast::AccessType::T}
rAccessType(L) ::= PRIVATE.   {L = Ast::AccessType::Private;}
rAccessType(L) ::= PUBLIC.    {L = Ast::AccessType::Public;}
rAccessType(L) ::= INTERNAL.  {L = Ast::AccessType::Internal;}
rAccessType(L) ::= EXTERNAL.  {L = Ast::AccessType::External;}

//-------------------------------------------------
// coercion statements
rGlobalCoerceStatement ::= COERCE rCoerceList(T) SEMI. {ref(pctx).aGlobalCoerceStatement(ref(T));}

%type rCoerceList {Ast::CoerceList*}
rCoerceList(L) ::= rCoerceList(R) LINK rTypeSpec(T). {L = ref(pctx).aCoerceList(ref(R), ref(T));}
rCoerceList(L) ::=                     rTypeSpec(T). {L = ref(pctx).aCoerceList(ref(T));}

//-------------------------------------------------
// default values for types
rGlobalDefaultStatement ::= DEFAULT rTypeSpec(T) ASSIGNEQUAL rExpr(E) SEMI. {ref(pctx).aGlobalDefaultStatement(ref(T), ref(E));}

//-------------------------------------------------
// definition specifiers
%type rDefinitionType {Ast::DefinitionType::T}
rDefinitionType(L) ::= NATIVE  . {L = Ast::DefinitionType::Native;}
rDefinitionType(L) ::= .         {L = Ast::DefinitionType::Direct;}

//-------------------------------------------------
// definition specifiers
%type rExDefinitionType {Ast::DefinitionType::T}
rExDefinitionType(L) ::= ABSTRACT. {L = Ast::DefinitionType::Abstract;}
rExDefinitionType(L) ::= rDefinitionType(R). {L = R;}

//-------------------------------------------------
%type rTypeSpecDef {Ast::UserDefinedTypeSpec*}
rTypeSpecDef(L) ::= rBasicTypeSpecDef(R).  {L = R;}
rTypeSpecDef(L) ::= rFunctionDecl(R).      {L = R;}
rTypeSpecDef(L) ::= rRootFunctionDefn(R).  {L = R;}
rTypeSpecDef(L) ::= rChildFunctionDefn(R). {L = R;}
rTypeSpecDef(L) ::= rEventDecl(R).         {L = R;}

//-------------------------------------------------
// types that are permitted within struct's
%type rBasicTypeSpecDef {Ast::UserDefinedTypeSpec*}
rBasicTypeSpecDef(L) ::= rTypedefDecl(R).       {L = R;}
rBasicTypeSpecDef(L) ::= rTypedefDefn(R).       {L = R;}
rBasicTypeSpecDef(L) ::= rTemplateDecl(R).      {L = R;}
rBasicTypeSpecDef(L) ::= rEnumDecl(R).          {L = R;}
rBasicTypeSpecDef(L) ::= rEnumDefn(R).          {L = R;}
rBasicTypeSpecDef(L) ::= rStructDecl(R).        {L = R;}
rBasicTypeSpecDef(L) ::= rRootStructDefn(R).    {L = R;}
rBasicTypeSpecDef(L) ::= rChildStructDefn(R).   {L = R;}
rBasicTypeSpecDef(L) ::= rRoutineDecl(R).       {L = R;}
rBasicTypeSpecDef(L) ::= rRoutineDefn(R).       {L = R;}

//-------------------------------------------------
// typedef declaration
%type rTypedefDecl {Ast::TypedefDecl*}
rTypedefDecl(L) ::= rPreTypedefDecl(R) SEMI. {L = R;}

//-------------------------------------------------
// this pre- mechanism is required to force the action to get executed before the end-of-line.
%type rPreTypedefDecl {Ast::TypedefDecl*}
rPreTypedefDecl(L) ::= TYPEDEF ID(name) rDefinitionType(D). {L = ref(pctx).aTypedefDecl(name, D);}

//-------------------------------------------------
// typedef definition
%type rTypedefDefn {Ast::TypedefDefn*}
rTypedefDefn(L) ::= rPreTypedefDefn(R) SEMI. {L = R;}

%type rPreTypedefDefn {Ast::TypedefDefn*}
rPreTypedefDefn(L) ::= TYPEDEF ID(name) rQualifiedTypeSpec(Q) rDefinitionType(D). {L = ref(pctx).aTypedefDefn(name, D, ref(Q));}

//-------------------------------------------------
// template declarations
%type rTemplateDecl {Ast::TemplateDecl*}
rTemplateDecl(L) ::= rPreTemplateDecl(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreTemplateDecl {Ast::TemplateDecl*}
rPreTemplateDecl(L) ::= TEMPLATE LT rTemplatePartList(list) GT ID(name) rDefinitionType(D). {L = ref(pctx).aTemplateDecl(name, D, ref(list));}

//-------------------------------------------------
%type rTemplatePartList {Ast::TemplatePartList*}
rTemplatePartList(L) ::= rTemplatePartList(R) COMMA ID(name). {L = ref(pctx).aTemplatePartList(ref(R), name);}
rTemplatePartList(L) ::=                            ID(name). {L = ref(pctx).aTemplatePartList(name);}

//-------------------------------------------------
// enum declaration
%type rEnumDecl {Ast::EnumDefn*}
rEnumDecl(L) ::= rPreEnumDecl(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreEnumDecl {Ast::EnumDefn*}
rPreEnumDecl(L) ::= ENUM ID(name) rDefinitionType(D). {L = ref(pctx).aEnumDefn(name, D);}

//-------------------------------------------------
// enum definition
%type rEnumDefn {Ast::EnumDefn*}
rEnumDefn(L) ::= rPreEnumDefn(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreEnumDefn {Ast::EnumDefn*}
rPreEnumDefn(L) ::= ENUM ID(name) rDefinitionType(D) LCURLY rEnumMemberDefnList(list) RCURLY. {L = ref(pctx).aEnumDefn(name, D, ref(list));}

//-------------------------------------------------
%type rEnumMemberDefnList {Ast::Scope*}
rEnumMemberDefnList(L) ::= rEnumMemberDefnList(list) rEnumMemberDefn(enumMemberDef). {L = ref(pctx).aEnumMemberDefnList(ref(list), ref(enumMemberDef));}
rEnumMemberDefnList(L) ::=                           rEnumMemberDefn(enumMemberDef). {L = ref(pctx).aEnumMemberDefnList(ref(enumMemberDef));}

//-------------------------------------------------
%type rEnumMemberDefn {Ast::VariableDefn*}
rEnumMemberDefn(L) ::= ID(name)                      SEMI. {L = ref(pctx).aEnumMemberDefn(name);}
rEnumMemberDefn(L) ::= ID(name) ASSIGNEQUAL rExpr(I) SEMI. {L = ref(pctx).aEnumMemberDefn(name, ref(I));}

//-------------------------------------------------
// struct declarations
%type rStructDecl {Ast::StructDecl*}
rStructDecl(L) ::= rPreStructDecl(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreStructDecl {Ast::StructDecl*}
rPreStructDecl(L) ::= STRUCT rStructId(name) rDefinitionType(D). {L = ref(pctx).aStructDecl(name, D);}

//-------------------------------------------------
// root struct definitions
%type rRootStructDefn {Ast::RootStructDefn*}
rRootStructDefn(L) ::= rPreRootStructDefn(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreRootStructDefn {Ast::RootStructDefn*}
rPreRootStructDefn(L) ::= rEnterRootStructDefn(S) rStructMemberDefnBlock. {L = ref(pctx).aLeaveRootStructDefn(ref(S));}

//-------------------------------------------------
// child struct definitions
%type rChildStructDefn {Ast::ChildStructDefn*}
rChildStructDefn(L) ::= rPreChildStructDefn(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreChildStructDefn {Ast::ChildStructDefn*}
rPreChildStructDefn(L) ::= rEnterChildStructDefn(S) rStructMemberDefnBlock. {L = ref(pctx).aLeaveChildStructDefn(ref(S));}

//-------------------------------------------------
%type rEnterRootStructDefn {Ast::RootStructDefn*}
rEnterRootStructDefn(L) ::= STRUCT rStructId(name) rExDefinitionType(D). {L = ref(pctx).aEnterRootStructDefn(name, D);}

//-------------------------------------------------
%type rEnterChildStructDefn {Ast::ChildStructDefn*}
rEnterChildStructDefn(L) ::= STRUCT rStructId(name) COLON rStructTypeSpec(B) rExDefinitionType(D). {L = ref(pctx).aEnterChildStructDefn(name, ref(B), D);}

//-------------------------------------------------
rStructId(L) ::= STRUCT_TYPE(R). {L = R;}
rStructId(L) ::= ID(R). {L = R;}

//-------------------------------------------------
rStructMemberDefnBlock ::= LCURLY rStructMemberDefnList RCURLY.
rStructMemberDefnBlock ::= LCURLY                       RCURLY.

//-------------------------------------------------
rStructMemberDefnList ::= rStructMemberDefnList rStructMemberDefn.
rStructMemberDefnList ::=                       rStructMemberDefn.

//-------------------------------------------------
rStructMemberDefn ::= rVariableDefn(R) SEMI.  {ref(pctx).aStructMemberVariableDefn(ref(R));}
rStructMemberDefn ::= rBasicTypeSpecDef(R).   {ref(pctx).aStructMemberTypeDefn(ref(R));}
rStructMemberDefn ::= rStructPropertyDecl(R). {ref(pctx).aStructMemberPropertyDefn(ref(R));}

//-------------------------------------------------
// struct index declarations
%type rStructPropertyDecl {Ast::PropertyDecl*}
rStructPropertyDecl(L) ::= PROPERTY(B) rQualifiedTypeSpec(T) ID(N) rDefinitionType(D) GET SET SEMI. {L = ref(pctx).aStructPropertyDeclRW(B, ref(T), N, D);}
rStructPropertyDecl(L) ::= PROPERTY(B) rQualifiedTypeSpec(T) ID(N) rDefinitionType(D) GET     SEMI. {L = ref(pctx).aStructPropertyDeclRO(B, ref(T), N, D);}

//-------------------------------------------------
// routine declarations
%type rRoutineDecl {Ast::RoutineDecl*}
rRoutineDecl(L) ::= rPreRoutineDecl(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreRoutineDecl {Ast::RoutineDecl*}
rPreRoutineDecl(L) ::= ROUTINE rQualifiedTypeSpec(out) rRoutineId(name) rInParamsList(in) rDefinitionType(D). {L = ref(pctx).aRoutineDecl(ref(out), name, ref(in), D);}

//-------------------------------------------------
// routine definition
%type rRoutineDefn {Ast::RoutineDefn*}
rRoutineDefn(L) ::= rPreRoutineDefn(R). {L = R;}

//-------------------------------------------------
%type rPreRoutineDefn {Ast::RoutineDefn*}
rPreRoutineDefn(L) ::= rEnterRoutineDefn(routineDefn) rCompoundStatement(block). {L = ref(pctx).aRoutineDefn(ref(routineDefn), ref(block));}

//-------------------------------------------------
%type rEnterRoutineDefn {Ast::RoutineDefn*}
rEnterRoutineDefn(L) ::= ROUTINE rQualifiedTypeSpec(out) rRoutineId(name) rInParamsList(in) rDefinitionType(D). {L = ref(pctx).aEnterRoutineDefn(ref(out), name, ref(in), D);}
rRoutineId(L) ::= ID(R). {L = R;}
rRoutineId(L) ::= ROUTINE_TYPE(R). {L = R;}

//-------------------------------------------------
// function definition
%type rFunctionDecl {Ast::FunctionDecl*}
rFunctionDecl(L) ::= rFunctionSig(functionSig) rDefinitionType(defType) SEMI. {L = ref(pctx).aFunctionDecl(ref(functionSig), defType);}

//-------------------------------------------------
// root function declarations
%type rRootFunctionDefn {Ast::RootFunctionDefn*}
rRootFunctionDefn(L) ::= rEnterRootFunctionDefn(functionDefn) rCompoundStatement(block). {L = ref(pctx).aRootFunctionDefn(ref(functionDefn), ref(block));}

//-------------------------------------------------
%type rEnterRootFunctionDefn {Ast::RootFunctionDefn*}
rEnterRootFunctionDefn(L) ::= rFunctionSig(functionSig) rDefinitionType(defType). {L = ref(pctx).aEnterRootFunctionDefn(ref(functionSig), defType);}

//-------------------------------------------------
// child function declaration
%type rChildFunctionDefn {Ast::ChildFunctionDefn*}
rChildFunctionDefn(L) ::= rEnterChildFunctionDefn(functionImpl) rCompoundStatement(block). {L = ref(pctx).aChildFunctionDefn(ref(functionImpl), ref(block));}

//-------------------------------------------------
%type rEnterChildFunctionDefn {Ast::ChildFunctionDefn*}
rEnterChildFunctionDefn(L) ::= rFunctionTypeSpec(base) ID(name). {L = ref(pctx).aEnterChildFunctionDefn(ref(base), name, Ast::DefinitionType::Direct);}

//-------------------------------------------------
// event declarations
%type rEventDecl {Ast::EventDecl*}
rEventDecl(L) ::= EVENT(B) LBRACKET rVariableDefn(in) RBRACKET LINK rFunctionSig(functionSig) rDefinitionType(D) SEMI. {L = ref(pctx).aEventDecl(B, ref(in), ref(functionSig), D);}

//-------------------------------------------------
// function signature.
%type rFunctionSig {Ast::FunctionSig*}
rFunctionSig(T) ::= FUNCTION rParamsList(out)        ID(name) rInParamsList(in). {T = ref(pctx).aFunctionSig(ref(out), name, ref(in));}
rFunctionSig(T) ::= FUNCTION rQualifiedTypeSpec(out) ID(name) rInParamsList(in). {T = ref(pctx).aFunctionSig(ref(out), name, ref(in));}

//-------------------------------------------------
// in parameter list
%type rInParamsList {Ast::Scope*}
rInParamsList(L) ::= rParamsList(scope). {L = ref(pctx).aInParamsList(ref(scope));}

//-------------------------------------------------
// parameter lists
%type rParamsList {Ast::Scope*}
rParamsList(L) ::= LBRACKET rParam(R)                    RBRACKET. {L = ref(pctx).aParamsList(ref(R));}
rParamsList(L) ::= LBRACKET rParam(R) COMMA rPosParam(P) RBRACKET. {L = ref(pctx).aParamsList(ref(R), ref(P));}

//-------------------------------------------------
// variable lists
%type rParam {Ast::Scope*}
rParam(L) ::= rParam(list) COMMA rVariableDefn(variableDef). {L = ref(pctx).aParam(ref(list), ref(variableDef));}
rParam(L) ::=                    rVariableDefn(variableDef). {L = ref(pctx).aParam(ref(variableDef));}
rParam(L) ::= .                                              {L = ref(pctx).aParam();}

//-------------------------------------------------
// positional parameter list
%type rPosParam {Ast::Scope*}
rPosParam(L) ::= rPosParam(list) COMMA rPosVariableDefn(variableDef). {L = ref(pctx).aParam(ref(list), ref(variableDef));}
rPosParam(L) ::=                       rPosVariableDefn(variableDef). {L = ref(pctx).aParam(ref(variableDef));}

//-------------------------------------------------
// variable def
%type rPosVariableDefn {const Ast::VariableDefn*}
rPosVariableDefn(L) ::=                                  ID(name) COLON rExpr(initExpr). {L = ref(pctx).aVariableDefn(name, ref(initExpr));}
rPosVariableDefn(L) ::= rQualifiedVariableDefn(qTypeRef) ID(name) COLON rExpr(initExpr). {L = ref(pctx).aVariableDefn(ref(qTypeRef), name, ref(initExpr));}

//-------------------------------------------------
// variable def
%type rVariableDefn {const Ast::VariableDefn*}
rVariableDefn(L) ::=                                  ID(name) ASSIGNEQUAL rExpr(initExpr). {L = ref(pctx).aVariableDefn(name, ref(initExpr));}
rVariableDefn(L) ::= rQualifiedVariableDefn(qTypeRef) ID(name).                             {L = ref(pctx).aVariableDefn(ref(qTypeRef), name);}
rVariableDefn(L) ::= rQualifiedVariableDefn(qTypeRef) ID(name) ASSIGNEQUAL rExpr(initExpr). {L = ref(pctx).aVariableDefn(ref(qTypeRef), name, ref(initExpr));}

//-------------------------------------------------
// qualified variable def
%type rQualifiedVariableDefn {const Ast::QualifiedTypeSpec*}
rQualifiedVariableDefn(L) ::= rQualifiedTypeSpec(R). {L = ref(pctx).aQualifiedVariableDefn(ref(R));}

//-------------------------------------------------
// qualified types
%type rQualifiedTypeSpec {const Ast::QualifiedTypeSpec*}
rQualifiedTypeSpec(L) ::=       rTypeSpec(typeSpec).               {L = ref(pctx).aQualifiedTypeSpec(false, ref(typeSpec), false);}
rQualifiedTypeSpec(L) ::=       rTypeSpec(typeSpec) BITWISEAND.    {L = ref(pctx).aQualifiedTypeSpec(false, ref(typeSpec), true );}
rQualifiedTypeSpec(L) ::= CONST rTypeSpec(typeSpec).               {L = ref(pctx).aQualifiedTypeSpec(true,  ref(typeSpec), false);}
rQualifiedTypeSpec(L) ::= CONST rTypeSpec(typeSpec) BITWISEAND.    {L = ref(pctx).aQualifiedTypeSpec(true,  ref(typeSpec), true );}

//-------------------------------------------------
// "public" type references, can be invoked from other rules
%type rTypeSpec {const Ast::TypeSpec*}
rTypeSpec(L) ::= rPreTypeSpec(R). {L = ref(pctx).aTypeSpec(ref(R));}
rTypeSpec(L) ::= rTemplateDefnTypeSpec(R). {L = R;}

//-------------------------------------------------
%type rTemplateDefnTypeSpec {const Ast::TemplateDefn*}
rTemplateDefnTypeSpec(L) ::= rTemplateTypeSpec(R) TLT rTemplateTypePartList(P) GT. {L = ref(pctx).aTemplateDefnTypeSpec(ref(R), ref(P));}

//-------------------------------------------------
%type rTemplateTypePartList {Ast::TemplateTypePartList*}
rTemplateTypePartList(L) ::= rTemplateTypePartList(R) COMMA rQualifiedTypeSpec(P). {L = ref(pctx).aTemplateTypePartList(ref(R), ref(P));}
rTemplateTypePartList(L) ::=                                rQualifiedTypeSpec(P). {L = ref(pctx).aTemplateTypePartList(ref(P));}

//-------------------------------------------------
%type rTemplateTypeSpec {const Ast::TemplateDecl*}
rTemplateTypeSpec(L) ::= rPreTemplateTypeSpec(R). {L = ref(pctx).aTemplateTypeSpec(ref(R));}

//-------------------------------------------------
%type rStructTypeSpec {const Ast::StructDefn*}
rStructTypeSpec(L) ::= rPreStructTypeSpec(R). {L = ref(pctx).aStructTypeSpec(ref(R));}

//-------------------------------------------------
%type rRoutineTypeSpec {const Ast::Routine*}
rRoutineTypeSpec(L) ::= rPreRoutineTypeSpec(R). {L = ref(pctx).aRoutineTypeSpec(ref(R));}

//-------------------------------------------------
%type rFunctionTypeSpec {const Ast::Function*}
rFunctionTypeSpec(L) ::= rPreFunctionTypeSpec(R). {L = ref(pctx).aFunctionTypeSpec(ref(R));}

//-------------------------------------------------
%type rEventTypeSpec {const Ast::EventDecl*}
rEventTypeSpec(L) ::= rPreEventTypeSpec(R). {L = ref(pctx).aEventTypeSpec(ref(R));}

//-------------------------------------------------
// "private" type references, can be only called by the public equivalent rules
%type rPreTypeSpec {const Ast::TypeSpec*}
rPreTypeSpec(L) ::= rPreTemplateTypeSpec(R). {L = R;}
rPreTypeSpec(L) ::= rPreStructTypeSpec(R).   {L = R;}
rPreTypeSpec(L) ::= rPreRoutineTypeSpec(R).  {L = R;}
rPreTypeSpec(L) ::= rPreFunctionTypeSpec(R). {L = R;}
rPreTypeSpec(L) ::= rPreEventTypeSpec(R).    {L = R;}
rPreTypeSpec(L) ::= rPreOtherTypeSpec(R).    {L = R;}

//-------------------------------------------------
%type rPreTemplateTypeSpec {const Ast::TemplateDecl*}
rPreTemplateTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE TEMPLATE_TYPE(name). {L = ref(pctx).aTemplateTypeSpec(ref(parent), name);}
rPreTemplateTypeSpec(L) ::=                            TEMPLATE_TYPE(name). {L = ref(pctx).aTemplateTypeSpec(name);}

//-------------------------------------------------
%type rPreStructTypeSpec {const Ast::StructDefn*}
rPreStructTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE STRUCT_TYPE(name). {L = ref(pctx).aStructTypeSpec(ref(parent), name);}
rPreStructTypeSpec(L) ::=                            STRUCT_TYPE(name). {L = ref(pctx).aStructTypeSpec(name);}

//-------------------------------------------------
%type rPreRoutineTypeSpec {const Ast::Routine*}
rPreRoutineTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE ROUTINE_TYPE(name). {L = ref(pctx).aRoutineTypeSpec(ref(parent), name);}
rPreRoutineTypeSpec(L) ::=                            ROUTINE_TYPE(name). {L = ref(pctx).aRoutineTypeSpec(name);}

//-------------------------------------------------
%type rPreFunctionTypeSpec {const Ast::Function*}
rPreFunctionTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE FUNCTION_TYPE(name). {L = ref(pctx).aFunctionTypeSpec(ref(parent), name);}
rPreFunctionTypeSpec(L) ::=                            FUNCTION_TYPE(name). {L = ref(pctx).aFunctionTypeSpec(name);}

//-------------------------------------------------
%type rPreEventTypeSpec {const Ast::EventDecl*}
rPreEventTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE EVENT_TYPE(name). {L = ref(pctx).aEventTypeSpec(ref(parent), name);}
rPreEventTypeSpec(L) ::=                            EVENT_TYPE(name). {L = ref(pctx).aEventTypeSpec(name);}

//-------------------------------------------------
%type rPreOtherTypeSpec {const Ast::TypeSpec*}
rPreOtherTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE OTHER_TYPE(name). {L = ref(pctx).aOtherTypeSpec(ref(parent), name);}
rPreOtherTypeSpec(L) ::=                            OTHER_TYPE(name). {L = ref(pctx).aOtherTypeSpec(name);}

//-------------------------------------------------
// statements
%type rInnerStatement {Ast::Statement*}
rInnerStatement(L) ::= rUserDefinedTypeSpecStatement(R). {L = R;}
rInnerStatement(L) ::= rAutoStatement(R).                {L = R;}
rInnerStatement(L) ::= rExprStatement(R).                {L = R;}
rInnerStatement(L) ::= rPrintStatement(R).               {L = R;}
rInnerStatement(L) ::= rIfStatement(R).                  {L = R;}
rInnerStatement(L) ::= rIfElseStatement(R).              {L = R;}
rInnerStatement(L) ::= rWhileStatement(R).               {L = R;}
rInnerStatement(L) ::= rDoWhileStatement(R).             {L = R;}
rInnerStatement(L) ::= rForStatement(R).                 {L = R;}
rInnerStatement(L) ::= rForeachStatement(R).             {L = R;}
rInnerStatement(L) ::= rSwitchStatement(R).              {L = R;}
rInnerStatement(L) ::= rBreakStatement(R).               {L = R;}
rInnerStatement(L) ::= rContinueStatement(R).            {L = R;}
rInnerStatement(L) ::= rAddEventHandlerStatement(R).     {L = R;}
rInnerStatement(L) ::= rRoutineReturnStatement(R).       {L = R;}
rInnerStatement(L) ::= rFunctionReturnStatement(R).      {L = R;}
rInnerStatement(L) ::= rCompoundStatement(R).            {L = R;}

//-------------------------------------------------
%type rUserDefinedTypeSpecStatement {Ast::UserDefinedTypeSpecStatement*}
rUserDefinedTypeSpecStatement(L) ::= rTypeSpecDef(typeSpec). {L = ref(pctx).aUserDefinedTypeSpecStatement(ref(typeSpec));}

//-------------------------------------------------
%type rAutoStatement {Ast::AutoStatement*}
rAutoStatement(L) ::= AUTO rVariableDefn(defn) SEMI. {L = ref(pctx).aAutoStatement(ref(defn));}

//-------------------------------------------------
%type rExprStatement {Ast::ExprStatement*}
rExprStatement(L) ::= rExpr(expr) SEMI. {L = ref(pctx).aExprStatement(ref(expr));}

//-------------------------------------------------
%type rPrintStatement {Ast::PrintStatement*}
rPrintStatement(L) ::= PRINT rExpr(expr) SEMI. {L = ref(pctx).aPrintStatement(ref(expr));}

//-------------------------------------------------
%type rIfStatement {Ast::IfStatement*}
rIfStatement(L) ::= IF LBRACKET rExpr(expr) RBRACKET rCompoundStatement(tblock). {L = ref(pctx).aIfStatement(ref(expr), ref(tblock));}

//-------------------------------------------------
%type rIfElseStatement {Ast::IfElseStatement*}
rIfElseStatement(L) ::= IF LBRACKET rExpr(expr) RBRACKET rCompoundStatement(tblock) ELSE rCompoundStatement(fblock). {L = ref(pctx).aIfElseStatement(ref(expr), ref(tblock), ref(fblock));}

//-------------------------------------------------
%type rWhileStatement {Ast::WhileStatement*}
rWhileStatement(L) ::= WHILE LBRACKET rExpr(expr) RBRACKET rCompoundStatement(block). {L = ref(pctx).aWhileStatement(ref(expr), ref(block));}

//-------------------------------------------------
%type rDoWhileStatement {Ast::DoWhileStatement*}
rDoWhileStatement(L) ::= DO rCompoundStatement(block) WHILE LBRACKET rExpr(expr) RBRACKET SEMI. {L = ref(pctx).aDoWhileStatement(ref(expr), ref(block));}

//-------------------------------------------------
%type rForStatement {Ast::ForStatement*}
rForStatement(L) ::= FOR LBRACKET rExpr(init) SEMI rExpr(expr) SEMI rExpr(incr) RBRACKET rCompoundStatement(block). {L = ref(pctx).aForStatement(ref(init), ref(expr), ref(incr), ref(block));}
rForStatement(L) ::= FOR LBRACKET rEnterForInit(init) SEMI rExpr(expr) SEMI rExpr(incr) RBRACKET rCompoundStatement(block). {L = ref(pctx).aForStatement(ref(init), ref(expr), ref(incr), ref(block));}

%type rEnterForInit {const Ast::VariableDefn*}
rEnterForInit(L) ::= AUTO rVariableDefn(init). {L = ref(pctx).aEnterForInit(ref(init));}

//-------------------------------------------------
%type rForeachStatement {Ast::ForeachStatement*}
rForeachStatement(L) ::= FOREACH LBRACKET rEnterForeachInit(vdef) RBRACKET rCompoundStatement(block). {L = ref(pctx).aForeachStatement(ref(vdef), ref(block));}

%type rEnterForeachInit {Ast::ForeachStatement*}
rEnterForeachInit(L) ::= ID(I) IN rExpr(list). {L = ref(pctx).aEnterForeachInit(I, ref(list));}
rEnterForeachInit(L) ::= ID(K) COMMA ID(V) IN rExpr(list). {L = ref(pctx).aEnterForeachInit(K, V, ref(list));}

//-------------------------------------------------
%type rSwitchStatement {Ast::SwitchStatement*}
rSwitchStatement(L) ::= SWITCH LBRACKET rExpr(expr) RBRACKET LCURLY rCaseList(list) RCURLY. {L = ref(pctx).aSwitchStatement(ref(expr), ref(list));}
rSwitchStatement(L) ::= SWITCH                               LCURLY rCaseList(list) RCURLY. {L = ref(pctx).aSwitchStatement(ref(list));}

//-------------------------------------------------
%type rCaseList {Ast::CompoundStatement*}
rCaseList(L) ::= rCaseList(R) rCaseStatement(S). {L = ref(pctx).aCaseList(ref(R), ref(S));}
rCaseList(L) ::=              rCaseStatement(S). {L = ref(pctx).aCaseList(ref(S));}

//-------------------------------------------------
%type rCaseStatement {Ast::CaseStatement*}
rCaseStatement(L) ::= CASE rExpr(expr) COLON rCompoundStatement(block). {L = ref(pctx).aCaseStatement(ref(expr), ref(block));}
rCaseStatement(L) ::= DEFAULT          COLON rCompoundStatement(block). {L = ref(pctx).aCaseStatement(ref(block));}

//-------------------------------------------------
%type rBreakStatement {Ast::BreakStatement*}
rBreakStatement(L) ::= BREAK SEMI. {L = ref(pctx).aBreakStatement();}

//-------------------------------------------------
%type rContinueStatement {Ast::ContinueStatement*}
rContinueStatement(L) ::= CONTINUE SEMI. {L = ref(pctx).aContinueStatement();}

//-------------------------------------------------
%type rAddEventHandlerStatement {Ast::AddEventHandlerStatement*}
rAddEventHandlerStatement(L) ::= rEventTypeSpec(E) LBRACKET rExpr(X) RBRACKET LINK rAnonymousFunctionExpr(F) SEMI. {L = ref(pctx).aAddEventHandlerStatement(ref(E), ref(X), ref(F));}

//-------------------------------------------------
%type rRoutineReturnStatement {Ast::RoutineReturnStatement*}
rRoutineReturnStatement(L) ::= RRETURN          SEMI. {L = ref(pctx).aRoutineReturnStatement();}
rRoutineReturnStatement(L) ::= RRETURN rExpr(S) SEMI. {L = ref(pctx).aRoutineReturnStatement(ref(S));}

//-------------------------------------------------
%type rFunctionReturnStatement {Ast::FunctionReturnStatement*}
rFunctionReturnStatement(L) ::= FRETURN rExprsList(S) SEMI. {L = ref(pctx).aFunctionReturnStatement(ref(S));}

//-------------------------------------------------
// simple list of statements
%type rCompoundStatement {Ast::CompoundStatement*}
rCompoundStatement(L)   ::= rEnterCompoundStatement rStatementList(R) rLeaveCompoundStatement. {L = R;}
rEnterCompoundStatement ::= LCURLY. {ref(pctx).aEnterCompoundStatement();}
rLeaveCompoundStatement ::= RCURLY. {ref(pctx).aLeaveCompoundStatement();}

%type rStatementList {Ast::CompoundStatement*}
rStatementList(L) ::= rStatementList(list) rInnerStatement(statement). {L = ref(pctx).aStatementList(ref(list), ref(statement));}
rStatementList(L) ::= .                                                {L = ref(pctx).aStatementList();}

//-------------------------------------------------
// expression list in brackets
%type rExprsList {Ast::ExprList*}
rExprsList(L) ::= LBRACKET rExprList(R) RBRACKET. {L = R;}

//-------------------------------------------------
// comma-separated list of expressions
%type rExprList {Ast::ExprList*}
rExprList(R) ::= rExprList(L) COMMA rExpr(E). {R = ref(pctx).aExprList(ref(L), ref(E));}
rExprList(R) ::=                    rExpr(E). {R = ref(pctx).aExprList(ref(E));}
rExprList(R) ::= .                            {R = ref(pctx).aExprList();}

//-------------------------------------------------
// expressions
%type rExpr {const Ast::Expr*}
rExpr(L) ::= rTernaryExpr(R).      {L = R;}
rExpr(L) ::= rBinaryExpr(R).       {L = R;}
rExpr(L) ::= rPostfixExpr(R).      {L = R;}
rExpr(L) ::= rPrefixExpr(R).       {L = R;}
rExpr(L) ::= rListExpr(R).         {L = R;}
rExpr(L) ::= rDictExpr(R).         {L = R;}
rExpr(L) ::= rFormatExpr(R).       {L = R;}
rExpr(L) ::= rCallExpr(R).         {L = R;}
rExpr(L) ::= rRunExpr(R).          {L = R;}
rExpr(L) ::= rOrderedExpr(R).      {L = R;}
rExpr(L) ::= rIndexExpr(R).        {L = R;}
rExpr(L) ::= rVariableRefExpr(R).       {L = R;}
rExpr(L) ::= rMemberVariableExpr(R).    {L = R;}
rExpr(L) ::= rTypeSpecMemberExpr(R).    {L = R;}
rExpr(L) ::= rStructInstanceExpr(R).    {L = R;}
rExpr(L) ::= rFunctionInstanceExpr(R).  {L = R;}
rExpr(L) ::= rAnonymousFunctionExpr(R). {L = R;}
rExpr(L) ::= rConstantExpr(R).          {L = R;}

//-------------------------------------------------
// It could be possible to implement creating local variables inline within expressions.
// Not sure how to implement it in the generated code. Not a priority, so on hold for now.
//rExpr(E) ::= ID(L) DEFINEEQUAL       rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(ref(L), z::string("="), ref(R)));}

//-------------------------------------------------
// ternary operators
%type rTernaryExpr {const Ast::TernaryOpExpr*}
rTernaryExpr(E) ::= rExpr(L) QUESTION(O1) rExpr(T) COLON(O2) rExpr(F). {E = ref(pctx).aTernaryExpr(O1, O2, ref(L), ref(T), ref(F));}

//-------------------------------------------------
// binary operators
%type rBinaryExpr {const Ast::Expr*}
rBinaryExpr(E) ::= rExpr(L) ASSIGNEQUAL(O)     rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) TIMESEQUAL(O)      rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) DIVIDEEQUAL(O)     rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) MINUSEQUAL(O)      rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) PLUSEQUAL(O)       rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) MODEQUAL(O)        rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) SHIFTLEFTEQUAL(O)  rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) SHIFTRIGHTEQUAL(O) rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEANDEQUAL(O) rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEXOREQUAL(O) rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEOREQUAL(O)  rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEAND(O)      rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEXOR(O)      rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEOR(O)       rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISENOT(O)      rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) AND(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) OR(O)              rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) EQUAL(O)           rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) NOTEQUAL(O)        rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) LT(O)              rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) GT(O)              rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) LTE(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) GTE(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) HAS(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) SHL(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) SHR(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) PLUS(O)            rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) MINUS(O)           rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) STAR(O)            rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) DIVIDE(O)          rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rBinaryExpr(E) ::= rExpr(L) MOD(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}

//-------------------------------------------------
// postfix operators
%type rPostfixExpr {const Ast::Expr*}
rPostfixExpr(E) ::= rExpr(L) INC(O). {E = ptr(ref(pctx).aPostfixExpr(O, ref(L)));}
rPostfixExpr(E) ::= rExpr(L) DEC(O). {E = ptr(ref(pctx).aPostfixExpr(O, ref(L)));}

//-------------------------------------------------
// prefix operators
%type rPrefixExpr {const Ast::Expr*}
rPrefixExpr(E) ::= NOT(O)        rExpr(R). {E = ptr(ref(pctx).aPrefixExpr(O, ref(R)));}
rPrefixExpr(E) ::= PLUS(O)       rExpr(R). {E = ptr(ref(pctx).aPrefixExpr(O, ref(R)));}
rPrefixExpr(E) ::= MINUS(O)      rExpr(R). {E = ptr(ref(pctx).aPrefixExpr(O, ref(R)));}
rPrefixExpr(E) ::= INC(O)        rExpr(R). {E = ptr(ref(pctx).aPrefixExpr(O, ref(R)));}
rPrefixExpr(E) ::= DEC(O)        rExpr(R). {E = ptr(ref(pctx).aPrefixExpr(O, ref(R)));}
rPrefixExpr(E) ::= BITWISENOT(O) rExpr(R). {E = ptr(ref(pctx).aPrefixExpr(O, ref(R)));}

//-------------------------------------------------
// string formatter
%type rFormatExpr {Ast::FormatExpr*}
rFormatExpr(L) ::= rExpr(A) AMP(B) rTreeExpr(T). {L = ref(pctx).aFormatExpr(B, ref(A), ref(T));}

//-------------------------------------------------
// list expression
%type rListExpr {Ast::ListExpr*}
rListExpr(L) ::= rListList(R) RSQUARE(B). {L = ref(pctx).aListExpr(B, ref(R));}

%type rListList {Ast::ListList*}
rListList(L) ::= rListsList(R)      . {L = R;}
rListList(L) ::= rListsList(R) COMMA. {L = R;}

%type rListsList {Ast::ListList*}
rListsList(L)  ::= rListsList(R) COMMA(B) rListItem(I). {L = ref(pctx).aListList(B, ref(R), ref(I));}
rListsList(L)  ::=             LSQUARE(B) rListItem(I). {L = ref(pctx).aListList(B, ref(I));}
rListsList(L)  ::=             LSQUARE(B) rQualifiedTypeSpec(Q). {L = ref(pctx).aListList(B, ref(Q));}
rListsList(L)  ::=             LSQUARE(B)                      . {L = ref(pctx).aListList(B);}

%type rListItem {Ast::ListItem*}
rListItem(L)  ::= rExpr(E). {L = ref(pctx).aListItem(ref(E));}

//-------------------------------------------------
// dict (strict type-checking for key and value)
%type rDictExpr {Ast::DictExpr*}
rDictExpr(L) ::= LSQUARE(B) rDictList(R) RSQUARE. {L = ref(pctx).aDictExpr(B, ref(R));}

%type rDictList {Ast::DictList*}
rDictList(L) ::= rDictsList(R)       . {L = R;}
rDictList(L) ::= rDictsList(R) COMMA . {L = R;}

%type rDictsList {Ast::DictList*}
rDictsList(L)  ::= rDictsList(R) COMMA(B) rDictItem(I). {L = ref(pctx).aDictList(B, ref(R), ref(I));}
rDictsList(L)  ::=                        rDictItem(I). {L = ref(pctx).aDictList(ref(I));}

// first item in list can be a type specifier
rDictsList(L) ::= rQualifiedTypeSpec(K) COLON rQualifiedTypeSpec(V). {L = ref(pctx).aDictList(ref(K), ref(V));}

%type rDictItem {Ast::DictItem*}
rDictItem(L)  ::= rExpr(K) COLON rExpr(E). {L = ref(pctx).aDictItem(ref(K), ref(E));}

//-------------------------------------------------
// tree (no type checking for key or value)
%type rTreeExpr {Ast::DictExpr*}
rTreeExpr(L) ::= LCURLY(B) rTreeList(R) RCURLY. {L = ref(pctx).aDictExpr(B, ref(R));}
rTreeExpr(L) ::= LCURLY(B)              RCURLY. {L = ref(pctx).aDictExpr(B);}

%type rTreeList {Ast::DictList*}
rTreeList(L) ::= rTreesList(R)       . {L = R;}
rTreeList(L) ::= rTreesList(R) COMMA . {L = R;}

%type rTreesList {Ast::DictList*}
rTreesList(L)  ::= rTreesList(R) COMMA(B) rTreeItem(I). {L = ref(pctx).aDictList(B, ref(R), ref(I));}
rTreesList(L)  ::=                        rTreeItem(I). {L = ref(pctx).aDictList(ref(I));}

%type rTreeItem {Ast::DictItem*}
rTreeItem(L)  ::= rExpr(K) COLON rExpr(E). {L = ref(pctx).aDictItem(ref(K), ref(E));}

//-------------------------------------------------
// ordered expression
%type rOrderedExpr {Ast::OrderedExpr*}
rOrderedExpr(L) ::= LBRACKET rExpr(innerExpr) RBRACKET. {L = ref(pctx).aOrderedExpr(ref(innerExpr));}

//-------------------------------------------------
// index expression
%type rIndexExpr {Ast::IndexExpr*}
rIndexExpr(L) ::= rExpr(E) LSQUARE(B) rExpr(innerExpr) RSQUARE. {L = ref(pctx).aIndexExpr(B, ref(E), ref(innerExpr));}
rIndexExpr(L) ::= rExpr(E) AMP(B) rKeyConstantExpr(innerExpr). {L = ref(pctx).aIndexExpr(B, ref(E), ref(innerExpr));}

//-------------------------------------------------
// type expression
rExpr(L) ::= TYPEOF(B) LBRACKET rQualifiedTypeSpec(T) RBRACKET. {L = ref(pctx).aTypeofTypeExpr(B, ref(T));}
rExpr(L) ::= TYPEOF(B) LBRACKET rExpr(E)              RBRACKET. {L = ref(pctx).aTypeofExprExpr(B, ref(E));}

//-------------------------------------------------
// type-cast expression
rExpr(L) ::= LT(B) rQualifiedTypeSpec(T) GT rExpr(E). {L = ref(pctx).aTypecastExpr(B, ref(T), ref(E));}

//-------------------------------------------------
// address-of expression
rExpr(L) ::= BITWISEAND(B)  rExpr(E). {L = ref(pctx).aPointerInstanceExpr(B, ref(E));}

// value-of expression
rExpr(L) ::= STAR(B) rExpr(E). {L = ref(pctx).aValueInstanceExpr(B, ref(E));}

//-------------------------------------------------
// template definition instance expression
rExpr(L) ::= rTemplateDefnTypeSpec(R) LBRACKET(B) rExprList(M) RBRACKET. {L = ref(pctx).aTemplateDefnInstanceExpr(B, ref(R), ref(M));}

//-------------------------------------------------
// variable ref expressions
%type rVariableRefExpr {Ast::Expr*}
rVariableRefExpr(L) ::= ID(I). {L = ref(pctx).aVariableRefExpr(I);}

//-------------------------------------------------
// variable member expressions
%type rMemberVariableExpr {Ast::MemberExpr*}
rMemberVariableExpr(L) ::= rExpr(R) DOT ID(M). {L = ref(pctx).aMemberVariableExpr(ref(R), M);}

//-------------------------------------------------
// type member expressions, e.g. enum member
%type rTypeSpecMemberExpr {Ast::TypeSpecMemberExpr*}
rTypeSpecMemberExpr(L) ::= rTypeSpec(R) DOT ID(M). {L = ref(pctx).aTypeSpecMemberExpr(ref(R), M);}

//-------------------------------------------------
// function instance expressions
%type rFunctionInstanceExpr {Ast::TypeSpecInstanceExpr*}
rFunctionInstanceExpr(L) ::= rFunctionTypeSpec(R) LSQUARE rExprList(M) RSQUARE. {L = ref(pctx).aFunctionInstanceExpr(ref(R), ref(M));}

//-------------------------------------------------
// function instance expressions
%type rAnonymousFunctionExpr {Ast::AnonymousFunctionExpr*}
rAnonymousFunctionExpr(L) ::= rEnterAnonymousFunction(R) rCompoundStatement(C). {L = ref(pctx).aAnonymousFunctionExpr(ref(R), ref(C));}

//-------------------------------------------------
// function instance expressions
%type rEnterAnonymousFunction {Ast::ChildFunctionDefn*}
rEnterAnonymousFunction(L) ::= rFunctionTypeSpec(R). {L = ref(pctx).aEnterAnonymousFunction(ref(R));}

//-------------------------------------------------
// struct instance expressions
%type rStructInstanceExpr {Ast::StructInstanceExpr*}
rStructInstanceExpr(L) ::= rEnterStructInstanceExpr(R) LCURLY(B) rStructInitPartList(P) rLeaveStructInstanceExpr. {L = ref(pctx).aStructInstanceExpr(B, ref(R), ref(P));}
rStructInstanceExpr(L) ::= rEnterStructInstanceExpr(R) LCURLY(B)                        rLeaveStructInstanceExpr. {L = ref(pctx).aStructInstanceExpr(B, ref(R));}

//-------------------------------------------------
// special case - struct can be instantiated with {} or () for syntactic equivalence with C/C++.
rStructInstanceExpr(L) ::= rStructTypeSpec(R) LBRACKET(B) rStructInitPartList(P) RBRACKET. {L = ref(pctx).aStructInstanceExpr(B, ref(R), ref(P));}
rStructInstanceExpr(L) ::= rStructTypeSpec(R) LBRACKET(B)                        RBRACKET. {L = ref(pctx).aStructInstanceExpr(B, ref(R));}

//-------------------------------------------------
%type rEnterStructInstanceExpr {const Ast::StructDefn*}
rEnterStructInstanceExpr(L) ::= rStructTypeSpec(R). {L = ref(pctx).aEnterStructInstanceExpr(ref(R));}

//-------------------------------------------------
rLeaveStructInstanceExpr ::= RCURLY. {ref(pctx).aLeaveStructInstanceExpr();}

//-------------------------------------------------
%type rStructInitPartList {Ast::StructInitPartList*}
rStructInitPartList(L) ::= rStructInitPartList(R) rStructInitPart(P). {L = ref(pctx).aStructInitPartList(ref(R), ref(P));}
rStructInitPartList(L) ::=                        rStructInitPart(P). {L = ref(pctx).aStructInitPartList(ref(P));}

//-------------------------------------------------
%type rStructInitPart {Ast::StructInitPart*}
rStructInitPart(L) ::= rEnterStructInitPart(R) COLON rExpr(E) rLeaveStructInitPart. {L = ref(pctx).aStructInitPart(ref(R), ref(E));}

//-------------------------------------------------
%type rEnterStructInitPart {const Ast::VariableDefn*}
rEnterStructInitPart(L) ::= ID(R). {L = ref(pctx).aEnterStructInitPart(R);}
rLeaveStructInitPart    ::= SEMI. {ref(pctx).aLeaveStructInitPart();}

//-------------------------------------------------
// functor call expressions
%type rCallExpr {Ast::CallExpr*}
rCallExpr(L) ::= rCallPart(R).  {L = R;}

//-------------------------------------------------
// functor call expressions
%type rCallPart {Ast::CallExpr*}
rCallPart(L) ::= rRoutineCallPart(R). {L = R;}
rCallPart(L) ::= rFunctorCallPart(R). {L = R;}

//-------------------------------------------------
// routine call expressions
%type rRoutineCallPart {Ast::RoutineCallExpr*}
rRoutineCallPart(L) ::= rRoutineTypeSpec(typeSpec) LBRACKET(B) rExprList(exprList) RBRACKET.  {L = ref(pctx).aRoutineCallExpr(B, ref(typeSpec), ref(exprList));}

//-------------------------------------------------
// function call type
%type rRunExpr {Ast::RunExpr*}
rRunExpr(L) ::= RUN(B) rFunctorCallPart(F).  {L = ref(pctx).aRunExpr(B, ref(F));}

//-------------------------------------------------
// functor call expressions
%type rFunctorCallPart {Ast::FunctorCallExpr*}
rFunctorCallPart(L) ::= ID(I)                       LBRACKET(B) rExprList(exprList) RBRACKET.  {L = ref(pctx).aFunctorCallExpr(B, I, ref(exprList));}
rFunctorCallPart(L) ::= rOrderedExpr(expr)          LBRACKET(B) rExprList(exprList) RBRACKET.  {L = ref(pctx).aFunctorCallExpr(B, ref(expr), ref(exprList));}
rFunctorCallPart(L) ::= rFunctionTypeSpec(typeSpec) LBRACKET(B) rExprList(exprList) RBRACKET.  {L = ref(pctx).aFunctionCallExpr(B, ref(typeSpec), ref(exprList));}

//-------------------------------------------------
// constant expressions
%type rConstantExpr {const Ast::ConstantExpr*}
rConstantExpr(L) ::= FLOAT_CONST  (value).  {L = ptr(ref(pctx).aConstantExpr("float",  value));}
rConstantExpr(L) ::= DOUBLE_CONST (value).  {L = ptr(ref(pctx).aConstantExpr("double", value));}
rConstantExpr(L) ::= TRUE_CONST   (value).  {L = ptr(ref(pctx).aConstantExpr("bool",   value));}
rConstantExpr(L) ::= FALSE_CONST  (value).  {L = ptr(ref(pctx).aConstantExpr("bool",   value));}
rConstantExpr(L) ::= STRING_CONST (value).  {L = ptr(ref(pctx).aConstantExpr("string", value));}
rConstantExpr(L) ::= CHAR_CONST   (value).  {L = ptr(ref(pctx).aConstantExpr("char",   value));}
rConstantExpr(L) ::= HEXINT_CONST (value).  {L = ptr(ref(pctx).aConstantExpr("int",    value));}
rConstantExpr(L) ::= DECINT_CONST (value).  {L = ptr(ref(pctx).aConstantExpr("int",    value));}
rConstantExpr(L) ::= OCTINT_CONST (value).  {L = ptr(ref(pctx).aConstantExpr("int",    value));}
rConstantExpr(L) ::= LHEXINT_CONST(value).  {L = ptr(ref(pctx).aConstantExpr("long",   value));}
rConstantExpr(L) ::= LDECINT_CONST(value).  {L = ptr(ref(pctx).aConstantExpr("long",   value));}
rConstantExpr(L) ::= LOCTINT_CONST(value).  {L = ptr(ref(pctx).aConstantExpr("long",   value));}
rConstantExpr(L) ::= rKeyConstantExpr(R).   {L = R;}

%type rKeyConstantExpr {const Ast::ConstantExpr*}
rKeyConstantExpr(L) ::= KEY_CONST(value).  {L = ptr(ref(pctx).aConstantExpr("string", value));}

/*
//-------------------------------------------------
// binary operators
rBinaryOp(L) ::= ASSIGNEQUAL(R)     . {L = R;}
rBinaryOp(L) ::= TIMESEQUAL(R)      . {L = R;}
rBinaryOp(L) ::= DIVIDEEQUAL(R)     . {L = R;}
rBinaryOp(L) ::= MINUSEQUAL(R)      . {L = R;}
rBinaryOp(L) ::= PLUSEQUAL(R)       . {L = R;}
rBinaryOp(L) ::= MODEQUAL(R)        . {L = R;}
rBinaryOp(L) ::= SHIFTLEFTEQUAL(R)  . {L = R;}
rBinaryOp(L) ::= SHIFTRIGHTEQUAL(R) . {L = R;}
rBinaryOp(L) ::= BITWISEANDEQUAL(R) . {L = R;}
rBinaryOp(L) ::= BITWISEXOREQUAL(R) . {L = R;}
rBinaryOp(L) ::= BITWISEOREQUAL(R)  . {L = R;}
rBinaryOp(L) ::= BITWISEAND(R)      . {L = R;}
rBinaryOp(L) ::= BITWISEXOR(R)      . {L = R;}
rBinaryOp(L) ::= BITWISEOR(R)       . {L = R;}
rBinaryOp(L) ::= BITWISENOT(R)      . {L = R;}
rBinaryOp(L) ::= AND(R)             . {L = R;}
rBinaryOp(L) ::= OR(R)              . {L = R;}
rBinaryOp(L) ::= EQUAL(R)           . {L = R;}
rBinaryOp(L) ::= NOTEQUAL(R)        . {L = R;}
rBinaryOp(L) ::= LT(R)              . {L = R;}
rBinaryOp(L) ::= GT(R)              . {L = R;}
rBinaryOp(L) ::= LTE(R)             . {L = R;}
rBinaryOp(L) ::= GTE(R)             . {L = R;}
rBinaryOp(L) ::= HAS(R)             . {L = R;}
rBinaryOp(L) ::= SHL(R)             . {L = R;}
rBinaryOp(L) ::= SHR(R)             . {L = R;}
rBinaryOp(L) ::= PLUS(R)            . {L = R;}
rBinaryOp(L) ::= MINUS(R)           . {L = R;}
rBinaryOp(L) ::= STAR(R)            . {L = R;}
rBinaryOp(L) ::= DIVIDE(R)          . {L = R;}
rBinaryOp(L) ::= MOD(R)             . {L = R;}
*/
