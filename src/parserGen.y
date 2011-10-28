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
rTypeSpecDef(L) ::= rTypedefDefn(R). {L = R;}
rTypeSpecDef(L) ::= rEnumDefn(R).    {L = R;}
rTypeSpecDef(L) ::= rStructDefn(R).  {L = R;}
rTypeSpecDef(L) ::= rRoutineDecl(R). {L = R;}
rTypeSpecDef(L) ::= rRoutineDefn(R). {L = R;}
rTypeSpecDef(L) ::= rFunctionDecl(R).{L = R;}
rTypeSpecDef(L) ::= rFunctionDefn(R).{L = R;}
rTypeSpecDef(L) ::= rEventDecl(R).   {L = R;}

//-------------------------------------------------
// typedef declarations
%type rTypedefDefn {Ast::TypedefDefn*}
rTypedefDefn(L) ::= TYPEDEF ID(name) NATIVE SEMI. {L = ref(pctx).aTypedefDefn(name, Ast::DefinitionType::Native);}

//-------------------------------------------------
// enum declarations
%type rEnumDefn {Ast::EnumDefn*}
rEnumDefn(L) ::= ENUM ID(name) NATIVE SEMI. {L = ref(pctx).aEnumDefn(name, Ast::DefinitionType::Native);}
rEnumDefn(L) ::= ENUM ID(name) LCURLY rEnumMemberDefnList(list) RCURLY SEMI. {L = ref(pctx).aEnumDefn(name, Ast::DefinitionType::Direct, ref(list));}

//-------------------------------------------------
%type rEnumMemberDefnList {Ast::EnumMemberDefnList*}
rEnumMemberDefnList(L) ::= rEnumMemberDefnList(list) rEnumMemberDefn(enumMemberDef). {L = ref(pctx).aEnumMemberDefnList(ref(list), ref(enumMemberDef));}
rEnumMemberDefnList(L) ::= rEnumMemberDefn(enumMemberDef). {L = ref(pctx).aEnumMemberDefnList(ref(enumMemberDef));}

//-------------------------------------------------
%type rEnumMemberDefn {Ast::EnumMemberDefn*}
rEnumMemberDefn(L) ::= ID(name) SEMI. {L = ref(pctx).aEnumMemberDefn(name);}

//-------------------------------------------------
// struct declarations
%type rStructDefn {Ast::StructDefn*}
rStructDefn(L) ::= STRUCT ID(name) NATIVE                                   SEMI. {L = ref(pctx).aStructDefn(name, Ast::DefinitionType::Native);}
rStructDefn(L) ::= STRUCT ID(name) LCURLY rStructMemberDefnList(list) RCURLY SEMI. {L = ref(pctx).aStructDefn(name, Ast::DefinitionType::Direct, ref(list));}
rStructDefn(L) ::= STRUCT ID(name) LCURLY                            RCURLY SEMI. {L = ref(pctx).aStructDefn(name, Ast::DefinitionType::Direct);}

//-------------------------------------------------
%type rStructMemberDefnList {Ast::Scope*}
rStructMemberDefnList(L) ::= rStructMemberDefnList(list) rVariableDefn(variableDef) SEMI. {L = ref(pctx).aStructMemberDefnList(ref(list), ref(variableDef));}
rStructMemberDefnList(L) ::=                            rVariableDefn(variableDef) SEMI. {L = ref(pctx).aStructMemberDefnList(ref(variableDef));}

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
// function declarations
%type rFunctionDefn {Ast::FunctionDefn*}
rFunctionDefn(L) ::= rEnterFunctionDefn(functionDefn) rCompoundStatement(block). {L = ref(pctx).aFunctionDefn(ref(functionDefn), ref(block));}

//-------------------------------------------------
%type rEnterFunctionDefn {Ast::FunctionDefn*}
rEnterFunctionDefn(L) ::= rFunctionSig(functionSig) rDefinitionType(defType). {L = ref(pctx).aEnterFunctionDefn(ref(functionSig), defType);}

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
rParamsList(L) ::= LBRACKET rScope(R) RBRACKET. {L = R;}

//-------------------------------------------------
// variable lists
%type rScope {Ast::Scope*}
rScope(L) ::= rScope(list) COMMA rVariableDefn(variableDef). {L = ref(pctx).aScope(ref(list), ref(variableDef));}
rScope(L) ::=                    rVariableDefn(variableDef). {L = ref(pctx).aScope(ref(variableDef));}
rScope(L) ::= .                                              {L = ref(pctx).aScope();}

//-------------------------------------------------
// variable def
%type rVariableDefn {const Ast::VariableDefn*}
rVariableDefn(L) ::= rQualifiedTypeSpec(qTypeRef) ID(name).                   {L = ref(pctx).aVariableDefn(ref(qTypeRef), name);}
rVariableDefn(L) ::= rQualifiedTypeSpec(qTypeRef) ID(name) ASSIGNEQUAL rExpr. {L = ref(pctx).aVariableDefn(ref(qTypeRef), name);}

//-------------------------------------------------
// qualified types
%type rQualifiedTypeSpec {const Ast::QualifiedTypeSpec*}
rQualifiedTypeSpec(L) ::=       rTypeSpec(typeSpec).               {L = ref(pctx).aQualifiedTypeSpec(false, ref(typeSpec), false);}
rQualifiedTypeSpec(L) ::=       rTypeSpec(typeSpec) BITWISEAND.    {L = ref(pctx).aQualifiedTypeSpec(false, ref(typeSpec), true);}
rQualifiedTypeSpec(L) ::= CONST rTypeSpec(typeSpec).               {L = ref(pctx).aQualifiedTypeSpec(true, ref(typeSpec), false);}
rQualifiedTypeSpec(L) ::= CONST rTypeSpec(typeSpec) BITWISEAND.    {L = ref(pctx).aQualifiedTypeSpec(true, ref(typeSpec), true);}

//-------------------------------------------------
// type references
%type rTypeSpec {const Ast::TypeSpec*}
rTypeSpec(L) ::= rTypeSpec(parent) SCOPE ID(name). {L = ref(pctx).aTypeSpec(ref(parent), name);}
rTypeSpec(L) ::=                         ID(name). {L = ref(pctx).aTypeSpec(name);}

//-------------------------------------------------
// statements
%type rLocalStatement {Ast::Statement*}
rLocalStatement(L) ::= rUserDefinedTypeSpecStatement(R). {L = R;}
rLocalStatement(L) ::= rExprStatement(R).                {L = R;}
rLocalStatement(L) ::= rRoutineReturnStatement(R).       {L = R;}
rLocalStatement(L) ::= rFunctionReturnStatement(R).      {L = R;}
rLocalStatement(L) ::= rCompoundStatement(R).            {L = R;}

//-------------------------------------------------
%type rUserDefinedTypeSpecStatement {Ast::UserDefinedTypeSpecStatement*}
rUserDefinedTypeSpecStatement(L) ::= rTypeSpecDef(typeSpec). {L = ref(pctx).aUserDefinedTypeSpecStatement(ref(typeSpec));}

//-------------------------------------------------
%type rExprStatement {Ast::ExprStatement*}
rExprStatement(L) ::= rExpr(expr) SEMI. {L = ref(pctx).aExprStatement(ref(expr));}

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
rCompoundStatement(L) ::= rEnterCompoundStatement rStatementList(R) rLeaveCompoundStatement. {L = R;}

rEnterCompoundStatement ::= LCURLY.
rLeaveCompoundStatement ::= RCURLY.

%type rStatementList {Ast::CompoundStatement*}
rStatementList(L) ::= rStatementList(list) rLocalStatement(statement). {L = ref(pctx).aStatementList(ref(list), ref(statement));}
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
rExprList(R) ::= .                           {R = ref(pctx).aExprList();}

//-------------------------------------------------
// expressions
%type rExpr {const Ast::Expr*}

//-------------------------------------------------
// binary operators

// It could be possible to implement creating local variables inline within expressions.
// Not sure how to implement it in the generated code. Not a priority, so on hold for now.
//rExpr(E) ::= ID(L) DEFINEEQUAL       rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(ref(L), z::string("="), ref(R)));}

rExpr(E) ::= rExpr(L) QUESTION(O1) rExpr(T) COLON(O2) rExpr(F). {E = ptr(ref(pctx).aTernaryExpr(O1, O2, ref(L), ref(T), ref(F)));}

rExpr(E) ::= rExpr(L) ASSIGNEQUAL(O)     rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) TIMESEQUAL(O)      rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) DIVIDEEQUAL(O)     rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) MINUSEQUAL(O)      rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) PLUSEQUAL(O)       rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) MODEQUAL(O)        rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) SHIFTLEFTEQUAL(O)  rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) SHIFTRIGHTEQUAL(O) rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) BITWISEANDEQUAL(O) rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) BITWISEXOREQUAL(O) rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) BITWISEOREQUAL(O)  rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) BITWISEAND(O)      rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) BITWISEXOR(O)      rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) BITWISEOR(O)       rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) BITWISENOT(O)      rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) AND(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) OR(O)              rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) EQUAL(O)           rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) NOTEQUAL(O)        rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) LT(O)              rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) GT(O)              rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) LTE(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) GTE(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) HAS(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) SHL(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) SHR(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) PLUS(O)            rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) MINUS(O)           rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) STAR(O)            rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) DIVIDE(O)          rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}
rExpr(E) ::= rExpr(L) MOD(O)             rExpr(R). {E = ptr(ref(pctx).aBinaryExpr(O, ref(L), ref(R)));}

rExpr(E) ::=          NOT(O)             rExpr(R). {E = ptr(ref(pctx).aPrefixExpr(O, ref(R)));}
rExpr(E) ::=          PLUS(O)            rExpr(R). {E = ptr(ref(pctx).aPrefixExpr(O,  ref(R)));}
rExpr(E) ::=          MINUS(O)           rExpr(R). {E = ptr(ref(pctx).aPrefixExpr(O,  ref(R)));}
rExpr(E) ::=          INC(O)             rExpr(R). {E = ptr(ref(pctx).aPrefixExpr(O, ref(R)));}
rExpr(E) ::=          DEC(O)             rExpr(R). {E = ptr(ref(pctx).aPrefixExpr(O, ref(R)));}
rExpr(E) ::=          BITWISENOT(O)      rExpr(R). {E = ptr(ref(pctx).aPrefixExpr(O,  ref(R)));}

rExpr(E) ::= rExpr(L) INC(O).                      {E = ptr(ref(pctx).aPostfixExpr(O, ref(L)));}
rExpr(E) ::= rExpr(L) DEC(O).                      {E = ptr(ref(pctx).aPostfixExpr(O, ref(L)));}

//-------------------------------------------------
// ordered expression
rExpr(R) ::= LBRACKET rExpr(L) RBRACKET. {R = L;}

//-------------------------------------------------
// variable member expressions
rExpr(L) ::= rExpr(R) DOT rExpr(M). {L;R;M;}

//-------------------------------------------------
// type member expressions
rExpr(L) ::= rTypeSpec(R) DOT ID(M). {L;R;M;}

//-------------------------------------------------
// constant expressions
rExpr(L) ::= rConstantExpr(R). {L = R;}

//-------------------------------------------------
%type rConstantExpr {const Ast::ConstantExpr*}
rConstantExpr(L) ::= FLOAT_CONST(value).   {L = ptr(ref(pctx).aConstantExpr("float", value));}
rConstantExpr(L) ::= DOUBLE_CONST(value).  {L = ptr(ref(pctx).aConstantExpr("double", value));}
rConstantExpr(L) ::= TRUE_CONST(value).    {L = ptr(ref(pctx).aConstantExpr("bool", value));}
rConstantExpr(L) ::= FALSE_CONST(value).   {L = ptr(ref(pctx).aConstantExpr("bool", value));}
rConstantExpr(L) ::= KEY_CONST(value).     {L = ptr(ref(pctx).aConstantExpr("string", value));}
rConstantExpr(L) ::= STRING_CONST(value).  {L = ptr(ref(pctx).aConstantExpr("string", value));}
rConstantExpr(L) ::= CHAR_CONST(value).    {L = ptr(ref(pctx).aConstantExpr("char", value));}
rConstantExpr(L) ::= HEXINT_CONST(value).  {L = ptr(ref(pctx).aConstantExpr("int", value));}
rConstantExpr(L) ::= DECINT_CONST(value).  {L = ptr(ref(pctx).aConstantExpr("int", value));}
rConstantExpr(L) ::= OCTINT_CONST(value).  {L = ptr(ref(pctx).aConstantExpr("int", value));}
rConstantExpr(L) ::= LHEXINT_CONST(value). {L = ptr(ref(pctx).aConstantExpr("long", value));}
rConstantExpr(L) ::= LDECINT_CONST(value). {L = ptr(ref(pctx).aConstantExpr("long", value));}
rConstantExpr(L) ::= LOCTINT_CONST(value). {L = ptr(ref(pctx).aConstantExpr("long", value));}
