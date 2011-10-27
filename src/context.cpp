#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "context.hpp"

inline Ast::TypeSpec& Context::getRootTypeSpec() const {
    return (_level == 0)?_unit.rootNS():_unit.importNS();
}

inline const Ast::TypeSpec* Context::findTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const {
    const Ast::TypeSpec* child = parent.hasChild(name.text());
    if(child)
        return child;
    const Ast::ChildTypeSpec* parentx = dynamic_cast<const Ast::ChildTypeSpec*>(ptr(parent));
    if(!parentx)
        return 0;
    return findTypeSpec(ref(parentx).parent(), name);
}

inline Ast::TypeSpec& Context::currentTypeSpec() const {
    assert(_typeSpecStack.size() > 0);
    return ref(_typeSpecStack.back());
}

inline Ast::TypeSpec& Context::enterTypeSpec(Ast::TypeSpec& typeSpec) {
    _typeSpecStack.push_back(ptr(typeSpec));
    return ref(_typeSpecStack.back());
}

inline Ast::TypeSpec& Context::leaveTypeSpec(Ast::TypeSpec& typeSpec) {
    Ast::TypeSpec* ct = _typeSpecStack.back();
    assert(ct == ptr(typeSpec));
    _typeSpecStack.pop_back();
    return ref(ct);
}

Context::Context(Compiler& compiler, Ast::Unit& unit, const int& level) : _compiler(compiler), _unit(unit), _level(level) {
    Ast::TypeSpec& rootTypeSpec = getRootTypeSpec();
    enterTypeSpec(rootTypeSpec);
}

Context::~Context() {
    assert(_typeSpecStack.size() == 1);
    Ast::TypeSpec& rootTypeSpec = getRootTypeSpec();
    leaveTypeSpec(rootTypeSpec);
}

Ast::Scope& Context::addScope() {
    Ast::Scope& scope = _unit.addNode(new Ast::Scope());
    return scope;
}

Ast::Scope& Context::enterScope(Ast::Scope& scope) {
    _scopeStack.push_back(ptr(scope));
    return scope;
}

Ast::Scope& Context::enterScope() {
    Ast::Scope& scope = addScope();
    return enterScope(scope);
}

Ast::Scope& Context::leaveScope() {
    Ast::Scope* s = _scopeStack.back();
    //assert(s == ptr(scope));
    _scopeStack.pop_back();
    return ref(s);
}

Ast::Namespace& Context::enterNamespace(const Ast::Token& name) {
    _unit.addNamespace(name);
    Ast::Namespace& ns = _unit.addNode(new Ast::Namespace(currentTypeSpec(), name));
    currentTypeSpec().addChild(ns);
    enterTypeSpec(ns);
    _namespaceStack.push_back(ptr(ns));
    return ns;
}

Ast::RoutineReturnStatement& Context::addRoutineReturnStatement() {
    Ast::ExprList& exprList = addExprList();
    Ast::RoutineReturnStatement& returnStatement = _unit.addNode(new Ast::RoutineReturnStatement(exprList));
    return returnStatement;
}

Ast::RoutineReturnStatement& Context::addRoutineReturnStatement(const Ast::Expr& expr) {
    Ast::ExprList& exprList = addExprList();
    exprList.addExpr(expr);
    Ast::RoutineReturnStatement& returnStatement = _unit.addNode(new Ast::RoutineReturnStatement(exprList));
    return returnStatement;
}

Ast::FunctionReturnStatement& Context::addFunctionReturnStatement(const Ast::ExprList& exprList) {
    Ast::FunctionReturnStatement& returnStatement = _unit.addNode(new Ast::FunctionReturnStatement(exprList));
    return returnStatement;
}

Ast::ImportStatement& Context::addImportStatement() {
    Ast::ImportStatement& importStatement = _unit.addNode(new Ast::ImportStatement());
    _unit.addImportStatement(importStatement);
    return importStatement;
}

void Context::importHeader(const Ast::ImportStatement &statement) {
    if(statement.defType() != Ast::DefinitionType::Native) {
        std::string filename;
        std::string sep = "";
        for(Ast::ImportStatement::Part::const_iterator it = statement.part().begin(); it != statement.part().end(); ++it) {
            const Ast::Token& name = *it;
            filename += sep;
            filename += name.text();
            sep = "/";
        }
        filename += ".ipp";
        _compiler.import(_unit, filename, _level);
    }
}

const Ast::TypeSpec& Context::getRootTypeSpec(const Ast::Token &name) const {
    const Ast::TypeSpec* typeSpec = findTypeSpec(currentTypeSpec(), name);
    if(!typeSpec) {
        if(_level == 0) {
            typeSpec = _unit.importNS().hasChild(name.text());
            if(!typeSpec) {
                throw Exception("Unknown root type '%s'\n", name.text());
            }
        }
    }
    return ref(typeSpec);
}

Ast::ExprList& Context::addExprList() {
    Ast::ExprList& exprList = _unit.addNode(new Ast::ExprList());
    return exprList;
}

inline const Ast::TypeSpec& coerce(const Ast::Expr& lhs, const Ast::Expr& rhs) {
    /// \todo
    return lhs.typeSpec();
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
inline Ast::EnumMemberDefnList& Context::addEnumMemberDefList() {
    Ast::EnumMemberDefnList& enumMemberDefnList = _unit.addNode(new Ast::EnumMemberDefnList());
    return enumMemberDefnList;
}

////////////////////////////////////////////////////////////
void Context::aUnitNamespaceId(const Ast::Token &name) {
    enterNamespace(name);
}

void Context::aLeaveNamespace() {
    while(_namespaceStack.size() > 0) {
        Ast::Namespace* ns = _namespaceStack.back();
        leaveTypeSpec(ref(ns));
        _namespaceStack.pop_back();
    }
}

void Context::aImportStatement(const Ast::HeaderType::T& headerType, Ast::ImportStatement& statement, const Ast::DefinitionType::T& defType) {
    statement.headerType(headerType);
    statement.defType(defType);
    importHeader(statement);
}

Ast::ImportStatement* Context::aImportNamespaceId(Ast::ImportStatement& statement, const Ast::Token& name) {
    statement.addPart(name);
    return ptr(statement);
}
Ast::ImportStatement* Context::aImportNamespaceId(const Ast::Token& name) {
    Ast::ImportStatement& statement = addImportStatement();
    return aImportNamespaceId(statement, name);
}

Ast::Statement* Context::aGlobalTypeSpecStatement(const Ast::AccessType::T& accessType, Ast::UserDefinedTypeSpec& typeSpec){
    typeSpec.accessType(accessType);
    Ast::Statement* statement = aUserDefinedTypeSpecStatement(typeSpec);
    return statement;
}

Ast::TypedefDefn* Context::aTypedefDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::TypedefDefn& typedefDefn = _unit.addNode(new Ast::TypedefDefn(currentTypeSpec(), name, defType));
    currentTypeSpec().addChild(typedefDefn);
    return ptr(typedefDefn);
}

Ast::EnumDefn* Context::aEnumDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::EnumMemberDefnList& list) {
    Ast::EnumDefn& enumDefn = _unit.addNode(new Ast::EnumDefn(currentTypeSpec(), name, defType, list));
    currentTypeSpec().addChild(enumDefn);
    return ptr(enumDefn);
}

Ast::EnumDefn* Context::aEnumDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    const Ast::EnumMemberDefnList& list = addEnumMemberDefList();
    return aEnumDefn(name, defType, list);
}

Ast::EnumMemberDefnList* Context::aEnumMemberDefnList(Ast::EnumMemberDefnList& list, const Ast::EnumMemberDefn& enumMemberDef) {
    list.addEnumMemberDef(enumMemberDef);
    return ptr(list);
}

Ast::EnumMemberDefnList* Context::aEnumMemberDefnList(const Ast::EnumMemberDefn& enumMemberDef) {
    Ast::EnumMemberDefnList& list = addEnumMemberDefList();
    return aEnumMemberDefnList(list, enumMemberDef);
}

Ast::EnumMemberDefn* Context::aEnumMemberDefn(const Ast::Token& name) {
    Ast::EnumMemberDefn& enumMemberDefn = _unit.addNode(new Ast::EnumMemberDefn(name));
    return ptr(enumMemberDefn);
}

Ast::StructDefn* Context::aStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list) {
    Ast::StructDefn& structDefn = _unit.addNode(new Ast::StructDefn(currentTypeSpec(), name, defType, list));
    currentTypeSpec().addChild(structDefn);
    return ptr(structDefn);
}

Ast::StructDefn* Context::aStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& scope = addScope();
    return aStructDefn(name, defType, scope);
}

Ast::Scope* Context::aStructMemberDefnList(Ast::Scope& list, const Ast::VariableDefn& enumMemberDefn) {
    list.addVariableDef(enumMemberDefn);
    return ptr(list);
}

Ast::Scope* Context::aStructMemberDefnList(const Ast::VariableDefn& enumMemberDefn) {
    Ast::Scope& list = addScope();
    return aStructMemberDefnList(list, enumMemberDefn);
}

Ast::RoutineDecl* Context::aRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::Scope& in, const Ast::DefinitionType::T& defType) {
    Ast::RoutineDecl& routineDecl = _unit.addNode(new Ast::RoutineDecl(currentTypeSpec(), outType, name, in, defType));
    currentTypeSpec().addChild(routineDecl);
    leaveScope();
    return ptr(routineDecl);
}

Ast::RoutineDefn* Context::aRoutineDefn(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::Scope& in, const Ast::DefinitionType::T& defType, const Ast::CompoundStatement& block) {
    Ast::RoutineDefn& routineDefn = _unit.addNode(new Ast::RoutineDefn(currentTypeSpec(), outType, name, in, defType, block));
    currentTypeSpec().addChild(routineDefn);
    _unit.addBlock(routineDefn);
    leaveScope();
    return ptr(routineDefn);
}

Ast::FunctionDecl* Context::aFunctionDecl(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();
    Ast::FunctionDecl& functionDecl = _unit.addNode(new Ast::FunctionDecl(currentTypeSpec(), name, defType, functionSig));
    currentTypeSpec().addChild(functionDecl);
    leaveScope();
    return ptr(functionDecl);
}

Ast::FunctionDefn* Context::aFunctionDefn(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType, const Ast::CompoundStatement& block) {
    const Ast::Token& name = functionSig.name();
    Ast::FunctionDefn& functionDefn = _unit.addNode(new Ast::FunctionDefn(currentTypeSpec(), name, defType, functionSig, block));
    currentTypeSpec().addChild(functionDefn);
    _unit.addBlock(functionDefn);
    leaveScope();
    return ptr(functionDefn);
}

Ast::EventDecl* Context::aEventDecl(const Ast::VariableDefn& in, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();
    Ast::EventDecl& eventDef = _unit.addNode(new Ast::EventDecl(currentTypeSpec(), name, in, functionSig, defType));
    currentTypeSpec().addChild(eventDef);
    leaveScope();
    return ptr(eventDef);
}

Ast::FunctionSig* Context::aFunctionSig(const Ast::Scope& out, const Ast::Token& name, const Ast::Scope& in) {
    Ast::FunctionSig& functionSig = _unit.addNode(new Ast::FunctionSig(out, name, in));
    return ptr(functionSig);
}

Ast::Scope* Context::aInParamsList(Ast::Scope& scope) {
    enterScope(scope);
    return ptr(scope);
}

Ast::Scope* Context::aScope(Ast::Scope& list, const Ast::VariableDefn& variableDefn) {
    list.addVariableDef(variableDefn);
    return ptr(list);
}

Ast::Scope* Context::aScope(const Ast::VariableDefn& variableDefn) {
    Ast::Scope& list = addScope();
    return aScope(list, variableDefn);
}

Ast::Scope* Context::aScope() {
    Ast::Scope& list = addScope();
    return ptr(list);
}

Ast::VariableDefn* Context::aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name) {
    Ast::VariableDefn& variableDef = _unit.addNode(new Ast::VariableDefn(qualifiedTypeSpec, name));
    return ptr(variableDef);
}

Ast::QualifiedTypeSpec* Context::aQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef) {
    Ast::QualifiedTypeSpec& qualifiedTypeSpec = _unit.addNode(new Ast::QualifiedTypeSpec(isConst, typeSpec, isRef));
    return ptr(qualifiedTypeSpec);
}

const Ast::TypeSpec* Context::aTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const {
    const Ast::TypeSpec* typeSpec = parent.hasChild(name.text());
    if(!typeSpec) {
        throw Exception("Unknown child type '%s'\n", name.text());
    }
    return typeSpec;
}

const Ast::TypeSpec* Context::aTypeSpec(const Ast::Token& name) const {
    const Ast::TypeSpec& typeSpec = getRootTypeSpec(name);
    return ptr(typeSpec);
}

Ast::UserDefinedTypeSpecStatement* Context::aUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec) {
    Ast::UserDefinedTypeSpecStatement& userDefinedTypeSpecStatement = _unit.addNode(new Ast::UserDefinedTypeSpecStatement(typeSpec));
    return ptr(userDefinedTypeSpecStatement);
}

Ast::ExprStatement* Context::aExprStatement(const Ast::Expr& expr) {
    Ast::ExprStatement& exprStatement = _unit.addNode(new Ast::ExprStatement(expr));
    return ptr(exprStatement);
}

Ast::CompoundStatement* Context::aStatementList() {
    Ast::CompoundStatement& statement = _unit.addNode(new Ast::CompoundStatement());
    return ptr(statement);
}

Ast::CompoundStatement* Context::aStatementList(Ast::CompoundStatement& list, const Ast::Statement& statement) {
    list.addStatement(statement);
    return ptr(list);
}

Ast::ExprList* Context::aExprList(Ast::ExprList& list, const Ast::Expr& expr) {
    list.addExpr(expr);
    return ptr(list);
}

Ast::ExprList* Context::aExprList(const Ast::Expr& expr) {
    Ast::ExprList& list = addExprList();
    list.addExpr(expr);
    return ptr(list);
}

Ast::ExprList* Context::aExprList() {
    Ast::ExprList& list = addExprList();
    return ptr(list);
}

Ast::TernaryOpExpr& Context::addTernaryOpExpr(const Ast::Token& op1, const Ast::Token& op2, const Ast::Expr& lhs, const Ast::Expr& rhs1, const Ast::Expr& rhs2) {
    const Ast::TypeSpec& typeSpec = coerce(rhs1, rhs2);
    Ast::TernaryOpExpr& expr = _unit.addNode(new Ast::TernaryOpExpr(typeSpec, op1, op2, lhs, rhs1, rhs2));
    return expr;
}

Ast::BinaryOpExpr& Context::addBinaryOpExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    const Ast::TypeSpec& typeSpec = coerce(lhs, rhs);
    Ast::BinaryOpExpr& expr = _unit.addNode(new Ast::BinaryOpExpr(typeSpec, op, lhs, rhs));
    return expr;
}

Ast::PostfixOpExpr& Context::addPostfixOpExpr(const Ast::Token& op, const Ast::Expr& lhs) {
    const Ast::TypeSpec& typeSpec = lhs.typeSpec();
    Ast::PostfixOpExpr& expr = _unit.addNode(new Ast::PostfixOpExpr(typeSpec, op, lhs));
    return expr;
}

Ast::PrefixOpExpr& Context::addPrefixOpExpr(const Ast::Token& op, const Ast::Expr& rhs) {
    const Ast::TypeSpec& typeSpec = rhs.typeSpec();
    Ast::PrefixOpExpr& expr = _unit.addNode(new Ast::PrefixOpExpr(typeSpec, op, rhs));
    return expr;
}

Ast::StructMemberRefExpr& Context::addStructMemberRefExpr(const Ast::StructDefn& structDef, const Ast::Token& name) {
    const Ast::TypeSpec& typeSpec = structDef;
    Ast::StructMemberRefExpr& expr = _unit.addNode(new Ast::StructMemberRefExpr(typeSpec, structDef, name));
    return expr;
}

Ast::EnumMemberRefExpr& Context::addEnumMemberRefExpr(const Ast::EnumDefn& enumDef, const Ast::Token& name) {
    const Ast::TypeSpec& typeSpec = enumDef;
    Ast::EnumMemberRefExpr& expr = _unit.addNode(new Ast::EnumMemberRefExpr(typeSpec, enumDef, name));
    return expr;
}

Ast::ConstantExpr& Context::addConstantExpr(const std::string& type, const Ast::Token& value) {
    Ast::Token token(value.row(), value.col(), type);
    const Ast::TypeSpec& typeSpec = getRootTypeSpec(token);
    Ast::ConstantExpr& expr = _unit.addNode(new Ast::ConstantExpr(typeSpec, value));
    return expr;
}
