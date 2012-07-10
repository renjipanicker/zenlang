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
    throw z::Exception("Parser", z::zfmt(z::t2t(TOKEN), "Syntax error at token: %{d} (%{s})").arg("d",TOKEN.id()).arg("s", TOKEN.text()));
}

%parse_accept {
//    zbl::mlog() << "Parse complete!";
}

%parse_failure {
//    throw z::exception(z::string::creator("%{err} Parse error").arg(z::any("err"), z::any(z::string(z::c2f(pctx).err(TOKEN)))).value());
}

%stack_overflow {
//    throw z::exception(z::string::creator("%{err} Stack overflow error").arg(z::any("err"), z::any(z::c2f(pctx).err())).value());
}

%token_destructor {z::unused_t(pctx);z::TokenData::deleteT($$);}

%name ZenParser

%token_type {z::TokenData}
%extra_argument {z::ParserContext* pctx}

%include {
    #ifdef _WIN32
    #pragma warning( disable : 4100)  /* unreferenced formal parameter */
    #pragma warning( disable : 4702)  /* unreachable code */
    #else
    #pragma GCC diagnostic ignored "-Wunused-variable"
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #endif

    #include "zenlang.hpp"
    #include "base/factory.hpp"
    #include "base/error.hpp"
    #include "base/compiler.hpp"
}

//-------------------------------------------------
// basic tokens
%nonassoc ERR EOF RESERVED.

//-------------------------------------------------
// All operators, in increasing order of precedence
// \todo make consistent with C/C++
%left DEFINEEQUAL.
%left ASSIGNEQUAL TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL.
%left QUESTION.
%left AND OR.
%left BITWISEAND BITWISEXOR BITWISEOR.
%left EQUAL NOTEQUAL.
%left LT GT LTE GTE HAS.
%left SHL SHR.
%left PLUS MINUS MOD.
%left DIVIDE STAR.

%left INC DEC BITWISENOT NOT.
%left LSQUARE RSQUARE.
%left LBRACKET RBRACKET.
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
rUnitStatementList ::= rImportStatementList rNamespaceStatement(N) rGlobalStatementList. {z::c2f(pctx).aUnitStatementList(z::ref(N));}

//-------------------------------------------------
// list of statements to specify imports
rImportStatementList ::= rImportStatementList rImportStatement.
rImportStatementList ::= .

//-------------------------------------------------
rImportStatement ::= rHeaderType(headerType) rImportNamespaceList(L) rDefinitionType(defType) rOptionalAccessType(A) SEMI(B). {
    z::string filename;
    z::Ast::Module::Level_t level = z::c2f(pctx).aImportStatement(z::t2t(B), A, headerType, defType, z::ref(L), filename);
    if(level > 0) {
        z::Ast::Module module(z::c2f(pctx).unit(), filename, level);
        c2c(pctx).import(module);
    }
}

//-------------------------------------------------
// import namespace list
%type rImportNamespaceList {z::Ast::NamespaceList*}
rImportNamespaceList(L) ::= rImportNamespaceList(R) SCOPE rAnyId(name). {L = z::c2f(pctx).aImportNamespaceList(z::ref(R), z::t2t(name));}
rImportNamespaceList(L) ::=                               rAnyId(name). {L = z::c2f(pctx).aImportNamespaceList(z::t2t(name));}

//-------------------------------------------------
// access specifiers
%type rOptionalAccessType {z::Ast::AccessType::T}
rOptionalAccessType(L) ::= rAccessType(R).    {L = R;}
rOptionalAccessType(L) ::=               .    {L = z::Ast::AccessType::Private;}

//-------------------------------------------------
// import type
%type rHeaderType {z::Ast::HeaderType::T}
rHeaderType(L) ::= INCLUDE.   {L = z::Ast::HeaderType::Include;}
rHeaderType(L) ::= IMPORT.    {L = z::Ast::HeaderType::Import;}

//-------------------------------------------------
// namespace statement
%type rNamespaceStatement {z::Ast::EnterNamespaceStatement*}
rNamespaceStatement(L) ::= NAMESPACE(B) rUnitNamespaceList(R) SEMI. {L = z::c2f(pctx).aNamespaceStatement(z::t2t(B), z::ref(R));}
rNamespaceStatement(L) ::=                                        . {L = z::c2f(pctx).aNamespaceStatement();}

//-------------------------------------------------
// namespace list
%type rUnitNamespaceList {z::Ast::NamespaceList*}
rUnitNamespaceList(L) ::= rUnitNamespaceList(R) SCOPE rAnyId(name). {L = z::c2f(pctx).aUnitNamespaceList(z::ref(R), z::t2t(name));}
rUnitNamespaceList(L) ::=                             rAnyId(name). {L = z::c2f(pctx).aUnitNamespaceList(z::t2t(name));}

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
%type rGlobalTypeSpecStatement {z::Ast::Statement*}
rGlobalTypeSpecStatement(L) ::= rAccessType(accessType)   rTypeSpecDef(typeSpec).  {L = z::c2f(pctx).aGlobalTypeSpecStatement(accessType, z::ref(typeSpec));}
rGlobalTypeSpecStatement(L) ::= rAccessType(accessType) rRootStructDefn(typeSpec). {L = z::c2f(pctx).aGlobalTypeSpecStatement(accessType, z::ref(typeSpec));}
rGlobalTypeSpecStatement(L) ::= PROTECTED rRootStructDefn(typeSpec).               {L = z::c2f(pctx).aGlobalTypeSpecStatement(z::Ast::AccessType::Protected, z::ref(typeSpec));}
rGlobalTypeSpecStatement(L) ::= rAccessType(accessType) rStructDecl(typeSpec).     {L = z::c2f(pctx).aGlobalTypeSpecStatement(accessType, z::ref(typeSpec));}
rGlobalTypeSpecStatement(L) ::= PROTECTED rStructDecl(typeSpec).                   {L = z::c2f(pctx).aGlobalTypeSpecStatement(z::Ast::AccessType::Protected, z::ref(typeSpec));}
rGlobalTypeSpecStatement(L) ::= rInnerStatement(R).                                {L = z::c2f(pctx).aGlobalStatement(z::ref(R));}

//-------------------------------------------------
// access specifiers
%type rAccessType {z::Ast::AccessType::T}
rAccessType(L) ::= PRIVATE.   {L = z::Ast::AccessType::Private;}
rAccessType(L) ::= PUBLIC.    {L = z::Ast::AccessType::Public;}
rAccessType(L) ::= INTERNAL.  {L = z::Ast::AccessType::Internal;}
rAccessType(L) ::= EXTERNAL.  {L = z::Ast::AccessType::External;}

//-------------------------------------------------
// coercion statements
rGlobalCoerceStatement ::= COERCE rCoerceList(T) SEMI. {z::c2f(pctx).aGlobalCoerceStatement(z::ref(T));}

%type rCoerceList {z::Ast::CoerceList*}
rCoerceList(L) ::= rCoerceList(R) LINK rTypeSpec(T). {L = z::c2f(pctx).aCoerceList(z::ref(R), z::ref(T));}
rCoerceList(L) ::=                     rTypeSpec(T). {L = z::c2f(pctx).aCoerceList(z::ref(T));}

//-------------------------------------------------
// default values for types
rGlobalDefaultStatement ::= DEFAULT rTypeSpec(T) ASSIGNEQUAL rExpr(E) SEMI. {z::c2f(pctx).aGlobalDefaultStatement(z::ref(T), z::ref(E));}

//-------------------------------------------------
// definition specifiers
%type rDefinitionType {z::Ast::DefinitionType::T}
rDefinitionType(L) ::= NATIVE. {L = z::Ast::DefinitionType::Native;}
rDefinitionType(L) ::= FINAL.  {L = z::Ast::DefinitionType::Final;}
rDefinitionType(L) ::= .       {L = z::Ast::DefinitionType::Final;}

//-------------------------------------------------
// definition specifiers
%type rExDefinitionType {z::Ast::DefinitionType::T}
rExDefinitionType(L) ::= ABSTRACT. {L = z::Ast::DefinitionType::Abstract;}
rExDefinitionType(L) ::= rDefinitionType(R). {L = R;}

//-------------------------------------------------
// definition specifiers
%type rAbstractDefinitionType {z::Ast::DefinitionType::T}
rAbstractDefinitionType(L) ::= ABSTRACT. {L = z::Ast::DefinitionType::Abstract;}
rAbstractDefinitionType(L) ::= . {L = z::Ast::DefinitionType::Abstract;}

//-------------------------------------------------
// all type def's.
%type rUserDefinedTypeSpecDef {z::Ast::UserDefinedTypeSpec*}
rUserDefinedTypeSpecDef(L) ::= rTypeSpecDef(R).    {L = R;}
rUserDefinedTypeSpecDef(L) ::= rRootStructDefn(R). {L = R;}
rUserDefinedTypeSpecDef(L) ::= rStructDecl(R).     {L = R;}

//-------------------------------------------------
// types that can be defined globally but are not permitted within struct's
%type rTypeSpecDef {z::Ast::UserDefinedTypeSpec*}
rTypeSpecDef(L) ::= rBasicTypeSpecDef(R).  {L = R;}
rTypeSpecDef(L) ::= rRoutineDecl(R).       {L = R;}
rTypeSpecDef(L) ::= rRoutineDefn(R).       {L = R;}
rTypeSpecDef(L) ::= rRootFunctionDecl(R).  {L = R;}
rTypeSpecDef(L) ::= rChildFunctionDecl(R). {L = R;}
rTypeSpecDef(L) ::= rRootFunctionDefn(R).  {L = R;}
rTypeSpecDef(L) ::= rChildFunctionDefn(R). {L = R;}
rTypeSpecDef(L) ::= rRootInterfaceDefn(R). {L = R;}
rTypeSpecDef(L) ::= rEventDecl(R).         {L = R;}

//-------------------------------------------------
// types that are permitted within struct's
%type rStructTypeSpecDef {z::Ast::UserDefinedTypeSpec*}
rStructTypeSpecDef(L) ::= rBasicTypeSpecDef(R). {L = R;}
rStructTypeSpecDef(L) ::= rRootStructDefn(R).   {L = R;}
rStructTypeSpecDef(L) ::= rStructDecl(R).       {L = R;}

//-------------------------------------------------
// basic types common to rTypeSpecDef and rStructTypeSpecDef
%type rBasicTypeSpecDef {z::Ast::UserDefinedTypeSpec*}
rBasicTypeSpecDef(L) ::= rTypedefDecl(R).       {L = R;}
rBasicTypeSpecDef(L) ::= rTypedefDefn(R).       {L = R;}
rBasicTypeSpecDef(L) ::= rTemplateDecl(R).      {L = R;}
rBasicTypeSpecDef(L) ::= rEnumDecl(R).          {L = R;}
rBasicTypeSpecDef(L) ::= rEnumDefn(R).          {L = R;}
rBasicTypeSpecDef(L) ::= rChildStructDefn(R).   {L = R;}

//-------------------------------------------------
// typedef declaration
%type rTypedefDecl {z::Ast::TypedefDecl*}
rTypedefDecl(L) ::= rPreTypedefDecl(R) SEMI. {L = R;}

//-------------------------------------------------
// this pre- mechanism is required to force the action to get executed before the end-of-line.
%type rPreTypedefDecl {z::Ast::TypedefDecl*}
rPreTypedefDecl(L) ::= TYPEDEF ID(name) rDefinitionType(D). {L = z::c2f(pctx).aTypedefDecl(z::t2t(name), D);}

//-------------------------------------------------
// typedef definition
%type rTypedefDefn {z::Ast::TypedefDefn*}
rTypedefDefn(L) ::= rPreTypedefDefn(R) SEMI. {L = R;}

%type rPreTypedefDefn {z::Ast::TypedefDefn*}
rPreTypedefDefn(L) ::= TYPEDEF ID(name) rQualifiedTypeSpec(Q) rDefinitionType(D). {L = z::c2f(pctx).aTypedefDefn(z::t2t(name), D, z::ref(Q));}

//-------------------------------------------------
// template declarations
%type rTemplateDecl {z::Ast::TemplateDecl*}
rTemplateDecl(L) ::= rPreTemplateDecl(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreTemplateDecl {z::Ast::TemplateDecl*}
rPreTemplateDecl(L) ::= TEMPLATE LT rTemplatePartList(list) GT ID(name) rDefinitionType(D). {L = z::c2f(pctx).aTemplateDecl(z::t2t(name), D, z::ref(list));}

//-------------------------------------------------
%type rTemplatePartList {z::Ast::TemplatePartList*}
rTemplatePartList(L) ::= rTemplatePartList(R) COMMA ID(name). {L = z::c2f(pctx).aTemplatePartList(z::ref(R), z::t2t(name));}
rTemplatePartList(L) ::=                            ID(name). {L = z::c2f(pctx).aTemplatePartList(z::t2t(name));}

//-------------------------------------------------
// enum declaration
%type rEnumDecl {z::Ast::EnumDecl*}
rEnumDecl(L) ::= ENUM ID(name) rDefinitionType(D) SEMI. {L = z::c2f(pctx).aEnumDecl(z::t2t(name), D);}

//-------------------------------------------------
// enum definition
%type rEnumDefn {z::Ast::EnumDefn*}
rEnumDefn(L) ::= ENUM ID(name) rDefinitionType(D) rEnumMemberDefnList(list) RCURLY SEMI. {L = z::c2f(pctx).aEnumDefn(z::t2t(name), D, z::ref(list));}

//-------------------------------------------------
%type rEnumMemberDefnList {z::Ast::Scope*}
rEnumMemberDefnList(L) ::= rEnumMemberDefnList(list) rEnumMemberDefn(enumMemberDef). {L = z::c2f(pctx).aEnumMemberDefnList(z::ref(list), z::ref(enumMemberDef));}
rEnumMemberDefnList(L) ::= LCURLY(B).                                                {L = z::c2f(pctx).aEnumMemberDefnListEmpty(z::t2t(B));}

//-------------------------------------------------
%type rEnumMemberDefn {z::Ast::VariableDefn*}
rEnumMemberDefn(L) ::= ID(name)                      SEMI. {L = z::c2f(pctx).aEnumMemberDefn(z::t2t(name));}
rEnumMemberDefn(L) ::= ID(name) ASSIGNEQUAL rExpr(I) SEMI. {L = z::c2f(pctx).aEnumMemberDefn(z::t2t(name), z::ref(I));}

//-------------------------------------------------
// struct declarations
%type rStructDecl {z::Ast::StructDecl*}
rStructDecl(L) ::= rPreStructDecl(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreStructDecl {z::Ast::StructDecl*}
rPreStructDecl(L) ::= STRUCT rStructId(name) rDefinitionType(D). {L = z::c2f(pctx).aStructDecl(z::t2t(name), D);}

//-------------------------------------------------
// root struct definitions
%type rRootStructDefn {z::Ast::RootStructDefn*}
rRootStructDefn(L) ::= rPreRootStructDefn(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreRootStructDefn {z::Ast::RootStructDefn*}
rPreRootStructDefn(L) ::= rEnterRootStructDefn(S) rStructMemberDefnBlock. {L = z::c2f(pctx).aLeaveRootStructDefn(z::ref(S));}

//-------------------------------------------------
%type rEnterRootStructDefn {z::Ast::RootStructDefn*}
rEnterRootStructDefn(L) ::= STRUCT rStructId(name) rExDefinitionType(D). {L = z::c2f(pctx).aEnterRootStructDefn(z::t2t(name), D);}

//-------------------------------------------------
// child struct definitions
%type rChildStructDefn {z::Ast::ChildStructDefn*}
rChildStructDefn(L) ::= rPreChildStructDefn(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreChildStructDefn {z::Ast::ChildStructDefn*}
rPreChildStructDefn(L) ::= rEnterChildStructDefn(S) rStructMemberDefnBlock. {L = z::c2f(pctx).aLeaveChildStructDefn(z::ref(S));}

//-------------------------------------------------
%type rEnterChildStructDefn {z::Ast::ChildStructDefn*}
rEnterChildStructDefn(L) ::= STRUCT rStructId(name) COLON rStructTypeSpec(B) rExDefinitionType(D). {L = z::c2f(pctx).aEnterChildStructDefn(z::t2t(name), z::ref(B), D);}

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
rStructMemberDefn ::= rVariableDefn(R) SEMI.  {z::c2f(pctx).aStructMemberVariableDefn(z::ref(R));}
rStructMemberDefn ::= rStructTypeSpecDef(R).  {z::c2f(pctx).aStructMemberTypeDefn(z::ref(R));}
rStructMemberDefn ::= rStructPropertyDecl(R). {z::c2f(pctx).aStructMemberPropertyDefn(z::ref(R));}

//-------------------------------------------------
// struct index declarations
%type rStructPropertyDecl {z::Ast::PropertyDecl*}
rStructPropertyDecl(L) ::= PROPERTY(B) rQualifiedTypeSpec(T) ID(N) rDefinitionType(D) GET SET SEMI. {L = z::c2f(pctx).aStructPropertyDeclRW(z::t2t(B), z::ref(T), z::t2t(N), D);}
rStructPropertyDecl(L) ::= PROPERTY(B) rQualifiedTypeSpec(T) ID(N) rDefinitionType(D) GET     SEMI. {L = z::c2f(pctx).aStructPropertyDeclRO(z::t2t(B), z::ref(T), z::t2t(N), D);}

//-------------------------------------------------
// routine declarations
%type rRoutineDecl {z::Ast::RoutineDecl*}
rRoutineDecl(L) ::= rPreRoutineDecl(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreRoutineDecl {z::Ast::RoutineDecl*}
rPreRoutineDecl(L) ::= ROUTINE rQualifiedTypeSpec(out) rRoutineId(name) rInParamsList(in) rDefinitionType(D). {L = z::c2f(pctx).aRoutineDecl(z::ref(out), z::t2t(name), z::ref(in), D);}
rPreRoutineDecl(L) ::= ROUTINE rQualifiedTypeSpec(out) rRoutineId(name) LBRACKET ELIPSIS RBRACKET NATIVE. {L = z::c2f(pctx).aVarArgRoutineDecl(z::ref(out), z::t2t(name), z::Ast::DefinitionType::Native);}

//-------------------------------------------------
// routine definition
%type rRoutineDefn {z::Ast::RoutineDefn*}
rRoutineDefn(L) ::= rPreRoutineDefn(R). {L = R;}

//-------------------------------------------------
%type rPreRoutineDefn {z::Ast::RoutineDefn*}
rPreRoutineDefn(L) ::= rEnterRoutineDefn(routineDefn) rCompoundStatement(block). {L = z::c2f(pctx).aRoutineDefn(z::ref(routineDefn), z::ref(block));}

//-------------------------------------------------
%type rEnterRoutineDefn {z::Ast::RoutineDefn*}
rEnterRoutineDefn(L) ::= ROUTINE rQualifiedTypeSpec(out) rRoutineId(name) rInParamsList(in) rDefinitionType(D). {L = z::c2f(pctx).aEnterRoutineDefn(z::ref(out), z::t2t(name), z::ref(in), D);}

rRoutineId(L) ::= ID(R). {L = R;}
rRoutineId(L) ::= ROUTINE_TYPE(R). {L = R;}

//-------------------------------------------------
// root function definition
%type rRootFunctionDecl {z::Ast::RootFunctionDecl*}
rRootFunctionDecl(L) ::= rFunctionSig(functionSig) rClosureList(cref) rExDefinitionType(defType) SEMI. {L = z::c2f(pctx).aRootFunctionDecl(z::ref(functionSig), defType, cref);}

//-------------------------------------------------
// child function definition
%type rChildFunctionDecl {z::Ast::ChildFunctionDecl*}
rChildFunctionDecl(L) ::= FUNCTION ID(name) COLON rFunctionTypeSpec(base) rClosureList(cref) rExDefinitionType(defType) SEMI. {L = z::c2f(pctx).aChildFunctionDecl(z::ref(base), z::t2t(name), defType, cref);}

//-------------------------------------------------
// root function declarations
%type rRootFunctionDefn {z::Ast::RootFunctionDefn*}
rRootFunctionDefn(L) ::= rEnterRootFunctionDefn(functionDefn) rFunctionBlock(block). {L = z::c2f(pctx).aRootFunctionDefn(z::ref(functionDefn), z::ref(block));}

//-------------------------------------------------
%type rEnterRootFunctionDefn {z::Ast::RootFunctionDefn*}
rEnterRootFunctionDefn(L) ::= rFunctionSig(functionSig) rClosureList(cref) rExDefinitionType(defType). {L = z::c2f(pctx).aEnterRootFunctionDefn(z::ref(functionSig), defType, cref);}

//-------------------------------------------------
// child function declaration
%type rChildFunctionDefn {z::Ast::ChildFunctionDefn*}
rChildFunctionDefn(L) ::= rEnterChildFunctionDefn(functionImpl) rFunctionBlock(block). {L = z::c2f(pctx).aChildFunctionDefn(z::ref(functionImpl), z::ref(block));}

//-------------------------------------------------
%type rEnterChildFunctionDefn {z::Ast::ChildFunctionDefn*}
rEnterChildFunctionDefn(L) ::= FUNCTION ID(name) COLON rFunctionTypeSpec(base) rClosureList(cref) rExDefinitionType(defType). {L = z::c2f(pctx).aEnterChildFunctionDefn(z::ref(base), z::t2t(name), defType, cref);}

//-------------------------------------------------
// interface definition
%type rRootInterfaceDefn {z::Ast::RootInterfaceDefn*}
rRootInterfaceDefn(L) ::= rPreRootInterfaceDefn(R) SEMI. {L = R;}

//-------------------------------------------------
%type rPreRootInterfaceDefn {z::Ast::RootInterfaceDefn*}
rPreRootInterfaceDefn(L) ::= rEnterRootInterfaceDefn(S) rInterfaceMemberDefnBlock. {L = z::c2f(pctx).aLeaveRootInterfaceDefn(z::ref(S));}

//-------------------------------------------------
%type rEnterRootInterfaceDefn {z::Ast::RootInterfaceDefn*}
rEnterRootInterfaceDefn(L) ::= INTERFACE ID(name) rExDefinitionType(D). {L = z::c2f(pctx).aEnterRootInterfaceDefn(z::t2t(name), D);}

//-------------------------------------------------
rInterfaceMemberDefnBlock ::= LCURLY rInterfaceMemberDefnList RCURLY.
rInterfaceMemberDefnBlock ::= LCURLY                          RCURLY.

//-------------------------------------------------
rInterfaceMemberDefnList ::= rInterfaceMemberDefnList rInterfaceMemberDefn.
rInterfaceMemberDefnList ::=                          rInterfaceMemberDefn.

//-------------------------------------------------
rInterfaceMemberDefn ::= rUserDefinedTypeSpecDef(R). {z::c2f(pctx).aInterfaceMemberTypeDefn(z::ref(R));}


//-------------------------------------------------
// event declarations
%type rEventDecl {z::Ast::EventDecl*}
rEventDecl(L) ::= EVENT(B) LBRACKET rVariableDefn(in) RBRACKET rDefinitionType(ED) LINK rFunctionSig(functionSig) rAbstractDefinitionType(HD) SEMI. {L = z::c2f(pctx).aEventDecl(z::t2t(B), z::ref(in), ED, z::ref(functionSig), HD);}

//-------------------------------------------------
// function signature.
%type rFunctionSig {z::Ast::FunctionSig*}
rFunctionSig(T) ::= FUNCTION rParamsList(out)        ID(name) rInParamsList(in). {T = z::c2f(pctx).aFunctionSig(z::ref(out), z::t2t(name), z::ref(in));}
rFunctionSig(T) ::= FUNCTION rQualifiedTypeSpec(out) ID(name) rInParamsList(in). {T = z::c2f(pctx).aFunctionSig(z::ref(out), z::t2t(name), z::ref(in));}

//-------------------------------------------------
// in parameter list
%type rClosureList {z::Ast::ClosureRef}
rClosureList(L) ::= LSQUARE rXClosure(R) SEMI rIClosure(I) RSQUARE. {L = z::c2f(pctx).aClosureList(z::ref(R), z::ref(I));}
rClosureList(L) ::= LSQUARE rXClosure(R)                   RSQUARE. {L = z::c2f(pctx).aClosureList(z::ref(R));}
rClosureList(L) ::= .                                               {L = z::c2f(pctx).aClosureList();}

//-------------------------------------------------
// variable lists
%type rXClosure {z::Ast::Scope*}
rXClosure(L) ::= rXClosure(list) COMMA rVariableDefn(variableDef). {L = z::c2f(pctx).aParam(z::ref(list), z::ref(variableDef));}
rXClosure(L) ::=                       rVariableDefn(variableDef). {L = z::c2f(pctx).aParam(z::ref(variableDef), z::Ast::ScopeType::XRef);}
rXClosure(L) ::= .                                                 {L = z::c2f(pctx).aParam(z::Ast::ScopeType::XRef);}

%type rIClosure {z::Ast::Scope*}
rIClosure(L) ::= rIClosure(list) COMMA rVariableDefn(variableDef). {L = z::c2f(pctx).aParam(z::ref(list), z::ref(variableDef));}
rIClosure(L) ::=                       rVariableDefn(variableDef). {L = z::c2f(pctx).aParam(z::ref(variableDef), z::Ast::ScopeType::IRef);}
rIClosure(L) ::= .                                                 {L = z::c2f(pctx).aParam(z::Ast::ScopeType::IRef);}

//-------------------------------------------------
// in parameter list
%type rInParamsList {z::Ast::Scope*}
rInParamsList(L) ::= rParamsList(scope). {L = z::c2f(pctx).aInParamsList(z::ref(scope));}

//-------------------------------------------------
// parameter lists
%type rParamsList {z::Ast::Scope*}
rParamsList(L) ::= LBRACKET rParam(R)                    RBRACKET. {L = z::c2f(pctx).aParamsList(z::ref(R));}
//rParamsList(L) ::= LBRACKET rParam(R) COMMA rPosParam(P) RBRACKET. {L = z::c2f(pctx).aParamsList(z::ref(R), z::ref(P));}

//-------------------------------------------------
// variable lists
%type rParam {z::Ast::Scope*}
rParam(L) ::= rParam(list) COMMA rVariableDefn(variableDef). {L = z::c2f(pctx).aParam(z::ref(list), z::ref(variableDef));}
rParam(L) ::=                    rVariableDefn(variableDef). {L = z::c2f(pctx).aParam(z::ref(variableDef), z::Ast::ScopeType::Param);}
rParam(L) ::= .                                              {L = z::c2f(pctx).aParam(z::Ast::ScopeType::Param);}

//-------------------------------------------------
// positional parameter list
//%type rPosParam {z::Ast::Scope*}
//rPosParam(L) ::= rPosParam(list) COMMA rPosVariableDefn(variableDef). {L = z::c2f(pctx).aParam(z::ref(list), z::ref(variableDef));}
//rPosParam(L) ::=                       rPosVariableDefn(variableDef). {L = z::c2f(pctx).aParam(z::ref(variableDef), z::Ast::ScopeType::Param);}

//-------------------------------------------------
// positional variable def
//%type rPosVariableDefn {const z::Ast::VariableDefn*}
//rPosVariableDefn(L) ::= rAutoQualifiedVariableDefn       ID(name) COLON rExpr(initExpr). {L = z::c2f(pctx).aVariableDefn(name, z::ref(initExpr));}
//rPosVariableDefn(L) ::= rQualifiedVariableDefn(qTypeRef) ID(name) COLON rExpr(initExpr). {L = z::c2f(pctx).aVariableDefn(z::ref(qTypeRef), name, z::ref(initExpr));}

//-------------------------------------------------
// variable def
%type rVariableDefn {const z::Ast::VariableDefn*}
rVariableDefn(L) ::= rAutoQualifiedVariableDefn       ID(name) ASSIGNEQUAL rExpr(initExpr). {L = z::c2f(pctx).aVariableDefn(z::t2t(name), z::ref(initExpr));}
rVariableDefn(L) ::= rQualifiedVariableDefn(qTypeRef) ID(name).                             {L = z::c2f(pctx).aVariableDefn(z::ref(qTypeRef), z::t2t(name));}
rVariableDefn(L) ::= rQualifiedVariableDefn(qTypeRef) ID(name) ASSIGNEQUAL rExpr(initExpr). {L = z::c2f(pctx).aVariableDefn(z::ref(qTypeRef), z::t2t(name), z::ref(initExpr));}

//-------------------------------------------------
// qualified variable def
%type rQualifiedVariableDefn {const z::Ast::QualifiedTypeSpec*}
rQualifiedVariableDefn(L) ::= rQualifiedTypeSpec(R). {L = z::c2f(pctx).aQualifiedVariableDefn(z::ref(R));}

//-------------------------------------------------
// auto qualified variable def
rAutoQualifiedVariableDefn ::= AUTO. {z::c2f(pctx).aAutoQualifiedVariableDefn();}

//-------------------------------------------------
// qualified types
%type rQualifiedTypeSpec {const z::Ast::QualifiedTypeSpec*}
rQualifiedTypeSpec(L) ::=          rTypeSpec(typeSpec).               {L = z::c2f(pctx).aQualifiedTypeSpec(           false, z::ref(typeSpec), false, false);}
rQualifiedTypeSpec(L) ::=          rTypeSpec(typeSpec) BITWISEAND(B). {L = z::c2f(pctx).aQualifiedTypeSpec(z::t2t(B), false, z::ref(typeSpec), true , false);}
rQualifiedTypeSpec(L) ::=          rTypeSpec(typeSpec) AND(B).        {L = z::c2f(pctx).aQualifiedTypeSpec(z::t2t(B), false, z::ref(typeSpec), true , true );}
rQualifiedTypeSpec(L) ::= CONST(B) rTypeSpec(typeSpec).               {L = z::c2f(pctx).aQualifiedTypeSpec(z::t2t(B), true,  z::ref(typeSpec), false, false);}
rQualifiedTypeSpec(L) ::= CONST(B) rTypeSpec(typeSpec) BITWISEAND.    {L = z::c2f(pctx).aQualifiedTypeSpec(z::t2t(B), true,  z::ref(typeSpec), true , false);}
rQualifiedTypeSpec(L) ::= CONST(B) rTypeSpec(typeSpec) AND.           {L = z::c2f(pctx).aQualifiedTypeSpec(z::t2t(B), true,  z::ref(typeSpec), true , true );}

//-------------------------------------------------
// "public" type references, can be invoked from other rules
%type rTypeSpec {const z::Ast::TypeSpec*}
rTypeSpec(L) ::= rPreTypeSpec(R). {L = z::c2f(pctx).aTypeSpec(z::ref(R));}
rTypeSpec(L) ::= rTemplateDefnTypeSpec(R). {L = R;}

//-------------------------------------------------
%type rTemplateDefnTypeSpec {const z::Ast::TemplateDefn*}
rTemplateDefnTypeSpec(L) ::= rTemplateTypeSpec(R) TLT rTemplateTypePartList(P) GT. {L = z::c2f(pctx).aTemplateDefnTypeSpec(z::ref(R), z::ref(P));}

//-------------------------------------------------
%type rTemplateTypePartList {z::Ast::TemplateTypePartList*}
rTemplateTypePartList(L) ::= rTemplateTypePartList(R) COMMA rQualifiedTypeSpec(P). {L = z::c2f(pctx).aTemplateTypePartList(z::ref(R), z::ref(P));}
rTemplateTypePartList(L) ::=                                rQualifiedTypeSpec(P). {L = z::c2f(pctx).aTemplateTypePartList(z::ref(P));}

//-------------------------------------------------
%type rTemplateTypeSpec {const z::Ast::TemplateDecl*}
rTemplateTypeSpec(L) ::= rPreTemplateTypeSpec(R). {L = z::c2f(pctx).aTemplateTypeSpec(z::ref(R));}

//-------------------------------------------------
%type rStructTypeSpec {const z::Ast::StructDefn*}
rStructTypeSpec(L) ::= rPreStructTypeSpec(R). {L = z::c2f(pctx).aStructTypeSpec(z::ref(R));}

//-------------------------------------------------
%type rRoutineTypeSpec {const z::Ast::Routine*}
rRoutineTypeSpec(L) ::= rPreRoutineTypeSpec(R). {L = z::c2f(pctx).aRoutineTypeSpec(z::ref(R));}

//-------------------------------------------------
%type rFunctionTypeSpec {const z::Ast::Function*}
rFunctionTypeSpec(L) ::= rPreFunctionTypeSpec(R). {L = z::c2f(pctx).aFunctionTypeSpec(z::ref(R));}

//-------------------------------------------------
%type rEventTypeSpec {const z::Ast::EventDecl*}
rEventTypeSpec(L) ::= rPreEventTypeSpec(R). {L = z::c2f(pctx).aEventTypeSpec(z::ref(R));}

//-------------------------------------------------
// "private" type references, can be only called by the public equivalent rules
%type rPreTypeSpec {const z::Ast::TypeSpec*}
rPreTypeSpec(L) ::= rPreTemplateTypeSpec(R). {L = R;}
rPreTypeSpec(L) ::= rPreStructTypeSpec(R).   {L = R;}
rPreTypeSpec(L) ::= rPreRoutineTypeSpec(R).  {L = R;}
rPreTypeSpec(L) ::= rPreFunctionTypeSpec(R). {L = R;}
rPreTypeSpec(L) ::= rPreEventTypeSpec(R).    {L = R;}
rPreTypeSpec(L) ::= rPreOtherTypeSpec(R).    {L = R;}

//-------------------------------------------------
%type rPreTemplateTypeSpec {const z::Ast::TemplateDecl*}
rPreTemplateTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE TEMPLATE_TYPE(name). {L = z::c2f(pctx).aTemplateTypeSpec(z::ref(parent), z::t2t(name));}
rPreTemplateTypeSpec(L) ::=                            TEMPLATE_TYPE(name). {L = z::c2f(pctx).aTemplateTypeSpec(z::t2t(name));}

//-------------------------------------------------
%type rPreStructTypeSpec {const z::Ast::StructDefn*}
rPreStructTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE STRUCT_TYPE(name). {L = z::c2f(pctx).aStructTypeSpec(z::ref(parent), z::t2t(name));}
rPreStructTypeSpec(L) ::=                            STRUCT_TYPE(name). {L = z::c2f(pctx).aStructTypeSpec(z::t2t(name));}

//-------------------------------------------------
%type rPreRoutineTypeSpec {const z::Ast::Routine*}
rPreRoutineTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE ROUTINE_TYPE(name). {L = z::c2f(pctx).aRoutineTypeSpec(z::ref(parent), z::t2t(name));}
rPreRoutineTypeSpec(L) ::=                            ROUTINE_TYPE(name). {L = z::c2f(pctx).aRoutineTypeSpec(z::t2t(name));}

//-------------------------------------------------
%type rPreFunctionTypeSpec {const z::Ast::Function*}
rPreFunctionTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE FUNCTION_TYPE(name). {L = z::c2f(pctx).aFunctionTypeSpec(z::ref(parent), z::t2t(name));}
rPreFunctionTypeSpec(L) ::=                            FUNCTION_TYPE(name). {L = z::c2f(pctx).aFunctionTypeSpec(z::t2t(name));}

//-------------------------------------------------
%type rPreEventTypeSpec {const z::Ast::EventDecl*}
rPreEventTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE EVENT_TYPE(name). {L = z::c2f(pctx).aEventTypeSpec(z::ref(parent), z::t2t(name));}
rPreEventTypeSpec(L) ::=                            EVENT_TYPE(name). {L = z::c2f(pctx).aEventTypeSpec(z::t2t(name));}

//-------------------------------------------------
%type rPreOtherTypeSpec {const z::Ast::TypeSpec*}
rPreOtherTypeSpec(L) ::= rPreTypeSpec(parent) SCOPE OTHER_TYPE(name). {L = z::c2f(pctx).aOtherTypeSpec(z::ref(parent), z::t2t(name));}
rPreOtherTypeSpec(L) ::=                            OTHER_TYPE(name). {L = z::c2f(pctx).aOtherTypeSpec(z::t2t(name));}

//-------------------------------------------------
// statements
%type rInnerStatement {z::Ast::Statement*}
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
rInnerStatement(L) ::= rRaiseStatement(R).               {L = R;}
rInnerStatement(L) ::= rExitStatement(R).                {L = R;}
rInnerStatement(L) ::= rCompoundStatement(R).            {L = R;}

//-------------------------------------------------
%type rUserDefinedTypeSpecStatement {z::Ast::UserDefinedTypeSpecStatement*}
rUserDefinedTypeSpecStatement(L) ::= rUserDefinedTypeSpecDef(typeSpec). {L = z::c2f(pctx).aUserDefinedTypeSpecStatement(z::ref(typeSpec));}

//-------------------------------------------------
%type rEmptyStatement {z::Ast::EmptyStatement*}
rEmptyStatement(L) ::= SEMI(B). {L = z::c2f(pctx).aEmptyStatement(z::t2t(B));}

//-------------------------------------------------
%type rAutoStatement {z::Ast::AutoStatement*}
rAutoStatement(L) ::= rVariableDefn(defn) SEMI. {L = z::c2f(pctx).aAutoStatement(z::ref(defn));}

//-------------------------------------------------
%type rExprStatement {z::Ast::ExprStatement*}
rExprStatement(L) ::= rExpr(expr) SEMI. {L = z::c2f(pctx).aExprStatement(z::ref(expr));}

//-------------------------------------------------
%type rPrintStatement {z::Ast::PrintStatement*}
rPrintStatement(L) ::= PRINT(B) rExpr(expr) SEMI. {L = z::c2f(pctx).aPrintStatement(z::t2t(B), z::ref(expr));}

//-------------------------------------------------
%type rIfStatement {z::Ast::IfStatement*}
rIfStatement(L) ::= IF(B) LBRACKET rExpr(expr) RBRACKET rCompoundStatement(tblock). {L = z::c2f(pctx).aIfStatement(z::t2t(B), z::ref(expr), z::ref(tblock));}

//-------------------------------------------------
%type rIfElseStatement {z::Ast::IfElseStatement*}
rIfElseStatement(L) ::= IF(B) LBRACKET rExpr(expr) RBRACKET rCompoundStatement(tblock) ELSE rCompoundStatement(fblock). {L = z::c2f(pctx).aIfElseStatement(z::t2t(B), z::ref(expr), z::ref(tblock), z::ref(fblock));}

//-------------------------------------------------
%type rWhileStatement {z::Ast::WhileStatement*}
rWhileStatement(L) ::= WHILE(B) LBRACKET rExpr(expr) RBRACKET rCompoundStatement(block). {L = z::c2f(pctx).aWhileStatement(z::t2t(B), z::ref(expr), z::ref(block));}

//-------------------------------------------------
%type rDoWhileStatement {z::Ast::DoWhileStatement*}
rDoWhileStatement(L) ::= DO(B) rCompoundStatement(block) WHILE LBRACKET rExpr(expr) RBRACKET SEMI. {L = z::c2f(pctx).aDoWhileStatement(z::t2t(B), z::ref(expr), z::ref(block));}

//-------------------------------------------------
%type rForStatement {z::Ast::ForStatement*}
rForStatement(L) ::= FOR(B) LBRACKET rExpr(init) SEMI rExpr(expr) SEMI rExpr(incr) RBRACKET rCompoundStatement(block). {L = z::c2f(pctx).aForStatement(z::t2t(B), z::ref(init), z::ref(expr), z::ref(incr), z::ref(block));}
rForStatement(L) ::= FOR(B) LBRACKET rEnterForInit(init) SEMI rExpr(expr) SEMI rExpr(incr) RBRACKET rCompoundStatement(block). {L = z::c2f(pctx).aForStatement(z::t2t(B), z::ref(init), z::ref(expr), z::ref(incr), z::ref(block));}

%type rEnterForInit {const z::Ast::VariableDefn*}
rEnterForInit(L) ::= rVariableDefn(init). {L = z::c2f(pctx).aEnterForInit(z::ref(init));}

//-------------------------------------------------
%type rForeachStatement {z::Ast::ForeachStatement*}
rForeachStatement(L) ::= FOREACH LBRACKET rEnterForeachInit(vdef) RBRACKET rCompoundStatement(block). {L = z::c2f(pctx).aForeachStatement(z::ref(vdef), z::ref(block));}

%type rEnterForeachInit {z::Ast::ForeachStatement*}
rEnterForeachInit(L) ::= ID(I) IN rExpr(list). {L = z::c2f(pctx).aEnterForeachInit(z::t2t(I), z::ref(list));}
rEnterForeachInit(L) ::= ID(K) COMMA ID(V) IN rExpr(list). {L = z::c2f(pctx).aEnterForeachInit(z::t2t(K), z::t2t(V), z::ref(list));}

//-------------------------------------------------
%type rSwitchStatement {z::Ast::SwitchStatement*}
rSwitchStatement(L) ::= SWITCH(B) LBRACKET rExpr(expr) RBRACKET LCURLY rCaseList(list) RCURLY. {L = z::c2f(pctx).aSwitchStatement(z::t2t(B), z::ref(expr), z::ref(list));}
rSwitchStatement(L) ::= SWITCH(B)                               LCURLY rCaseList(list) RCURLY. {L = z::c2f(pctx).aSwitchStatement(z::t2t(B), z::ref(list));}

//-------------------------------------------------
%type rCaseList {z::Ast::CompoundStatement*}
rCaseList(L) ::= rCaseList(R) rCaseStatement(S). {L = z::c2f(pctx).aCaseList(z::ref(R), z::ref(S));}
rCaseList(L) ::=              rCaseStatement(S). {L = z::c2f(pctx).aCaseList(z::ref(S));}

//-------------------------------------------------
%type rCaseStatement {z::Ast::CaseStatement*}
rCaseStatement(L) ::= CASE(B) rExpr(expr) COLON rCompoundStatement(block). {L = z::c2f(pctx).aCaseStatement(z::t2t(B), z::ref(expr), z::ref(block));}
rCaseStatement(L) ::= DEFAULT(B)          COLON rCompoundStatement(block). {L = z::c2f(pctx).aCaseStatement(z::t2t(B), z::ref(block));}

//-------------------------------------------------
%type rBreakStatement {z::Ast::BreakStatement*}
rBreakStatement(L) ::= BREAK(B) SEMI. {L = z::c2f(pctx).aBreakStatement(z::t2t(B));}

//-------------------------------------------------
%type rContinueStatement {z::Ast::ContinueStatement*}
rContinueStatement(L) ::= CONTINUE(B) SEMI. {L = z::c2f(pctx).aContinueStatement(z::t2t(B));}

//-------------------------------------------------
%type rAddEventHandlerStatement {z::Ast::AddEventHandlerStatement*}
rAddEventHandlerStatement(L) ::= rEnterAddEventHandler(E) LBRACKET(B) rExpr(X) RBRACKET rAnonymousFunction(F) SEMI. {L = z::c2f(pctx).aAddEventHandlerStatement(z::t2t(B), z::ref(E), z::ref(X), z::ref(F));}

//-------------------------------------------------
// anonymous function instance
%type rAnonymousFunction {z::Ast::AnonymousFunctionExpr*}
rAnonymousFunction(L) ::= rEnterAnonymousFunction(R) rFunctionBlock(C). {L = z::c2f(pctx).aAnonymousFunctionExpr(z::ref(R), z::ref(C));}

//-------------------------------------------------
%type rEnterAnonymousFunction {z::Ast::ChildFunctionDefn*}
rEnterAnonymousFunction(L) ::= LINK rClosureList(C) rFunctionTypeSpec(R). {L = z::c2f(pctx).aEnterAnonymousFunction(z::ref(R), C);}
rEnterAnonymousFunction(L) ::= LINK rClosureList(C) FUNCTION(R). {L = z::c2f(pctx).aEnterAutoAnonymousFunction(z::t2t(R), C);}

//-------------------------------------------------
%type rEnterAddEventHandler {const z::Ast::EventDecl*}
rEnterAddEventHandler(L) ::= rEventTypeSpec(R). {L = z::c2f(pctx).aEnterAddEventHandler(z::ref(R));}

//-------------------------------------------------
%type rRoutineReturnStatement {z::Ast::RoutineReturnStatement*}
rRoutineReturnStatement(L) ::= RRETURN(B)          SEMI. {L = z::c2f(pctx).aRoutineReturnStatement(z::t2t(B));}
rRoutineReturnStatement(L) ::= RRETURN(B) rExpr(S) SEMI. {L = z::c2f(pctx).aRoutineReturnStatement(z::t2t(B), z::ref(S));}

//-------------------------------------------------
%type rFunctionReturnStatement {z::Ast::FunctionReturnStatement*}
rFunctionReturnStatement(L) ::= FRETURN(B) rExprsList(S) SEMI. {L = z::c2f(pctx).aFunctionReturnStatement(z::t2t(B), z::ref(S));}

//-------------------------------------------------
%type rRaiseStatement {z::Ast::RaiseStatement*}
rRaiseStatement(L) ::= RAISE(B) rEventTypeSpec(E) LBRACKET rExpr(S) COLON rExprList(X) RBRACKET SEMI. {L = z::c2f(pctx).aRaiseStatement(z::t2t(B), z::ref(E), z::ref(S), z::ref(X));}

//-------------------------------------------------
%type rExitStatement {z::Ast::ExitStatement*}
rExitStatement(L) ::= EXIT(B) rExpr(S) SEMI. {L = z::c2f(pctx).aExitStatement(z::t2t(B), z::ref(S));}

//-------------------------------------------------
// simple list of statements
%type rCompoundStatement {z::Ast::CompoundStatement*}
rCompoundStatement(L)   ::= rEnterCompoundStatement rStatementList(R) rLeaveCompoundStatement. {L = R;}
rEnterCompoundStatement ::= LCURLY(B). {z::c2f(pctx).aEnterCompoundStatement(z::t2t(B));}
rLeaveCompoundStatement ::= RCURLY.    {z::c2f(pctx).aLeaveCompoundStatement();}

//-------------------------------------------------
// simple list of statements
%type rFunctionBlock {z::Ast::CompoundStatement*}
rFunctionBlock(L)   ::= rEnterFunctionBlock rStatementList(R) rLeaveFunctionBlock. {L = R;}
rEnterFunctionBlock ::= LCURLY(B). {z::c2f(pctx).aEnterFunctionBlock(z::t2t(B));}
rLeaveFunctionBlock ::= RCURLY.    {z::c2f(pctx).aLeaveFunctionBlock();}

%type rStatementList {z::Ast::CompoundStatement*}
rStatementList(L) ::= rStatementList(list) rInnerStatement(statement). {L = z::c2f(pctx).aStatementList(z::ref(list), z::ref(statement));}
rStatementList(L) ::= .                                                {L = z::c2f(pctx).aStatementList();}

//-------------------------------------------------
// expression list in brackets
%type rExprsList {z::Ast::ExprList*}
rExprsList(L) ::= LBRACKET rExprList(R) RBRACKET. {L = R;}

//-------------------------------------------------
// comma-separated list of expressions
%type rExprList {z::Ast::ExprList*}
rExprList(R) ::= rExprList(L) COMMA rExpr(E). {R = z::c2f(pctx).aExprList(z::ref(L), z::ref(E));}
rExprList(R) ::=                    rExpr(E). {R = z::c2f(pctx).aExprList(z::ref(E));}
rExprList(R) ::=                            . {R = z::c2f(pctx).aExprList();}

//-------------------------------------------------
// expressions
%type rExpr {const z::Ast::Expr*}
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
//rExpr(E) ::= ID(L) DEFINEEQUAL       rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryExpr(z::ref(L), z::string("="), z::ref(R)));}

//-------------------------------------------------
// ternary operators
%type rTernaryExpr {const z::Ast::TernaryOpExpr*}
rTernaryExpr(E) ::= rExpr(L) QUESTION(O1) rExpr(T) COLON(O2) rExpr(F). {E = z::c2f(pctx).aConditionalExpr(z::t2t(O1), z::t2t(O2), z::ref(L), z::ref(T), z::ref(F));}

//-------------------------------------------------
// boolean operators
%type rBooleanExpr {const z::Ast::Expr*}
rBooleanExpr(E) ::= rExpr(L) AND(O)             rExpr(R). {E = z::ptr(z::c2f(pctx).aBooleanAndExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) OR(O)              rExpr(R). {E = z::ptr(z::c2f(pctx).aBooleanOrExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) EQUAL(O)           rExpr(R). {E = z::ptr(z::c2f(pctx).aBooleanEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) NOTEQUAL(O)        rExpr(R). {E = z::ptr(z::c2f(pctx).aBooleanNotEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) LT(O)              rExpr(R). {E = z::ptr(z::c2f(pctx).aBooleanLessThanExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) GT(O)              rExpr(R). {E = z::ptr(z::c2f(pctx).aBooleanGreaterThanExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) LTE(O)             rExpr(R). {E = z::ptr(z::c2f(pctx).aBooleanLessThanOrEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) GTE(O)             rExpr(R). {E = z::ptr(z::c2f(pctx).aBooleanGreaterThanOrEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBooleanExpr(E) ::= rExpr(L) HAS(O)             rExpr(R). {E = z::ptr(z::c2f(pctx).aBooleanHasExpr(z::t2t(O), z::ref(L), z::ref(R)));}

//-------------------------------------------------
// binary operators
%type rBinaryExpr {const z::Ast::Expr*}
rBinaryExpr(E) ::= rExpr(L) ASSIGNEQUAL(O)     rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryAssignEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) PLUSEQUAL(O)       rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryPlusEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) MINUSEQUAL(O)      rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryMinusEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) TIMESEQUAL(O)      rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryTimesEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) DIVIDEEQUAL(O)     rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryDivideEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) MODEQUAL(O)        rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryModEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEANDEQUAL(O) rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryBitwiseAndEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEOREQUAL(O)  rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryBitwiseOrEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEXOREQUAL(O) rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryBitwiseXorEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) SHIFTLEFTEQUAL(O)  rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryShiftLeftEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) SHIFTRIGHTEQUAL(O) rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryShiftRightEqualExpr(z::t2t(O), z::ref(L), z::ref(R)));}

rBinaryExpr(E) ::= rExpr(L) PLUS(O)            rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryPlusExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) MINUS(O)           rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryMinusExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) STAR(O)            rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryTimesExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) DIVIDE(O)          rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryDivideExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) MOD(O)             rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryModExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEAND(O)      rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryBitwiseAndExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEOR(O)       rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryBitwiseOrExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) BITWISEXOR(O)      rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryBitwiseXorExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) SHL(O)             rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryShiftLeftExpr(z::t2t(O), z::ref(L), z::ref(R)));}
rBinaryExpr(E) ::= rExpr(L) SHR(O)             rExpr(R). {E = z::ptr(z::c2f(pctx).aBinaryShiftRightExpr(z::t2t(O), z::ref(L), z::ref(R)));}

//-------------------------------------------------
// postfix operators
%type rPostfixExpr {const z::Ast::Expr*}
rPostfixExpr(E) ::= rExpr(L) INC(O). {E = z::ptr(z::c2f(pctx).aPostfixIncExpr(z::t2t(O), z::ref(L)));}
rPostfixExpr(E) ::= rExpr(L) DEC(O). {E = z::ptr(z::c2f(pctx).aPostfixDecExpr(z::t2t(O), z::ref(L)));}

//-------------------------------------------------
// prefix operators
%type rPrefixExpr {const z::Ast::Expr*}
rPrefixExpr(E) ::= NOT(O)        rExpr(R). {E = z::ptr(z::c2f(pctx).aPrefixNotExpr(z::t2t(O), z::ref(R)));}
rPrefixExpr(E) ::= PLUS(O)       rExpr(R). {E = z::ptr(z::c2f(pctx).aPrefixPlusExpr(z::t2t(O), z::ref(R)));}
rPrefixExpr(E) ::= MINUS(O)      rExpr(R). {E = z::ptr(z::c2f(pctx).aPrefixMinusExpr(z::t2t(O), z::ref(R)));}
rPrefixExpr(E) ::= INC(O)        rExpr(R). {E = z::ptr(z::c2f(pctx).aPrefixIncExpr(z::t2t(O), z::ref(R)));}
rPrefixExpr(E) ::= DEC(O)        rExpr(R). {E = z::ptr(z::c2f(pctx).aPrefixDecExpr(z::t2t(O), z::ref(R)));}
rPrefixExpr(E) ::= BITWISENOT(O) rExpr(R). {E = z::ptr(z::c2f(pctx).aPrefixBitwiseNotExpr(z::t2t(O), z::ref(R)));}

//-------------------------------------------------
// string formatter
%type rFormatExpr {z::Ast::FormatExpr*}
rFormatExpr(L) ::= rExpr(A) AMP(B) rTreeExpr(T). {L = z::c2f(pctx).aFormatExpr(z::t2t(B), z::ref(A), z::ref(T));}

//-------------------------------------------------
// list expression
%type rListExpr {z::Ast::ListExpr*}
rListExpr(L) ::= rListList(R) RSQUARE(B). {L = z::c2f(pctx).aListExpr(z::t2t(B), z::ref(R));}

%type rListList {z::Ast::ListList*}
rListList(L) ::= rListsList(R)      . {L = R;}
rListList(L) ::= rListsList(R) COMMA. {L = R;}

%type rListsList {z::Ast::ListList*}
rListsList(L)  ::= rListsList(R) COMMA(B) rListItem(I).  {L = z::c2f(pctx).aListList(z::t2t(B), z::ref(R), z::ref(I));}
rListsList(L)  ::=      rEnterList(B) rListItem(I).      {L = z::c2f(pctx).aListList(z::t2t(B), z::ref(I));}
rListsList(L)  ::=      rEnterList(B) rQualifiedTypeSpec(Q). {L = z::c2f(pctx).aListList(z::t2t(B), z::ref(Q));}
rListsList(L)  ::=      rEnterList(B)                      . {L = z::c2f(pctx).aListList(z::t2t(B));}

%type rListItem {z::Ast::ListItem*}
rListItem(L)  ::= rExpr(E). {L = z::c2f(pctx).aListItem(z::ref(E));}

//-------------------------------------------------
// dict (strict type-checking for key and value)
%type rDictExpr {z::Ast::DictExpr*}
rDictExpr(L) ::= rDictList(R) RSQUARE(B). {L = z::c2f(pctx).aDictExpr(z::t2t(B), z::ref(R));}

%type rDictList {z::Ast::DictList*}
rDictList(L) ::= rDictsList(R)       . {L = R;}
rDictList(L) ::= rDictsList(R) COMMA . {L = R;}

%type rDictsList {z::Ast::DictList*}
rDictsList(L)  ::= rDictsList(R) COMMA(B) rDictItem(I). {L = z::c2f(pctx).aDictList(z::t2t(B), z::ref(R), z::ref(I), false);}
rDictsList(L)  ::=          rEnterList(B) rDictItem(I). {L = z::c2f(pctx).aDictList(z::t2t(B), z::ref(I));}

// first item in list can be a type specifier
rDictsList(L) ::= rEnterList(B) rQualifiedTypeSpec(K) COLON rQualifiedTypeSpec(V). {L = z::c2f(pctx).aDictList(z::t2t(B), z::ref(K), z::ref(V));}

%type rDictItem {z::Ast::DictItem*}
rDictItem(L)  ::= rDictKey(K) COLON(B) rExpr(E). {L = z::c2f(pctx).aDictItem(z::t2t(B), z::ref(K), z::ref(E), false);}

%type rDictKey {const z::Ast::Expr*}
rDictKey(L) ::= rExpr(R). {L = z::c2f(pctx).aDictKey(z::ref(R));}

//-------------------------------------------------
rEnterList(L)  ::= LSQUARE(R). {L = R; z::c2f(pctx).aEnterList(z::t2t(R)); }

//-------------------------------------------------
// tree (no type checking for key or value)
%type rTreeExpr {z::Ast::DictExpr*}
rTreeExpr(L) ::= rTreeList(R) RCURLY(B). {L = z::c2f(pctx).aDictExpr(z::t2t(B), z::ref(R));}

%type rTreeList {z::Ast::DictList*}
rTreeList(L) ::= rTreesList(R)       . {L = R;}
rTreeList(L) ::= rTreesList(R) COMMA . {L = R;}

%type rTreesList {z::Ast::DictList*}
rTreesList(L)  ::= rTreesList(R) COMMA(B)      rTreeItem(I).     {L = z::c2f(pctx).aDictList(z::t2t(B), z::ref(R), z::ref(I), true);}
rTreesList(L)  ::=               rEnterTree(B) rTreeItem(I). {L = z::c2f(pctx).aDictList(z::t2t(B), z::ref(I));}

%type rTreeItem {z::Ast::DictItem*}
rTreeItem(L)  ::= rDictKey(K) COLON(B) rExpr(E). {L = z::c2f(pctx).aDictItem(z::t2t(B), z::ref(K), z::ref(E), true);}

//-------------------------------------------------
rEnterTree(L)  ::= LCURLY(R). {L = R; z::c2f(pctx).aEnterList(z::t2t(R)); }

//-------------------------------------------------
// ordered expression
%type rOrderedExpr {z::Ast::OrderedExpr*}
rOrderedExpr(L) ::= LBRACKET(B) rExpr(innerExpr) RBRACKET. {L = z::c2f(pctx).aOrderedExpr(z::t2t(B), z::ref(innerExpr));}

//-------------------------------------------------
// index expression
%type rIndexExpr {z::Ast::IndexExpr*}
rIndexExpr(L) ::= rExpr(E) LSQUARE(B) rExpr(innerExpr) RSQUARE. {L = z::c2f(pctx).aIndexExpr(z::t2t(B), z::ref(E), z::ref(innerExpr));}
rIndexExpr(L) ::= rExpr(E) AMP(B) rKeyConstantExpr(innerExpr). {L = z::c2f(pctx).aIndexExpr(z::t2t(B), z::ref(E), z::ref(innerExpr));}

//-------------------------------------------------
// splice expression
%type rSpliceExpr {z::Ast::SpliceExpr*}
rSpliceExpr(L) ::= rExpr(E) LSQUARE(B) rExpr(fromExpr) COLON rExpr(toExpr) RSQUARE. {L = z::c2f(pctx).aSpliceExpr(z::t2t(B), z::ref(E), z::ref(fromExpr), z::ref(toExpr));}

//-------------------------------------------------
// sizeof expression
rExpr(L) ::= SIZEOF(B) LBRACKET rQualifiedTypeSpec(T) RBRACKET.  {L = z::c2f(pctx).aSizeofTypeExpr(z::t2t(B), z::ref(T));}
rExpr(L) ::= SIZEOF(B) LBRACKET rExpr(E) RBRACKET.  {L = z::c2f(pctx).aSizeofExprExpr(z::t2t(B), z::ref(E));}

//-------------------------------------------------
// typeof expression
rExpr(L) ::= TYPEOF(B) LBRACKET rQualifiedTypeSpec(T) RBRACKET. {L = z::c2f(pctx).aTypeofTypeExpr(z::t2t(B), z::ref(T));}
rExpr(L) ::= TYPEOF(B) LBRACKET rExpr(E)              RBRACKET. {L = z::c2f(pctx).aTypeofExprExpr(z::t2t(B), z::ref(E));}

//-------------------------------------------------
// type-cast expression
rExpr(L) ::= LBRACKET(B) rQualifiedTypeSpec(T) RBRACKET rExpr(E). {L = z::c2f(pctx).aTypecastExpr(z::t2t(B), z::ref(T), z::ref(E));}

//-------------------------------------------------
// address-of expression
rExpr(L) ::= BITWISEAND(B)  rExpr(E). {L = z::c2f(pctx).aPointerInstanceExpr(z::t2t(B), z::ref(E));}

// value-of expression
rExpr(L) ::= STAR(B) rExpr(E). {L = z::c2f(pctx).aValueInstanceExpr(z::t2t(B), z::ref(E));}

//-------------------------------------------------
// template definition instance expression
rExpr(L) ::= rTemplateDefnTypeSpec(R) LBRACKET(B) rExprList(M) RBRACKET. {L = z::c2f(pctx).aTemplateDefnInstanceExpr(z::t2t(B), z::ref(R), z::ref(M));}

//-------------------------------------------------
// variable z::ref expressions
%type rVariableRefExpr {z::Ast::Expr*}
rVariableRefExpr(L) ::= ID(I). {L = z::c2f(pctx).aVariableRefExpr(z::t2t(I));}

//-------------------------------------------------
// variable member expressions, e.g. struct member
%type rMemberVariableExpr {z::Ast::MemberExpr*}
rMemberVariableExpr(L) ::= rExpr(R) DOT ID(M). {L = z::c2f(pctx).aMemberVariableExpr(z::ref(R), z::t2t(M));}

//-------------------------------------------------
// type member expressions, e.g. enum member
%type rTypeSpecMemberExpr {z::Ast::TypeSpecMemberExpr*}
rTypeSpecMemberExpr(L) ::= rTypeSpec(R) DOT ID(M). {L = z::c2f(pctx).aTypeSpecMemberExpr(z::ref(R), z::t2t(M));}

//-------------------------------------------------
// function instance expressions
%type rFunctionInstanceExpr {z::Ast::TypeSpecInstanceExpr*}
rFunctionInstanceExpr(L) ::= rFunctionTypeSpec(R) LSQUARE(B) rExprList(M) RSQUARE. {L = z::c2f(pctx).aFunctionInstanceExpr(z::t2t(B), z::ref(R), z::ref(M));}

//-------------------------------------------------
// anonymous function instance expressions
%type rAnonymousFunctionExpr {z::Ast::AnonymousFunctionExpr*}
rAnonymousFunctionExpr(L) ::= rEnterAnonymousFunctionExpr(R) rFunctionBlock(C). {L = z::c2f(pctx).aAnonymousFunctionExpr(z::ref(R), z::ref(C));}

//-------------------------------------------------
%type rEnterAnonymousFunctionExpr {z::Ast::ChildFunctionDefn*}
rEnterAnonymousFunctionExpr(L) ::= rFunctionTypeSpec(R). {L = z::c2f(pctx).aEnterAnonymousFunctionExpr(z::ref(R));}

//-------------------------------------------------
// struct instance expressions
%type rStructInstanceExpr {z::Ast::StructInstanceExpr*}
rStructInstanceExpr(L) ::= rEnterStructInstanceExpr(R) LCURLY(B) rStructInitPartList(P) rLeaveStructInstanceExpr. {L = z::c2f(pctx).aStructInstanceExpr(z::t2t(B), z::ref(R), z::ref(P));}
rStructInstanceExpr(L) ::= rEnterStructInstanceExpr(R) LCURLY(B)                        rLeaveStructInstanceExpr. {L = z::c2f(pctx).aStructInstanceExpr(z::t2t(B), z::ref(R));}

//-------------------------------------------------
// special case - struct can be instantiated with {} or () for syntactic equivalence with C/C++.
rStructInstanceExpr(L) ::= rEnterStructInstanceExpr(R) LBRACKET(B) rStructInitPartList(P) RBRACKET. {L = z::c2f(pctx).aStructInstanceExpr(z::t2t(B), z::ref(R), z::ref(P));}
rStructInstanceExpr(L) ::= rEnterStructInstanceExpr(R) LBRACKET(B)                        RBRACKET. {L = z::c2f(pctx).aStructInstanceExpr(z::t2t(B), z::ref(R));}

//-------------------------------------------------
// auto struct instance expressions
%type rAutoStructInstanceExpr {z::Ast::Expr*}
rAutoStructInstanceExpr(L) ::= STRUCT(B) rEnterAutoStructInstanceExpr(R) rStructInitPartList(P) rLeaveStructInstanceExpr. {L = z::c2f(pctx).aAutoStructInstanceExpr(z::t2t(B), z::ref(R), z::ref(P));}
rAutoStructInstanceExpr(L) ::= STRUCT(B) rEnterAutoStructInstanceExpr(R)                        rLeaveStructInstanceExpr. {L = z::c2f(pctx).aAutoStructInstanceExpr(z::t2t(B), z::ref(R));}

//-------------------------------------------------
%type rEnterStructInstanceExpr {const z::Ast::StructDefn*}
rEnterStructInstanceExpr(L) ::= rStructTypeSpec(R). {L = z::c2f(pctx).aEnterStructInstanceExpr(z::ref(R));}

//-------------------------------------------------
%type rEnterAutoStructInstanceExpr {const z::Ast::StructDefn*}
rEnterAutoStructInstanceExpr(L) ::= LCURLY(R). {L = z::c2f(pctx).aEnterAutoStructInstanceExpr(z::t2t(R));}

//-------------------------------------------------
rLeaveStructInstanceExpr ::= RCURLY. {z::c2f(pctx).aLeaveStructInstanceExpr();}

//-------------------------------------------------
%type rStructInitPartList {z::Ast::StructInitPartList*}
rStructInitPartList(L) ::= rStructInitPartList(R) rStructInitPart(P). {L = z::c2f(pctx).aStructInitPartList(z::ref(R), z::ref(P));}
rStructInitPartList(L) ::=                        rStructInitPart(P). {L = z::c2f(pctx).aStructInitPartList(z::ref(P));}

//-------------------------------------------------
%type rStructInitPart {z::Ast::StructInitPart*}
rStructInitPart(L) ::= rEnterStructInitPart(R) COLON(B) rExpr(E) rLeaveStructInitPart. {L = z::c2f(pctx).aStructInitPart(z::t2t(B), z::ref(R), z::ref(E));}

//-------------------------------------------------
%type rEnterStructInitPart {const z::Ast::VariableDefn*}
rEnterStructInitPart(L) ::= ID(R). {L = z::c2f(pctx).aEnterStructInitPart(z::t2t(R));}
rLeaveStructInitPart    ::= SEMI(B).  {z::c2f(pctx).aLeaveStructInitPart(z::t2t(B));}
rLeaveStructInitPart    ::= COMMA(B). {z::c2f(pctx).aLeaveStructInitPart(z::t2t(B));}

//-------------------------------------------------
// functor call expressions
%type rCallExpr {z::Ast::CallExpr*}
rCallExpr(L) ::= rCallPart(R).  {L = R;}

//-------------------------------------------------
// function call type
%type rRunExpr {z::Ast::RunExpr*}
rRunExpr(L) ::= RUN(B) rFunctorCallPart(F).  {L = z::c2f(pctx).aRunExpr(z::t2t(B), z::ref(F));}

//-------------------------------------------------
// functor call expressions
%type rCallPart {z::Ast::CallExpr*}
rCallPart(L) ::= rRoutineCallPart(R). {L = R;}
rCallPart(L) ::= rFunctorCallPart(R). {L = R;}

//-------------------------------------------------
// routine call expressions
%type rRoutineCallPart {z::Ast::RoutineCallExpr*}
rRoutineCallPart(L) ::= rEnterRoutineCall(typeSpec) LBRACKET(B) rCallArgList(exprList) RBRACKET.  {L = z::c2f(pctx).aRoutineCallExpr(z::t2t(B), z::ref(typeSpec), z::ref(exprList));}

//-------------------------------------------------
// functor expressions
%type rEnterRoutineCall {const z::Ast::Routine*}
rEnterRoutineCall(L) ::= rRoutineTypeSpec(typeSpec). { L = z::c2f(pctx).aEnterRoutineCall(z::ref(typeSpec));}

//-------------------------------------------------
// functor call expressions
%type rFunctorCallPart {z::Ast::FunctorCallExpr*}
rFunctorCallPart(L) ::= rEnterFunctorCall(expr) LBRACKET(B) rCallArgList(exprList) RBRACKET.  {L = z::c2f(pctx).aFunctorCallExpr(z::t2t(B), z::ref(expr), z::ref(exprList));}

//-------------------------------------------------
// functor expressions
%type rEnterFunctorCall {z::Ast::Expr*}
rEnterFunctorCall(L) ::= rOrderedExpr(expr).          { L = z::c2f(pctx).aEnterFunctorCall(z::ref(expr));}
rEnterFunctorCall(L) ::= ID(I).                       { L = z::c2f(pctx).aEnterFunctorCall(z::t2t(I));}
rEnterFunctorCall(L) ::= rFunctionTypeSpec(typeSpec). { L = z::c2f(pctx).aEnterFunctorCall(z::ref(typeSpec));}

//-------------------------------------------------
// comma-separated list of function-call args
%type rCallArgList {z::Ast::ExprList*}
rCallArgList(L) ::= rCallArgList(R) COMMA(B) rExpr(E). {L = z::c2f(pctx).aCallArgList(z::t2t(B), z::ref(R), z::ref(E));}
rCallArgList(L) ::=                          rExpr(E). {L = z::c2f(pctx).aCallArgList(z::ref(E));}
rCallArgList(L) ::= .                                  {L = z::c2f(pctx).aCallArgList();}

//-------------------------------------------------
// constant expressions
%type rConstantExpr {const z::Ast::ConstantExpr*}
rConstantExpr(L) ::= NULL_CONST(value).     {L = z::ptr(z::c2f(pctx).aConstantNullExpr(z::t2t(value)));}

rConstantExpr(L) ::= FLOAT_CONST  (value).  {L = z::ptr(z::c2f(pctx).aConstantFloatExpr(z::t2t(value)));}
rConstantExpr(L) ::= DOUBLE_CONST (value).  {L = z::ptr(z::c2f(pctx).aConstantDoubleExpr(z::t2t(value)));}
rConstantExpr(L) ::= TRUE_CONST   (value).  {L = z::ptr(z::c2f(pctx).aConstantBooleanExpr(z::t2t(value)));}
rConstantExpr(L) ::= FALSE_CONST  (value).  {L = z::ptr(z::c2f(pctx).aConstantBooleanExpr(z::t2t(value)));}
rConstantExpr(L) ::= STRING_CONST (value).  {L = z::ptr(z::c2f(pctx).aConstantStringExpr(z::t2t(value)));}
rConstantExpr(L) ::= CHAR_CONST   (value).  {L = z::ptr(z::c2f(pctx).aConstantCharExpr(z::t2t(value)));}

rConstantExpr(L) ::= LHEXINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantLongExpr(z::t2t(value), 'x'));}
rConstantExpr(L) ::= LDECINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantLongExpr(z::t2t(value), 'd'));}
rConstantExpr(L) ::= LOCTINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantLongExpr(z::t2t(value), 'o'));}

rConstantExpr(L) ::= HEXINT_CONST (value).  {L = z::ptr(z::c2f(pctx).aConstantIntExpr(z::t2t(value), 'x'));}
rConstantExpr(L) ::= DECINT_CONST (value).  {L = z::ptr(z::c2f(pctx).aConstantIntExpr(z::t2t(value), 'd'));}
rConstantExpr(L) ::= OCTINT_CONST (value).  {L = z::ptr(z::c2f(pctx).aConstantIntExpr(z::t2t(value), 'o'));}

rConstantExpr(L) ::= SHEXINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantShortExpr(z::t2t(value), 'x'));}
rConstantExpr(L) ::= SDECINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantShortExpr(z::t2t(value), 'd'));}
rConstantExpr(L) ::= SOCTINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantShortExpr(z::t2t(value), 'o'));}

rConstantExpr(L) ::= BHEXINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantByteExpr(z::t2t(value), 'x'));}
rConstantExpr(L) ::= BDECINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantByteExpr(z::t2t(value), 'd'));}
rConstantExpr(L) ::= BOCTINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantByteExpr(z::t2t(value), 'o'));}

rConstantExpr(L) ::= ULHEXINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantUnLongExpr(z::t2t(value), 'x'));}
rConstantExpr(L) ::= ULDECINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantUnLongExpr(z::t2t(value), 'd'));}
rConstantExpr(L) ::= ULOCTINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantUnLongExpr(z::t2t(value), 'o'));}

rConstantExpr(L) ::= UHEXINT_CONST (value).  {L = z::ptr(z::c2f(pctx).aConstantUnIntExpr(z::t2t(value), 'x'));}
rConstantExpr(L) ::= UDECINT_CONST (value).  {L = z::ptr(z::c2f(pctx).aConstantUnIntExpr(z::t2t(value), 'd'));}
rConstantExpr(L) ::= UOCTINT_CONST (value).  {L = z::ptr(z::c2f(pctx).aConstantUnIntExpr(z::t2t(value), 'o'));}

rConstantExpr(L) ::= USHEXINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantUnShortExpr(z::t2t(value), 'x'));}
rConstantExpr(L) ::= USDECINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantUnShortExpr(z::t2t(value), 'd'));}
rConstantExpr(L) ::= USOCTINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantUnShortExpr(z::t2t(value), 'o'));}

rConstantExpr(L) ::= UBHEXINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantUnByteExpr(z::t2t(value), 'x'));}
rConstantExpr(L) ::= UBDECINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantUnByteExpr(z::t2t(value), 'd'));}
rConstantExpr(L) ::= UBOCTINT_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantUnByteExpr(z::t2t(value), 'o'));}

rConstantExpr(L) ::= rKeyConstantExpr(R).   {L = R;}

%type rKeyConstantExpr {const z::Ast::ConstantExpr*}
rKeyConstantExpr(L) ::= KEY_CONST(value).  {L = z::ptr(z::c2f(pctx).aConstantStringExpr(z::t2t(value)));}

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
