#pragma once
#include "base/unit.hpp"

namespace z {
namespace Ast {
    struct ClosureRef {
        Ast::Scope* xref;
        Ast::Scope* iref;
    };

    class Factory {
    public:
        Factory(Ast::Module& module);
        ~Factory();

    public:
        inline const Unit& unit() const {return _module.unit();}
        inline Unit& unit() {return _module.unit();}
        inline const z::string& filename() const {return _module.filename();}
        inline const Ast::TypeSpec* hasRootTypeSpec(const Ast::Token& name) const {return unit().hasRootTypeSpec(_module.level(), name);}
    private:
        inline const Ast::TemplateDefn& getTemplateDefn(const Ast::Token& name, const Ast::Expr& expr, const z::string& cname, const Ast::TemplateDefn::size_type& len);
        inline const Ast::Expr& getDefaultValue(const Ast::TypeSpec& typeSpec, const Ast::Token& name);
        inline const Ast::Expr& convertExprToExpectedTypeSpec(const Ast::Token& pos, const Ast::Expr& initExpr);
    private:
        inline const Ast::Token& getToken() const {return _lastToken;}
        inline Ast::Namespace& getUnitNamespace(const Ast::Token& name);
        inline Ast::ExprList& addExprList(const Ast::Token& pos);
        inline Ast::QualifiedTypeSpec& addQualifiedTypeSpec(const Ast::Token& pos, const bool& isConst, const TypeSpec& typeSpec, const bool& isRef, const bool& isStrong);
        inline const Ast::QualifiedTypeSpec& getQualifiedTypeSpec(const Ast::Token& pos, const z::string& name);
        inline Ast::Scope& addScope(const Ast::Token& pos, const Ast::ScopeType::T& type);
        inline Ast::VariableDefn& addVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name);
        inline Ast::RootFunctionDecl& addRootFunctionDecl(const Ast::TypeSpec& parent, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType, const Ast::ClosureRef& cref);
        inline Ast::ChildFunctionDecl& addChildFunctionDecl(const Ast::TypeSpec& parent, const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::TypeSpec& base, const Ast::ClosureRef& cref);
        inline Ast::ValueInstanceExpr& getValueInstanceExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec, const Ast::TemplateDefn& srcTemplateDefn, const Ast::TemplateDefn& templateDefn, const Ast::ExprList& exprList);
        inline Ast::ChildFunctionDefn& createChildFunctionDefn(Ast::TypeSpec& parent, const Ast::Function& base, const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::ClosureRef& cref);
        inline const Ast::Expr& switchDictKeyValue(const Ast::Token& pos, const Unit::ExpectedTypeSpec::Type& popType, const Unit::ExpectedTypeSpec::Type& pushType, const Ast::TemplateDefn::size_type& idx, const Ast::Expr& initExpr);
        inline const Ast::QualifiedTypeSpec& getFunctionReturnType(const Ast::Token& pos, const Ast::Function& function);
        inline const Ast::FunctionRetn& getFunctionRetn(const Ast::Token& pos, const Ast::Function& function);
        inline Ast::TemplateDefn& createTemplateDefn(const Ast::Token& pos, const z::string& name, const Ast::TemplateTypePartList& list);
        inline void setDefaultDummyValue(const Ast::Token& name, Ast::TypeSpec& typeSpec);

    private:
        template <typename T> inline Ast::Expr& createBooleanExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        template <typename T> inline Ast::Expr& createBinaryExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        template <typename T> inline Ast::Expr& createBinaryOpExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        template <typename T> inline T& createPostfixExpr(const Ast::Token& op, const Ast::Expr& lhs);
        template <typename T> inline T& createPrefixExpr(const Ast::Token& op, const Ast::Expr& lhs);

    private:
        Ast::Module& _module;
        Ast::Token _lastToken;

    public:
        void                     aUnitStatementList(const Ast::EnterNamespaceStatement& ns);
        Ast::Module::Level_t     aImportStatement(const Ast::Token& pos, const Ast::AccessType::T& accessType, const Ast::HeaderType::T& headerType, const Ast::DefinitionType::T& defType, Ast::NamespaceList& list, z::string& filename);
        Ast::NamespaceList*      aImportNamespaceList(Ast::NamespaceList& list, const Ast::Token& name);
        Ast::NamespaceList*      aImportNamespaceList(const Ast::Token& name);
        Ast::EnterNamespaceStatement* aNamespaceStatement(const Ast::Token& pos, Ast::NamespaceList& list);
        Ast::EnterNamespaceStatement* aNamespaceStatement();
        Ast::NamespaceList*      aUnitNamespaceList(Ast::NamespaceList& list, const Ast::Token& name);
        Ast::NamespaceList*      aUnitNamespaceList(const Ast::Token& name);
        void                     aLeaveNamespace();
        Ast::Statement*          aGlobalTypeSpecStatement(const Ast::AccessType::T& accessType, Ast::UserDefinedTypeSpec& typeSpec);
        Ast::Statement*          aGlobalStatement(Ast::Statement& statement);

        void                     aGlobalCoerceStatement(Ast::CoerceList& list);
        Ast::CoerceList*         aCoerceList(Ast::CoerceList& list, const Ast::TypeSpec& typeSpec);
        Ast::CoerceList*         aCoerceList(const Ast::TypeSpec& typeSpec);

        void                     aGlobalDefaultStatement(const Ast::TypeSpec& typeSpec, const Ast::Expr& expr);

    public:
        Ast::TypedefDecl*        aTypedefDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType);
        Ast::TypedefDefn*        aTypedefDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::QualifiedTypeSpec& qTypeSpec);
        Ast::TemplatePartList*   aTemplatePartList(Ast::TemplatePartList& list, const Ast::Token& name);
        Ast::TemplatePartList*   aTemplatePartList(const Ast::Token& name);
        Ast::TemplateDecl*       aTemplateDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::TemplatePartList& list);
        Ast::EnumDefn*           aEnumDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list);
        Ast::EnumDecl*           aEnumDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType);
        Ast::Scope*              aEnumMemberDefnList(Ast::Scope& list, const Ast::VariableDefn& variableDefn);
        Ast::Scope*              aEnumMemberDefnListEmpty(const Ast::Token& pos);
        Ast::VariableDefn*       aEnumMemberDefn(const Ast::Token& name);
        Ast::VariableDefn*       aEnumMemberDefn(const Ast::Token& name, const Ast::Expr& initExpr);
        Ast::StructDecl*         aStructDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType);
        Ast::RootStructDefn*     aLeaveRootStructDefn(Ast::RootStructDefn& structDefn);
        Ast::ChildStructDefn*    aLeaveChildStructDefn(Ast::ChildStructDefn& structDefn);
        Ast::RootStructDefn*     aEnterRootStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType);
        Ast::ChildStructDefn*    aEnterChildStructDefn(const Ast::Token& name, const Ast::StructDefn& base, const Ast::DefinitionType::T& defType);
        void                     aStructMemberVariableDefn(const Ast::VariableDefn& vdef);
        void                     aStructMemberTypeDefn(Ast::UserDefinedTypeSpec& typeSpec);
        void                     aStructMemberPropertyDefn(Ast::PropertyDecl& typeSpec);
        Ast::PropertyDeclRW*     aStructPropertyDeclRW(const Ast::Token& pos, const Ast::QualifiedTypeSpec& propertyType, const Ast::Token& name, const Ast::DefinitionType::T& defType);
        Ast::PropertyDeclRO*     aStructPropertyDeclRO(const Ast::Token& pos, const Ast::QualifiedTypeSpec& propertyType, const Ast::Token& name, const Ast::DefinitionType::T& defType);
        Ast::RoutineDecl*        aRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const Ast::DefinitionType::T& defType);
        Ast::RoutineDecl*        aVarArgRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::DefinitionType::T& defType);
        Ast::RoutineDefn*        aRoutineDefn(Ast::RoutineDefn& routineDefn, const Ast::CompoundStatement& block);
        Ast::RoutineDefn*        aEnterRoutineDefn(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const Ast::DefinitionType::T& defType);
        Ast::RootFunctionDecl*   aRootFunctionDecl(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType, const Ast::ClosureRef& cref);
        Ast::ChildFunctionDecl*  aChildFunctionDecl(const Ast::TypeSpec& base, const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::ClosureRef& cref);
        Ast::RootFunctionDefn*   aRootFunctionDefn(Ast::RootFunctionDefn& functionDefn, const Ast::CompoundStatement& block);
        Ast::RootFunctionDefn*   aEnterRootFunctionDefn(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType, const Ast::ClosureRef& cref);
        Ast::ChildFunctionDefn*  aChildFunctionDefn(Ast::ChildFunctionDefn& functionImpl, const Ast::CompoundStatement& block);
        Ast::ChildFunctionDefn*  aEnterChildFunctionDefn(const Ast::TypeSpec& base, const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::ClosureRef& cref);
        Ast::RootInterfaceDefn*  aLeaveRootInterfaceDefn(Ast::RootInterfaceDefn& InterfaceDefn);
        Ast::RootInterfaceDefn*  aEnterRootInterfaceDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType);
        void                     aInterfaceMemberTypeDefn(Ast::UserDefinedTypeSpec& typeSpec);
        Ast::EventDecl*          aEventDecl(const Ast::Token& pos, const Ast::VariableDefn& in, const Ast::DefinitionType::T& eventDefType, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& handlerDefType);
        Ast::FunctionSig*        aFunctionSig(const Ast::Scope& out, const Ast::Token& name, Ast::Scope& in);
        Ast::FunctionSig*        aFunctionSig(const Ast::QualifiedTypeSpec& out, const Ast::Token& name, Ast::Scope& in);
        Ast::ClosureRef          aClosureList(Ast::Scope& xref, Ast::Scope& iref);
        Ast::ClosureRef          aClosureList(Ast::Scope& xref);
        Ast::ClosureRef          aClosureList();
        Ast::Scope*              aInParamsList(Ast::Scope& scope);
        Ast::Scope*              aParamsList(Ast::Scope& scope);
        Ast::Scope*              aParamsList(Ast::Scope& scope, const Ast::Scope& posParam);
        Ast::Scope*              aParam(Ast::Scope& list, const Ast::VariableDefn& variableDefn);
        Ast::Scope*              aParam(const Ast::VariableDefn& variableDefn, const ScopeType::T& type);
        Ast::Scope*              aParam(const ScopeType::T& type);
        Ast::VariableDefn*       aVariableDefn(const Ast::Token& name, const Ast::Expr& initExpr);
        Ast::VariableDefn*       aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name);
        Ast::VariableDefn*       aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name, const Ast::Expr& initExpr);
        const Ast::QualifiedTypeSpec* aQualifiedVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec);
        void                     aAutoQualifiedVariableDefn();
        Ast::QualifiedTypeSpec*  aQualifiedTypeSpec(const Ast::Token& pos, const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef, const bool& isStrong);
        Ast::QualifiedTypeSpec*  aQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef, const bool& isStrong);
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
        const Ast::EventDecl*    aEventTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name);
        const Ast::EventDecl*    aEventTypeSpec(const Ast::Token& name);
        const Ast::EventDecl*    aEventTypeSpec(const Ast::EventDecl& event);
        const Ast::TypeSpec*     aOtherTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name);
        const Ast::TypeSpec*     aOtherTypeSpec(const Ast::Token& name);
        const Ast::TypeSpec*     aTypeSpec(const Ast::TypeSpec& typeSpec);
        const Ast::TemplateDefn* aTemplateDefnTypeSpec(const Ast::TemplateDecl& typeSpec, const Ast::TemplateTypePartList& list);
        Ast::TemplateTypePartList* aTemplateTypePartList(Ast::TemplateTypePartList& list, const Ast::QualifiedTypeSpec& qTypeSpec);
        Ast::TemplateTypePartList* aTemplateTypePartList(const Ast::QualifiedTypeSpec& qTypeSpec);

    public:
        Ast::UserDefinedTypeSpecStatement* aUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec);
        Ast::EmptyStatement*               aEmptyStatement(const Ast::Token& pos);
        Ast::AutoStatement*                aAutoStatement(const Ast::VariableDefn& defn);
        Ast::ExprStatement*                aExprStatement(const Ast::Expr& expr);
        Ast::PrintStatement*               aPrintStatement(const Ast::Token& pos, const Ast::Expr& expr);
        Ast::IfStatement*                  aIfStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& tblock);
        Ast::IfElseStatement*              aIfElseStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& tblock, const Ast::CompoundStatement& fblock);
        Ast::WhileStatement*               aWhileStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& block);
        Ast::DoWhileStatement*             aDoWhileStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& block);
        Ast::ForStatement*                 aForStatement(const Ast::Token& pos, const Ast::Expr& init, const Ast::Expr& expr, const Ast::Expr& incr, const Ast::CompoundStatement& block);
        Ast::ForStatement*                 aForStatement(const Ast::Token& pos, const Ast::VariableDefn& init, const Ast::Expr& expr, const Ast::Expr& incr, const Ast::CompoundStatement& block);
        const Ast::VariableDefn*           aEnterForInit(const Ast::VariableDefn& init);
        Ast::ForeachStatement*             aForeachStatement(Ast::ForeachStatement& statement, const Ast::CompoundStatement& block);
        Ast::ForeachStatement*             aEnterForeachInit(const Ast::Token& valName, const Ast::Expr& expr);
        Ast::ForeachDictStatement*         aEnterForeachInit(const Ast::Token& keyName, const Ast::Token& valName, const Ast::Expr& expr);
        Ast::SwitchValueStatement*         aSwitchStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& list);
        Ast::SwitchExprStatement*          aSwitchStatement(const Ast::Token& pos, const Ast::CompoundStatement& list);
        Ast::CompoundStatement*            aCaseList(Ast::CompoundStatement& list, const Ast::CaseStatement& stmt);
        Ast::CompoundStatement*            aCaseList(const Ast::CaseStatement& stmt);
        Ast::CaseStatement*                aCaseStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& block);
        Ast::CaseStatement*                aCaseStatement(const Ast::Token& pos, const Ast::CompoundStatement& block);
        Ast::BreakStatement*               aBreakStatement(const Ast::Token& pos);
        Ast::ContinueStatement*            aContinueStatement(const Ast::Token& pos);
        Ast::AddEventHandlerStatement*     aAddEventHandlerStatement(const Ast::Token& pos, const Ast::EventDecl& event, const Ast::Expr& source, Ast::FunctionTypeInstanceExpr& functor);
        const Ast::EventDecl*              aEnterAddEventHandler(const Ast::EventDecl& eventDecl);
        Ast::RoutineReturnStatement*       aRoutineReturnStatement(const Ast::Token& pos);
        Ast::RoutineReturnStatement*       aRoutineReturnStatement(const Ast::Token& pos, const Ast::Expr& expr);
        Ast::FunctionReturnStatement*      aFunctionReturnStatement(const Ast::Token& pos, const Ast::ExprList& exprList);
        Ast::RaiseStatement*               aRaiseStatement(const Ast::Token& pos, const Ast::EventDecl& eventDecl, const Ast::Expr& expr, const Ast::ExprList& exprList);
        Ast::ExitStatement*                aExitStatement(const Ast::Token& pos, const Ast::Expr& expr);
        Ast::CompoundStatement*            aStatementList();
        Ast::CompoundStatement*            aStatementList(Ast::CompoundStatement& list, const Ast::Statement& statement);
        void                               aEnterCompoundStatement(const Ast::Token& pos);
        void                               aLeaveCompoundStatement();
        void                               aEnterFunctionBlock(const Ast::Token& pos);
        void                               aLeaveFunctionBlock();
        Ast::ExprList*                     aExprList(Ast::ExprList& list, const Ast::Expr& expr);
        Ast::ExprList*                     aExprList(const Ast::Expr& expr);
        Ast::ExprList*                     aExprList();

    public:
        Ast::TernaryOpExpr* aConditionalExpr(const Ast::Token& op1, const Ast::Token& op2, const Ast::Expr& lhs, const Ast::Expr& rhs1, const Ast::Expr& rhs2);

    public:
        Ast::Expr& aBooleanAndExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBooleanOrExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBooleanEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBooleanNotEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBooleanLessThanExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBooleanGreaterThanExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBooleanLessThanOrEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBooleanGreaterThanOrEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBooleanHasExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);

    public:
        Ast::Expr& aBinaryAssignEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryPlusEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryMinusEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryTimesEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryDivideEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryModEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryBitwiseAndEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryBitwiseOrEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryBitwiseXorEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryShiftLeftEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryShiftRightEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);

    public:
        Ast::Expr& aBinaryAssignExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryPlusExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryMinusExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryTimesExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryDivideExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryModExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryBitwiseAndExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryBitwiseOrExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryBitwiseXorExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryShiftLeftExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);
        Ast::Expr& aBinaryShiftRightExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs);

    public:
        Ast::PostfixIncExpr&       aPostfixIncExpr(const Ast::Token& op, const Ast::Expr& lhs);
        Ast::PostfixDecExpr&       aPostfixDecExpr(const Ast::Token& op, const Ast::Expr& lhs);

        Ast::PrefixNotExpr&        aPrefixNotExpr(const Ast::Token& op, const Ast::Expr& rhs);
        Ast::PrefixPlusExpr&       aPrefixPlusExpr(const Ast::Token& op, const Ast::Expr& rhs);
        Ast::PrefixMinusExpr&      aPrefixMinusExpr(const Ast::Token& op, const Ast::Expr& rhs);
        Ast::PrefixIncExpr&        aPrefixIncExpr(const Ast::Token& op, const Ast::Expr& rhs);
        Ast::PrefixDecExpr&        aPrefixDecExpr(const Ast::Token& op, const Ast::Expr& rhs);
        Ast::PrefixBitwiseNotExpr& aPrefixBitwiseNotExpr(const Ast::Token& op, const Ast::Expr& rhs);

        Ast::ListExpr*            aListExpr(const Ast::Token& pos, const Ast::ListList& list);
        Ast::ListList*            aListList(const Ast::Token& pos, Ast::ListList& list, const Ast::ListItem& item);
        Ast::ListList*            aListList(const Ast::Token& pos, const Ast::ListItem& item);
        Ast::ListList*            aListList(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec);
        Ast::ListList*            aListList(const Ast::Token& pos);
        Ast::ListItem*            aListItem(const Ast::Expr& valueExpr);

        Ast::DictExpr*            aDictExpr(const Ast::Token& pos, const Ast::DictList& list);
        Ast::DictList*            aDictList(const Ast::Token& pos, Ast::DictList& list, const Ast::DictItem& item);
        Ast::DictList*            aDictList(const Ast::Token& pos, const Ast::DictItem& item);
        Ast::DictList*            aDictList(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qKeyTypeSpec, const Ast::QualifiedTypeSpec& qValueTypeSpec);
        Ast::DictItem*            aDictItem(const Ast::Token& pos, const Ast::Expr& keyExpr, const Ast::Expr& valueExpr);
        const Ast::Expr*          aDictKey(const Ast::Expr& keyExpr);
        const Ast::Token&         aEnterList(const Ast::Token& pos);

        Ast::FormatExpr*          aFormatExpr(const Ast::Token& pos, const Ast::Expr& stringExpr, const Ast::DictExpr& dictExpr);
        Ast::RunExpr*             aRunExpr(const Ast::Token& pos, const Ast::FunctorCallExpr& callExpr);

        Ast::FunctorCallExpr*     aFunctorCallExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::ExprList& exprList);
        Ast::Expr*                aEnterFunctorCall(Ast::Expr& expr);
        Ast::Expr*                aEnterFunctorCall(const Ast::Token& name);
        Ast::Expr*                aEnterFunctorCall(const Ast::Function& function);

        Ast::RoutineCallExpr*     aRoutineCallExpr(const Ast::Token& pos, const Ast::Routine& routine, const Ast::ExprList& exprList);
        const Ast::Routine*       aEnterRoutineCall(const Ast::Routine& routine);

        Ast::ExprList*            aCallArgList(const Ast::Token& pos, Ast::ExprList& list, const Ast::Expr& expr);
        Ast::ExprList*            aCallArgList(const Ast::Expr& expr);
        Ast::ExprList*            aCallArgList();

        Ast::OrderedExpr*         aOrderedExpr(const Ast::Token& pos, const Ast::Expr& expr);
        Ast::IndexExpr*           aIndexExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::Expr& index);
        Ast::SpliceExpr*          aSpliceExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::Expr& from, const Ast::Expr& to);
        Ast::SizeofTypeExpr*      aSizeofTypeExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& typeSpec);
        Ast::SizeofExprExpr*      aSizeofExprExpr(const Ast::Token& pos, const Ast::Expr& expr);
        Ast::TypeofTypeExpr*      aTypeofTypeExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& typeSpec);
        Ast::TypeofExprExpr*      aTypeofExprExpr(const Ast::Token& pos, const Ast::Expr& expr);
        Ast::TypecastExpr*        aTypecastExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr);
        Ast::PointerInstanceExpr* aPointerInstanceExpr(const Ast::Token& pos, const Ast::Expr& expr);
        Ast::TemplateDefnInstanceExpr* aValueInstanceExpr(const Ast::Token& pos, const Ast::Expr& expr);
        Ast::TemplateDefnInstanceExpr* aTemplateDefnInstanceExpr(const Ast::Token& pos, const Ast::TemplateDefn& templateDefn, const Ast::ExprList& exprList);
        Ast::VariableRefExpr*     aVariableRefExpr(const Ast::Token& name);
        Ast::MemberExpr*          aMemberVariableExpr(const Ast::Expr& expr, const Ast::Token& name);
        Ast::TypeSpecMemberExpr*  aTypeSpecMemberExpr(const Ast::TypeSpec& typeSpec, const Ast::Token& name);
        Ast::StructInstanceExpr*  aStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn, const Ast::StructInitPartList& list);
        Ast::StructInstanceExpr*  aStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn);
        Ast::Expr*                aAutoStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn, const Ast::StructInitPartList& list);
        Ast::Expr*                aAutoStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn);
        const Ast::StructDefn*    aEnterStructInstanceExpr(const Ast::StructDefn& structDefn);
        const Ast::StructDefn*    aEnterAutoStructInstanceExpr(const Ast::Token& pos);
        void                      aLeaveStructInstanceExpr();
        const Ast::VariableDefn*  aEnterStructInitPart(const Ast::Token& name);
        void                      aLeaveStructInitPart(const Ast::Token& pos);
        Ast::StructInitPartList*  aStructInitPartList(Ast::StructInitPartList& list, const Ast::StructInitPart& part);
        Ast::StructInitPartList*  aStructInitPartList(const Ast::StructInitPart& part);
        Ast::StructInitPart*      aStructInitPart(const Ast::Token& pos, const Ast::VariableDefn& vdef, const Ast::Expr& initExpr);
        Ast::FunctionInstanceExpr*  aFunctionInstanceExpr(const Ast::Token& pos, const Ast::TypeSpec& typeSpec, const Ast::ExprList& exprList);
        Ast::AnonymousFunctionExpr* aAnonymousFunctionExpr(Ast::ChildFunctionDefn& functionDefn, const Ast::CompoundStatement& compoundStatement);
        Ast::ChildFunctionDefn*   aEnterAnonymousFunction(const Ast::Function& function, const Ast::ClosureRef& closureRef);
        Ast::ChildFunctionDefn*   aEnterAutoAnonymousFunction(const Ast::Token& pos, const Ast::ClosureRef& closureRef);
        Ast::ChildFunctionDefn*   aEnterAnonymousFunctionExpr(const Ast::Function& function);

        Ast::ConstantNullExpr&    aConstantNullExpr(const Ast::Token& value);
        Ast::ConstantFloatExpr&   aConstantFloatExpr(const Ast::Token& value);
        Ast::ConstantDoubleExpr&  aConstantDoubleExpr(const Ast::Token& value);
        Ast::ConstantBooleanExpr& aConstantBooleanExpr(const Ast::Token& value);
        Ast::ConstantStringExpr&  aConstantStringExpr(const Ast::Token& value);
        Ast::ConstantCharExpr&    aConstantCharExpr(const Ast::Token& value);

        Ast::ConstantLongExpr&    aConstantLongExpr(const Ast::Token& value, const char& fmt);
        Ast::ConstantIntExpr&     aConstantIntExpr(const Ast::Token& value, const char& fmt);
        Ast::ConstantShortExpr&   aConstantShortExpr(const Ast::Token& value, const char& fmt);
        Ast::ConstantByteExpr&    aConstantByteExpr(const Ast::Token& value, const char& fmt);

        Ast::ConstantUnLongExpr&    aConstantUnLongExpr(const Ast::Token& value, const char& fmt);
        Ast::ConstantUnIntExpr&     aConstantUnIntExpr(const Ast::Token& value, const char& fmt);
        Ast::ConstantUnShortExpr&   aConstantUnShortExpr(const Ast::Token& value, const char& fmt);
        Ast::ConstantUnByteExpr&    aConstantUnByteExpr(const Ast::Token& value, const char& fmt);
    };
}
}
