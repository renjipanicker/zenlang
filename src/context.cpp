#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "context.hpp"
#include "error.hpp"

inline Ast::Root& Context::getRootNamespace() const {
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

inline Ast::QualifiedTypeSpec& Context::addQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef) {
    Ast::QualifiedTypeSpec& qualifiedTypeSpec = _unit.addNode(new Ast::QualifiedTypeSpec(isConst, typeSpec, isRef));
    return qualifiedTypeSpec;
}

inline const Ast::QualifiedTypeSpec& Context::getQualifiedTypeSpec(const Ast::Token& pos, const std::string& name) {
    Ast::Token token(pos.row(), pos.col(), name);
    const Ast::TypeSpec& typeSpec = getRootTypeSpec<Ast::TypeSpec>(token);
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, typeSpec, false);
    return qTypeSpec;
}

Context::Context(Compiler& compiler, Ast::Unit& unit, const int& level) : _compiler(compiler), _unit(unit), _level(level) {
    Ast::Root& rootTypeSpec = getRootNamespace();
    enterTypeSpec(rootTypeSpec);
}

Context::~Context() {
    assert(_typeSpecStack.size() == 1);
    Ast::Root& rootTypeSpec = getRootNamespace();
    leaveTypeSpec(rootTypeSpec);
}

inline Ast::Scope& Context::addScope(const Ast::ScopeType::T& type) {
    Ast::Scope& scope = _unit.addNode(new Ast::Scope(type));
    return scope;
}

inline Ast::Scope& Context::enterScope(Ast::Scope& scope) {
    _scopeStack.push_back(ptr(scope));
    return scope;
}

inline Ast::Scope& Context::leaveScope() {
    Ast::Scope* s = _scopeStack.back();
    assert(_scopeStack.size() > 0);
    _scopeStack.pop_back();
    return ref(s);
}

inline Ast::Scope& Context::currentScope() {
    assert(_scopeStack.size() > 0);
    Ast::Scope* scope = _scopeStack.back();
    return ref(scope);
}

const Ast::TypeSpec* Context::hasRootTypeSpec(const Ast::Token& name) const {
    const Ast::TypeSpec* typeSpec = findTypeSpec(currentTypeSpec(), name);
    if(typeSpec)
        return typeSpec;

    if(_level == 0) {
        typeSpec = _unit.importNS().hasChild(name.text());
        if(typeSpec)
            return typeSpec;
    }

    return 0;
}

template <typename T>
inline const T& Context::getRootTypeSpec(const Ast::Token &name) const {
    const Ast::TypeSpec* typeSpec = hasRootTypeSpec(name);
    if(!typeSpec) {
        throw Exception("Unknown root type '%s'\n", name.text());
    }
    const T* tTypeSpec = dynamic_cast<const T*>(typeSpec);
    if(!tTypeSpec) {
        throw Exception("Type mismatch '%s'\n", name.text());
    }
    return ref(tTypeSpec);
}

inline const Ast::VariableDefn* Context::hasMember(const Ast::Scope& scope, const Ast::Token& name) {
    for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
        const Ast::VariableDefn& vref = ref(*it);
        if(vref.name().string() == name.string())
            return ptr(vref);
    }
    return 0;
}

inline Ast::ExprList& Context::addExprList() {
    Ast::ExprList& exprList = _unit.addNode(new Ast::ExprList());
    return exprList;
}

inline const Ast::QualifiedTypeSpec& coerce(const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs) {
    /// \todo
    return lhs;
}

inline const Ast::Expr& Context::getInitExpr(const Ast::TypeSpec& typeSpec, const Ast::Token& name) {
    if((typeSpec.name().string() == "short")
            || (typeSpec.name().string() == "int")
            || (typeSpec.name().string() == "long")
            || (typeSpec.name().string() == "float")
            || (typeSpec.name().string() == "double")
            ) {
        Ast::Token value(name.row(), name.col(), "0");
        return aConstantExpr(typeSpec.name().string(), value);
    }

    if((typeSpec.name().string() == "char")
            || (typeSpec.name().string() == "string")
            ) {
        Ast::Token value(name.row(), name.col(), "");
        return aConstantExpr(typeSpec.name().string(), value);
    }

    if((typeSpec.name().string() == "bool")) {
        Ast::Token value(name.row(), name.col(), "false");
        return aConstantExpr(typeSpec.name().string(), value);
    }

    // dummy code for now.
    Ast::Token value(name.row(), name.col(), "#");
    return aConstantExpr("int", value);

    //throw Exception("Unknown init value for type '%s'\n", typeSpec.name().text());
}

inline Ast::TemplateDefn& Context::createTemplateDefn(const Ast::Token& pos, const std::string& name) {
    Ast::Token token(pos.row(), pos.col(), name);
    const Ast::TemplateDecl& templateDecl = getRootTypeSpec<Ast::TemplateDecl>(token);
    Ast::TemplateDefn& templateDefn = _unit.addNode(new Ast::TemplateDefn(currentTypeSpec(), token, Ast::DefinitionType::Direct, templateDecl));
    return templateDefn;
}

////////////////////////////////////////////////////////////
void Context::aUnitNamespaceId(const Ast::Token &name) {
    Ast::Namespace& ns = _unit.addNode(new Ast::Namespace(currentTypeSpec(), name));
    currentTypeSpec().addChild(ns);
    enterTypeSpec(ns);
    _namespaceStack.push_back(ptr(ns));
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

Ast::ImportStatement* Context::aImportNamespaceId(Ast::ImportStatement& statement, const Ast::Token& name) {
    statement.addPart(name);
    return ptr(statement);
}
Ast::ImportStatement* Context::aImportNamespaceId(const Ast::Token& name) {
    Ast::ImportStatement& statement = _unit.addNode(new Ast::ImportStatement());
    _unit.addImportStatement(statement);
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

Ast::TemplatePartList* Context::aTemplatePartList(Ast::TemplatePartList& list, const Ast::Token& name) {
    list.addPart(name);
    return ptr(list);
}

Ast::TemplatePartList* Context::aTemplatePartList(const Ast::Token& name) {
    Ast::TemplatePartList& list = _unit.addNode(new Ast::TemplatePartList());
    list.addPart(name);
    return ptr(list);
}

Ast::TemplateDecl* Context::aTemplateDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::TemplatePartList& list) {
    Ast::TemplateDecl& templateDefn = _unit.addNode(new Ast::TemplateDecl(currentTypeSpec(), name, defType, list));
    currentTypeSpec().addChild(templateDefn);
    return ptr(templateDefn);
}

Ast::EnumDefn* Context::aEnumDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list) {
    Ast::EnumDefn& enumDefn = _unit.addNode(new Ast::EnumDefn(currentTypeSpec(), name, defType, list));
    currentTypeSpec().addChild(enumDefn);
    return ptr(enumDefn);
}

Ast::EnumDefn* Context::aEnumDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& scope = addScope(Ast::ScopeType::Member);
    return aEnumDefn(name, defType, scope);
}

Ast::Scope* Context::aEnumMemberDefnList(Ast::Scope& list, const Ast::VariableDefn& variableDefn) {
    list.addVariableDef(variableDefn);
    return ptr(list);
}

Ast::Scope* Context::aEnumMemberDefnList(const Ast::VariableDefn& variableDefn) {
    Ast::Scope& scope = addScope(Ast::ScopeType::Member);
    return aEnumMemberDefnList(scope, variableDefn);
}

Ast::VariableDefn* Context::aEnumMemberDefn(const Ast::Token& name) {
    Ast::Token value(name.row(), name.col(), "#");
    const Ast::ConstantExpr& initExpr = aConstantExpr("int", value);
    Ast::VariableDefn& variableDefn = _unit.addNode(new Ast::VariableDefn(initExpr.qTypeSpec(), name, initExpr));
    return ptr(variableDefn);
}

Ast::VariableDefn* Context::aEnumMemberDefn(const Ast::Token& name, const Ast::Expr& initExpr) {
    Ast::VariableDefn& variableDefn = _unit.addNode(new Ast::VariableDefn(initExpr.qTypeSpec(), name, initExpr));
    return ptr(variableDefn);
}

Ast::StructDefn* Context::aStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list) {
    Ast::StructDefn& structDefn = _unit.addNode(new Ast::StructDefn(currentTypeSpec(), name, defType, list));
    currentTypeSpec().addChild(structDefn);
    return ptr(structDefn);
}

Ast::StructDefn* Context::aStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& scope = addScope(Ast::ScopeType::Member);
    return aStructDefn(name, defType, scope);
}

Ast::Scope* Context::aStructMemberDefnList(Ast::Scope& list, const Ast::VariableDefn& enumMemberDefn) {
    list.addVariableDef(enumMemberDefn);
    return ptr(list);
}

Ast::Scope* Context::aStructMemberDefnList(const Ast::VariableDefn& enumMemberDefn) {
    Ast::Scope& list = addScope(Ast::ScopeType::Member);
    return aStructMemberDefnList(list, enumMemberDefn);
}

Ast::RoutineDecl* Context::aRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::Scope& in, const Ast::DefinitionType::T& defType) {
    Ast::RoutineDecl& routineDecl = _unit.addNode(new Ast::RoutineDecl(currentTypeSpec(), outType, name, in, defType));
    currentTypeSpec().addChild(routineDecl);
    return ptr(routineDecl);
}

Ast::RoutineDefn* Context::aRoutineDefn(Ast::RoutineDefn& routineDefn, const Ast::CompoundStatement& block) {
    routineDefn.setBlock(block);
    leaveScope();
    leaveTypeSpec(routineDefn);
    _unit.addBody(_unit.addNode(new Ast::RoutineBody(routineDefn, block)));
    return ptr(routineDefn);
}

Ast::RoutineDefn* Context::aEnterRoutineDefn(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const Ast::DefinitionType::T& defType) {
    Ast::RoutineDefn& routineDefn = _unit.addNode(new Ast::RoutineDefn(currentTypeSpec(), outType, name, in, defType));
    currentTypeSpec().addChild(routineDefn);
    enterScope(in);
    enterTypeSpec(routineDefn);
    return ptr(routineDefn);
}

Ast::FunctionDecl* Context::aFunctionDecl(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();
    Ast::FunctionDecl& functionDecl = _unit.addNode(new Ast::FunctionDecl(currentTypeSpec(), name, defType, functionSig));
    currentTypeSpec().addChild(functionDecl);
    return ptr(functionDecl);
}

Ast::RootFunctionDefn* Context::aRootFunctionDefn(Ast::RootFunctionDefn& functionDefn, const Ast::CompoundStatement& block) {
    functionDefn.setBlock(block);
    leaveScope();
    leaveTypeSpec(functionDefn);
    _unit.addBody(_unit.addNode(new Ast::FunctionBody(functionDefn, block)));
    return ptr(functionDefn);
}

Ast::RootFunctionDefn* Context::aEnterRootFunctionDefn(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();
    Ast::RootFunctionDefn& functionDefn = _unit.addNode(new Ast::RootFunctionDefn(currentTypeSpec(), name, defType, functionSig));
    currentTypeSpec().addChild(functionDefn);
    enterScope(functionSig.inScope());
    enterTypeSpec(functionDefn);

    Ast::Token token1(name.row(), name.col(), "_Out");
    Ast::FunctionRetn& functionRetn = _unit.addNode(new Ast::FunctionRetn(functionDefn, token1, functionSig.outScope()));
    functionDefn.addChild(functionRetn);

    Ast::Token token2(name.row(), name.col(), "_Impl");
    Ast::Functor& functor = _unit.addNode(new Ast::Functor(functionDefn, token2, functionDefn));
    functionDefn.addChild(functor);

    return ptr(functionDefn);
}

Ast::ChildFunctionDefn* Context::aChildFunctionDefn(Ast::ChildFunctionDefn& functionImpl, const Ast::CompoundStatement& block) {
    functionImpl.setBlock(block);
    leaveScope();
    leaveTypeSpec(functionImpl);
    _unit.addBody(_unit.addNode(new Ast::FunctionBody(functionImpl, block)));
    return ptr(functionImpl);
}

Ast::ChildFunctionDefn* Context::aEnterChildFunctionDefn(const Ast::TypeSpec& base, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(ptr(base));
    if(function == 0) {
        throw Exception("base type not a function '%s'\n", base.name().text());
    }
    Ast::ChildFunctionDefn& functionImpl = _unit.addNode(new Ast::ChildFunctionDefn(currentTypeSpec(), name, defType, ref(function).sig(), ref(function)));
    currentTypeSpec().addChild(functionImpl);
    enterScope(ref(function).sig().inScope());
    enterTypeSpec(functionImpl);

    return ptr(functionImpl);
}

Ast::EventDecl* Context::aEventDecl(const Ast::VariableDefn& in, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();
    Ast::EventDecl& eventDef = _unit.addNode(new Ast::EventDecl(currentTypeSpec(), name, in, functionSig, defType));
    currentTypeSpec().addChild(eventDef);
    return ptr(eventDef);
}

Ast::FunctionSig* Context::aFunctionSig(const Ast::Scope& out, const Ast::Token& name, Ast::Scope& in) {
    Ast::FunctionSig& functionSig = _unit.addNode(new Ast::FunctionSig(out, name, in));
    return ptr(functionSig);
}

Ast::Scope* Context::aInParamsList(Ast::Scope& scope) {
    return ptr(scope);
}

Ast::Scope* Context::aParam(Ast::Scope& list, const Ast::VariableDefn& variableDefn) {
    list.addVariableDef(variableDefn);
    return ptr(list);
}

Ast::Scope* Context::aParam(const Ast::VariableDefn& variableDefn) {
    Ast::Scope& list = addScope(Ast::ScopeType::Param);
    return aParam(list, variableDefn);
}

Ast::Scope* Context::aParam() {
    Ast::Scope& list = addScope(Ast::ScopeType::Param);
    return ptr(list);
}

Ast::VariableDefn* Context::aVariableDefn(const Ast::Token& name, const Ast::Expr& initExpr) {
    const Ast::QualifiedTypeSpec& qualifiedTypeSpec = initExpr.qTypeSpec();
    Ast::VariableDefn& variableDef = _unit.addNode(new Ast::VariableDefn(qualifiedTypeSpec, name, initExpr));
    return ptr(variableDef);
}

Ast::VariableDefn* Context::aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name) {
    const Ast::Expr& initExpr = getInitExpr(qualifiedTypeSpec.typeSpec(), name);
    Ast::VariableDefn& variableDef = _unit.addNode(new Ast::VariableDefn(qualifiedTypeSpec, name, initExpr));
    return ptr(variableDef);
}

Ast::QualifiedTypeSpec* Context::aQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef) {
    Ast::QualifiedTypeSpec& qualifiedTypeSpec = addQualifiedTypeSpec(isConst, typeSpec, isRef);
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
    const Ast::TypeSpec& typeSpec = getRootTypeSpec<Ast::TypeSpec>(name);
    return ptr(typeSpec);
}

Ast::UserDefinedTypeSpecStatement* Context::aUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec) {
    Ast::UserDefinedTypeSpecStatement& userDefinedTypeSpecStatement = _unit.addNode(new Ast::UserDefinedTypeSpecStatement(typeSpec));
    return ptr(userDefinedTypeSpecStatement);
}

Ast::LocalStatement* Context::aLocalStatement(const Ast::VariableDefn& defn) {
    Ast::LocalStatement& localStatement = _unit.addNode(new Ast::LocalStatement(defn));
    currentScope().addVariableDef(defn);
    return ptr(localStatement);
}

Ast::ExprStatement* Context::aExprStatement(const Ast::Expr& expr) {
    Ast::ExprStatement& exprStatement = _unit.addNode(new Ast::ExprStatement(expr));
    return ptr(exprStatement);
}

Ast::PrintStatement* Context::aPrintStatement(const Ast::FormatExpr& expr) {
    Ast::PrintStatement& printStatement = _unit.addNode(new Ast::PrintStatement(expr));
    return ptr(printStatement);
}

Ast::RoutineReturnStatement* Context::aRoutineReturnStatement() {
    Ast::ExprList& exprList = addExprList();
    Ast::RoutineReturnStatement& returnStatement = _unit.addNode(new Ast::RoutineReturnStatement(exprList));
    return ptr(returnStatement);
}

Ast::RoutineReturnStatement* Context::aRoutineReturnStatement(const Ast::Expr& expr) {
    Ast::ExprList& exprList = addExprList();
    exprList.addExpr(expr);
    Ast::RoutineReturnStatement& returnStatement = _unit.addNode(new Ast::RoutineReturnStatement(exprList));
    return ptr(returnStatement);
}

Ast::FunctionReturnStatement* Context::aFunctionReturnStatement(const Ast::ExprList& exprList) {
    Ast::FunctionReturnStatement& returnStatement = _unit.addNode(new Ast::FunctionReturnStatement(exprList));
    return ptr(returnStatement);
}

Ast::CompoundStatement* Context::aStatementList() {
    Ast::CompoundStatement& statement = _unit.addNode(new Ast::CompoundStatement());
    return ptr(statement);
}

Ast::CompoundStatement* Context::aStatementList(Ast::CompoundStatement& list, const Ast::Statement& statement) {
    list.addStatement(statement);
    return ptr(list);
}

void Context::aEnterCompoundStatement() {
    Ast::Scope& scope = addScope(Ast::ScopeType::Local);
    enterScope(scope);
}

void Context::aLeaveCompoundStatement() {
    leaveScope();
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

Ast::TernaryOpExpr* Context::aTernaryExpr(const Ast::Token& op1, const Ast::Token& op2, const Ast::Expr& lhs, const Ast::Expr& rhs1, const Ast::Expr& rhs2) {
    const Ast::QualifiedTypeSpec& qTypeSpec = coerce(rhs1.qTypeSpec(), rhs2.qTypeSpec());
    Ast::TernaryOpExpr& expr = _unit.addNode(new Ast::TernaryOpExpr(qTypeSpec, op1, op2, lhs, rhs1, rhs2));
    return ptr(expr);
}

Ast::BinaryOpExpr& Context::aBinaryExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    const Ast::QualifiedTypeSpec& qTypeSpec = coerce(lhs.qTypeSpec(), rhs.qTypeSpec());
    Ast::BinaryOpExpr& expr = _unit.addNode(new Ast::BinaryOpExpr(qTypeSpec, op, lhs, rhs));
    return expr;
}

Ast::PostfixOpExpr& Context::aPostfixExpr(const Ast::Token& op, const Ast::Expr& lhs) {
    const Ast::QualifiedTypeSpec& qTypeSpec = lhs.qTypeSpec();
    Ast::PostfixOpExpr& expr = _unit.addNode(new Ast::PostfixOpExpr(qTypeSpec, op, lhs));
    return expr;
}

Ast::PrefixOpExpr& Context::aPrefixExpr(const Ast::Token& op, const Ast::Expr& rhs) {
    const Ast::QualifiedTypeSpec& qTypeSpec = rhs.qTypeSpec();
    Ast::PrefixOpExpr& expr = _unit.addNode(new Ast::PrefixOpExpr(qTypeSpec, op, rhs));
    return expr;
}

Ast::StructMemberRefExpr& Context::aStructMemberRefExpr(const Ast::StructDefn& structDef, const Ast::Token& name) {
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, structDef, false);
    Ast::StructMemberRefExpr& expr = _unit.addNode(new Ast::StructMemberRefExpr(qTypeSpec, structDef, name));
    return expr;
}

Ast::EnumMemberRefExpr& Context::aEnumMemberRefExpr(const Ast::EnumDefn& enumDef, const Ast::Token& name) {
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, enumDef, false);
    Ast::EnumMemberRefExpr& expr = _unit.addNode(new Ast::EnumMemberRefExpr(qTypeSpec, enumDef, name));
    return expr;
}

Ast::ListExpr* Context::aListExpr(const Ast::Token& pos, const Ast::ListList& list) {
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "list");
    templateDefn.addType(list.valueType());
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, templateDefn, false);

    Ast::ListExpr& expr = _unit.addNode(new Ast::ListExpr(qTypeSpec, list));
    return ptr(expr);
}

Ast::ListList* Context::aListList(Ast::ListList& list, const Ast::ListItem& item) {
    list.addItem(item);
    const Ast::QualifiedTypeSpec& qValueTypeSpec = coerce(list.valueType(), item.valueExpr().qTypeSpec());
    list.valueType(qValueTypeSpec);
    return ptr(list);
}

Ast::ListList* Context::aListList(const Ast::ListItem& item) {
    Ast::ListList& list = _unit.addNode(new Ast::ListList());
    list.addItem(item);
    list.valueType(item.valueExpr().qTypeSpec());
    return ptr(list);
}

Ast::ListList* Context::aListList() {
    Ast::ListList& list = _unit.addNode(new Ast::ListList());
    return ptr(list);
}

Ast::ListItem* Context::aListItem(const Ast::Expr& valueExpr) {
    Ast::ListItem& item = _unit.addNode(new Ast::ListItem(valueExpr));
    return ptr(item);
}

Ast::DictExpr* Context::aDictExpr(const Ast::Token& pos, const Ast::DictList& list) {
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "dict");
    templateDefn.addType(list.keyType());
    templateDefn.addType(list.valueType());
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, templateDefn, false);

    Ast::DictExpr& expr = _unit.addNode(new Ast::DictExpr(qTypeSpec, list));
    return ptr(expr);
}

Ast::DictList* Context::aDictList(Ast::DictList& list, const Ast::DictItem& item) {
    list.addItem(item);
    const Ast::QualifiedTypeSpec& qKeyTypeSpec = coerce(list.keyType(), item.keyExpr().qTypeSpec());
    const Ast::QualifiedTypeSpec& qValueTypeSpec = coerce(list.valueType(), item.valueExpr().qTypeSpec());
    list.keyType(qKeyTypeSpec);
    list.valueType(qValueTypeSpec);
    return ptr(list);
}

Ast::DictList* Context::aDictList(const Ast::DictItem& item) {
    Ast::DictList& list = _unit.addNode(new Ast::DictList());
    list.addItem(item);
    list.keyType(item.keyExpr().qTypeSpec());
    list.valueType(item.valueExpr().qTypeSpec());
    return ptr(list);
}

Ast::DictList* Context::aDictList() {
    Ast::DictList& list = _unit.addNode(new Ast::DictList());
    return ptr(list);
}

Ast::DictItem* Context::aDictItem(const Ast::Expr& keyExpr, const Ast::Expr& valueExpr) {
    Ast::DictItem& item = _unit.addNode(new Ast::DictItem(keyExpr, valueExpr));
    return ptr(item);
}

Ast::FormatExpr* Context::aFormatExpr(const Ast::Token& pos, const Ast::Expr& stringExpr, const Ast::DictExpr& dictExpr) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "string");
    Ast::FormatExpr& formatExpr = _unit.addNode(new Ast::FormatExpr(qTypeSpec, stringExpr, dictExpr));
    return ptr(formatExpr);
}

Ast::CallExpr* Context::aCallExpr(const Ast::TypeSpec& typeSpec, const Ast::ExprList& exprList) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(ptr(typeSpec));
    if(function != 0) {
        const Ast::TypeSpec* retn = typeSpec.hasChild("_Out");
        const Ast::FunctionRetn* functionRetn = dynamic_cast<const Ast::FunctionRetn*>(retn);
        if(functionRetn == 0) {
            throw Exception("Unknown function return type '%s'\n", typeSpec.name().text());
        }
        Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, ref(functionRetn), false);
        Ast::FunctionCallExpr& functionCallExpr = _unit.addNode(new Ast::FunctionCallExpr(qTypeSpec, ref(function), exprList));
        return ptr(functionCallExpr);
    }
    const Ast::Routine* routine = dynamic_cast<const Ast::Routine*>(ptr(typeSpec));
    if(routine != 0) {
        const Ast::QualifiedTypeSpec& qTypeSpec = ref(routine).outType();
        Ast::RoutineCallExpr& routineCallExpr = _unit.addNode(new Ast::RoutineCallExpr(qTypeSpec, ref(routine), exprList));
        return ptr(routineCallExpr);
    }
    throw Exception("Unknown function being called '%s'\n", typeSpec.name().text());
}

Ast::CallExpr* Context::aCallExpr(const Ast::Expr& expr, const Ast::ExprList& exprList) {
    const Ast::TypeSpec* retn = expr.qTypeSpec().typeSpec().hasChild("_Out");
    const Ast::FunctionRetn* functionRetn = dynamic_cast<const Ast::FunctionRetn*>(retn);
    if(functionRetn == 0) {
        throw Exception("Unknown function return type '%s'\n", expr.qTypeSpec().typeSpec().name().text());
    }
    Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, ref(functionRetn), false);
    Ast::FunctorCallExpr& functorCallExpr = _unit.addNode(new Ast::FunctorCallExpr(qTypeSpec, expr, exprList));
    return ptr(functorCallExpr);
}

Ast::OrderedExpr* Context::aOrderedExpr(const Ast::Expr& innerExpr) {
    Ast::OrderedExpr& expr = _unit.addNode(new Ast::OrderedExpr(innerExpr.qTypeSpec(), innerExpr));
    return ptr(expr);
}

Ast::VariableRefExpr* Context::aVariableRefExpr(const Ast::Token& name) {
    Ast::RefType::T refType = Ast::RefType::Local;
    for(ScopeStack::const_reverse_iterator it = _scopeStack.rbegin(); it != _scopeStack.rend(); ++it) {
        const Ast::Scope& scope = ref(*it);

        switch(refType) {
            case Ast::RefType::Global:
                break;
            case Ast::RefType::XRef:
                break;
            case Ast::RefType::Param:
                switch(scope.type()) {
                    case Ast::ScopeType::Global:
                        throw Exception("Internal error: Invalid reference to global variable '%s'\n", name.text());
                    case Ast::ScopeType::Member:
                        throw Exception("Internal error: Invalid reference to member variable '%s'\n", name.text());
                    case Ast::ScopeType::Param:
                        refType = Ast::RefType::XRef;
                        break;
                    case Ast::ScopeType::Local:
                        refType = Ast::RefType::XRef;
                        break;
                }
                break;
            case Ast::RefType::Local:
                switch(scope.type()) {
                    case Ast::ScopeType::Global:
                        throw Exception("Internal error: Invalid reference to global variable '%s'\n", name.text());
                    case Ast::ScopeType::Member:
                        throw Exception("Internal error: Invalid reference to member variable '%s'\n", name.text());
                    case Ast::ScopeType::Param:
                        refType = Ast::RefType::Param;
                        break;
                    case Ast::ScopeType::Local:
                        break;
                }
                break;
        }

        const Ast::VariableDefn* vref = hasMember(scope, name);
        if(vref != 0) {
            Ast::VariableRefExpr& vrefExpr = _unit.addNode(new Ast::VariableRefExpr(ref(vref).qualifiedTypeSpec(), ref(vref), refType));
            return ptr(vrefExpr);
        }
    }
    throw Exception("Variable not found: '%s'\n", name.text());
}

Ast::VariableMemberExpr* Context::aVariableMemberExpr(const Ast::Expr& expr, const Ast::Token& name) {
    const Ast::TypeSpec& typeSpec = expr.qTypeSpec().typeSpec();

    const Ast::StructDefn* structDefn = dynamic_cast<const Ast::StructDefn*>(ptr(typeSpec));
    if(structDefn != 0) {
        const Ast::VariableDefn* vref = hasMember(ref(structDefn).scope(), name);
        if(vref == 0) {
            throw Exception("%s %s is not a member of expression type '%s'\n", err(_unit, typeSpec.name()).c_str(), name.text(), typeSpec.name().text());
        }
        Ast::VariableMemberExpr& vdefExpr = _unit.addNode(new Ast::VariableMemberExpr(ref(vref).qualifiedTypeSpec(), expr, ref(vref)));
        return ptr(vdefExpr);
    }

    throw Exception("%s Not a aggregate expression type '%s'\n", err(_unit, typeSpec.name()).c_str(), typeSpec.name().text());
}

Ast::TypeSpecMemberExpr* Context::aTypeSpecMemberExpr(const Ast::TypeSpec& typeSpec, const Ast::Token& name) {
    const Ast::EnumDefn* enumDefn = dynamic_cast<const Ast::EnumDefn*>(ptr(typeSpec));
    if(enumDefn != 0) {
        const Ast::VariableDefn* vref = hasMember(ref(enumDefn).scope(), name);
        if(vref == 0) {
            throw Exception("%s %s is not a member of type '%s'\n", err(_unit, typeSpec.name()).c_str(), name.text(), typeSpec.name().text());
        }
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, typeSpec, false);
        Ast::TypeSpecMemberExpr& typeSpecMemberExpr = _unit.addNode(new Ast::TypeSpecMemberExpr(qTypeSpec, typeSpec, ref(vref)));
        return ptr(typeSpecMemberExpr);
    }

    const Ast::StructDefn* structDefn = dynamic_cast<const Ast::StructDefn*>(ptr(typeSpec));
    if(structDefn != 0) {
        const Ast::VariableDefn* vref = hasMember(ref(structDefn).scope(), name);
        if(vref == 0) {
            throw Exception("%s %s is not a member of type '%s'\n", err(_unit, typeSpec.name()).c_str(), name.text(), typeSpec.name().text());
        }
        Ast::TypeSpecMemberExpr& typeSpecMemberExpr = _unit.addNode(new Ast::TypeSpecMemberExpr(ref(vref).qualifiedTypeSpec(), typeSpec, ref(vref)));
        return ptr(typeSpecMemberExpr);
    }

    throw Exception("%s Not an aggregate type '%s'\n", err(_unit, name).c_str(), typeSpec.name().text());
}

Ast::StructInstanceExpr* Context::aStructInstanceExpr(const Ast::Token& pos, const Ast::TypeSpec& typeSpec, const Ast::StructInitPartList& list) {
    const Ast::StructDefn* structDefn = dynamic_cast<const Ast::StructDefn*>(ptr(typeSpec));
    if(structDefn != 0) {
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, typeSpec, false);
        Ast::StructInstanceExpr& structInstanceExpr = _unit.addNode(new Ast::StructInstanceExpr(qTypeSpec, ref(structDefn), list));
        return ptr(structInstanceExpr);
    }

    throw Exception("%s Not a struct type '%s'\n", err(_unit, pos).c_str(), typeSpec.name().text());
}

Ast::StructInitPartList* Context::aStructInitPartList(Ast::StructInitPartList& list, const Ast::StructInitPart& part) {
    list.addPart(part);
    return ptr(list);
}

Ast::StructInitPartList* Context::aStructInitPartList(const Ast::StructInitPart& part) {
    Ast::StructInitPartList& list = _unit.addNode(new Ast::StructInitPartList());
    list.addPart(part);
    return ptr(list);
}

Ast::StructInitPartList* Context::aStructInitPartList() {
    Ast::StructInitPartList& list = _unit.addNode(new Ast::StructInitPartList());
    return ptr(list);
}

Ast::StructInitPart* Context::aStructInitPart(const Ast::Token& name, const Ast::Expr& expr) {
    Ast::StructInitPart& part = _unit.addNode(new Ast::StructInitPart(name, expr));
    return ptr(part);
}

Ast::FunctionInstanceExpr* Context::aFunctionInstanceExpr(const Ast::TypeSpec& typeSpec, const Ast::ExprList& exprList) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(ptr(typeSpec));
    if(function != 0) {
        Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, ref(function), false);
        Ast::FunctionInstanceExpr& functionInstanceExpr = _unit.addNode(new Ast::FunctionInstanceExpr(qTypeSpec, ref(function), exprList));
        return ptr(functionInstanceExpr);
    }

    throw Exception("%s: Not a aggregate type '%s'\n", err(_unit, typeSpec.name()).c_str(), typeSpec.name().text());
}

Ast::ConstantExpr& Context::aConstantExpr(const std::string& type, const Ast::Token& value) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(value, type);
    Ast::ConstantExpr& expr = _unit.addNode(new Ast::ConstantExpr(qTypeSpec, value));
    return expr;
}
