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
// All keywords, etc
%nonassoc ERR EOF RESERVED.
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

%start_symbol rstart

//-------------------------------------------------
rstart ::= rSubStart EOF.

//-------------------------------------------------
// source file.
rSubStart ::= rUnitStatementList.

//-------------------------------------------------
rUnitStatementList ::= rImportStatementList rEnterNamespace rGlobalStatementList rLeaveNamespace.

//-------------------------------------------------
rEnterNamespace ::= NAMESPACE rUnitNamespaceId SEMI.
rEnterNamespace ::= .

rUnitNamespaceId ::= rUnitNamespaceId TYPE_SCOPE ID(name). {ref(pctx).aUnitNamespaceId(name);}
rUnitNamespaceId ::=                             ID(name). {ref(pctx).aUnitNamespaceId(name);}

rLeaveNamespace ::= . {ref(pctx).aLeaveNamespace();}

//-------------------------------------------------
// list of statements to specify imports
rImportStatementList ::= rImportStatementList rImportStatement.
rImportStatementList ::= .

//-------------------------------------------------
rImportStatement ::= rHeaderType(headerType) rImportNamespaceId(id) rDefinitionType(defType) SEMI. {ref(pctx).aImportStatement(headerType, ref(id), defType);}

//-------------------------------------------------
// import type
%type rHeaderType {Ast::HeaderType::T}
rHeaderType(L) ::= INCLUDE.   {L = Ast::HeaderType::Include;}
rHeaderType(L) ::= IMPORT.    {L = Ast::HeaderType::Import;}

//-------------------------------------------------
%type rImportNamespaceId {Ast::ImportStatement*}
rImportNamespaceId(L) ::= rImportNamespaceId(statement) SCOPE ID(name). {L = ref(pctx).aImportNamespaceId(ref(statement), name);}
rImportNamespaceId(L) ::=                               ID(name). {L = ref(pctx).aImportNamespaceId(name);}

//-------------------------------------------------
rGlobalStatementList ::= rGlobalStatementList rGlobalStatement.
rGlobalStatementList ::= .

//-------------------------------------------------
rGlobalStatement ::= rGlobalTypeSpecStatement.

//-------------------------------------------------
%type rGlobalTypeSpecStatement {Ast::Statement*}
rGlobalTypeSpecStatement(L) ::= rAccessType(accessType) rTypeSpecDef(typeSpec). {L = ref(pctx).aGlobalTypeSpecStatement(accessType, ref(typeSpec));}

//-------------------------------------------------
// access specifiers
%type rAccessType {Ast::AccessType::T}
rAccessType(L) ::= PRIVATE.   {L = Ast::AccessType::Private;}
rAccessType(L) ::= INTERNAL.  {L = Ast::AccessType::Internal;}
rAccessType(L) ::= PROTECTED. {L = Ast::AccessType::Protected;}
rAccessType(L) ::= PUBLIC.    {L = Ast::AccessType::Public;}
rAccessType(L) ::= EXPORT.    {L = Ast::AccessType::Export;}
rAccessType(L) ::= .          {L = Ast::AccessType::Private;}

//-------------------------------------------------
// definition specifiers
%type rDefinitionType {Ast::DefinitionType::T}
rDefinitionType(L) ::= NATIVE. {L = Ast::DefinitionType::Native;}
rDefinitionType(L) ::= .       {L = Ast::DefinitionType::Direct;}

//-------------------------------------------------
%type rTypeSpecDef {Ast::UserDefinedTypeSpec*}
rTypeSpecDef(L) ::= rTypedefDefn(R).       {L = R;}
rTypeSpecDef(L) ::= rTemplateDecl(R).      {L = R;}
rTypeSpecDef(L) ::= rEnumDefn(R).          {L = R;}
rTypeSpecDef(L) ::= rStructDefn(R).        {L = R;}
rTypeSpecDef(L) ::= rRoutineDecl(R).       {L = R;}
rTypeSpecDef(L) ::= rRoutineDefn(R).       {L = R;}
rTypeSpecDef(L) ::= rFunctionDecl(R).      {L = R;}
rTypeSpecDef(L) ::= rRootFunctionDefn(R).  {L = R;}
rTypeSpecDef(L) ::= rChildFunctionDefn(R). {L = R;}
rTypeSpecDef(L) ::= rEventDecl(R).         {L = R;}

//-------------------------------------------------
// typedef declarations
%type rTypedefDefn {Ast::TypedefDefn*}
rTypedefDefn(L) ::= TYPEDEF ID(name) NATIVE SEMI. {L = ref(pctx).aTypedefDefn(name, Ast::DefinitionType::Native);}

//-------------------------------------------------
// template declarations
%type rTemplateDecl {Ast::TemplateDecl*}
rTemplateDecl(L) ::= TEMPLATE LT rTemplatePartList(list) GT ID(name) NATIVE SEMI. {L = ref(pctx).aTemplateDecl(name, Ast::DefinitionType::Native, ref(list));}

//-------------------------------------------------
%type rTemplatePartList {Ast::TemplatePartList*}
rTemplatePartList(L) ::= rTemplatePartList(R) COMMA ID(name). {L = ref(pctx).aTemplatePartList(ref(R), name);}
rTemplatePartList(L) ::=                            ID(name). {L = ref(pctx).aTemplatePartList(name);}

//-------------------------------------------------
// enum declarations
%type rEnumDefn {Ast::EnumDefn*}
rEnumDefn(L) ::= ENUM ID(name) NATIVE                                  SEMI. {L = ref(pctx).aEnumDefn(name, Ast::DefinitionType::Native);}
rEnumDefn(L) ::= ENUM ID(name) LCURLY rEnumMemberDefnList(list) RCURLY SEMI. {L = ref(pctx).aEnumDefn(name, Ast::DefinitionType::Direct, ref(list));}

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
%type rStructDefn {Ast::StructDefn*}
rStructDefn(L) ::= STRUCT ID(name) NATIVE                                    SEMI. {L = ref(pctx).aStructDefn(name, Ast::DefinitionType::Native);}
rStructDefn(L) ::= STRUCT ID(name) LCURLY rStructMemberDefnList(list) RCURLY SEMI. {L = ref(pctx).aStructDefn(name, Ast::DefinitionType::Direct, ref(list));}
rStructDefn(L) ::= STRUCT ID(name) LCURLY                             RCURLY SEMI. {L = ref(pctx).aStructDefn(name, Ast::DefinitionType::Direct);}

//-------------------------------------------------
%type rStructMemberDefnList {Ast::Scope*}
rStructMemberDefnList(L) ::= rStructMemberDefnList(list) rVariableDefn(variableDef) SEMI. {L = ref(pctx).aStructMemberDefnList(ref(list), ref(variableDef));}
rStructMemberDefnList(L) ::=                             rVariableDefn(variableDef) SEMI. {L = ref(pctx).aStructMemberDefnList(ref(variableDef));}

//-------------------------------------------------
// routine declarations
%type rRoutineDecl {Ast::RoutineDecl*}
rRoutineDecl(L) ::= ROUTINE rQualifiedTypeSpec(out) ID(name) rInParamsList(in) NATIVE SEMI. {L = ref(pctx).aRoutineDecl(ref(out), name, ref(in), Ast::DefinitionType::Native);}

//-------------------------------------------------
// routine declarations
%type rRoutineDefn {Ast::RoutineDefn*}
rRoutineDefn(L) ::= rEnterRoutineDefn(routineDefn) rCompoundStatement(block). {L = ref(pctx).aRoutineDefn(ref(routineDefn), ref(block));}

//-------------------------------------------------
%type rEnterRoutineDefn {Ast::RoutineDefn*}
rEnterRoutineDefn(L) ::= ROUTINE rQualifiedTypeSpec(out) ID(name) rInParamsList(in). {L = ref(pctx).aEnterRoutineDefn(ref(out), name, ref(in), Ast::DefinitionType::Direct);}

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
rEnterChildFunctionDefn(L) ::= rTypeSpec(base) ID(name). {L = ref(pctx).aEnterChildFunctionDefn(ref(base), name, Ast::DefinitionType::Direct);}

//-------------------------------------------------
// event declarations
%type rEventDecl {Ast::EventDecl*}
rEventDecl(L) ::= EVENT LBRACKET rVariableDefn(in) RBRACKET LINK rFunctionSig(functionSig) SEMI. {L = ref(pctx).aEventDecl(ref(in), ref(functionSig), Ast::DefinitionType::Direct);}

//-------------------------------------------------
// function signature.
%type rFunctionSig {Ast::FunctionSig*}
rFunctionSig(T) ::= FUNCTION rParamsList(out) ID(name) rInParamsList(in). {T = ref(pctx).aFunctionSig(ref(out), name, ref(in));}

//-------------------------------------------------
// in parameter list
%type rInParamsList {Ast::Scope*}
rInParamsList(L) ::= rParamsList(scope). {L = ref(pctx).aInParamsList(ref(scope));}

//-------------------------------------------------
// parameter lists
%type rParamsList {Ast::Scope*}
rParamsList(L) ::= LBRACKET rParam(R) RBRACKET. {L = R;}

//-------------------------------------------------
// variable lists
%type rParam {Ast::Scope*}
rParam(L) ::= rParam(list) COMMA rVariableDefn(variableDef). {L = ref(pctx).aParam(ref(list), ref(variableDef));}
rParam(L) ::=                    rVariableDefn(variableDef). {L = ref(pctx).aParam(ref(variableDef));}
rParam(L) ::= .                                              {L = ref(pctx).aParam();}

//-------------------------------------------------
// variable def
%type rVariableDefn {const Ast::VariableDefn*}
rVariableDefn(L) ::=                              ID(name) ASSIGNEQUAL rExpr(initExpr). {L = ref(pctx).aVariableDefn(name, ref(initExpr));}
rVariableDefn(L) ::= rQualifiedTypeSpec(qTypeRef) ID(name).                             {L = ref(pctx).aVariableDefn(ref(qTypeRef), name);}

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

//-------------------------------------------------
%type rStructTypeSpec {const Ast::StructDefn*}
rStructTypeSpec(L) ::= rPreStructTypeSpec(R). {L = ref(pctx).aStructTypeSpec(ref(R));}

//-------------------------------------------------
%type rFunctionTypeSpec {const Ast::Function*}
rFunctionTypeSpec(L) ::= rPreFunctionTypeSpec(R). {L = ref(pctx).aFunctionTypeSpec(ref(R));}

//-------------------------------------------------
// "private" type references, can be only called by the public equivalent rules
%type rPreTypeSpec {const Ast::TypeSpec*}
rPreTypeSpec(L) ::= rPreFunctionTypeSpec(R). {L = R;}
rPreTypeSpec(L) ::= rPreStructTypeSpec(R).   {L = R;}
rPreTypeSpec(L) ::= rPreOtherTypeSpec(R).    {L = R;}

//-------------------------------------------------
%type rPreFunctionTypeSpec {const Ast::Function*}
rPreFunctionTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE FUNCTION_TYPE(name). {L = ref(pctx).aFunctionTypeSpec(ref(parent), name);}
rPreFunctionTypeSpec(L) ::=                            FUNCTION_TYPE(name). {L = ref(pctx).aFunctionTypeSpec(name);}

//-------------------------------------------------
%type rPreStructTypeSpec {const Ast::StructDefn*}
rPreStructTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE STRUCT_TYPE(name). {L = ref(pctx).aStructTypeSpec(ref(parent), name);}
rPreStructTypeSpec(L) ::=                            STRUCT_TYPE(name). {L = ref(pctx).aStructTypeSpec(name);}

//-------------------------------------------------
%type rPreOtherTypeSpec {const Ast::TypeSpec*}
rPreOtherTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE OTHER_TYPE(name). {L = ref(pctx).aOtherTypeSpec(ref(parent), name);}
rPreOtherTypeSpec(L) ::=                            OTHER_TYPE(name). {L = ref(pctx).aOtherTypeSpec(name);}

//-------------------------------------------------
// statements
%type rInnerStatement {Ast::Statement*}
rInnerStatement(L) ::= rUserDefinedTypeSpecStatement(R). {L = R;}
rInnerStatement(L) ::= rLocalStatement(R).               {L = R;}
rInnerStatement(L) ::= rExprStatement(R).                {L = R;}
rInnerStatement(L) ::= rPrintStatement(R).               {L = R;}
rInnerStatement(L) ::= rRoutineReturnStatement(R).       {L = R;}
rInnerStatement(L) ::= rFunctionReturnStatement(R).      {L = R;}
rInnerStatement(L) ::= rCompoundStatement(R).            {L = R;}

//-------------------------------------------------
%type rUserDefinedTypeSpecStatement {Ast::UserDefinedTypeSpecStatement*}
rUserDefinedTypeSpecStatement(L) ::= rTypeSpecDef(typeSpec). {L = ref(pctx).aUserDefinedTypeSpecStatement(ref(typeSpec));}

//-------------------------------------------------
%type rLocalStatement {Ast::LocalStatement*}
rLocalStatement(L) ::= LOCAL rVariableDefn(defn) SEMI. {L = ref(pctx).aLocalStatement(ref(defn));}

//-------------------------------------------------
%type rExprStatement {Ast::ExprStatement*}
rExprStatement(L) ::= rExpr(expr) SEMI. {L = ref(pctx).aExprStatement(ref(expr));}

//-------------------------------------------------
%type rPrintStatement {Ast::PrintStatement*}
rPrintStatement(L) ::= PRINT rFormatExpr(expr) SEMI. {L = ref(pctx).aPrintStatement(ref(expr));}

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
rExpr(L) ::= rCallExpr(R).              {L = R;}
rExpr(L) ::= rVariableRefExpr(R).       {L = R;}
rExpr(L) ::= rVariableMemberExpr(R).    {L = R;}
rExpr(L) ::= rTypeSpecMemberExpr(R).    {L = R;}
rExpr(L) ::= rStructInstanceExpr(R).    {L = R;}
rExpr(L) ::= rFunctionInstanceExpr(R).  {L = R;}
rExpr(L) ::= rAnonymousFunctionExpr(R).  {L = R;}
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
rListExpr(L) ::= LSQUARE(B) rListList(R) RSQUARE. {L = ref(pctx).aListExpr(B, ref(R));}

%type rListList {Ast::ListList*}
rListList(L) ::= rListsList(R)      . {L = R;}
rListList(L) ::= rListsList(R) COMMA. {L = R;}

%type rListsList {Ast::ListList*}
rListsList(L)  ::= rListsList(R) COMMA rListItem(I). {L = ref(pctx).aListList(ref(R), ref(I));}
rListsList(L)  ::=                     rListItem(I). {L = ref(pctx).aListList(ref(I));}
rListsList(L)  ::=                                 . {L = ref(pctx).aListList();}

%type rListItem {Ast::ListItem*}
rListItem(L)  ::= rExpr(E). {L = ref(pctx).aListItem(ref(E));}

//-------------------------------------------------
// dict (strict type-checking for key and value)
%type rDictExpr {Ast::DictExpr*}
rDictExpr(L) ::= LSQUARE(B) rDictList(R) RSQUARE. {L = ref(pctx).aDictExpr(B, ref(R));}

%type rDictList {Ast::DictList*}
rDictList(L) ::= rDictsList(R)       . {L = R;}
rDictList(L) ::= rDictsList(R) COMMA . {L = R;}
rDictList(L) ::= COLON               . {L = ref(pctx).aDictList();}

%type rDictsList {Ast::DictList*}
rDictsList(L)  ::= rDictsList(R) COMMA rDictItem(I). {L = ref(pctx).aDictList(ref(R), ref(I));}
rDictsList(L)  ::=                     rDictItem(I). {L = ref(pctx).aDictList(ref(I));}

%type rDictItem {Ast::DictItem*}
rDictItem(L)  ::= rExpr(K) COLON rExpr(E). {L = ref(pctx).aDictItem(ref(K), ref(E));}

//-------------------------------------------------
// tree (no type checking for key or value)
%type rTreeExpr {Ast::DictExpr*}
rTreeExpr(L) ::= LCURLY(B) rTreeList(R) RCURLY. {L = ref(pctx).aDictExpr(B, ref(R));}

%type rTreeList {Ast::DictList*}
rTreeList(L) ::= rTreesList(R)       . {L = R;}
rTreeList(L) ::= rTreesList(R) COMMA . {L = R;}
rTreeList(L) ::= COLON               . {L = ref(pctx).aDictList();}

%type rTreesList {Ast::DictList*}
rTreesList(L)  ::= rTreesList(R) COMMA rTreeItem(I). {L = ref(pctx).aDictList(ref(R), ref(I));}
rTreesList(L)  ::=                     rTreeItem(I). {L = ref(pctx).aDictList(ref(I));}

%type rTreeItem {Ast::DictItem*}
rTreeItem(L)  ::= rExpr(K) COLON rExpr(E). {L = ref(pctx).aDictItem(ref(K), ref(E));}

//-------------------------------------------------
// ordered expression
rExpr(L) ::= LBRACKET rExpr(innerExpr) RBRACKET. {L = ref(pctx).aOrderedExpr(ref(innerExpr));}

//-------------------------------------------------
// variable ref expressions
%type rVariableRefExpr {Ast::Expr*}
rVariableRefExpr(L) ::= ID(I). {L = ref(pctx).aVariableRefExpr(I);}

//-------------------------------------------------
// variable member expressions
%type rVariableMemberExpr {Ast::VariableMemberExpr*}
rVariableMemberExpr(L) ::= rExpr(R) DOT ID(M). {L = ref(pctx).aVariableMemberExpr(ref(R), M);}

//-------------------------------------------------
// type member expressions, e.g. enum member
%type rTypeSpecMemberExpr {Ast::TypeSpecMemberExpr*}
rTypeSpecMemberExpr(L) ::= rTypeSpec(R) DOT ID(M). {L = ref(pctx).aTypeSpecMemberExpr(ref(R), M);}

//-------------------------------------------------
// function instance expressions
%type rFunctionInstanceExpr {Ast::TypeSpecInstanceExpr*}
rFunctionInstanceExpr(L) ::= rTypeSpec(R) LSQUARE rExprList(M) RSQUARE. {L = ref(pctx).aFunctionInstanceExpr(ref(R), ref(M));}

//-------------------------------------------------
// function instance expressions
%type rAnonymousFunctionExpr {Ast::FunctionInstanceExpr*}
rAnonymousFunctionExpr(L) ::= rFunctionTypeSpec(R) rCompoundStatement(C). {L = ref(pctx).aAnonymousFunctionExpr(ref(R), ref(C));}

//-------------------------------------------------
// struct instance expressions
%type rStructInstanceExpr {Ast::StructInstanceExpr*}
rStructInstanceExpr(L) ::= rStructTypeSpec(R) LCURLY(B) rStructInitPartList(P) RCURLY. {L = ref(pctx).aStructInstanceExpr(B, ref(R), ref(P));}

//-------------------------------------------------
%type rStructInitPartList {Ast::StructInitPartList*}
rStructInitPartList(L) ::= rStructInitPartList(R) COMMA rStructInitPart(P). {L = ref(pctx).aStructInitPartList(ref(R), ref(P));}
rStructInitPartList(L) ::=                              rStructInitPart(P). {L = ref(pctx).aStructInitPartList(ref(P));}
rStructInitPartList(L) ::=                                                . {L = ref(pctx).aStructInitPartList();}

//-------------------------------------------------
%type rStructInitPart {Ast::StructInitPart*}
rStructInitPart(L) ::= ID(R) COLON rExpr(E). {L = ref(pctx).aStructInitPart(R, ref(E));}

//-------------------------------------------------
// function call expressions
%type rCallExpr {Ast::CallExpr*}
rCallExpr(L) ::= rTypeSpec(typeSpec) LBRACKET(B) rExprList(exprList) RBRACKET.  {L = ref(pctx).aCallExpr(B, ref(typeSpec), ref(exprList));}

//-------------------------------------------------
// functor call expressions
rCallExpr(L) ::= rExpr(expr) LBRACKET(B) rExprList(exprList) RBRACKET.  {L = ref(pctx).aCallExpr(B, ref(expr), ref(exprList));}

//-------------------------------------------------
// constant expressions
%type rConstantExpr {const Ast::ConstantExpr*}
rConstantExpr(L) ::= FLOAT_CONST  (value).  {L = ptr(ref(pctx).aConstantExpr("float",  value));}
rConstantExpr(L) ::= DOUBLE_CONST (value).  {L = ptr(ref(pctx).aConstantExpr("double", value));}
rConstantExpr(L) ::= TRUE_CONST   (value).  {L = ptr(ref(pctx).aConstantExpr("bool",   value));}
rConstantExpr(L) ::= FALSE_CONST  (value).  {L = ptr(ref(pctx).aConstantExpr("bool",   value));}
rConstantExpr(L) ::= KEY_CONST    (value).  {L = ptr(ref(pctx).aConstantExpr("string", value));}
rConstantExpr(L) ::= STRING_CONST (value).  {L = ptr(ref(pctx).aConstantExpr("string", value));}
rConstantExpr(L) ::= CHAR_CONST   (value).  {L = ptr(ref(pctx).aConstantExpr("char",   value));}
rConstantExpr(L) ::= HEXINT_CONST (value).  {L = ptr(ref(pctx).aConstantExpr("int",    value));}
rConstantExpr(L) ::= DECINT_CONST (value).  {L = ptr(ref(pctx).aConstantExpr("int",    value));}
rConstantExpr(L) ::= OCTINT_CONST (value).  {L = ptr(ref(pctx).aConstantExpr("int",    value));}
rConstantExpr(L) ::= LHEXINT_CONST(value).  {L = ptr(ref(pctx).aConstantExpr("long",   value));}
rConstantExpr(L) ::= LDECINT_CONST(value).  {L = ptr(ref(pctx).aConstantExpr("long",   value));}
rConstantExpr(L) ::= LOCTINT_CONST(value).  {L = ptr(ref(pctx).aConstantExpr("long",   value));}

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
