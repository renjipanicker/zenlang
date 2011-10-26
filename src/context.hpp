#pragma once

#include "ast.hpp"
#include "compiler.hpp"

class Context {
public:
    Context(Compiler& compiler, Ast::Unit& unit, const int& level);
    ~Context();

public:
    const Ast::TypeSpec& getRootTypeSpec(const Ast::Token& name) const;
    const Ast::TypeSpec& getChildTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const;

public:
    Ast::QualifiedTypeSpec& addQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef);
    Ast::VariableDef& addVariableDef(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name);
    Ast::VariableDefList& addVariableDefList();

public:
    Ast::FunctionSig& addFunctionSig(const Ast::VariableDefList& out, const Ast::Token& name, const Ast::VariableDefList& in);
public:
    Ast::TypeDef& addTypeDefSpec(const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::EnumDef& addEnumDefSpec(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::EnumMemberDefList& list);
    Ast::EnumDef& addEnumDefSpecEmpty(const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::EnumMemberDef& addEnumMemberDef(const Ast::Token& name);
    Ast::EnumMemberDefList& addEnumMemberDefList();
    Ast::StructDef& addStructDefSpec(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::VariableDefList& list);
    Ast::StructDef& addStructDefSpecEmpty(const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::RoutineDef& addRoutineDefSpec(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::VariableDefList& in, const Ast::DefinitionType::T& defType);
    Ast::FunctionDef& addFunctionDefSpec(const Ast::FunctionSig& sig, const Ast::DefinitionType::T& defType);
    Ast::EventDef& addEventDefSpec(const Ast::VariableDef& in, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType);
    Ast::Namespace& addNamespace(const Ast::Token& name);

public:
    Ast::ExprList& addExprList();
public:
    Ast::TernaryOpExpr& addTernaryOpExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs1, const Ast::Expr& rhs2);
    Ast::BinaryOpExpr& addBinaryOpExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
    Ast::PostfixOpExpr& addPostfixOpExpr(const Ast::Token& op, const Ast::Expr& lhs);
    Ast::PrefixOpExpr& addPrefixOpExpr(const Ast::Token& op, const Ast::Expr& rhs);
    Ast::StructMemberRefExpr& addStructMemberRefExpr(const Ast::StructDef& structDef, const Ast::Token& name);
    Ast::EnumMemberRefExpr& addEnumMemberRefExpr(const Ast::EnumDef& enumDef, const Ast::Token& name);
    Ast::ConstantExpr& addConstantExpr(const std::string& type, const Ast::Token& value);

public:
    Ast::UserDefinedTypeSpecStatement& addUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec);
    Ast::ExprStatement& addExprStatement(const Ast::Expr& expr);
    Ast::ReturnStatement& addReturnStatement(const Ast::Expr& expr);
    Ast::ReturnStatement& addReturnStatement(const Ast::ExprList& exprList);
    Ast::CompoundStatement& addCompoundStatement();
    Ast::ImportStatement& addImportStatement();

public:
    Ast::FunctionImpl& addFunctionImpl(const Ast::FunctionDef& functionDef, const Ast::CompoundStatement& body);

public:
    void importHeader(const Ast::ImportStatement& statement);
    void addGlobalStatement(const Ast::Statement& statement);

private:
    const Ast::TypeSpec* findTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const;
    inline Ast::TypeSpec& currentTypeSpec() const {assert(_typeSpecStack.size() > 0); return ref(_typeSpecStack.back());}

private:
    Compiler& _compiler;
    Ast::Unit& _unit;
    const int _level;
private:
    std::list<Ast::TypeSpec*> _typeSpecStack;
};
