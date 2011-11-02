#pragma once

#include "ast.hpp"
#include "compiler.hpp"

class Context {
public:
    typedef std::list<Ast::TypeSpec*> TypeSpecStack;
public:
    Context(Compiler& compiler, Ast::Unit& unit, const int& level);
    ~Context();

private:
    inline Ast::ExprList& addExprList();
    inline Ast::Root& getRootNamespace() const;
    template <typename T>
    inline const T& getRootTypeSpec(const Ast::Token& name) const;
    inline const Ast::TypeSpec* findTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const;

public:
    const Ast::TypeSpec* hasRootTypeSpec(const Ast::Token& name) const;
    inline const TypeSpecStack& typeSpecStack() const {return _typeSpecStack;}

private:
    inline Ast::TypeSpec& currentTypeSpec() const;
    inline Ast::TypeSpec& enterTypeSpec(Ast::TypeSpec& typeSpec);
    inline Ast::TypeSpec& leaveTypeSpec(Ast::TypeSpec& typeSpec);
    inline Ast::QualifiedTypeSpec& addQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef);
    inline const Ast::QualifiedTypeSpec& getQualifiedTypeSpec(const Ast::Token& pos, const std::string& name);
    inline const Ast::VariableDefn* hasMember(const Ast::Scope& scope, const Ast::Token& name);
    inline Ast::TemplateDefn& createTemplateDefn(const Ast::Token& pos, const std::string& name);
    inline const Ast::Expr& getInitExpr(const Ast::TypeSpec& typeSpec, const Ast::Token& name);

private:
    inline Ast::Scope& addScope();
    inline Ast::Scope& enterScope(Ast::Scope& scope);
    inline Ast::Scope& leaveScope();

private:
    Compiler& _compiler;
    Ast::Unit& _unit;
    const int _level;

private:
    typedef std::list<Ast::Scope*> ScopeStack;

private:
    ScopeStack                  _scopeStack;
    TypeSpecStack              _typeSpecStack;
    std::list<Ast::Namespace*> _namespaceStack;

public:
    void                     aUnitNamespaceId(const Ast::Token& name);
    void                     aLeaveNamespace();
    void                     aImportStatement(const Ast::HeaderType::T& headerType, Ast::ImportStatement& statement, const Ast::DefinitionType::T& defType);
    Ast::ImportStatement*    aImportNamespaceId(Ast::ImportStatement& statement, const Ast::Token& name);
    Ast::ImportStatement*    aImportNamespaceId(const Ast::Token& name);
    Ast::Statement*          aGlobalTypeSpecStatement(const Ast::AccessType::T& accessType, Ast::UserDefinedTypeSpec& typeSpec);
    Ast::TypedefDefn*        aTypedefDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::TemplatePartList*   aTemplatePartList(Ast::TemplatePartList& list, const Ast::Token& name);
    Ast::TemplatePartList*   aTemplatePartList(const Ast::Token& name);
    Ast::TemplateDecl*       aTemplateDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::TemplatePartList& list);
    Ast::EnumDefn*           aEnumDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list);
    Ast::EnumDefn*           aEnumDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::Scope*              aEnumMemberDefnList(Ast::Scope& list, const Ast::VariableDefn& variableDefn);
    Ast::Scope*              aEnumMemberDefnList(const Ast::VariableDefn& variableDefn);
    Ast::VariableDefn*       aEnumMemberDefn(const Ast::Token& name);
    Ast::VariableDefn*       aEnumMemberDefn(const Ast::Token& name, const Ast::Expr& initExpr);
    Ast::StructDefn*         aStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list);
    Ast::StructDefn*         aStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::Scope*              aStructMemberDefnList(Ast::Scope& list, const Ast::VariableDefn& enumMemberDefn);
    Ast::Scope*              aStructMemberDefnList(const Ast::VariableDefn& enumMemberDefn);
    Ast::RoutineDecl*        aRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::Scope& in, const Ast::DefinitionType::T& defType);
    Ast::RoutineDefn*        aRoutineDefn(Ast::RoutineDefn& routineDefn, const Ast::CompoundStatement& block);
    Ast::RoutineDefn*        aEnterRoutineDefn(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const Ast::DefinitionType::T& defType);
    Ast::FunctionDecl*       aFunctionDecl(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType);
    Ast::FunctionDefn*       aFunctionDefn(Ast::FunctionDefn& functionDefn, const Ast::CompoundStatement& block);
    Ast::FunctionDefn*       aEnterFunctionDefn(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType);
    Ast::FunctionImpl*       aFunctionImpl(Ast::FunctionImpl& functionImpl, const Ast::CompoundStatement& block);
    Ast::FunctionImpl*       aEnterFunctionImpl(const Ast::TypeSpec& base, const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::EventDecl*          aEventDecl(const Ast::VariableDefn& in, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType);
    Ast::FunctionSig*        aFunctionSig(const Ast::Scope& out, const Ast::Token& name, Ast::Scope& in);
    Ast::Scope*              aInParamsList(Ast::Scope& scope);
    Ast::Scope*              aScope(Ast::Scope& list, const Ast::VariableDefn& variableDefn);
    Ast::Scope*              aScope(const Ast::VariableDefn& variableDefn);
    Ast::Scope*              aScope();
    Ast::VariableDefn*       aVariableDefn(const Ast::Token& name, const Ast::Expr& initExpr);
    Ast::VariableDefn*       aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name);
    Ast::QualifiedTypeSpec*  aQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef);
    const Ast::TypeSpec*     aTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const;
    const Ast::TypeSpec*     aTypeSpec(const Ast::Token& name) const;

public:
    Ast::UserDefinedTypeSpecStatement* aUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec);
    Ast::LocalStatement*               aLocalStatement(const Ast::VariableDefn& defn);
    Ast::ExprStatement*                aExprStatement(const Ast::Expr& expr);
    Ast::PrintStatement*               aPrintStatement(const Ast::FormatExpr& expr);
    Ast::RoutineReturnStatement*       aRoutineReturnStatement();
    Ast::RoutineReturnStatement*       aRoutineReturnStatement(const Ast::Expr& expr);
    Ast::FunctionReturnStatement*      aFunctionReturnStatement(const Ast::ExprList& exprList);
    Ast::CompoundStatement*            aStatementList();
    Ast::CompoundStatement*            aStatementList(Ast::CompoundStatement& list, const Ast::Statement& statement);
    Ast::ExprList*                     aExprList(Ast::ExprList& list, const Ast::Expr& expr);
    Ast::ExprList*                     aExprList(const Ast::Expr& expr);
    Ast::ExprList*                     aExprList();

public:
    Ast::TernaryOpExpr*       aTernaryExpr(const Ast::Token& op1, const Ast::Token& op2, const Ast::Expr& lhs, const Ast::Expr& rhs1, const Ast::Expr& rhs2);
    Ast::BinaryOpExpr&        aBinaryExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
    Ast::PostfixOpExpr&       aPostfixExpr(const Ast::Token& op, const Ast::Expr& lhs);
    Ast::PrefixOpExpr&        aPrefixExpr(const Ast::Token& op, const Ast::Expr& rhs);
    Ast::StructMemberRefExpr& aStructMemberRefExpr(const Ast::StructDefn& structDef, const Ast::Token& name);
    Ast::EnumMemberRefExpr&   aEnumMemberRefExpr(const Ast::EnumDefn& enumDef, const Ast::Token& name);

    Ast::ListExpr*            aListExpr(const Ast::Token& pos, const Ast::ListList& list);
    Ast::ListList*            aListList(Ast::ListList& list, const Ast::ListItem& item);
    Ast::ListList*            aListList(const Ast::ListItem& item);
    Ast::ListList*            aListList();
    Ast::ListItem*            aListItem(const Ast::Expr& valueExpr);

    Ast::DictExpr*            aDictExpr(const Ast::Token& pos, const Ast::DictList& list);
    Ast::DictList*            aDictList(Ast::DictList& list, const Ast::DictItem& item);
    Ast::DictList*            aDictList(const Ast::DictItem& item);
    Ast::DictList*            aDictList();
    Ast::DictItem*            aDictItem(const Ast::Expr& keyExpr, const Ast::Expr& valueExpr);

    Ast::FormatExpr*          aFormatExpr(const Ast::Token& pos, const Ast::Expr& stringExpr, const Ast::DictExpr& dictExpr);
    Ast::FunctionCallExpr*    aFunctionCallExpr(const Ast::TypeSpec& typeSpec, const Ast::ExprList& exprList);
    Ast::OrderedExpr*         aOrderedExpr(const Ast::Expr& expr);
    Ast::VariableRefExpr*     aVariableRefExpr(const Ast::Token& name);
    Ast::VariableMemberExpr*  aVariableMemberExpr(const Ast::Expr& expr, const Ast::Token& name);
    Ast::TypeSpecMemberExpr*  aTypeSpecMemberExpr(const Ast::TypeSpec& typeSpec, const Ast::Token& name);
    Ast::ConstantExpr&        aConstantExpr(const std::string& type, const Ast::Token& value);
};
