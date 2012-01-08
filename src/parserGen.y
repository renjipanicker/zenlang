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
    throw z::Exception("%s Syntax error at token: %d (%s)\n", err(z::ref(pctx).filename(), TOKEN).c_str(), TOKEN.id(), TOKEN.text());
}

%parse_accept {
//    zbl::mlog() << "Parse complete!";
}

%parse_failure {
//    throw z::exception(z::string::creator("%{err} Parse error").arg(z::any("err"), z::any(z::string(z::ref(pctx).err(TOKEN)))).value());
}

%stack_overflow {
//    throw z::exception(z::string::creator("%{err} Stack overflow error").arg(z::any("err"), z::any(z::ref(pctx).err())).value());
}

%token_destructor {
    unused(pctx);
    TokenData::deleteT($$);
}

%name ZenParser

%token_type {TokenData}
%extra_argument {Ast::NodeFactory* pctx}

%include {
#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "error.hpp"
#include "NodeFactory.hpp"
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
rUnitStatementList ::= rImportStatementList rNamespaceStatement(N) rGlobalStatementList. {z::ref(pctx).aUnitStatementList(z::ref(N));}

//-------------------------------------------------
// list of statements to specify imports
rImportStatementList ::= rImportStatementList rImportStatement.
rImportStatementList ::= .

//-------------------------------------------------
rImportStatement ::= rHeaderType(headerType) rImportNamespaceList(L) rDefinitionType(defType) rOptionalAccessType(A) SEMI(B). {z::ref(pctx).aImportStatement(B, A, headerType, defType, z::ref(L));}

//-------------------------------------------------
// import namespace list
%type rImportNamespaceList {Ast::NamespaceList*}
rImportNamespaceList(L) ::= rImportNamespaceList(R) SCOPE rAnyId(name). {L = z::ref(pctx).aImportNamespaceList(z::ref(R), name);}
rImportNamespaceList(L) ::=                               rAnyId(name). {L = z::ref(pctx).aImportNamespaceList(name);}

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
rNamespaceStatement(L) ::= NAMESPACE(B) rUnitNamespaceList(R) SEMI. {L = z::ref(pctx).aNamespaceStatement(B, z::ref(R));}
rNamespaceStatement(L) ::=                                        . {L = z::ref(pctx).aNamespaceStatement();}

//-------------------------------------------------
// namespace list
%type rUnitNamespaceList {Ast::NamespaceList*}
rUnitNamespaceList(L) ::= rUnitNamespaceList(R) SCOPE rAnyId(name). {L = z::ref(pctx).aUnitNamespaceList(z::ref(R), name);}
rUnitNamespaceList(L) ::=                             rAnyId(name). {L = z::ref(pctx).aUnitNamespaceList(name);}

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
rGlobalTypeSpecStatement(L) ::= rAccessType(accessType) rTypeSpecDef(typeSpec). {L = z::ref(pctx).aGlobalTypeSpecStatement(accessType, z::ref(typeSpec));}
rGlobalTypeSpecStatement(L) ::= rInnerStatement(R).                             {L = z::ref(pctx).aGlobalStatement(z::ref(R));}

//-------------------------------------------------
// access specifiers
%type rAccessType {Ast::AccessType::T}
rAccessType(L) ::= PRIVATE.   {L = Ast::AccessType::Private;}
rAccessType(L) ::= PUBLIC.    {L = Ast::AccessType::Public;}
rAccessType(L) ::= INTERNAL.  {L = Ast::AccessType::Internal;}
rAccessType(L) ::= EXTERNAL.  {L = Ast::AccessType::External;}

//-------------------------------------------------
// coercion statements
rGlobalCoerceStatement ::= COERCE rCoerceList(T) SEMI. {z::ref(pctx).aGlobalCoerceStatement(z::ref(T));}

%type rCoerceList {Ast::CoerceList*}
rCoerceList(L) ::= rCoerceList(R) LINK rTypeSpec(T). {L = z::ref(pctx).aCoerceList(z::ref(R), z::ref(T));}
rCoerceList(L) ::=                     rTypeSpec(T). {L = z::ref(pctx).aCoerceList(z::ref(T));}

//-------------------------------------------------
// default values for types
rGlobalDefaultStatement ::= DEFAULT rTypeSpec(T) ASSIGNEQUAL rExpr(E) SEMI. {z::ref(pctx).aGlobalDefaultStatement(z::ref(T), z::ref(E));}

//-------------------------------------------------
// definition specifiers
%type rDefinitionType {Ast::DefinitionType::T}
rDefinitionType(L) ::= NATIVE. {L = Ast::DefinitionType::Native;}
rDefinitionType(L) ::= FINAL.  {L = Ast::DefinitionType::Final;}
rDefinitionType(L) ::= .       {L = Ast::DefinitionType::Final;}

//-------------------------------------------------
// definition specifiers
%type rExDefinitionType {Ast::DefinitionType::T}
rExDefinitionType(L) ::= ABSTRACT. {L = Ast::DefinitionType::Abstract;}
rExDefinitionType(L) ::= rDefinitionType(R). {L = R;}

//-------------------------------------------------
// definition specifiers
%type rAbstractDefinitionType {Ast::DefinitionType::T}
rAbstractDefinitionType(L) ::= ABSTRACT. {L = Ast::DefinitionType::Abstract;}
rAbstractDefinitionType(L) ::= . {L = Ast::DefinitionType::Abstract;}

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
rPreTypedefDecl(L) ::= TYPEDEF ID(name) rDefinitionType(D). {L = z::ref(pctx).aTypedefDecl(name, D);}

//-------------------------------------------------
// typedef definition
%type rTypedefDefn {Ast::TypedefDefn*}
rTypedefDefn(L) ::= rPreTypedefDefn(R) SEMI. {L = R;}

%type rPreTypedefDefn {Ast::TypedefDefn*}
rPreTypedefDefn(L) ::= TYPEDEF ID(name) rQualifiedTypeSpec(Q) rDefinitionType(D). {L = z::ref(pctx).aTypedefDefn(name, D, z::ref(Q));}

//-------------------------------------------------
// template declarations
%type rTemplateDecl {Ast::TemplateDecl*}
rTemplateDecl(L) ::= rPreTemplateDecl(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreTemplateDecl {Ast::TemplateDecl*}
rPreTemplateDecl(L) ::= TEMPLATE LT rTemplatePartList(list) GT ID(name) rDefinitionType(D). {L = z::ref(pctx).aTemplateDecl(name, D, z::ref(list));}

//-------------------------------------------------
%type rTemplatePartList {Ast::TemplatePartList*}
rTemplatePartList(L) ::= rTemplatePartList(R) COMMA ID(name). {L = z::ref(pctx).aTemplatePartList(z::ref(R), name);}
rTemplatePartList(L) ::=                            ID(name). {L = z::ref(pctx).aTemplatePartList(name);}

//-------------------------------------------------
// enum declaration
%type rEnumDecl {Ast::EnumDefn*}
rEnumDecl(L) ::= rPreEnumDecl(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreEnumDecl {Ast::EnumDefn*}
rPreEnumDecl(L) ::= ENUM ID(name) rDefinitionType(D). {L = z::ref(pctx).aEnumDefn(name, D);}

//-------------------------------------------------
// enum definition
%type rEnumDefn {Ast::EnumDefn*}
rEnumDefn(L) ::= rPreEnumDefn(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreEnumDefn {Ast::EnumDefn*}
rPreEnumDefn(L) ::= ENUM ID(name) rDefinitionType(D) LCURLY rEnumMemberDefnList(list) RCURLY. {L = z::ref(pctx).aEnumDefn(name, D, z::ref(list));}

//-------------------------------------------------
%type rEnumMemberDefnList {Ast::Scope*}
rEnumMemberDefnList(L) ::= rEnumMemberDefnList(list) rEnumMemberDefn(enumMemberDef). {L = z::ref(pctx).aEnumMemberDefnList(z::ref(list), z::ref(enumMemberDef));}
rEnumMemberDefnList(L) ::=                           rEnumMemberDefn(enumMemberDef). {L = z::ref(pctx).aEnumMemberDefnList(z::ref(enumMemberDef));}

//-------------------------------------------------
%type rEnumMemberDefn {Ast::VariableDefn*}
rEnumMemberDefn(L) ::= ID(name)                      SEMI. {L = z::ref(pctx).aEnumMemberDefn(name);}
rEnumMemberDefn(L) ::= ID(name) ASSIGNEQUAL rExpr(I) SEMI. {L = z::ref(pctx).aEnumMemberDefn(name, z::ref(I));}

//-------------------------------------------------
// struct declarations
%type rStructDecl {Ast::StructDecl*}
rStructDecl(L) ::= rPreStructDecl(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreStructDecl {Ast::StructDecl*}
rPreStructDecl(L) ::= STRUCT rStructId(name) rDefinitionType(D). {L = z::ref(pctx).aStructDecl(name, D);}

//-------------------------------------------------
// root struct definitions
%type rRootStructDefn {Ast::RootStructDefn*}
rRootStructDefn(L) ::= rPreRootStructDefn(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreRootStructDefn {Ast::RootStructDefn*}
rPreRootStructDefn(L) ::= rEnterRootStructDefn(S) rStructMemberDefnBlock. {L = z::ref(pctx).aLeaveRootStructDefn(z::ref(S));}

//-------------------------------------------------
// child struct definitions
%type rChildStructDefn {Ast::ChildStructDefn*}
rChildStructDefn(L) ::= rPreChildStructDefn(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreChildStructDefn {Ast::ChildStructDefn*}
rPreChildStructDefn(L) ::= rEnterChildStructDefn(S) rStructMemberDefnBlock. {L = z::ref(pctx).aLeaveChildStructDefn(z::ref(S));}

//-------------------------------------------------
%type rEnterRootStructDefn {Ast::RootStructDefn*}
rEnterRootStructDefn(L) ::= STRUCT rStructId(name) rExDefinitionType(D). {L = z::ref(pctx).aEnterRootStructDefn(name, D);}

//-------------------------------------------------
%type rEnterChildStructDefn {Ast::ChildStructDefn*}
rEnterChildStructDefn(L) ::= STRUCT rStructId(name) COLON rStructTypeSpec(B) rExDefinitionType(D). {L = z::ref(pctx).aEnterChildStructDefn(name, z::ref(B), D);}

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
rStructMemberDefn ::= rVariableDefn(R) SEMI.  {z::ref(pctx).aStructMemberVariableDefn(z::ref(R));}
rStructMemberDefn ::= rBasicTypeSpecDef(R).   {z::ref(pctx).aStructMemberTypeDefn(z::ref(R));}
rStructMemberDefn ::= rStructPropertyDecl(R). {z::ref(pctx).aStructMemberPropertyDefn(z::ref(R));}

//-------------------------------------------------
// struct index declarations
%type rStructPropertyDecl {Ast::PropertyDecl*}
rStructPropertyDecl(L) ::= PROPERTY(B) rQualifiedTypeSpec(T) ID(N) rDefinitionType(D) GET SET SEMI. {L = z::ref(pctx).aStructPropertyDeclRW(B, z::ref(T), N, D);}
rStructPropertyDecl(L) ::= PROPERTY(B) rQualifiedTypeSpec(T) ID(N) rDefinitionType(D) GET     SEMI. {L = z::ref(pctx).aStructPropertyDeclRO(B, z::ref(T), N, D);}

//-------------------------------------------------
// routine declarations
%type rRoutineDecl {Ast::RoutineDecl*}
rRoutineDecl(L) ::= rPreRoutineDecl(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreRoutineDecl {Ast::RoutineDecl*}
rPreRoutineDecl(L) ::= ROUTINE rQualifiedTypeSpec(out) rRoutineId(name) rInParamsList(in) rDefinitionType(D). {L = z::ref(pctx).aRoutineDecl(z::ref(out), name, z::ref(in), D);}
rPreRoutineDecl(L) ::= ROUTINE rQualifiedTypeSpec(out) rRoutineId(name) LBRACKET ELIPSIS RBRACKET NATIVE. {L = z::ref(pctx).aVarArgRoutineDecl(z::ref(out), name, Ast::DefinitionType::Native);}

//-------------------------------------------------
// routine definition
%type rRoutineDefn {Ast::RoutineDefn*}
rRoutineDefn(L) ::= rPreRoutineDefn(R). {L = R;}

//-------------------------------------------------
%type rPreRoutineDefn {Ast::RoutineDefn*}
rPreRoutineDefn(L) ::= rEnterRoutineDefn(routineDefn) rCompoundStatement(block). {L = z::ref(pctx).aRoutineDefn(z::ref(routineDefn), z::ref(block));}

//-------------------------------------------------
%type rEnterRoutineDefn {Ast::RoutineDefn*}
rEnterRoutineDefn(L) ::= ROUTINE rQualifiedTypeSpec(out) rRoutineId(name) rInParamsList(in) rDefinitionType(D). {L = z::ref(pctx).aEnterRoutineDefn(z::ref(out), name, z::ref(in), D);}
rRoutineId(L) ::= ID(R). {L = R;}
rRoutineId(L) ::= ROUTINE_TYPE(R). {L = R;}

//-------------------------------------------------
// function definition
%type rFunctionDecl {Ast::FunctionDecl*}
rFunctionDecl(L) ::= rFunctionSig(functionSig) rExDefinitionType(defType) SEMI. {L = z::ref(pctx).aFunctionDecl(z::ref(functionSig), defType);}

//-------------------------------------------------
// root function declarations
%type rRootFunctionDefn {Ast::RootFunctionDefn*}
rRootFunctionDefn(L) ::= rEnterRootFunctionDefn(functionDefn) rCompoundStatement(block). {L = z::ref(pctx).aRootFunctionDefn(z::ref(functionDefn), z::ref(block));}

//-------------------------------------------------
%type rEnterRootFunctionDefn {Ast::RootFunctionDefn*}
rEnterRootFunctionDefn(L) ::= rFunctionSig(functionSig) rExDefinitionType(defType). {L = z::ref(pctx).aEnterRootFunctionDefn(z::ref(functionSig), defType);}

//-------------------------------------------------
// child function declaration
%type rChildFunctionDefn {Ast::ChildFunctionDefn*}
rChildFunctionDefn(L) ::= rEnterChildFunctionDefn(functionImpl) rCompoundStatement(block). {L = z::ref(pctx).aChildFunctionDefn(z::ref(functionImpl), z::ref(block));}

//-------------------------------------------------
%type rEnterChildFunctionDefn {Ast::ChildFunctionDefn*}
rEnterChildFunctionDefn(L) ::= FUNCTION ID(name) COLON rFunctionTypeSpec(base) rExDefinitionType(defType). {L = z::ref(pctx).aEnterChildFunctionDefn(z::ref(base), name, defType);}

//-------------------------------------------------
// event declarations
%type rEventDecl {Ast::EventDecl*}
rEventDecl(L) ::= EVENT(B) LBRACKET rVariableDefn(in) RBRACKET rDefinitionType(ED) LINK rFunctionSig(functionSig) rAbstractDefinitionType(HD) SEMI. {L = z::ref(pctx).aEventDecl(B, z::ref(in), ED, z::ref(functionSig), HD);}

//-------------------------------------------------
// function signature.
%type rFunctionSig {Ast::FunctionSig*}
rFunctionSig(T) ::= FUNCTION rParamsList(out)        ID(name) rInParamsList(in). {T = z::ref(pctx).aFunctionSig(z::ref(out), name, z::ref(in));}
rFunctionSig(T) ::= FUNCTION rQualifiedTypeSpec(out) ID(name) rInParamsList(in). {T = z::ref(pctx).aFunctionSig(z::ref(out), name, z::ref(in));}

//-------------------------------------------------
// in parameter list
%type rInParamsList {Ast::Scope*}
rInParamsList(L) ::= rParamsList(scope). {L = z::ref(pctx).aInParamsList(z::ref(scope));}

//-------------------------------------------------
// parameter lists
%type rParamsList {Ast::Scope*}
rParamsList(L) ::= LBRACKET rParam(R)                    RBRACKET. {L = z::ref(pctx).aParamsList(z::ref(R));}
//rParamsList(L) ::= LBRACKET rParam(R) COMMA rPosParam(P) RBRACKET. {L = z::ref(pctx).aParamsList(z::ref(R), z::ref(P));}

//-------------------------------------------------
// variable lists
%type rParam {Ast::Scope*}
rParam(L) ::= rParam(list) COMMA rVariableDefn(variableDef). {L = z::ref(pctx).aParam(z::ref(list), z::ref(variableDef));}
rParam(L) ::=                    rVariableDefn(variableDef). {L = z::ref(pctx).aParam(z::ref(variableDef));}
rParam(L) ::= .                                              {L = z::ref(pctx).aParam();}

//-------------------------------------------------
// positional parameter list
//%type rPosParam {Ast::Scope*}
//rPosParam(L) ::= rPosParam(list) COMMA rPosVariableDefn(variableDef). {L = z::ref(pctx).aParam(z::ref(list), z::ref(variableDef));}
//rPosParam(L) ::=                       rPosVariableDefn(variableDef). {L = z::ref(pctx).aParam(z::ref(variableDef));}

//-------------------------------------------------
// positional variable def
//%type rPosVariableDefn {const Ast::VariableDefn*}
//rPosVariableDefn(L) ::= rAutoQualifiedVariableDefn       ID(name) COLON rExpr(initExpr). {L = z::ref(pctx).aVariableDefn(name, z::ref(initExpr));}
//rPosVariableDefn(L) ::= rQualifiedVariableDefn(qTypeRef) ID(name) COLON rExpr(initExpr). {L = z::ref(pctx).aVariableDefn(z::ref(qTypeRef), name, z::ref(initExpr));}

//-------------------------------------------------
// variable def
%type rVariableDefn {const Ast::VariableDefn*}
rVariableDefn(L) ::= rAutoQualifiedVariableDefn       ID(name) ASSIGNEQUAL rExpr(initExpr). {L = z::ref(pctx).aVariableDefn(name, z::ref(initExpr));}
rVariableDefn(L) ::= rQualifiedVariableDefn(qTypeRef) ID(name).                             {L = z::ref(pctx).aVariableDefn(z::ref(qTypeRef), name);}
rVariableDefn(L) ::= rQualifiedVariableDefn(qTypeRef) ID(name) ASSIGNEQUAL rExpr(initExpr). {L = z::ref(pctx).aVariableDefn(z::ref(qTypeRef), name, z::ref(initExpr));}

//-------------------------------------------------
// qualified variable def
%type rQualifiedVariableDefn {const Ast::QualifiedTypeSpec*}
rQualifiedVariableDefn(L) ::= rQualifiedTypeSpec(R). {L = z::ref(pctx).aQualifiedVariableDefn(z::ref(R));}

//-------------------------------------------------
// auto qualified variable def
rAutoQualifiedVariableDefn ::= AUTO. {z::ref(pctx).aAutoQualifiedVariableDefn();}

//-------------------------------------------------
// qualified types
%type rQualifiedTypeSpec {const Ast::QualifiedTypeSpec*}
rQualifiedTypeSpec(L) ::=          rTypeSpec(typeSpec).               {L = z::ref(pctx).aQualifiedTypeSpec(false, z::ref(typeSpec), false);}
rQualifiedTypeSpec(L) ::=          rTypeSpec(typeSpec) BITWISEAND(B). {L = z::ref(pctx).aQualifiedTypeSpec(B, false, z::ref(typeSpec), true );}
rQualifiedTypeSpec(L) ::= CONST(B) rTypeSpec(typeSpec).               {L = z::ref(pctx).aQualifiedTypeSpec(B, true,  z::ref(typeSpec), false);}
rQualifiedTypeSpec(L) ::= CONST(B) rTypeSpec(typeSpec) BITWISEAND.    {L = z::ref(pctx).aQualifiedTypeSpec(B, true,  z::ref(typeSpec), true );}

//-------------------------------------------------
// "public" type references, can be invoked from other rules
%type rTypeSpec {const Ast::TypeSpec*}
rTypeSpec(L) ::= rPreTypeSpec(R). {L = z::ref(pctx).aTypeSpec(z::ref(R));}
rTypeSpec(L) ::= rTemplateDefnTypeSpec(R). {L = R;}

//-------------------------------------------------
%type rTemplateDefnTypeSpec {const Ast::TemplateDefn*}
rTemplateDefnTypeSpec(L) ::= rTemplateTypeSpec(R) TLT rTemplateTypePartList(P) GT. {L = z::ref(pctx).aTemplateDefnTypeSpec(z::ref(R), z::ref(P));}

//-------------------------------------------------
%type rTemplateTypePartList {Ast::TemplateTypePartList*}
rTemplateTypePartList(L) ::= rTemplateTypePartList(R) COMMA rQualifiedTypeSpec(P). {L = z::ref(pctx).aTemplateTypePartList(z::ref(R), z::ref(P));}
rTemplateTypePartList(L) ::=                                rQualifiedTypeSpec(P). {L = z::ref(pctx).aTemplateTypePartList(z::ref(P));}

//-------------------------------------------------
%type rTemplateTypeSpec {const Ast::TemplateDecl*}
rTemplateTypeSpec(L) ::= rPreTemplateTypeSpec(R). {L = z::ref(pctx).aTemplateTypeSpec(z::ref(R));}

//-------------------------------------------------
%type rStructTypeSpec {const Ast::StructDefn*}
rStructTypeSpec(L) ::= rPreStructTypeSpec(R). {L = z::ref(pctx).aStructTypeSpec(z::ref(R));}

//-------------------------------------------------
%type rRoutineTypeSpec {const Ast::Routine*}
rRoutineTypeSpec(L) ::= rPreRoutineTypeSpec(R). {L = z::ref(pctx).aRoutineTypeSpec(z::ref(R));}

//-------------------------------------------------
%type rFunctionTypeSpec {const Ast::Function*}
rFunctionTypeSpec(L) ::= rPreFunctionTypeSpec(R). {L = z::ref(pctx).aFunctionTypeSpec(z::ref(R));}

//-------------------------------------------------
%type rEventTypeSpec {const Ast::EventDecl*}
rEventTypeSpec(L) ::= rPreEventTypeSpec(R). {L = z::ref(pctx).aEventTypeSpec(z::ref(R));}

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
rPreTemplateTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE TEMPLATE_TYPE(name). {L = z::ref(pctx).aTemplateTypeSpec(z::ref(parent), name);}
rPreTemplateTypeSpec(L) ::=                            TEMPLATE_TYPE(name). {L = z::ref(pctx).aTemplateTypeSpec(name);}

//-------------------------------------------------
%type rPreStructTypeSpec {const Ast::StructDefn*}
rPreStructTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE STRUCT_TYPE(name). {L = z::ref(pctx).aStructTypeSpec(z::ref(parent), name);}
rPreStructTypeSpec(L) ::=                            STRUCT_TYPE(name). {L = z::ref(pctx).aStructTypeSpec(name);}

//-------------------------------------------------
%type rPreRoutineTypeSpec {const Ast::Routine*}
rPreRoutineTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE ROUTINE_TYPE(name). {L = z::ref(pctx).aRoutineTypeSpec(z::ref(parent), name);}
rPreRoutineTypeSpec(L) ::=                            ROUTINE_TYPE(name). {L = z::ref(pctx).aRoutineTypeSpec(name);}

//-------------------------------------------------
%type rPreFunctionTypeSpec {const Ast::Function*}
rPreFunctionTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE FUNCTION_TYPE(name). {L = z::ref(pctx).aFunctionTypeSpec(z::ref(parent), name);}
rPreFunctionTypeSpec(L) ::=                            FUNCTION_TYPE(name). {L = z::ref(pctx).aFunctionTypeSpec(name);}

//-------------------------------------------------
%type rPreEventTypeSpec {const Ast::EventDecl*}
rPreEventTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE EVENT_TYPE(name). {L = z::ref(pctx).aEventTypeSpec(z::ref(parent), name);}
rPreEventTypeSpec(L) ::=                            EVENT_TYPE(name). {L = z::ref(pctx).aEventTypeSpec(name);}

//-------------------------------------------------
%type rPreOtherTypeSpec {const Ast::TypeSpec*}
rPreOtherTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE OTHER_TYPE(name). {L = z::ref(pctx).aOtherTypeSpec(z::ref(parent), name);}
rPreOtherTypeSpec(L) ::=                            OTHER_TYPE(name). {L = z::ref(pctx).aOtherTypeSpec(name);}

//-------------------------------------------------
// statements
%type rInnerStatement {Ast::Statement*}
rInnerStatement(L) ::= rUserDefinedTypeSpecStatement(R). {L = R;}
rInnerStatement(L) ::= rEmptyStatement(R).               {L = R;}
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
rUserDefinedTypeSpecStatement(L) ::= rTypeSpecDef(typeSpec). {L = z::ref(pctx).aUserDefinedTypeSpecStatement(z::ref(typeSpec));}

//-------------------------------------------------
%type rEmptyStatement {Ast::EmptyStatement*}
rEmptyStatement(L) ::= SEMI(B). {L = z::ref(pctx).aEmptyStatement(B);}

//-------------------------------------------------
%type rAutoStatement {Ast::AutoStatement*}
rAutoStatement(L) ::= rVariableDefn(defn) SEMI. {L = z::ref(pctx).aAutoStatement(z::ref(defn));}

//-------------------------------------------------
%type rExprStatement {Ast::ExprStatement*}
rExprStatement(L) ::= rExpr(expr) SEMI. {L = z::ref(pctx).aExprStatement(z::ref(expr));}

//-------------------------------------------------
%type rPrintStatement {Ast::PrintStatement*}
rPrintStatement(L) ::= PRINT(B) rExpr(expr) SEMI. {L = z::ref(pctx).aPrintStatement(B, z::ref(expr));}

//-------------------------------------------------
%type rIfStatement {Ast::IfStatement*}
rIfStatement(L) ::= IF(B) LBRACKET rExpr(expr) RBRACKET rCompoundStatement(tblock). {L = z::ref(pctx).aIfStatement(B, z::ref(expr), z::ref(tblock));}

//-------------------------------------------------
%type rIfElseStatement {Ast::IfElseStatement*}
rIfElseStatement(L) ::= IF(B) LBRACKET rExpr(expr) RBRACKET rCompoundStatement(tblock) ELSE rCompoundStatement(fblock). {L = z::ref(pctx).aIfElseStatement(B, z::ref(expr), z::ref(tblock), z::ref(fblock));}

//-------------------------------------------------
%type rWhileStatement {Ast::WhileStatement*}
rWhileStatement(L) ::= WHILE(B) LBRACKET rExpr(expr) RBRACKET rCompoundStatement(block). {L = z::ref(pctx).aWhileStatement(B, z::ref(expr), z::ref(block));}

//-------------------------------------------------
%type rDoWhileStatement {Ast::DoWhileStatement*}
rDoWhileStatement(L) ::= DO(B) rCompoundStatement(block) WHILE LBRACKET rExpr(expr) RBRACKET SEMI. {L = z::ref(pctx).aDoWhileStatement(B, z::ref(expr), z::ref(block));}

//-------------------------------------------------
%type rForStatement {Ast::ForStatement*}
rForStatement(L) ::= FOR(B) LBRACKET rExpr(init) SEMI rExpr(expr) SEMI rExpr(incr) RBRACKET rCompoundStatement(block). {L = z::ref(pctx).aForStatement(B, z::ref(init), z::ref(expr), z::ref(incr), z::ref(block));}
rForStatement(L) ::= FOR(B) LBRACKET rEnterForInit(init) SEMI rExpr(expr) SEMI rExpr(incr) RBRACKET rCompoundStatement(block). {L = z::ref(pctx).aForStatement(B, z::ref(init), z::ref(expr), z::ref(incr), z::ref(block));}

%type rEnterForInit {const Ast::VariableDefn*}
rEnterForInit(L) ::= rVariableDefn(init). {L = z::ref(pctx).aEnterForInit(z::ref(init));}

//-------------------------------------------------
%type rForeachStatement {Ast::ForeachStatement*}
rForeachStatement(L) ::= FOREACH LBRACKET rEnterForeachInit(vdef) RBRACKET rCompoundStatement(block). {L = z::ref(pctx).aForeachStatement(z::ref(vdef), z::ref(block));}

%type rEnterForeachInit {Ast::ForeachStatement*}
rEnterForeachInit(L) ::= ID(I) IN rExpr(list). {L = z::ref(pctx).aEnterForeachInit(I, z::ref(list));}
rEnterForeachInit(L) ::= ID(K) COMMA ID(V) IN rExpr(list). {L = z::ref(pctx).aEnterForeachInit(K, V, z::ref(list));}

//-------------------------------------------------
%type rSwitchStatement {Ast::SwitchStatement*}
rSwitchStatement(L) ::= SWITCH(B) LBRACKET rExpr(expr) RBRACKET LCURLY rCaseList(list) RCURLY. {L = z::ref(pctx).aSwitchStatement(B, z::ref(expr), z::ref(list));}
rSwitchStatement(L) ::= SWITCH(B)                               LCURLY rCaseList(list) RCURLY. {L = z::ref(pctx).aSwitchStatement(B, z::ref(list));}

//-------------------------------------------------
%type rCaseList {Ast::CompoundStatement*}
rCaseList(L) ::= rCaseList(R) rCaseStatement(S). {L = z::ref(pctx).aCaseList(z::ref(R), z::ref(S));}
rCaseList(L) ::=              rCaseStatement(S). {L = z::ref(pctx).aCaseList(z::ref(S));}

//-------------------------------------------------
%type rCaseStatement {Ast::CaseStatement*}
rCaseStatement(L) ::= CASE(B) rExpr(expr) COLON rCompoundStatement(block). {L = z::ref(pctx).aCaseStatement(B, z::ref(expr), z::ref(block));}
rCaseStatement(L) ::= DEFAULT(B)          COLON rCompoundStatement(block). {L = z::ref(pctx).aCaseStatement(B, z::ref(block));}

//-------------------------------------------------
%type rBreakStatement {Ast::BreakStatement*}
rBreakStatement(L) ::= BREAK(B) SEMI. {L = z::ref(pctx).aBreakStatement(B);}

//-------------------------------------------------
%type rContinueStatement {Ast::ContinueStatement*}
rContinueStatement(L) ::= CONTINUE(B) SEMI. {L = z::ref(pctx).aContinueStatement(B);}

//-------------------------------------------------
%type rAddEventHandlerStatement {Ast::AddEventHandlerStatement*}
rAddEventHandlerStatement(L) ::= rEnterAddEventHandler(E) LBRACKET(B) rExpr(X) RBRACKET LINK rAnonymousFunctionExpr(F) SEMI. {L = z::ref(pctx).aAddEventHandlerStatement(B, z::ref(E), z::ref(X), z::ref(F));}

//-------------------------------------------------
%type rEnterAddEventHandler {const Ast::EventDecl*}
rEnterAddEventHandler(L) ::= rEventTypeSpec(R). {L = z::ref(pctx).aEnterAddEventHandler(z::ref(R));}

//-------------------------------------------------
%type rRoutineReturnStatement {Ast::RoutineReturnStatement*}
rRoutineReturnStatement(L) ::= RRETURN(B)          SEMI. {L = z::ref(pctx).aRoutineReturnStatement(B);}
rRoutineReturnStatement(L) ::= RRETURN(B) rExpr(S) SEMI. {L = z::ref(pctx).aRoutineReturnStatement(B, z::ref(S));}

//-------------------------------------------------
%type rFunctionReturnStatement {Ast::FunctionReturnStatement*}
rFunctionReturnStatement(L) ::= FRETURN(B) rExprsList(S) SEMI. {L = z::ref(pctx).aFunctionReturnStatement(B, z::ref(S));}

//-------------------------------------------------
// simple list of statements
%type rCompoundStatement {Ast::CompoundStatement*}
rCompoundStatement(L)   ::= rEnterCompoundStatement rStatementList(R) rLeaveCompoundStatement. {L = R;}
rEnterCompoundStatement ::= LCURLY(B). {z::ref(pctx).aEnterCompoundStatement(B);}
rLeaveCompoundStatement ::= RCURLY.    {z::ref(pctx).aLeaveCompoundStatement();}

%type rStatementList {Ast::CompoundStatement*}
rStatementList(L) ::= rStatementList(list) rInnerStatement(statement). {L = z::ref(pctx).aStatementList(z::ref(list), z::ref(statement));}
rStatementList(L) ::= .                                                {L = z::ref(pctx).aStatementList();}

//-------------------------------------------------
// expression list in brackets
%type rExprsList {Ast::ExprList*}
rExprsList(L) ::= LBRACKET rExprList(R) RBRACKET. {L = R;}

//-------------------------------------------------
// comma-separated list of expressions
%type rExprList {Ast::ExprList*}
rExprList(R) ::= rExprList(L) COMMA rExpr(E). {R = z::ref(pctx).aExprList(z::ref(L), z::ref(E));}
rExprList(R) ::=                    rExpr(E). {R = z::ref(pctx).aExprList(z::ref(E));}
rExprList(R) ::= .                            {R = z::ref(pctx).aExprList();}

//-------------------------------------------------
// expressions
%type rExpr {const Ast::Expr*}
rExpr(L) ::= rTernaryExpr(R).           {L = R;}
rExpr(L) ::= rBooleanExpr(R).           {L = R;}
rExpr(L) ::= rBinaryExpr(R).            {L = R;}
rExpr(L) ::= rPostfixExpr(R).           {L = R;}
rExpr(L) ::= rPrefixExpr(R).            {L = R;}
rExpr(L) ::= rListExpr(R).              {L = R;}
rExpr(L) ::= rDictExpr(R).              {L = R;}
rExpr(L) ::= rFormatExpr(R).            {L = R;}
rExpr(L) ::= rCallExpr(R).              {L = R;}
rExpr(L) ::= rRunExpr(R).               {L = R;}
rExpr(L) ::= rOrderedExpr(R).           {L = R;}
rExpr(L) ::= rIndexExpr(R).             {L = R;}
rExpr(L) ::= rSpliceExpr(R).            {L = R;}
rExpr(L) ::= rVariableRefExpr(R).       {L = R;}
rExpr(L) ::= rMemberVariableExpr(R).    {L = R;}
rExpr(L) ::= rTypeSpecMemberExpr(R).    {L = R;}
rExpr(L) ::= rStructInstanceExpr(R).    {L = R;}
rExpr(L) ::= rAutoStructInstanceExpr(R).{L = R;}
rExpr(L) ::= rFunctionInstanceExpr(R).  {L = R;}
rExpr(L) ::= rAnonymousFunctionExpr(R). {L = R;}
rExpr(L) ::= rConstantExpr(R).          {L = R;}

//-------------------------------------------------
// It could be possible to implement creating local variables inline within expressions.
// Not sure how to implement it in the generated code. Not a priority, so on hold for now.
//rExpr(E) ::= ID(L) DEFINEEQUAL       rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryExpr(z::ref(L), z::string("="), z::ref(R)));}

//-------------------------------------------------
// ternary operators
%type rTernaryExpr {const Ast::TernaryOpExpr*}
rTernaryExpr(E) ::= rExpr(L) QUESTION(O1) rExpr(T) COLON(O2) rExpr(F). {E = z::ref(pctx).aConditionalExpr(O1, O2, z::ref(L), z::ref(T), z::ref(F));}

//-------------------------------------------------
// boolean operators
%type rBooleanExpr {const Ast::Expr*}
rBooleanExpr(E) ::= rExpr(L) AND(O)             rExpr(R). {E = z::ptr(z::ref(pctx).aBooleanAndExpr(O, z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) OR(O)              rExpr(R). {E = z::ptr(z::ref(pctx).aBooleanOrExpr(O, z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) EQUAL(O)           rExpr(R). {E = z::ptr(z::ref(pctx).aBooleanEqualExpr(O, z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) NOTEQUAL(O)        rExpr(R). {E = z::ptr(z::ref(pctx).aBooleanNotEqualExpr(O, z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) LT(O)              rExpr(R). {E = z::ptr(z::ref(pctx).aBooleanLessThanExpr(O, z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) GT(O)              rExpr(R). {E = z::ptr(z::ref(pctx).aBooleanGreaterThanExpr(O, z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) LTE(O)             rExpr(R). {E = z::ptr(z::ref(pctx).aBooleanLessThanOrEqualExpr(O, z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) GTE(O)             rExpr(R). {E = z::ptr(z::ref(pctx).aBooleanGreaterThanOrEqualExpr(O, z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) HAS(O)             rExpr(R). {E = z::ptr(z::ref(pctx).aBooleanHasExpr(O, z::ref(L), z::ref(R)));}

//-------------------------------------------------
// binary operators
%type rBinaryExpr {const Ast::Expr*}
rBinaryExpr(E) ::= rExpr(L) ASSIGNEQUAL(O)     rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryAssignEqualExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) PLUSEQUAL(O)       rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryPlusEqualExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) MINUSEQUAL(O)      rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryMinusEqualExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) TIMESEQUAL(O)      rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryTimesEqualExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) DIVIDEEQUAL(O)     rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryDivideEqualExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) MODEQUAL(O)        rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryModEqualExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEANDEQUAL(O) rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryBitwiseAndEqualExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEOREQUAL(O)  rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryBitwiseOrEqualExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEXOREQUAL(O) rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryBitwiseXorEqualExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) SHIFTLEFTEQUAL(O)  rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryShiftLeftEqualExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) SHIFTRIGHTEQUAL(O) rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryShiftRightEqualExpr(O, z::ref(L), z::ref(R)));}

rBinaryExpr(E) ::= rExpr(L) PLUS(O)            rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryPlusExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) MINUS(O)           rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryMinusExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) STAR(O)            rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryTimesExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) DIVIDE(O)          rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryDivideExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) MOD(O)             rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryModExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEAND(O)      rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryBitwiseAndExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEOR(O)       rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryBitwiseOrExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEXOR(O)      rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryBitwiseXorExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) SHL(O)             rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryShiftLeftExpr(O, z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) SHR(O)             rExpr(R). {E = z::ptr(z::ref(pctx).aBinaryShiftRightExpr(O, z::ref(L), z::ref(R)));}

//-------------------------------------------------
// postfix operators
%type rPostfixExpr {const Ast::Expr*}
rPostfixExpr(E) ::= rExpr(L) INC(O). {E = z::ptr(z::ref(pctx).aPostfixIncExpr(O, z::ref(L)));}
rPostfixExpr(E) ::= rExpr(L) DEC(O). {E = z::ptr(z::ref(pctx).aPostfixDecExpr(O, z::ref(L)));}

//-------------------------------------------------
// prefix operators
%type rPrefixExpr {const Ast::Expr*}
rPrefixExpr(E) ::= NOT(O)        rExpr(R). {E = z::ptr(z::ref(pctx).aPrefixNotExpr(O, z::ref(R)));}
rPrefixExpr(E) ::= PLUS(O)       rExpr(R). {E = z::ptr(z::ref(pctx).aPrefixPlusExpr(O, z::ref(R)));}
rPrefixExpr(E) ::= MINUS(O)      rExpr(R). {E = z::ptr(z::ref(pctx).aPrefixMinusExpr(O, z::ref(R)));}
rPrefixExpr(E) ::= INC(O)        rExpr(R). {E = z::ptr(z::ref(pctx).aPrefixIncExpr(O, z::ref(R)));}
rPrefixExpr(E) ::= DEC(O)        rExpr(R). {E = z::ptr(z::ref(pctx).aPrefixDecExpr(O, z::ref(R)));}
rPrefixExpr(E) ::= BITWISENOT(O) rExpr(R). {E = z::ptr(z::ref(pctx).aPrefixBitwiseNotExpr(O, z::ref(R)));}

//-------------------------------------------------
// string formatter
%type rFormatExpr {Ast::FormatExpr*}
rFormatExpr(L) ::= rExpr(A) AMP(B) rTreeExpr(T). {L = z::ref(pctx).aFormatExpr(B, z::ref(A), z::ref(T));}

//-------------------------------------------------
// list expression
%type rListExpr {Ast::ListExpr*}
rListExpr(L) ::= rListList(R) RSQUARE(B). {L = z::ref(pctx).aListExpr(B, z::ref(R));}

%type rListList {Ast::ListList*}
rListList(L) ::= rListsList(R)      . {L = R;}
rListList(L) ::= rListsList(R) COMMA. {L = R;}

%type rListsList {Ast::ListList*}
rListsList(L)  ::= rListsList(R) COMMA(B) rListItem(I).  {L = z::ref(pctx).aListList(B, z::ref(R), z::ref(I));}
rListsList(L)  ::=      rEnterList(B) rListItem(I).      {L = z::ref(pctx).aListList(B, z::ref(I));}
rListsList(L)  ::=      rEnterList(B) rQualifiedTypeSpec(Q). {L = z::ref(pctx).aListList(B, z::ref(Q));}
rListsList(L)  ::=      rEnterList(B)                      . {L = z::ref(pctx).aListList(B);}

%type rListItem {Ast::ListItem*}
rListItem(L)  ::= rExpr(E). {L = z::ref(pctx).aListItem(z::ref(E));}

//-------------------------------------------------
// dict (strict type-checking for key and value)
%type rDictExpr {Ast::DictExpr*}
rDictExpr(L) ::= rDictList(R) RSQUARE(B). {L = z::ref(pctx).aDictExpr(B, z::ref(R));}

%type rDictList {Ast::DictList*}
rDictList(L) ::= rDictsList(R)       . {L = R;}
rDictList(L) ::= rDictsList(R) COMMA . {L = R;}

%type rDictsList {Ast::DictList*}
rDictsList(L)  ::= rDictsList(R) COMMA(B) rDictItem(I). {L = z::ref(pctx).aDictList(B, z::ref(R), z::ref(I));}
rDictsList(L)  ::=          rEnterList(B) rDictItem(I). {L = z::ref(pctx).aDictList(B, z::ref(I));}

// first item in list can be a type specifier
rDictsList(L) ::= rEnterList(B) rQualifiedTypeSpec(K) COLON rQualifiedTypeSpec(V). {L = z::ref(pctx).aDictList(B, z::ref(K), z::ref(V));}

%type rDictItem {Ast::DictItem*}
rDictItem(L)  ::= rDictKey(K) COLON(B) rExpr(E). {L = z::ref(pctx).aDictItem(B, z::ref(K), z::ref(E));}

%type rDictKey {const Ast::Expr*}
rDictKey(L) ::= rExpr(R). {L = z::ref(pctx).aDictKey(z::ref(R));}

//-------------------------------------------------
rEnterList(L)  ::= LSQUARE(R). {L = R; z::ref(pctx).aEnterList(R); }

//-------------------------------------------------
// tree (no type checking for key or value)
%type rTreeExpr {Ast::DictExpr*}
rTreeExpr(L) ::= rTreeList(R) RCURLY(B). {L = z::ref(pctx).aDictExpr(B, z::ref(R));}

%type rTreeList {Ast::DictList*}
rTreeList(L) ::= rTreesList(R)       . {L = R;}
rTreeList(L) ::= rTreesList(R) COMMA . {L = R;}

%type rTreesList {Ast::DictList*}
rTreesList(L)  ::= rTreesList(R) COMMA(B) rTreeItem(I).     {L = z::ref(pctx).aDictList(B, z::ref(R), z::ref(I));}
rTreesList(L)  ::=              rEnterTree(B) rTreeItem(I). {L = z::ref(pctx).aDictList(B, z::ref(I));}

%type rTreeItem {Ast::DictItem*}
rTreeItem(L)  ::= rDictKey(K) COLON(B) rExpr(E). {L = z::ref(pctx).aDictItem(B, z::ref(K), z::ref(E));}

//-------------------------------------------------
rEnterTree(L)  ::= LCURLY(R). {L = R; z::ref(pctx).aEnterList(R); }

//-------------------------------------------------
// ordered expression
%type rOrderedExpr {Ast::OrderedExpr*}
rOrderedExpr(L) ::= LBRACKET(B) rExpr(innerExpr) RBRACKET. {L = z::ref(pctx).aOrderedExpr(B, z::ref(innerExpr));}

//-------------------------------------------------
// index expression
%type rIndexExpr {Ast::IndexExpr*}
rIndexExpr(L) ::= rExpr(E) LSQUARE(B) rExpr(innerExpr) RSQUARE. {L = z::ref(pctx).aIndexExpr(B, z::ref(E), z::ref(innerExpr));}
rIndexExpr(L) ::= rExpr(E) AMP(B) rKeyConstantExpr(innerExpr). {L = z::ref(pctx).aIndexExpr(B, z::ref(E), z::ref(innerExpr));}

//-------------------------------------------------
// splice expression
%type rSpliceExpr {Ast::SpliceExpr*}
rSpliceExpr(L) ::= rExpr(E) LSQUARE(B) rExpr(fromExpr) COLON rExpr(toExpr) RSQUARE. {L = z::ref(pctx).aSpliceExpr(B, z::ref(E), z::ref(fromExpr), z::ref(toExpr));}

//-------------------------------------------------
// type expression
rExpr(L) ::= TYPEOF(B) LBRACKET rQualifiedTypeSpec(T) RBRACKET. {L = z::ref(pctx).aTypeofTypeExpr(B, z::ref(T));}
rExpr(L) ::= TYPEOF(B) LBRACKET rExpr(E)              RBRACKET. {L = z::ref(pctx).aTypeofExprExpr(B, z::ref(E));}

//-------------------------------------------------
// type-cast expression
rExpr(L) ::= LT(B) rQualifiedTypeSpec(T) GT rExpr(E). {L = z::ref(pctx).aTypecastExpr(B, z::ref(T), z::ref(E));}

//-------------------------------------------------
// address-of expression
rExpr(L) ::= BITWISEAND(B)  rExpr(E). {L = z::ref(pctx).aPointerInstanceExpr(B, z::ref(E));}

// value-of expression
rExpr(L) ::= STAR(B) rExpr(E). {L = z::ref(pctx).aValueInstanceExpr(B, z::ref(E));}

//-------------------------------------------------
// template definition instance expression
rExpr(L) ::= rTemplateDefnTypeSpec(R) LBRACKET(B) rExprList(M) RBRACKET. {L = z::ref(pctx).aTemplateDefnInstanceExpr(B, z::ref(R), z::ref(M));}

//-------------------------------------------------
// variable z::ref expressions
%type rVariableRefExpr {Ast::Expr*}
rVariableRefExpr(L) ::= ID(I). {L = z::ref(pctx).aVariableRefExpr(I);}

//-------------------------------------------------
// variable member expressions, e.g. struct member
%type rMemberVariableExpr {Ast::MemberExpr*}
rMemberVariableExpr(L) ::= rExpr(R) DOT ID(M). {L = z::ref(pctx).aMemberVariableExpr(z::ref(R), M);}

//-------------------------------------------------
// type member expressions, e.g. enum member
%type rTypeSpecMemberExpr {Ast::TypeSpecMemberExpr*}
rTypeSpecMemberExpr(L) ::= rTypeSpec(R) DOT ID(M). {L = z::ref(pctx).aTypeSpecMemberExpr(z::ref(R), M);}

//-------------------------------------------------
// function instance expressions
%type rFunctionInstanceExpr {Ast::TypeSpecInstanceExpr*}
rFunctionInstanceExpr(L) ::= rFunctionTypeSpec(R) LSQUARE(B) rExprList(M) RSQUARE. {L = z::ref(pctx).aFunctionInstanceExpr(B, z::ref(R), z::ref(M));}

//-------------------------------------------------
// anonymous function instance expressions
%type rAnonymousFunctionExpr {Ast::AnonymousFunctionExpr*}
rAnonymousFunctionExpr(L) ::= rEnterAnonymousFunction(R) rCompoundStatement(C). {L = z::ref(pctx).aAnonymousFunctionExpr(z::ref(R), z::ref(C));}

//-------------------------------------------------
%type rEnterAnonymousFunction {Ast::ChildFunctionDefn*}
rEnterAnonymousFunction(L) ::= rFunctionTypeSpec(R). {L = z::ref(pctx).aEnterAnonymousFunction(z::ref(R));}
rEnterAnonymousFunction(L) ::= FUNCTION(R). {L = z::ref(pctx).aEnterAutoAnonymousFunction(R);}

//-------------------------------------------------
// struct instance expressions
%type rStructInstanceExpr {Ast::StructInstanceExpr*}
rStructInstanceExpr(L) ::= rEnterStructInstanceExpr(R) LCURLY(B) rStructInitPartList(P) rLeaveStructInstanceExpr. {L = z::ref(pctx).aStructInstanceExpr(B, z::ref(R), z::ref(P));}
rStructInstanceExpr(L) ::= rEnterStructInstanceExpr(R) LCURLY(B)                        rLeaveStructInstanceExpr. {L = z::ref(pctx).aStructInstanceExpr(B, z::ref(R));}

//-------------------------------------------------
// auto struct instance expressions
%type rAutoStructInstanceExpr {Ast::Expr*}
rAutoStructInstanceExpr(L) ::= STRUCT(B) rEnterAutoStructInstanceExpr(R) rStructInitPartList(P) rLeaveStructInstanceExpr. {L = z::ref(pctx).aAutoStructInstanceExpr(B, z::ref(R), z::ref(P));}
rAutoStructInstanceExpr(L) ::= STRUCT(B) rEnterAutoStructInstanceExpr(R)                        rLeaveStructInstanceExpr. {L = z::ref(pctx).aAutoStructInstanceExpr(B, z::ref(R));}

//-------------------------------------------------
// special case - struct can be instantiated with {} or () for syntactic equivalence with C/C++.
rStructInstanceExpr(L) ::= rStructTypeSpec(R) LBRACKET(B) rStructInitPartList(P) RBRACKET. {L = z::ref(pctx).aStructInstanceExpr(B, z::ref(R), z::ref(P));}
rStructInstanceExpr(L) ::= rStructTypeSpec(R) LBRACKET(B)                        RBRACKET. {L = z::ref(pctx).aStructInstanceExpr(B, z::ref(R));}

//-------------------------------------------------
%type rEnterStructInstanceExpr {const Ast::StructDefn*}
rEnterStructInstanceExpr(L) ::= rStructTypeSpec(R). {L = z::ref(pctx).aEnterStructInstanceExpr(z::ref(R));}

//-------------------------------------------------
%type rEnterAutoStructInstanceExpr {const Ast::StructDefn*}
rEnterAutoStructInstanceExpr(L) ::= LCURLY(R). {L = z::ref(pctx).aEnterAutoStructInstanceExpr(R);}

//-------------------------------------------------
rLeaveStructInstanceExpr ::= RCURLY. {z::ref(pctx).aLeaveStructInstanceExpr();}

//-------------------------------------------------
%type rStructInitPartList {Ast::StructInitPartList*}
rStructInitPartList(L) ::= rStructInitPartList(R) rStructInitPart(P). {L = z::ref(pctx).aStructInitPartList(z::ref(R), z::ref(P));}
rStructInitPartList(L) ::=                        rStructInitPart(P). {L = z::ref(pctx).aStructInitPartList(z::ref(P));}

//-------------------------------------------------
%type rStructInitPart {Ast::StructInitPart*}
rStructInitPart(L) ::= rEnterStructInitPart(R) COLON(B) rExpr(E) rLeaveStructInitPart. {L = z::ref(pctx).aStructInitPart(B, z::ref(R), z::ref(E));}

//-------------------------------------------------
%type rEnterStructInitPart {const Ast::VariableDefn*}
rEnterStructInitPart(L) ::= ID(R). {L = z::ref(pctx).aEnterStructInitPart(R);}
rLeaveStructInitPart    ::= SEMI(B). {z::ref(pctx).aLeaveStructInitPart(B);}

//-------------------------------------------------
// functor call expressions
%type rCallExpr {Ast::CallExpr*}
rCallExpr(L) ::= rCallPart(R).  {L = R;}

//-------------------------------------------------
// function call type
%type rRunExpr {Ast::RunExpr*}
rRunExpr(L) ::= RUN(B) rFunctorCallPart(F).  {L = z::ref(pctx).aRunExpr(B, z::ref(F));}

//-------------------------------------------------
// functor call expressions
%type rCallPart {Ast::CallExpr*}
rCallPart(L) ::= rRoutineCallPart(R). {L = R;}
rCallPart(L) ::= rFunctorCallPart(R). {L = R;}

//-------------------------------------------------
// routine call expressions
%type rRoutineCallPart {Ast::RoutineCallExpr*}
rRoutineCallPart(L) ::= rEnterRoutineCall(typeSpec) LBRACKET(B) rCallArgList(exprList) RBRACKET.  {L = z::ref(pctx).aRoutineCallExpr(B, z::ref(typeSpec), z::ref(exprList));}

//-------------------------------------------------
// functor expressions
%type rEnterRoutineCall {const Ast::Routine*}
rEnterRoutineCall(L) ::= rRoutineTypeSpec(typeSpec). { L = z::ref(pctx).aEnterRoutineCall(z::ref(typeSpec));}

//-------------------------------------------------
// functor call expressions
%type rFunctorCallPart {Ast::FunctorCallExpr*}
rFunctorCallPart(L) ::= rEnterFunctorCall(expr) LBRACKET(B) rCallArgList(exprList) RBRACKET.  {L = z::ref(pctx).aFunctorCallExpr(B, z::ref(expr), z::ref(exprList));}

//-------------------------------------------------
// functor expressions
%type rEnterFunctorCall {Ast::Expr*}
rEnterFunctorCall(L) ::= rOrderedExpr(expr).          { L = z::ref(pctx).aEnterFunctorCall(z::ref(expr));}
rEnterFunctorCall(L) ::= ID(I).                       { L = z::ref(pctx).aEnterFunctorCall(I);}
rEnterFunctorCall(L) ::= rFunctionTypeSpec(typeSpec). { L = z::ref(pctx).aEnterFunctorCall(z::ref(typeSpec));}

//-------------------------------------------------
// comma-separated list of function-call args
%type rCallArgList {Ast::ExprList*}
rCallArgList(L) ::= rCallArgList(R) COMMA(B) rExpr(E). {L = z::ref(pctx).aCallArgList(B, z::ref(R), z::ref(E));}
rCallArgList(L) ::=                          rExpr(E). {L = z::ref(pctx).aCallArgList(z::ref(E));}
rCallArgList(L) ::= .                                  {L = z::ref(pctx).aCallArgList();}

//-------------------------------------------------
// constant expressions
%type rConstantExpr {const Ast::ConstantExpr*}
rConstantExpr(L) ::= FLOAT_CONST  (value).  {L = z::ptr(z::ref(pctx).aConstantFloatExpr(value));}
rConstantExpr(L) ::= DOUBLE_CONST (value).  {L = z::ptr(z::ref(pctx).aConstantDoubleExpr(value));}
rConstantExpr(L) ::= TRUE_CONST   (value).  {L = z::ptr(z::ref(pctx).aConstantBooleanExpr(value));}
rConstantExpr(L) ::= FALSE_CONST  (value).  {L = z::ptr(z::ref(pctx).aConstantBooleanExpr(value));}
rConstantExpr(L) ::= STRING_CONST (value).  {L = z::ptr(z::ref(pctx).aConstantStringExpr(value));}
rConstantExpr(L) ::= CHAR_CONST   (value).  {L = z::ptr(z::ref(pctx).aConstantCharExpr(value));}
rConstantExpr(L) ::= LHEXINT_CONST(value).  {L = z::ptr(z::ref(pctx).aConstantLongExpr(value));}
rConstantExpr(L) ::= LDECINT_CONST(value).  {L = z::ptr(z::ref(pctx).aConstantLongExpr(value));}
rConstantExpr(L) ::= LOCTINT_CONST(value).  {L = z::ptr(z::ref(pctx).aConstantLongExpr(value));}
rConstantExpr(L) ::= HEXINT_CONST (value).  {L = z::ptr(z::ref(pctx).aConstantIntExpr(value));}
rConstantExpr(L) ::= DECINT_CONST (value).  {L = z::ptr(z::ref(pctx).aConstantIntExpr(value));}
rConstantExpr(L) ::= OCTINT_CONST (value).  {L = z::ptr(z::ref(pctx).aConstantIntExpr(value));}

rConstantExpr(L) ::= UDECINT_CONST(value).  {L = z::ptr(z::ref(pctx).aConstantIntExpr(value));}

rConstantExpr(L) ::= rKeyConstantExpr(R).   {L = R;}

%type rKeyConstantExpr {const Ast::ConstantExpr*}
rKeyConstantExpr(L) ::= KEY_CONST(value).  {L = z::ptr(z::ref(pctx).aConstantStringExpr(value));}

/*
//-------------------------------------------------
// binary operators
rBinaryOp(L) ::= AND(R)             . {L = R;}
rBinaryOp(L) ::= OR(R)              . {L = R;}
rBinaryOp(L) ::= EQUAL(R)           . {L = R;}
rBinaryOp(L) ::= NOTEQUAL(R)        . {L = R;}
rBinaryOp(L) ::= LT(R)              . {L = R;}
rBinaryOp(L) ::= GT(R)              . {L = R;}
rBinaryOp(L) ::= LTE(R)             . {L = R;}
rBinaryOp(L) ::= GTE(R)             . {L = R;}
rBinaryOp(L) ::= HAS(R)             . {L = R;}
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
rBinaryOp(L) ::= SHL(R)             . {L = R;}
rBinaryOp(L) ::= SHR(R)             . {L = R;}
rBinaryOp(L) ::= PLUS(R)            . {L = R;}
rBinaryOp(L) ::= MINUS(R)           . {L = R;}
rBinaryOp(L) ::= STAR(R)            . {L = R;}
rBinaryOp(L) ::= DIVIDE(R)          . {L = R;}
rBinaryOp(L) ::= MOD(R)             . {L = R;}
*/
