#pragma once

#include "ast.hpp"
#include "compiler.hpp"

class Context {
public:
    typedef std::list<Ast::TypeSpec*> TypeSpecStack;
public:
    Context(Compiler& compiler, Ast::Unit& unit, const int& level, const std::string& filename);
    ~Context();

public:
    inline const std::string& filename() const {return _filename;}
    inline const Ast::TypeSpec* currentTypeRef() const {return _currentTypeRef;}

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
    inline const Ast::Expr& getDefaultValue(const Ast::TypeSpec& typeSpec, const Ast::Token& name);
    inline const Ast::FunctionRetn& getFunctionRetn(const Ast::Token& pos, const Ast::Function& function);

private:
    inline Ast::Scope& addScope(const Ast::ScopeType::T& type);
    inline Ast::Scope& enterScope(Ast::Scope& scope);
    inline Ast::Scope& leaveScope();
    inline Ast::Scope& leaveScope(Ast::Scope& scope);
    inline Ast::Scope& currentScope();

private:
    inline void setCurrentTypeRef(const Ast::TypeSpec& typeSpec);
    inline void resetCurrentTypeRef();
    inline const Ast::QualifiedTypeSpec& coerce(const Ast::Token& pos, const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs);
    inline Ast::VariableDefn& addVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name);
    inline const Ast::TemplateDefn& getTemplateDefn(const Ast::Token& name, const Ast::Expr& expr, const std::string& cname, const size_t& len);
    inline Ast::FunctionDecl& addFunctionDecl(const Ast::TypeSpec& parent, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType);

private:
    Compiler& _compiler;
    Ast::Unit& _unit;
    const int _level;
    const std::string _filename;

private:
    typedef std::list<Ast::Scope*> ScopeStack;

private:
    ScopeStack                 _scopeStack;
    TypeSpecStack              _typeSpecStack;
    std::list<Ast::Namespace*> _namespaceStack;
    const Ast::TypeSpec*       _currentTypeRef;

public:
    void                     aUnitNamespaceId(const Ast::Token& name);
    void                     aLeaveNamespace();
    void                     aImportStatement(const Ast::HeaderType::T& headerType, Ast::ImportStatement& statement, const Ast::DefinitionType::T& defType);
    Ast::ImportStatement*    aImportNamespaceId(Ast::ImportStatement& statement, const Ast::Token& name);
    Ast::ImportStatement*    aImportNamespaceId(const Ast::Token& name);
    Ast::Statement*          aGlobalTypeSpecStatement(const Ast::AccessType::T& accessType, Ast::UserDefinedTypeSpec& typeSpec);

    void                     aGlobalCoerceStatement(Ast::CoerceList& list);
    Ast::CoerceList*         aCoerceList(Ast::CoerceList& list, const Ast::TypeSpec& typeSpec);
    Ast::CoerceList*         aCoerceList(const Ast::TypeSpec& typeSpec);

    void                     aGlobalDefaultStatement(const Ast::TypeSpec& typeSpec, const Ast::Expr& expr);

public:
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
    Ast::RootStructDefn*     aRootStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list);
    Ast::RootStructDefn*     aRootStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::ChildStructDefn*    aChildStructDefn(const Ast::StructDefn& base, const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list);
    Ast::ChildStructDefn*    aChildStructDefn(const Ast::StructDefn& base, const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::Scope*              aStructMemberDefnList(Ast::Scope& list, const Ast::VariableDefn& enumMemberDefn);
    Ast::Scope*              aStructMemberDefnList(const Ast::VariableDefn& enumMemberDefn);
    Ast::RoutineDecl*        aRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const Ast::DefinitionType::T& defType);
    Ast::RoutineDefn*        aRoutineDefn(Ast::RoutineDefn& routineDefn, const Ast::CompoundStatement& block);
    Ast::RoutineDefn*        aEnterRoutineDefn(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const Ast::DefinitionType::T& defType);
    Ast::FunctionDecl*       aFunctionDecl(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType);
    Ast::RootFunctionDefn*   aRootFunctionDefn(Ast::RootFunctionDefn& functionDefn, const Ast::CompoundStatement& block);
    Ast::RootFunctionDefn*   aEnterRootFunctionDefn(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType);
    Ast::ChildFunctionDefn*  aChildFunctionDefn(Ast::ChildFunctionDefn& functionImpl, const Ast::CompoundStatement& block);
    Ast::ChildFunctionDefn*  aEnterChildFunctionDefn(const Ast::TypeSpec& base, const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::EventDecl*          aEventDecl(const Ast::Token& pos, const Ast::VariableDefn& in, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType);
    Ast::FunctionSig*        aFunctionSig(const Ast::Scope& out, const Ast::Token& name, Ast::Scope& in);
    Ast::Scope*              aInParamsList(Ast::Scope& scope);
    Ast::Scope*              aParam(Ast::Scope& list, const Ast::VariableDefn& variableDefn);
    Ast::Scope*              aParam(const Ast::VariableDefn& variableDefn);
    Ast::Scope*              aParam();
    Ast::VariableDefn*       aVariableDefn(const Ast::Token& name, const Ast::Expr& initExpr);
    Ast::VariableDefn*       aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name);
    Ast::VariableDefn*       aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name, const Ast::Expr& initExpr);
    Ast::QualifiedTypeSpec*  aQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef);
    const Ast::TemplateDecl* aTemplateTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name);
    const Ast::TemplateDecl* aTemplateTypeSpec(const Ast::Token& name);
    const Ast::TemplateDecl* aTemplateTypeSpec(const Ast::TemplateDecl& templateDecl);
    const Ast::StructDefn*   aStructTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name);
    const Ast::StructDefn*   aStructTypeSpec(const Ast::Token& name);
    const Ast::StructDefn*   aStructTypeSpec(const Ast::StructDefn& structDefn);
    const Ast::Routine*      aRoutineTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name);
    const Ast::Routine*      aRoutineTypeSpec(const Ast::Token& name);
    const Ast::Routine*      aRoutineTypeSpec(const Ast::Routine& routine);
    const Ast::Function*     aFunctionTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name);
    const Ast::Function*     aFunctionTypeSpec(const Ast::Token& name);
    const Ast::Function*     aFunctionTypeSpec(const Ast::Function& function);
    const Ast::TypeSpec*     aOtherTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name);
    const Ast::TypeSpec*     aOtherTypeSpec(const Ast::Token& name);
    const Ast::TypeSpec*     aTypeSpec(const Ast::TypeSpec& typeSpec);

public:
    Ast::UserDefinedTypeSpecStatement* aUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec);
    Ast::LocalStatement*               aLocalStatement(const Ast::VariableDefn& defn);
    Ast::ExprStatement*                aExprStatement(const Ast::Expr& expr);
    Ast::PrintStatement*               aPrintStatement(const Ast::FormatExpr& expr);
    Ast::IfStatement*                  aIfStatement(const Ast::Expr& expr, const Ast::CompoundStatement& tblock);
    Ast::IfElseStatement*              aIfElseStatement(const Ast::Expr& expr, const Ast::CompoundStatement& tblock, const Ast::CompoundStatement& fblock);
    Ast::WhileStatement*               aWhileStatement(const Ast::Expr& expr, const Ast::CompoundStatement& block);
    Ast::DoWhileStatement*             aDoWhileStatement(const Ast::Expr& expr, const Ast::CompoundStatement& block);
    Ast::ForStatement*                 aForStatement(const Ast::Expr& init, const Ast::Expr& expr, const Ast::Expr& incr, const Ast::CompoundStatement& block);
    Ast::ForStatement*                 aForStatement(const Ast::VariableDefn& init, const Ast::Expr& expr, const Ast::Expr& incr, const Ast::CompoundStatement& block);
    const Ast::VariableDefn*           aEnterForInit(const Ast::VariableDefn& init);
    Ast::ForeachStatement*             aForeachStatement(Ast::ForeachStatement& statement, const Ast::CompoundStatement& block);
    Ast::ForeachListStatement*         aEnterForeachInit(const Ast::Token& valName, const Ast::Expr& expr);
    Ast::ForeachDictStatement*         aEnterForeachInit(const Ast::Token& keyName, const Ast::Token& valName, const Ast::Expr& expr);
    Ast::SwitchValueStatement*         aSwitchStatement(const Ast::Expr& expr, const Ast::CompoundStatement& list);
    Ast::SwitchExprStatement*          aSwitchStatement(const Ast::CompoundStatement& list);
    Ast::CompoundStatement*            aCaseList(Ast::CompoundStatement& list, const Ast::CaseStatement& stmt);
    Ast::CompoundStatement*            aCaseList(const Ast::CaseStatement& stmt);
    Ast::CaseStatement*                aCaseStatement(const Ast::Expr& expr, const Ast::CompoundStatement& block);
    Ast::CaseStatement*                aCaseStatement(const Ast::CompoundStatement& block);
    Ast::BreakStatement*               aBreakStatement();
    Ast::ContinueStatement*            aContinueStatement();
    Ast::RoutineReturnStatement*       aRoutineReturnStatement();
    Ast::RoutineReturnStatement*       aRoutineReturnStatement(const Ast::Expr& expr);
    Ast::FunctionReturnStatement*      aFunctionReturnStatement(const Ast::ExprList& exprList);
    Ast::CompoundStatement*            aStatementList();
    Ast::CompoundStatement*            aStatementList(Ast::CompoundStatement& list, const Ast::Statement& statement);
    void                               aEnterCompoundStatement();
    void                               aLeaveCompoundStatement();
    Ast::ExprList*                     aExprList(Ast::ExprList& list, const Ast::Expr& expr);
    Ast::ExprList*                     aExprList(const Ast::Expr& expr);
    Ast::ExprList*                     aExprList();

public:
    Ast::TernaryOpExpr*       aTernaryExpr(const Ast::Token& op1, const Ast::Token& op2, const Ast::Expr& lhs, const Ast::Expr& rhs1, const Ast::Expr& rhs2);
    Ast::BinaryOpExpr&        aBinaryExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
    Ast::PostfixOpExpr&       aPostfixExpr(const Ast::Token& op, const Ast::Expr& lhs);
    Ast::PrefixOpExpr&        aPrefixExpr(const Ast::Token& op, const Ast::Expr& rhs);

    Ast::ListExpr*            aListExpr(const Ast::Token& pos, const Ast::ListList& list);
    Ast::ListList*            aListList(const Ast::Token& pos, Ast::ListList& list, const Ast::ListItem& item);
    Ast::ListList*            aListList(const Ast::ListItem& item);
    Ast::ListList*            aListList();
    Ast::ListItem*            aListItem(const Ast::Expr& valueExpr);

    Ast::DictExpr*            aDictExpr(const Ast::Token& pos, const Ast::DictList& list);
    Ast::DictList*            aDictList(const Ast::Token& pos, Ast::DictList& list, const Ast::DictItem& item);
    Ast::DictList*            aDictList(const Ast::DictItem& item);
    Ast::DictList*            aDictList();
    Ast::DictItem*            aDictItem(const Ast::Expr& keyExpr, const Ast::Expr& valueExpr);

    Ast::FormatExpr*          aFormatExpr(const Ast::Token& pos, const Ast::Expr& stringExpr, const Ast::DictExpr& dictExpr);

    Ast::RunExpr*             aRunExpr(const Ast::Token& pos, const Ast::FunctorCallExpr& callExpr);

    Ast::FunctorCallExpr*     aFunctorCallExpr(const Ast::Token& pos, const Ast::Token& name, const Ast::ExprList& exprList);
    Ast::FunctorCallExpr*     aFunctorCallExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::ExprList& exprList);
    Ast::FunctorCallExpr*     aFunctionCallExpr(const Ast::Token& pos, const Ast::Function& function, const Ast::ExprList& exprList);
    Ast::RoutineCallExpr*     aRoutineCallExpr(const Ast::Token& pos, const Ast::Routine& routine, const Ast::ExprList& exprList);

    Ast::OrderedExpr*         aOrderedExpr(const Ast::Expr& expr);
    Ast::TypeofExpr*          aTypeofExpr(const Ast::Token& pos, const Ast::TypeSpec& typeSpec);
    Ast::VariableRefExpr*     aVariableRefExpr(const Ast::Token& name);
    Ast::VariableMemberExpr*  aVariableMemberExpr(const Ast::Expr& expr, const Ast::Token& name);
    Ast::TypeSpecMemberExpr*  aTypeSpecMemberExpr(const Ast::TypeSpec& typeSpec, const Ast::Token& name);
    Ast::StructInstanceExpr*  aStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn, const Ast::StructInitPartList& list);
    Ast::StructInstanceExpr*  aStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn);
    Ast::StructInitPartList*  aStructInitPartList(Ast::StructInitPartList& list, const Ast::StructInitPart& part);
    Ast::StructInitPartList*  aStructInitPartList(const Ast::StructInitPart& part);
    Ast::StructInitPartList*  aStructInitPartList();
    Ast::StructInitPart*      aStructInitPart(const Ast::Token& name, const Ast::Expr& expr);
    Ast::FunctionInstanceExpr* aFunctionInstanceExpr(const Ast::TypeSpec& typeSpec, const Ast::ExprList& exprList);
    Ast::FunctionInstanceExpr* aAnonymousFunctionExpr(Ast::ChildFunctionDefn& functionDefn, const Ast::CompoundStatement& compoundStatement);
    Ast::ChildFunctionDefn*   aEnterAnonymousFunction(const Ast::Function& function);
    Ast::ConstantExpr&        aConstantExpr(const std::string& type, const Ast::Token& value);
};
