#pragma once

#include "ast.hpp"
#include "compiler.hpp"

class Context {
public:
    Context(Compiler& compiler, Ast::Unit& unit, const int& level);
    ~Context();

public:
    const Ast::TypeSpec& getRootTypeSpec(const Ast::Token& name) const;

public:
    Ast::Scope& addScope();
    Ast::Scope& enterScope(Ast::Scope& scope);
    Ast::Scope& enterScope();
    Ast::Scope& leaveScope();

public:
    Ast::FunctionSig& addFunctionSig(const Ast::Scope& out, const Ast::Token& name, const Ast::Scope& in);
public:
    Ast::Namespace& enterNamespace(const Ast::Token& name);

public:
    Ast::ExprList& addExprList();

public:
    Ast::RoutineReturnStatement& addRoutineReturnStatement();
    Ast::RoutineReturnStatement& addRoutineReturnStatement(const Ast::Expr& expr);
    Ast::FunctionReturnStatement& addFunctionReturnStatement(const Ast::ExprList& exprList);
    Ast::ImportStatement& addImportStatement();

public:
    void importHeader(const Ast::ImportStatement& statement);

private:
    inline Ast::TypeSpec& getRootTypeSpec() const;
    inline const Ast::TypeSpec* findTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const;
    inline Ast::TypeSpec& currentTypeSpec() const;
    inline Ast::TypeSpec& enterTypeSpec(Ast::TypeSpec& typeSpec);
    inline Ast::TypeSpec& leaveTypeSpec(Ast::TypeSpec& typeSpec);

private:
    inline Ast::EnumMemberDefnList& addEnumMemberDefList();
private:
    Compiler& _compiler;
    Ast::Unit& _unit;
    const int _level;
private:
    std::list<Ast::Scope*>     _scopeStack;
    std::list<Ast::TypeSpec*>  _typeSpecStack;
    std::list<Ast::Namespace*> _namespaceStack;

public:
    void                     aUnitNamespaceId(const Ast::Token& name);
    void                     aLeaveNamespace();
    void                     aImportStatement(const Ast::HeaderType::T& headerType, Ast::ImportStatement& statement, const Ast::DefinitionType::T& defType);
    Ast::ImportStatement*    aImportNamespaceId(Ast::ImportStatement& statement, const Ast::Token& name);
    Ast::ImportStatement*    aImportNamespaceId(const Ast::Token& name);
    Ast::Statement*          aGlobalTypeSpecStatement(const Ast::AccessType::T& accessType, Ast::UserDefinedTypeSpec& typeSpec);
    Ast::TypedefDefn*        aTypedefDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::EnumDefn*           aEnumDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::EnumDefn*           aEnumDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::EnumMemberDefnList& list);
    Ast::EnumMemberDefnList* aEnumMemberDefnList(Ast::EnumMemberDefnList& list, const Ast::EnumMemberDefn& enumMemberDef);
    Ast::EnumMemberDefnList* aEnumMemberDefnList(const Ast::EnumMemberDefn& enumMemberDef);
    Ast::EnumMemberDefn*     aEnumMemberDefn(const Ast::Token& name);
    Ast::StructDefn*         aStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list);
    Ast::StructDefn*         aStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::Scope*              aStructMemberDefnList(Ast::Scope& list, const Ast::VariableDefn& enumMemberDefn);
    Ast::Scope*              aStructMemberDefnList(const Ast::VariableDefn& enumMemberDefn);
    Ast::RoutineDecl*        aRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::Scope& in, const Ast::DefinitionType::T& defType);
    Ast::RoutineDefn*        aRoutineDefn(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::Scope& in, const Ast::DefinitionType::T& defType, const Ast::CompoundStatement& block);
    Ast::FunctionDecl*       aFunctionDecl(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType);
    Ast::FunctionDefn*       aFunctionDefn(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType, const Ast::CompoundStatement& block);
    Ast::EventDecl*          aEventDecl(const Ast::VariableDefn& in, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType);
    Ast::FunctionSig*        aFunctionSig(const Ast::Scope& out, const Ast::Token& name, const Ast::Scope& in);
    Ast::Scope*              aInParamsList(Ast::Scope& scope);
    Ast::Scope*              aScope(Ast::Scope& list, const Ast::VariableDefn& variableDefn);
    Ast::Scope*              aScope(const Ast::VariableDefn& variableDefn);
    Ast::Scope*              aScope();
    Ast::VariableDefn*       aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name);
    Ast::QualifiedTypeSpec*  aQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef);
    const Ast::TypeSpec*     aTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const;
    const Ast::TypeSpec*     aTypeSpec(const Ast::Token& name) const;

public:
    Ast::UserDefinedTypeSpecStatement* aUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec);
    Ast::ExprStatement*                aExprStatement(const Ast::Expr& expr);
    Ast::CompoundStatement*            aStatementList();
    Ast::CompoundStatement*            aStatementList(Ast::CompoundStatement& list, const Ast::Statement& statement);
    Ast::ExprList*                     aExprList(Ast::ExprList& list, const Ast::Expr& expr);
    Ast::ExprList*                     aExprList(const Ast::Expr& expr);
    Ast::ExprList*                     aExprList();

public:
    Ast::TernaryOpExpr&       addTernaryOpExpr(const Ast::Token& op1, const Ast::Token& op2, const Ast::Expr& lhs, const Ast::Expr& rhs1, const Ast::Expr& rhs2);
    Ast::BinaryOpExpr&        addBinaryOpExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
    Ast::PostfixOpExpr&       addPostfixOpExpr(const Ast::Token& op, const Ast::Expr& lhs);
    Ast::PrefixOpExpr&        addPrefixOpExpr(const Ast::Token& op, const Ast::Expr& rhs);
    Ast::StructMemberRefExpr& addStructMemberRefExpr(const Ast::StructDefn& structDef, const Ast::Token& name);
    Ast::EnumMemberRefExpr&   addEnumMemberRefExpr(const Ast::EnumDefn& enumDef, const Ast::Token& name);
    Ast::ConstantExpr&        addConstantExpr(const std::string& type, const Ast::Token& value);
};
