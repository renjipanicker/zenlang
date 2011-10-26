#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "context.hpp"

Context::Context(Compiler& compiler, Ast::Unit& unit, const int& level) : _compiler(compiler), _unit(unit), _level(level) {
    Ast::TypeSpec* currentTypeSpec = ptr((_level == 0)?_unit.rootNS():_unit.importNS());
    _typeSpecStack.push_back(currentTypeSpec);
}

Context::~Context() {
    //assert(_typeSpecStack.size() == 1);
    _typeSpecStack.pop_back();
}

Ast::VariableDefList& Context::addVariableDefList() {
    Ast::VariableDefList& variableDefList = _unit.addNode(new Ast::VariableDefList());
    return variableDefList;
}

Ast::VariableDef& Context::addVariableDef(const Ast::QualifiedTypeSpec &qualifiedTypeSpec, const Ast::Token &name) {
    Ast::VariableDef& paramDef = _unit.addNode(new Ast::VariableDef(qualifiedTypeSpec, name));
    return paramDef;
}

Ast::QualifiedTypeSpec & Context::addQualifiedTypeSpec(const bool &isConst, const Ast::TypeSpec &typeSpec, const bool &isRef) {
    Ast::QualifiedTypeSpec& qualifiedTypeSpec = _unit.addNode(new Ast::QualifiedTypeSpec(isConst, typeSpec, isRef));
    return qualifiedTypeSpec;
}

Ast::TypeDef& Context::addTypeDefSpec(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::TypeDef& typeDef = _unit.addNode(new Ast::TypeDef(currentTypeSpec(), name, defType));
    currentTypeSpec().addChild(typeDef);
    return typeDef;
}

Ast::EnumMemberDef& Context::addEnumMemberDef(const Ast::Token &name) {
    Ast::EnumMemberDef& enumMemberDef = _unit.addNode(new Ast::EnumMemberDef(name));
    return enumMemberDef;
}

Ast::EnumMemberDefList& Context::addEnumMemberDefList() {
    Ast::EnumMemberDefList& enumMemberDefList = _unit.addNode(new Ast::EnumMemberDefList());
    return enumMemberDefList;
}

Ast::EnumDef& Context::addEnumDefSpec(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::EnumMemberDefList& list) {
    Ast::EnumDef& enumDef = _unit.addNode(new Ast::EnumDef(currentTypeSpec(), name, defType, list));
    currentTypeSpec().addChild(enumDef);
    return enumDef;
}

Ast::EnumDef& Context::addEnumDefSpecEmpty(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    const Ast::EnumMemberDefList& list = addEnumMemberDefList();
    return addEnumDefSpec(name, defType, list);
}

Ast::StructDef& Context::addStructDefSpec(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::VariableDefList& list) {
    Ast::StructDef& structDef = _unit.addNode(new Ast::StructDef(currentTypeSpec(), name, defType, list));
    currentTypeSpec().addChild(structDef);
    return structDef;
}

Ast::StructDef& Context::addStructDefSpecEmpty(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::VariableDefList& variableDefList = addVariableDefList();
    return addStructDefSpec(name, defType, variableDefList);
}

Ast::RoutineDef& Context::addRoutineDefSpec(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::VariableDefList& in, const Ast::DefinitionType::T& defType) {
    Ast::RoutineDef& routineDef = _unit.addNode(new Ast::RoutineDef(currentTypeSpec(), outType, name, in, defType));
    currentTypeSpec().addChild(routineDef);
    return routineDef;
}

Ast::FunctionSig& Context::addFunctionSig(const Ast::VariableDefList& out, const Ast::Token& name, const Ast::VariableDefList& in) {
    Ast::FunctionSig& functionSig = _unit.addNode(new Ast::FunctionSig(out, name, in));
    return functionSig;
}

Ast::FunctionDef& Context::addFunctionDefSpec(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();
    Ast::FunctionDef& functionDef = _unit.addNode(new Ast::FunctionDef(currentTypeSpec(), name, defType, functionSig));
    currentTypeSpec().addChild(functionDef);
    return functionDef;
}

Ast::EventDef & Context::addEventDefSpec(const Ast::VariableDef& in, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();
    Ast::EventDef& eventDef = _unit.addNode(new Ast::EventDef(currentTypeSpec(), name, in, functionSig, defType));
    currentTypeSpec().addChild(eventDef);
    return eventDef;
}

Ast::Namespace& Context::addNamespace(const Ast::Token& name) {
    _unit.addNamespace(name);
    Ast::Namespace& ns = _unit.addNode(new Ast::Namespace(currentTypeSpec(), name));
    currentTypeSpec().addChild(ns);
    _typeSpecStack.push_back(ptr(ns));
    return ns;
}

Ast::UserDefinedTypeSpecStatement& Context::addUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec) {
    Ast::UserDefinedTypeSpecStatement& userDefinedTypeSpecStatement = _unit.addNode(new Ast::UserDefinedTypeSpecStatement(typeSpec));
    return userDefinedTypeSpecStatement;
}

Ast::ExprStatement & Context::addExprStatement(const Ast::Expr &expr) {
    Ast::ExprStatement& exprStatement = _unit.addNode(new Ast::ExprStatement(expr));
    return exprStatement;
}

Ast::ReturnStatement& Context::addReturnStatement(const Ast::ExprList& exprList) {
    Ast::ReturnStatement& returnStatement = _unit.addNode(new Ast::ReturnStatement(exprList));
    return returnStatement;
}

Ast::ReturnStatement& Context::addReturnStatement(const Ast::Expr& expr) {
    Ast::ExprList& exprList = addExprList();
    exprList.addExpr(expr);
    return addReturnStatement(exprList);
}

Ast::CompoundStatement& Context::addCompoundStatement() {
    Ast::CompoundStatement& statement = _unit.addNode(new Ast::CompoundStatement());
    return statement;
}
Ast::ImportStatement& Context::addImportStatement() {
    Ast::ImportStatement& importStatement = _unit.addNode(new Ast::ImportStatement());
    _unit.addImportStatement(importStatement);
    return importStatement;
}

void Context::addGlobalStatement(const Ast::Statement &statement) {
    if(_level > 0)
        return;
    _unit.addGlobalStatement(statement);
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

const Ast::TypeSpec* Context::findTypeSpec(const Ast::TypeSpec &parent, const Ast::Token &name) const {
    const Ast::TypeSpec* child = parent.hasChild(name.text());
    if(child)
        return child;
    const Ast::ChildTypeSpec* parentx = dynamic_cast<const Ast::ChildTypeSpec*>(ptr(parent));
    if(!parentx)
        return 0;
    return findTypeSpec(ref(parentx).parent(), name);
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

const Ast::TypeSpec& Context::getChildTypeSpec(const Ast::TypeSpec &parent, const Ast::Token &name) const {
    const Ast::TypeSpec* typeSpec = parent.hasChild(name.text());
    if(!typeSpec) {
        throw Exception("Unknown child type '%s'\n", name.text());
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

Ast::TernaryOpExpr& Context::addTernaryOpExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs1, const Ast::Expr& rhs2) {
    const Ast::TypeSpec& typeSpec = coerce(rhs1, rhs2);
    Ast::TernaryOpExpr& expr = _unit.addNode(new Ast::TernaryOpExpr(typeSpec, op, lhs, rhs1, rhs2));
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

Ast::StructMemberRefExpr& Context::addStructMemberRefExpr(const Ast::StructDef& structDef, const Ast::Token& name) {
    const Ast::TypeSpec& typeSpec = structDef;
    Ast::StructMemberRefExpr& expr = _unit.addNode(new Ast::StructMemberRefExpr(typeSpec, structDef, name));
    return expr;
}

Ast::EnumMemberRefExpr& Context::addEnumMemberRefExpr(const Ast::EnumDef& enumDef, const Ast::Token& name) {
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

Ast::FunctionImpl& Context::addFunctionImpl(const Ast::FunctionDef& functionDef, const Ast::CompoundStatement& body) {
    Ast::FunctionImpl& functionImpl = _unit.addNode(new Ast::FunctionImpl(functionDef, body));
    return functionImpl;
}
