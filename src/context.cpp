#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "context.hpp"
#include "error.hpp"

struct StructBaseIterator {
    inline StructBaseIterator(const Ast::StructDefn* structDefn) : _structDefn(structDefn) {}
    inline bool hasNext() const {return (_structDefn != 0);}
    inline const Ast::StructDefn& get() const {return ref(_structDefn);}
    inline void next() {
        const Ast::ChildStructDefn* csd = dynamic_cast<const Ast::ChildStructDefn*>(_structDefn);
        if(csd) {
            _structDefn = ptr(ref(csd).base());
        } else {
            _structDefn = 0;
        }
    }

private:
    const Ast::StructDefn* _structDefn;
};

inline Ast::Root& Context::getRootNamespace() const {
    return (_level == 0)?_unit.rootNS():_unit.importNS();
}

inline const Ast::TypeSpec* Context::findTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const {
    const Ast::TypeSpec* child = parent.hasChild<const Ast::TypeSpec>(name.text());
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

inline void Context::setCurrentTypeRef(const Ast::TypeSpec& typeSpec) {
    _currentTypeRef = ptr(typeSpec);
}

inline void Context::resetCurrentTypeRef() {
    _currentTypeRef = 0;
}

Context::Context(Compiler& compiler, Ast::Unit& unit, const int& level, const std::string& filename) : _compiler(compiler), _unit(unit), _level(level), _filename(filename), _currentTypeRef(0) {
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

inline Ast::Scope& Context::leaveScope(Ast::Scope& scope) {
    Ast::Scope& s = leaveScope();
    assert(ptr(s) == ptr(scope));
    return s;
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
        typeSpec = _unit.importNS().hasChild<const Ast::TypeSpec>(name.text());
        if(typeSpec)
            return typeSpec;
    }

    return 0;
}

template <typename T>
inline const T& Context::getRootTypeSpec(const Ast::Token &name) const {
    const Ast::TypeSpec* typeSpec = hasRootTypeSpec(name);
    if(!typeSpec) {
        throw Exception("%s Unknown root type '%s'\n", err(_filename, name).c_str(), name.text());
    }
    const T* tTypeSpec = dynamic_cast<const T*>(typeSpec);
    if(!tTypeSpec) {
        throw Exception("%s Type mismatch '%s'\n", err(_filename, name).c_str(), name.text());
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

inline const Ast::QualifiedTypeSpec* Context::canCoerce(const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs) {
    if(ptr(lhs.typeSpec()) == ptr(rhs.typeSpec())) {
        return ptr(lhs);
    }

    for(Ast::Unit::CoerceListList::const_iterator it = _unit.coercionList().begin(); it != _unit.coercionList().end(); ++it) {
        const Ast::CoerceList& coerceList = ref(*it);
        int lidx = -1;
        int ridx = -1;
        int cidx = 0;
        for(Ast::CoerceList::List::const_iterator cit = coerceList.list().begin(); cit != coerceList.list().end(); ++cit, ++cidx) {
            const Ast::TypeSpec& typeSpec = ref(*cit);
            if(ptr(typeSpec) == ptr(lhs.typeSpec())) {
                lidx = cidx;
            }
            if(ptr(typeSpec) == ptr(rhs.typeSpec())) {
                ridx = cidx;
            }
        }
        if((lidx >= 0) && (ridx >= 0)) {
            if(lidx >= ridx)
                return ptr(lhs);
            return ptr(rhs);
        }
    }

    const Ast::StructDefn* lsd = dynamic_cast<const Ast::StructDefn*>(ptr(lhs.typeSpec()));
    const Ast::StructDefn* rsd = dynamic_cast<const Ast::StructDefn*>(ptr(rhs.typeSpec()));
    if((lsd != 0) && (rsd != 0)) {
        for(StructBaseIterator sbi(lsd); sbi.hasNext(); sbi.next()) {
            if(ptr(sbi.get()) == rsd) {
                return ptr(rhs);
            }
        }
        for(StructBaseIterator sbi(rsd); sbi.hasNext(); sbi.next()) {
            if(ptr(sbi.get()) == lsd) {
                return ptr(lhs);
            }
        }
    }

    const Ast::TemplateDefn* ltd = dynamic_cast<const Ast::TemplateDefn*>(ptr(lhs.typeSpec()));
    const Ast::TemplateDefn* rtd = dynamic_cast<const Ast::TemplateDefn*>(ptr(rhs.typeSpec()));
    if((ltd != 0) && (rtd != 0)) {
        printf("is tempdefn\n");
        if(ref(ltd).name().string() == ref(rtd).name().string()) {
            const Ast::QualifiedTypeSpec& lSubType = ref(ltd).at(0);
            const Ast::QualifiedTypeSpec& rSubType = ref(rtd).at(0);
            printf("%s (%lu) %s(%lu)\n", lSubType.typeSpec().name().text(), (unsigned long)ptr(lSubType), rSubType.typeSpec().name().text(), (unsigned long)ptr(rSubType));
            const Ast::QualifiedTypeSpec* val = canCoerce(lSubType, rSubType);
            printf("val %lu\n", (unsigned long)val);
            if(val == ptr(lSubType)) {
                return ptr(lhs);
            }
            if(val == ptr(rSubType)) {
                return ptr(rhs);
            }
        }
    }

    return 0;
}

inline const Ast::QualifiedTypeSpec& Context::coerce(const Ast::Token& pos, const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs) {
    printf("%s checking '%s' and '%s'\n", err(_filename, pos).c_str(), lhs.typeSpec().name().text(), rhs.typeSpec().name().text());
    const Ast::QualifiedTypeSpec* val = canCoerce(lhs, rhs);
    if(!val) {
        throw Exception("%s Cannot coerce '%s' and '%s'\n", err(_filename, pos).c_str(), lhs.typeSpec().name().text(), rhs.typeSpec().name().text());
    }
    return ref(val);
}

inline const Ast::Expr& Context::getDefaultValue(const Ast::TypeSpec& typeSpec, const Ast::Token& name) {
    const Ast::Unit::DefaultValueList& list = _unit.defaultValueList();
    Ast::Unit::DefaultValueList::const_iterator it = list.find(ptr(typeSpec));
    if(it != list.end()) {
        return ref(it->second);
    }

    const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(ptr(typeSpec));
    if(td != 0) {
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, ref(td), false);
        Ast::ListList& llist = _unit.addNode(new Ast::ListList());
        const Ast::QualifiedTypeSpec* qlType = ref(td).list().at(0);
        llist.valueType(ref(qlType));
        Ast::ListExpr& expr = _unit.addNode(new Ast::ListExpr(qTypeSpec, llist));
        return expr;
    }

    const Ast::EnumDefn* ed = dynamic_cast<const Ast::EnumDefn*>(ptr(typeSpec));
    if(ed != 0) {
        const Ast::Scope::List::const_iterator rit = ref(ed).list().begin();
        if(rit == ref(ed).list().end()) {
            throw Exception("%s empty enum type '%s'\n", err(_filename, typeSpec.name()).c_str(), ref(ed).name().text());
        }
        const Ast::VariableDefn& vref = ref(*rit);
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, typeSpec, false);
        Ast::EnumMemberExpr& typeSpecMemberExpr = _unit.addNode(new Ast::EnumMemberExpr(qTypeSpec, typeSpec, vref));
        return typeSpecMemberExpr;
    }

    const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(ptr(typeSpec));
    if(sd != 0) {
        Ast::StructInstanceExpr* expr = aStructInstanceExpr(name, ref(sd));
        return ref(expr);
    }

    const Ast::Function* fd = dynamic_cast<const Ast::Function*>(ptr(typeSpec));
    if(fd != 0) {
        Ast::ExprList& exprList = addExprList();
        Ast::FunctionInstanceExpr* expr = aFunctionInstanceExpr(ref(fd), exprList);
        return ref(expr);
    }

    throw Exception("%s No default value for type '%s'\n", err(_filename, name).c_str(), typeSpec.name().text());
}

inline Ast::TemplateDefn& Context::createTemplateDefn(const Ast::Token& pos, const std::string& name) {
    Ast::Token token(pos.row(), pos.col(), name);
    const Ast::TemplateDecl& templateDecl = getRootTypeSpec<Ast::TemplateDecl>(token);
    Ast::TemplateDefn& templateDefn = _unit.addNode(new Ast::TemplateDefn(currentTypeSpec(), token, Ast::DefinitionType::Direct, templateDecl));
    return templateDefn;
}

inline const Ast::FunctionRetn& Context::getFunctionRetn(const Ast::Token& pos, const Ast::Function& function) {
    const Ast::Function* base = ptr(function);
    while(base != 0) {
        const Ast::ChildFunctionDefn* childFunctionDefn = dynamic_cast<const Ast::ChildFunctionDefn*>(base);
        if(childFunctionDefn == 0)
            break;
        base = ptr(ref(childFunctionDefn).base());
    }
    if(base != 0) {
        const Ast::FunctionRetn* functionRetn = ref(base).hasChild<const Ast::FunctionRetn>("_Out");
        if(functionRetn != 0) {
            return ref(functionRetn);
        }
    }
    throw Exception("%s Unknown function return type '%s'\n", err(_filename, pos).c_str(), function.name().text());
}

inline Ast::VariableDefn& Context::addVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name) {
    const Ast::Expr& initExpr = getDefaultValue(qualifiedTypeSpec.typeSpec(), name);
    Ast::VariableDefn& variableDef = _unit.addNode(new Ast::VariableDefn(qualifiedTypeSpec, name, initExpr));
    return variableDef;
}

inline const Ast::TemplateDefn& Context::getTemplateDefn(const Ast::Token& name, const Ast::Expr& expr, const std::string& cname, const size_t& len) {
    const Ast::TypeSpec& typeSpec = expr.qTypeSpec().typeSpec();
    if(typeSpec.name().string() != cname) {
        throw Exception("%s Expression is not of %s type: %s (1)\n", err(_filename, name).c_str(), cname.c_str(), typeSpec.name().text());
    }
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(ptr(typeSpec));
    if(templateDefn == 0) {
        throw Exception("%s Expression is not of %s type: %s (2)\n", err(_filename, name).c_str(), cname.c_str(), typeSpec.name().text());
    }
    if(ref(templateDefn).list().size() != len) {
        throw Exception("%s Expression is not of %s type: %s (3)\n", err(_filename, name).c_str(), cname.c_str(), typeSpec.name().text());
    }
    return ref(templateDefn);
}

inline Ast::FunctionDecl& Context::addFunctionDecl(const Ast::TypeSpec& parent, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();
    Ast::Scope& xref = addScope(Ast::ScopeType::XRef);
    Ast::FunctionDecl& functionDecl = _unit.addNode(new Ast::FunctionDecl(parent, name, defType, functionSig, xref));
    Ast::Token token1(name.row(), name.col(), "_Out");
    Ast::FunctionRetn& functionRetn = _unit.addNode(new Ast::FunctionRetn(functionDecl, token1, functionSig.outScope()));
    functionDecl.addChild(functionRetn);
    return functionDecl;
}

////////////////////////////////////////////////////////////
void Context::aUnitNamespaceId(const Ast::Token &name) {
    Ast::Namespace& ns = _unit.addNode(new Ast::Namespace(currentTypeSpec(), name));
    currentTypeSpec().addChild(ns);
    enterTypeSpec(ns);
    if(_level == 0) {
        _unit.addNamespacePart(name);
    }
    _namespaceStack.push_back(ptr(ns));
}

void Context::aLeaveNamespace() {
    while(_namespaceStack.size() > 0) {
        Ast::Namespace* ns = _namespaceStack.back();
        leaveTypeSpec(ref(ns));
        _namespaceStack.pop_back();
    }
}

void Context::aImportStatement(const Ast::AccessType::T& accessType, const Ast::HeaderType::T& headerType, Ast::ImportStatement& statement, const Ast::DefinitionType::T& defType) {
    statement.accessType(accessType);
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

void Context::aGlobalCoerceStatement(Ast::CoerceList& list) {
    _unit.addCoercionList(list);
}

Ast::CoerceList* Context::aCoerceList(Ast::CoerceList& list, const Ast::TypeSpec& typeSpec) {
    list.addTypeSpec(typeSpec);
    return ptr(list);
}

Ast::CoerceList* Context::aCoerceList(const Ast::TypeSpec& typeSpec) {
    Ast::CoerceList& list = _unit.addNode(new Ast::CoerceList());
    list.addTypeSpec(typeSpec);
    return ptr(list);
}

void Context::aGlobalDefaultStatement(const Ast::TypeSpec& typeSpec, const Ast::Expr& expr) {
    _unit.addDefaultValue(typeSpec, expr);
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

Ast::RootStructDefn* Context::aRootStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list) {
    Ast::RootStructDefn& structDefn = _unit.addNode(new Ast::RootStructDefn(currentTypeSpec(), name, defType, list));
    currentTypeSpec().addChild(structDefn);
    return ptr(structDefn);
}

Ast::RootStructDefn* Context::aRootStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& scope = addScope(Ast::ScopeType::Member);
    return aRootStructDefn(name, defType, scope);
}

Ast::ChildStructDefn* Context::aChildStructDefn(const Ast::StructDefn& base, const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list) {
    Ast::ChildStructDefn& structDefn = _unit.addNode(new Ast::ChildStructDefn(currentTypeSpec(), base, name, defType, list));
    currentTypeSpec().addChild(structDefn);
    return ptr(structDefn);
}

Ast::ChildStructDefn* Context::aChildStructDefn(const Ast::StructDefn& base, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& scope = addScope(Ast::ScopeType::Member);
    return aChildStructDefn(base, name, defType, scope);
}

Ast::Scope* Context::aStructMemberDefnList(Ast::Scope& list, const Ast::VariableDefn& enumMemberDefn) {
    list.addVariableDef(enumMemberDefn);
    return ptr(list);
}

Ast::Scope* Context::aStructMemberDefnList(const Ast::VariableDefn& enumMemberDefn) {
    Ast::Scope& list = addScope(Ast::ScopeType::Member);
    return aStructMemberDefnList(list, enumMemberDefn);
}

Ast::RoutineDecl* Context::aRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const Ast::DefinitionType::T& defType) {
    Ast::RoutineDecl& routineDecl = _unit.addNode(new Ast::RoutineDecl(currentTypeSpec(), outType, name, in, defType));
    currentTypeSpec().addChild(routineDecl);
    return ptr(routineDecl);
}

Ast::RoutineDefn* Context::aRoutineDefn(Ast::RoutineDefn& routineDefn, const Ast::CompoundStatement& block) {
    routineDefn.setBlock(block);
    leaveScope(routineDefn.inScope());
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
    Ast::FunctionDecl& functionDecl = addFunctionDecl(currentTypeSpec(), functionSig, defType);
    currentTypeSpec().addChild(functionDecl);
    return ptr(functionDecl);
}

Ast::RootFunctionDefn* Context::aRootFunctionDefn(Ast::RootFunctionDefn& functionDefn, const Ast::CompoundStatement& block) {
    functionDefn.setBlock(block);
    leaveScope(functionDefn.sig().inScope());
    leaveScope(functionDefn.xrefScope());
    leaveTypeSpec(functionDefn);
    _unit.addBody(_unit.addNode(new Ast::FunctionBody(functionDefn, block)));
    return ptr(functionDefn);
}

Ast::RootFunctionDefn* Context::aEnterRootFunctionDefn(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();
    Ast::Scope& xref = addScope(Ast::ScopeType::XRef);
    Ast::RootFunctionDefn& functionDefn = _unit.addNode(new Ast::RootFunctionDefn(currentTypeSpec(), name, defType, functionSig, xref));
    currentTypeSpec().addChild(functionDefn);
    enterScope(functionDefn.xrefScope());
    enterScope(functionSig.inScope());
    enterTypeSpec(functionDefn);

    Ast::Token token1(name.row(), name.col(), "_Out");
    Ast::FunctionRetn& functionRetn = _unit.addNode(new Ast::FunctionRetn(functionDefn, token1, functionSig.outScope()));
    functionDefn.addChild(functionRetn);

    return ptr(functionDefn);
}

Ast::ChildFunctionDefn* Context::aChildFunctionDefn(Ast::ChildFunctionDefn& functionDefn, const Ast::CompoundStatement& block) {
    functionDefn.setBlock(block);
    leaveScope(functionDefn.sig().inScope());
    leaveScope(functionDefn.xrefScope());
    leaveTypeSpec(functionDefn);
    _unit.addBody(_unit.addNode(new Ast::FunctionBody(functionDefn, block)));
    return ptr(functionDefn);
}

Ast::ChildFunctionDefn* Context::aEnterChildFunctionDefn(const Ast::TypeSpec& base, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(ptr(base));
    if(function == 0) {
        throw Exception("%s base type not a function '%s'\n", err(_filename, name).c_str(), base.name().text());
    }
    Ast::Scope& xref = addScope(Ast::ScopeType::XRef);
    Ast::ChildFunctionDefn& functionDefn = _unit.addNode(new Ast::ChildFunctionDefn(currentTypeSpec(), name, defType, ref(function).sig(), xref, ref(function)));
    currentTypeSpec().addChild(functionDefn);
    enterScope(functionDefn.xrefScope());
    enterScope(ref(function).sig().inScope());
    enterTypeSpec(functionDefn);


    return ptr(functionDefn);
}

Ast::EventDecl* Context::aEventDecl(const Ast::Token& pos, const Ast::VariableDefn& in, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();

    Ast::Token eventName(pos.row(), pos.col(), name.string());
    Ast::EventDecl& eventDef = _unit.addNode(new Ast::EventDecl(currentTypeSpec(), eventName, in, defType));
    currentTypeSpec().addChild(eventDef);

    Ast::Token handlerName(pos.row(), pos.col(), "Handler");
    Ast::FunctionSig* handlerSig = aFunctionSig(functionSig.outScope(), handlerName, functionSig.inScope());
    Ast::FunctionDecl& funDecl = addFunctionDecl(eventDef, ref(handlerSig), Ast::DefinitionType::Direct);
    eventDef.setHandler(funDecl);

    Ast::QualifiedTypeSpec& qFunTypeSpec = addQualifiedTypeSpec(false, funDecl, false);
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "functor");
    templateDefn.addType(qFunTypeSpec);
    const Ast::QualifiedTypeSpec& qFunctorTypeSpec = addQualifiedTypeSpec(false, templateDefn, false);

    Ast::Token hVarName(pos.row(), pos.col(), "handler");
    Ast::VariableDefn& vdef = addVariableDefn(qFunctorTypeSpec, hVarName);

    Ast::Scope& outAdd = addScope(Ast::ScopeType::Param);
    Ast::Scope& inAdd  = addScope(Ast::ScopeType::Param);
    Ast::Token nameAdd(pos.row(), pos.col(), "Add");
    Ast::FunctionSig* addSig = aFunctionSig(outAdd, nameAdd, inAdd);
    Ast::FunctionDecl& addDecl = addFunctionDecl(eventDef, ref(addSig), defType);
    eventDef.addChild(addDecl);

    inAdd.addVariableDef(in);
    inAdd.addVariableDef(vdef);

    return ptr(eventDef);
}

Ast::FunctionSig* Context::aFunctionSig(const Ast::Scope& out, const Ast::Token& name, Ast::Scope& in) {
    Ast::FunctionSig& functionSig = _unit.addNode(new Ast::FunctionSig(out, name, in));
    return ptr(functionSig);
}

Ast::Scope* Context::aInParamsList(Ast::Scope& scope) {
    return ptr(scope);
}

Ast::Scope* Context::aParamsList(Ast::Scope& scope) {
    return ptr(scope);
}

Ast::Scope* Context::aParamsList(Ast::Scope& scope, const Ast::Scope& posParam) {
    scope.posParam(posParam);
    return aParamsList(scope);
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

Ast::VariableDefn* Context::aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name, const Ast::Expr& initExpr) {
    Ast::VariableDefn& variableDef = _unit.addNode(new Ast::VariableDefn(qualifiedTypeSpec, name, initExpr));
    return ptr(variableDef);
}

Ast::VariableDefn* Context::aVariableDefn(const Ast::Token& name, const Ast::Expr& initExpr) {
    const Ast::QualifiedTypeSpec& qualifiedTypeSpec = initExpr.qTypeSpec();
    Ast::VariableDefn& variableDef = _unit.addNode(new Ast::VariableDefn(qualifiedTypeSpec, name, initExpr));
    return ptr(variableDef);
}

Ast::VariableDefn* Context::aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name) {
    Ast::VariableDefn& variableDef = addVariableDefn(qualifiedTypeSpec, name);
    return ptr(variableDef);
}

Ast::QualifiedTypeSpec* Context::aQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef) {
    Ast::QualifiedTypeSpec& qualifiedTypeSpec = addQualifiedTypeSpec(isConst, typeSpec, isRef);
    return ptr(qualifiedTypeSpec);
}

const Ast::TemplateDecl* Context::aTemplateTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    const Ast::TemplateDecl* templateDecl = parent.hasChild<const Ast::TemplateDecl>(name.text());
    if(!templateDecl) {
        throw Exception("%s template type expected '%s'\n", err(_filename, name).c_str(), name.text());
    }
    setCurrentTypeRef(ref(templateDecl));
    return templateDecl;
}

const Ast::TemplateDecl* Context::aTemplateTypeSpec(const Ast::Token& name) {
    const Ast::TemplateDecl& templateDecl = getRootTypeSpec<Ast::TemplateDecl>(name);
    setCurrentTypeRef(templateDecl);
    return ptr(templateDecl);
}

const Ast::TemplateDecl* Context::aTemplateTypeSpec(const Ast::TemplateDecl& templateDecl) {
    resetCurrentTypeRef();
    return ptr(templateDecl);
}

const Ast::StructDefn* Context::aStructTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    const Ast::StructDefn* structDefn = parent.hasChild<const Ast::StructDefn>(name.text());
    if(!structDefn) {
        throw Exception("%s struct type expected '%s'\n", err(_filename, name).c_str(), name.text());
    }
    setCurrentTypeRef(ref(structDefn));
    return structDefn;
}

const Ast::StructDefn* Context::aStructTypeSpec(const Ast::Token& name) {
    const Ast::StructDefn& structDefn = getRootTypeSpec<Ast::StructDefn>(name);
    setCurrentTypeRef(structDefn);
    return ptr(structDefn);
}

const Ast::StructDefn* Context::aStructTypeSpec(const Ast::StructDefn& structDefn) {
    resetCurrentTypeRef();
    return ptr(structDefn);
}

const Ast::Routine* Context::aRoutineTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    const Ast::Routine* routine = parent.hasChild<const Ast::Routine>(name.text());
    if(!routine) {
        throw Exception("%s routine type expected '%s'\n", err(_filename, name).c_str(), name.text());
    }
    setCurrentTypeRef(ref(routine));
    return routine;
}

const Ast::Routine* Context::aRoutineTypeSpec(const Ast::Token& name) {
    const Ast::Routine& routine = getRootTypeSpec<Ast::Routine>(name);
    setCurrentTypeRef(routine);
    return ptr(routine);
}

const Ast::Routine* Context::aRoutineTypeSpec(const Ast::Routine& routine) {
    resetCurrentTypeRef();
    return ptr(routine);
}

const Ast::Function* Context::aFunctionTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    const Ast::Function* function = parent.hasChild<const Ast::Function>(name.text());
    if(!function) {
        throw Exception("%s function type expected '%s'\n", err(_filename, name).c_str(), name.text());
    }
    setCurrentTypeRef(ref(function));
    return function;
}

const Ast::Function* Context::aFunctionTypeSpec(const Ast::Token& name) {
    const Ast::Function& function = getRootTypeSpec<Ast::Function>(name);
    setCurrentTypeRef(function);
    return ptr(function);
}

const Ast::Function* Context::aFunctionTypeSpec(const Ast::Function& function) {
    resetCurrentTypeRef();
    return ptr(function);
}

const Ast::EventDecl* Context::aEventTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    const Ast::EventDecl* event = parent.hasChild<const Ast::EventDecl>(name.text());
    if(!event) {
        throw Exception("%s event type expected '%s'\n", err(_filename, name).c_str(), name.text());
    }
    setCurrentTypeRef(ref(event));
    return event;
}

const Ast::EventDecl* Context::aEventTypeSpec(const Ast::Token& name) {
    const Ast::EventDecl& event = getRootTypeSpec<Ast::EventDecl>(name);
    setCurrentTypeRef(event);
    return ptr(event);
}

const Ast::EventDecl* Context::aEventTypeSpec(const Ast::EventDecl& event) {
    resetCurrentTypeRef();
    return ptr(event);
}

const Ast::TypeSpec* Context::aOtherTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    const Ast::TypeSpec* typeSpec = parent.hasChild<const Ast::TypeSpec>(name.text());
    if(!typeSpec) {
        throw Exception("%s Unknown child type '%s'\n", err(_filename, name).c_str(), name.text());
    }
    setCurrentTypeRef(ref(typeSpec));
    return typeSpec;
}

const Ast::TypeSpec* Context::aOtherTypeSpec(const Ast::Token& name) {
    const Ast::TypeSpec& typeSpec = getRootTypeSpec<Ast::TypeSpec>(name);
    setCurrentTypeRef(typeSpec);
    return ptr(typeSpec);
}

const Ast::TypeSpec* Context::aTypeSpec(const Ast::TypeSpec& typeSpec) {
    resetCurrentTypeRef();
    return ptr(typeSpec);
}

const Ast::TemplateDefn* Context::aTemplateDefnTypeSpec(const Ast::TemplateDecl& typeSpec, const Ast::TemplateTypePartList& list) {
    Ast::TemplateDefn& templateDefn = _unit.addNode(new Ast::TemplateDefn(currentTypeSpec(), typeSpec.name(), Ast::DefinitionType::Direct, typeSpec));
    for(Ast::TemplateTypePartList::List::const_iterator it = list.list().begin(); it != list.list().end(); ++it) {
        const Ast::QualifiedTypeSpec& part = ref(*it);
        templateDefn.addType(part);
    }
    return ptr(templateDefn);
}

Ast::TemplateTypePartList* Context::aTemplateTypePartList(Ast::TemplateTypePartList& list, const Ast::QualifiedTypeSpec& qTypeSpec) {
    list.addType(qTypeSpec);
    return ptr(list);
}

Ast::TemplateTypePartList* Context::aTemplateTypePartList(const Ast::QualifiedTypeSpec& qTypeSpec) {
    Ast::TemplateTypePartList& list = _unit.addNode(new Ast::TemplateTypePartList());
    return aTemplateTypePartList(list, qTypeSpec);
}

Ast::UserDefinedTypeSpecStatement* Context::aUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec) {
    Ast::UserDefinedTypeSpecStatement& userDefinedTypeSpecStatement = _unit.addNode(new Ast::UserDefinedTypeSpecStatement(typeSpec));
    return ptr(userDefinedTypeSpecStatement);
}

Ast::AutoStatement* Context::aAutoStatement(const Ast::VariableDefn& defn) {
    Ast::AutoStatement& localStatement = _unit.addNode(new Ast::AutoStatement(defn));
    currentScope().addVariableDef(defn);
    return ptr(localStatement);
}

Ast::ExprStatement* Context::aExprStatement(const Ast::Expr& expr) {
    Ast::ExprStatement& exprStatement = _unit.addNode(new Ast::ExprStatement(expr));
    return ptr(exprStatement);
}

Ast::PrintStatement* Context::aPrintStatement(const Ast::Expr& expr) {
    Ast::PrintStatement& printStatement = _unit.addNode(new Ast::PrintStatement(expr));
    return ptr(printStatement);
}

Ast::IfStatement* Context::aIfStatement(const Ast::Expr& expr, const Ast::CompoundStatement& tblock) {
    Ast::IfStatement& ifStatement = _unit.addNode(new Ast::IfStatement(expr, tblock));
    return ptr(ifStatement);
}

Ast::IfElseStatement* Context::aIfElseStatement(const Ast::Expr& expr, const Ast::CompoundStatement& tblock, const Ast::CompoundStatement& fblock) {
    Ast::IfElseStatement& ifElseStatement = _unit.addNode(new Ast::IfElseStatement(expr, tblock, fblock));
    return ptr(ifElseStatement);
}

Ast::WhileStatement* Context::aWhileStatement(const Ast::Expr& expr, const Ast::CompoundStatement& block) {
    Ast::WhileStatement& whileStatement = _unit.addNode(new Ast::WhileStatement(expr, block));
    return ptr(whileStatement);
}

Ast::DoWhileStatement* Context::aDoWhileStatement(const Ast::Expr& expr, const Ast::CompoundStatement& block) {
    Ast::DoWhileStatement& doWhileStatement = _unit.addNode(new Ast::DoWhileStatement(expr, block));
    return ptr(doWhileStatement);
}

Ast::ForStatement* Context::aForStatement(const Ast::Expr& init, const Ast::Expr& expr, const Ast::Expr& incr, const Ast::CompoundStatement& block) {
    Ast::ForExprStatement& forStatement = _unit.addNode(new Ast::ForExprStatement(init, expr, incr, block));
    return ptr(forStatement);
}

Ast::ForStatement* Context::aForStatement(const Ast::VariableDefn& init, const Ast::Expr& expr, const Ast::Expr& incr, const Ast::CompoundStatement& block) {
    Ast::ForInitStatement& forStatement = _unit.addNode(new Ast::ForInitStatement(init, expr, incr, block));
    leaveScope();
    return ptr(forStatement);
}

const Ast::VariableDefn* Context::aEnterForInit(const Ast::VariableDefn& init) {
    Ast::Scope& scope = addScope(Ast::ScopeType::Local);
    scope.addVariableDef(init);
    enterScope(scope);
    return ptr(init);
}

Ast::ForeachStatement* Context::aForeachStatement(Ast::ForeachStatement& statement, const Ast::CompoundStatement& block) {
    statement.setBlock(block);
    leaveScope();
    return ptr(statement);
}

Ast::ForeachListStatement* Context::aEnterForeachInit(const Ast::Token& valName, const Ast::Expr& expr) {
    const Ast::TemplateDefn& templateDefn = getTemplateDefn(valName, expr, "list", 1);
    const Ast::QualifiedTypeSpec& valTypeSpec = templateDefn.at(0);
    const Ast::VariableDefn& valDef = addVariableDefn(valTypeSpec, valName);
    Ast::Scope& scope = addScope(Ast::ScopeType::Local);
    scope.addVariableDef(valDef);
    enterScope(scope);

    Ast::ForeachListStatement& foreachStatement = _unit.addNode(new Ast::ForeachListStatement(valDef, expr));
    return ptr(foreachStatement);
}

Ast::ForeachDictStatement* Context::aEnterForeachInit(const Ast::Token& keyName, const Ast::Token& valName, const Ast::Expr& expr) {
    const Ast::TemplateDefn& templateDefn = getTemplateDefn(valName, expr, "dict", 2);
    const Ast::QualifiedTypeSpec& keyTypeSpec = templateDefn.at(0);
    const Ast::QualifiedTypeSpec& valTypeSpec = templateDefn.at(1);
    const Ast::VariableDefn& keyDef = addVariableDefn(keyTypeSpec, keyName);
    const Ast::VariableDefn& valDef = addVariableDefn(valTypeSpec, valName);
    Ast::Scope& scope = addScope(Ast::ScopeType::Local);
    scope.addVariableDef(keyDef);
    scope.addVariableDef(valDef);
    enterScope(scope);

    Ast::ForeachDictStatement& foreachStatement = _unit.addNode(new Ast::ForeachDictStatement(keyDef, valDef, expr));
    return ptr(foreachStatement);
}

Ast::SwitchValueStatement* Context::aSwitchStatement(const Ast::Expr& expr, const Ast::CompoundStatement& list) {
    Ast::SwitchValueStatement& switchStatement = _unit.addNode(new Ast::SwitchValueStatement(expr, list));
    return ptr(switchStatement);
}

Ast::SwitchExprStatement* Context::aSwitchStatement(const Ast::CompoundStatement& list) {
    Ast::SwitchExprStatement& switchStatement = _unit.addNode(new Ast::SwitchExprStatement(list));
    return ptr(switchStatement);
}

Ast::CompoundStatement* Context::aCaseList(Ast::CompoundStatement& list, const Ast::CaseStatement& stmt) {
    list.addStatement(stmt);
    return ptr(list);
}

Ast::CompoundStatement* Context::aCaseList(const Ast::CaseStatement& stmt) {
    Ast::CompoundStatement& list = _unit.addNode(new Ast::CompoundStatement());
    list.addStatement(stmt);
    return ptr(list);
}

Ast::CaseStatement* Context::aCaseStatement(const Ast::Expr& expr, const Ast::CompoundStatement& block) {
    Ast::CaseExprStatement& caseStatement = _unit.addNode(new Ast::CaseExprStatement(expr, block));
    return ptr(caseStatement);
}

Ast::CaseStatement* Context::aCaseStatement(const Ast::CompoundStatement& block) {
    Ast::CaseDefaultStatement& caseStatement = _unit.addNode(new Ast::CaseDefaultStatement(block));
    return ptr(caseStatement);
}

Ast::BreakStatement* Context::aBreakStatement() {
    Ast::BreakStatement& breakStatement = _unit.addNode(new Ast::BreakStatement());
    return ptr(breakStatement);
}

Ast::ContinueStatement* Context::aContinueStatement() {
    Ast::ContinueStatement& continueStatement = _unit.addNode(new Ast::ContinueStatement());
    return ptr(continueStatement);
}

Ast::AddEventHandlerStatement* Context::aAddEventHandlerStatement(const Ast::EventDecl& event, const Ast::Expr& source, Ast::FunctionInstanceExpr& functor) {
    Ast::AddEventHandlerStatement& addEventHandlerStatement = _unit.addNode(new Ast::AddEventHandlerStatement(event, source, functor));
    return ptr(addEventHandlerStatement);
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
    const Ast::QualifiedTypeSpec& qTypeSpec = coerce(op2, rhs1.qTypeSpec(), rhs2.qTypeSpec());
    Ast::TernaryOpExpr& expr = _unit.addNode(new Ast::TernaryOpExpr(qTypeSpec, op1, op2, lhs, rhs1, rhs2));
    return ptr(expr);
}

Ast::BinaryOpExpr& Context::aBinaryExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    const Ast::QualifiedTypeSpec& qTypeSpec = coerce(op, lhs.qTypeSpec(), rhs.qTypeSpec());
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

Ast::ListExpr* Context::aListExpr(const Ast::Token& pos, const Ast::ListList& list) {
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "list");
    templateDefn.addType(list.valueType());
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, templateDefn, false);

    Ast::ListExpr& expr = _unit.addNode(new Ast::ListExpr(qTypeSpec, list));
    return ptr(expr);
}

Ast::ListList* Context::aListList(const Ast::Token& pos, Ast::ListList& list, const Ast::ListItem& item) {
    list.addItem(item);
    const Ast::QualifiedTypeSpec& qValueTypeSpec = coerce(pos, list.valueType(), item.valueExpr().qTypeSpec());
    list.valueType(qValueTypeSpec);
    return ptr(list);
}

Ast::ListList* Context::aListList(const Ast::ListItem& item) {
    Ast::ListList& list = _unit.addNode(new Ast::ListList());
    list.addItem(item);
    list.valueType(item.valueExpr().qTypeSpec());
    return ptr(list);
}

Ast::ListList* Context::aListList(const Ast::QualifiedTypeSpec& qTypeSpec) {
    Ast::ListList& list = _unit.addNode(new Ast::ListList());
    list.valueType(qTypeSpec);
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

Ast::DictExpr* Context::aDictExpr(const Ast::Token& pos) {
    Ast::DictList& list = _unit.addNode(new Ast::DictList());
    return aDictExpr(pos, list);
}

Ast::DictList* Context::aDictList(const Ast::Token& pos, Ast::DictList& list, const Ast::DictItem& item) {
    list.addItem(item);
    const Ast::QualifiedTypeSpec& qKeyTypeSpec = coerce(pos, list.keyType(), item.keyExpr().qTypeSpec());
    const Ast::QualifiedTypeSpec& qValueTypeSpec = coerce(pos, list.valueType(), item.valueExpr().qTypeSpec());
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

Ast::DictList* Context::aDictList(const Ast::QualifiedTypeSpec& qKeyTypeSpec, const Ast::QualifiedTypeSpec& qValueTypeSpec) {
    Ast::DictList& list = _unit.addNode(new Ast::DictList());
    list.keyType(qKeyTypeSpec);
    list.valueType(qValueTypeSpec);
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

Ast::RoutineCallExpr* Context::aRoutineCallExpr(const Ast::Token& pos, const Ast::Routine& routine, const Ast::ExprList& exprList) {
    unused(pos);
    const Ast::QualifiedTypeSpec& qTypeSpec = routine.outType();
    Ast::RoutineCallExpr& routineCallExpr = _unit.addNode(new Ast::RoutineCallExpr(qTypeSpec, routine, exprList));
    return ptr(routineCallExpr);
}

Ast::FunctorCallExpr* Context::aFunctionCallExpr(const Ast::Token& pos, const Ast::Function& function, const Ast::ExprList& exprList) {
    unused(pos);
    Ast::QualifiedTypeSpec& qExprTypeSpec = addQualifiedTypeSpec(false, function, false);
    Ast::FunctionInstanceExpr& functionInstanceExpr = _unit.addNode(new Ast::FunctionInstanceExpr(qExprTypeSpec, function, exprList));

    const Ast::FunctionRetn& functionRetn = getFunctionRetn(pos, function);
    Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, functionRetn, false);
    Ast::FunctorCallExpr& functorCallExpr = _unit.addNode(new Ast::FunctorCallExpr(qTypeSpec, functionInstanceExpr, exprList));
    return ptr(functorCallExpr);
}

Ast::FunctorCallExpr* Context::aFunctorCallExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::ExprList& exprList) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(ptr(expr.qTypeSpec().typeSpec()));
    if(function != 0) {
        const Ast::FunctionRetn& functionRetn = getFunctionRetn(pos, ref(function));
        Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, functionRetn, false);
        Ast::FunctorCallExpr& functorCallExpr = _unit.addNode(new Ast::FunctorCallExpr(qTypeSpec, expr, exprList));
        return ptr(functorCallExpr);
    }
    throw Exception("%s Unknown functor being called '%s'\n", err(_filename, pos).c_str(), expr.qTypeSpec().typeSpec().name().text());
}

Ast::FunctorCallExpr* Context::aFunctorCallExpr(const Ast::Token& pos, const Ast::Token& name, const Ast::ExprList& exprList) {
    Ast::VariableRefExpr* expr = aVariableRefExpr(name);
    return aFunctorCallExpr(pos, ref(expr), exprList);
}

Ast::RunExpr* Context::aRunExpr(const Ast::Token& pos, const Ast::FunctorCallExpr& callExpr) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(ptr(callExpr.expr().qTypeSpec().typeSpec()));
    if(function != 0) {
        Ast::QualifiedTypeSpec& qRetTypeSpec = addQualifiedTypeSpec(false, ref(function), false);

        Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "future");
        templateDefn.addType(qRetTypeSpec);
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, templateDefn, false);

        Ast::RunExpr& runExpr = _unit.addNode(new Ast::RunExpr(qTypeSpec, callExpr));
        return ptr(runExpr);
    }
    throw Exception("%s Unknown functor in run expression '%s'\n", err(_filename, pos).c_str(), callExpr.expr().qTypeSpec().typeSpec().name().text());
}

Ast::OrderedExpr* Context::aOrderedExpr(const Ast::Expr& innerExpr) {
    Ast::OrderedExpr& expr = _unit.addNode(new Ast::OrderedExpr(innerExpr.qTypeSpec(), innerExpr));
    return ptr(expr);
}

Ast::TypeofExpr* Context::aTypeofExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& typeSpec, const Ast::Expr& expr) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "string");
    Ast::TypeofExpr& typeofExpr = _unit.addNode(new Ast::TypeofExpr(qTypeSpec, typeSpec, expr));
    return ptr(typeofExpr);
}

Ast::TemplateDefnInstanceExpr* Context::aTemplateDefnInstanceExpr(const Ast::Token& pos, const Ast::TemplateDefn& templateDefn, const Ast::ExprList& exprList) {
    if(templateDefn.name().string() == "pointer") {
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, templateDefn, false);
        Ast::PointerInstanceExpr& expr = _unit.addNode(new Ast::PointerInstanceExpr(qTypeSpec, templateDefn, exprList));
        return ptr(expr);
    }
    throw Exception("%s Invalid template instantiation %s\n", err(_filename, pos).c_str(), templateDefn.name().text());
}

Ast::VariableRefExpr* Context::aVariableRefExpr(const Ast::Token& name) {
    Ast::RefType::T refType = Ast::RefType::Local;
    typedef std::list<Ast::Scope*> ScopeList;
    ScopeList scopeList;

    for(ScopeStack::const_reverse_iterator it = _scopeStack.rbegin(); it != _scopeStack.rend(); ++it) {
        Ast::Scope& scope = ref(*it);

        if(scope.type() == Ast::ScopeType::XRef) {
            scopeList.push_back(ptr(scope));
        }

        switch(refType) {
            case Ast::RefType::Global:
                break;
            case Ast::RefType::XRef:
                break;
            case Ast::RefType::Param:
                switch(scope.type()) {
                    case Ast::ScopeType::Global:
                        throw Exception("%s Internal error: Invalid vref %s: Param-Global\n", err(_filename, name).c_str(), name.text());
                    case Ast::ScopeType::Member:
                        throw Exception("%s Internal error: Invalid vref %s: Param-Member\n", err(_filename, name).c_str(), name.text());
                    case Ast::ScopeType::XRef:
                        refType = Ast::RefType::XRef;
                        break;
                    case Ast::ScopeType::Param:
                        throw Exception("%s Internal error: Invalid vref %s: Param-Param\n", err(_filename, name).c_str(), name.text());
                    case Ast::ScopeType::Local:
                        refType = Ast::RefType::XRef;
                        break;
                }
                break;
            case Ast::RefType::Local:
                switch(scope.type()) {
                    case Ast::ScopeType::Global:
                        throw Exception("%s Internal error: Invalid vref %s: Local-Global\n", err(_filename, name).c_str(), name.text());
                    case Ast::ScopeType::Member:
                        throw Exception("%s Internal error: Invalid vref %s: Local-Member\n", err(_filename, name).c_str(), name.text());
                    case Ast::ScopeType::XRef:
                        throw Exception("%s Internal error: Invalid vref %s: Local-XRef\n", err(_filename, name).c_str(), name.text());
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
            if(refType == Ast::RefType::XRef) {
                assert(scopeList.size() > 0);
                for(ScopeList::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
                    Ast::Scope& scope = ref(*it);

                    // check if vref already exists in this scope
                    bool found = false;
                    for(Ast::Scope::List::const_iterator xit = scope.list().begin(); xit != scope.list().end(); ++xit) {
                        const Ast::VariableDefn* xref = *xit;
                        if(vref == xref) {
                            found = true;
                            break;
                        }
                    }

                    // if not exists, add it
                    if(!found)
                        scope.addVariableDef(ref(vref));
                }
            }

            // create vref expression
            Ast::VariableRefExpr& vrefExpr = _unit.addNode(new Ast::VariableRefExpr(ref(vref).qualifiedTypeSpec(), ref(vref), refType));
            return ptr(vrefExpr);
        }
    }
    throw Exception("%s Variable not found: '%s'\n", err(_filename, name).c_str(), name.text());
}

Ast::VariableMemberExpr* Context::aVariableMemberExpr(const Ast::Expr& expr, const Ast::Token& name) {
    const Ast::TypeSpec& typeSpec = expr.qTypeSpec().typeSpec();

    const Ast::StructDefn* structDefn = dynamic_cast<const Ast::StructDefn*>(ptr(typeSpec));
    if(structDefn != 0) {
        const Ast::VariableDefn* vref = hasMember(ref(structDefn).scope(), name);
        if(vref == 0) {
            throw Exception("%s %s is not a member of expression type '%s'\n", err(_filename, typeSpec.name()).c_str(), name.text(), typeSpec.name().text());
        }
        Ast::VariableMemberExpr& vdefExpr = _unit.addNode(new Ast::VariableMemberExpr(ref(vref).qualifiedTypeSpec(), expr, ref(vref)));
        return ptr(vdefExpr);
    }

    const Ast::FunctionRetn* functionRetn = dynamic_cast<const Ast::FunctionRetn*>(ptr(typeSpec));
    if(functionRetn != 0) {
        const Ast::VariableDefn* vref = hasMember(ref(functionRetn).outScope(), name);
        if(vref == 0) {
            throw Exception("%s %s is not a member of expression type '%s'\n", err(_filename, typeSpec.name()).c_str(), name.text(), typeSpec.name().text());
        }
        Ast::VariableMemberExpr& vdefExpr = _unit.addNode(new Ast::VariableMemberExpr(ref(vref).qualifiedTypeSpec(), expr, ref(vref)));
        return ptr(vdefExpr);
    }

    throw Exception("%s Not a aggregate expression type '%s'\n", err(_filename, typeSpec.name()).c_str(), typeSpec.name().text());
}

Ast::TypeSpecMemberExpr* Context::aTypeSpecMemberExpr(const Ast::TypeSpec& typeSpec, const Ast::Token& name) {
    const Ast::EnumDefn* enumDefn = dynamic_cast<const Ast::EnumDefn*>(ptr(typeSpec));
    if(enumDefn != 0) {
        const Ast::VariableDefn* vref = hasMember(ref(enumDefn).scope(), name);
        if(vref == 0) {
            throw Exception("%s %s is not a member of type '%s'\n", err(_filename, typeSpec.name()).c_str(), name.text(), typeSpec.name().text());
        }
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, typeSpec, false);
        Ast::EnumMemberExpr& typeSpecMemberExpr = _unit.addNode(new Ast::EnumMemberExpr(qTypeSpec, typeSpec, ref(vref)));
        return ptr(typeSpecMemberExpr);
    }

    const Ast::StructDefn* structDefn = dynamic_cast<const Ast::StructDefn*>(ptr(typeSpec));
    if(structDefn != 0) {
        const Ast::VariableDefn* vref = hasMember(ref(structDefn).scope(), name);
        if(vref == 0) {
            throw Exception("%s %s is not a member of type '%s'\n", err(_filename, typeSpec.name()).c_str(), name.text(), typeSpec.name().text());
        }
        Ast::StructMemberExpr& typeSpecMemberExpr = _unit.addNode(new Ast::StructMemberExpr(ref(vref).qualifiedTypeSpec(), typeSpec, ref(vref)));
        return ptr(typeSpecMemberExpr);
    }

    throw Exception("%s Not an aggregate type '%s'\n", err(_filename, name).c_str(), typeSpec.name().text());
}

Ast::StructInstanceExpr* Context::aStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn, const Ast::StructInitPartList& list) {
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, structDefn, false);
    Ast::StructInstanceExpr& structInstanceExpr = _unit.addNode(new Ast::StructInstanceExpr(qTypeSpec, structDefn, list));
    return ptr(structInstanceExpr);
}

Ast::StructInstanceExpr* Context::aStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn) {
    Ast::StructInitPartList& list = _unit.addNode(new Ast::StructInitPartList());
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, structDefn, false);
    Ast::StructInstanceExpr& structInstanceExpr = _unit.addNode(new Ast::StructInstanceExpr(qTypeSpec, structDefn, list));
    return ptr(structInstanceExpr);
}

const Ast::StructDefn* Context::aEnterStructInstanceExpr(const Ast::StructDefn& structDefn) {
    _structInitStack.push_back(ptr(structDefn));
    return ptr(structDefn);
}

void Context::aLeaveStructInstanceExpr() {
    _structInitStack.pop_back();
}

const Ast::VariableDefn* Context::aEnterStructInitPart(const Ast::Token& name) {
    if(_structInitStack.size() == 0) {
        throw Exception("%s: Internal error initializing struct-member\n", err(_filename, name).c_str());
    }

    const Ast::StructDefn* structDefn = _structInitStack.back();
    assert(structDefn);

    for(StructBaseIterator sbi(structDefn); sbi.hasNext(); sbi.next()) {
        for(Ast::Scope::List::const_iterator it = sbi.get().list().begin(); it != sbi.get().list().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            if(vdef.name().string() == name.string()) {
                return ptr(vdef);
            }
        }
    }

    throw Exception("%s: struct-member %s not found in %s\n", err(_filename, name).c_str(), name.text(), ref(structDefn).name().text());
}

void Context::aLeaveStructInitPart() {
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

Ast::StructInitPart* Context::aStructInitPart(const Ast::VariableDefn& vdef, const Ast::Expr& expr) {
    Ast::StructInitPart& part = _unit.addNode(new Ast::StructInitPart(vdef, expr));
    return ptr(part);
}

Ast::FunctionInstanceExpr* Context::aFunctionInstanceExpr(const Ast::TypeSpec& typeSpec, const Ast::ExprList& exprList) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(ptr(typeSpec));
    if(function != 0) {
        Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, ref(function), false);
        Ast::FunctionInstanceExpr& functionInstanceExpr = _unit.addNode(new Ast::FunctionInstanceExpr(qTypeSpec, ref(function), exprList));
        return ptr(functionInstanceExpr);
    }

    throw Exception("%s: Not a aggregate type '%s'\n", err(_filename, typeSpec.name()).c_str(), typeSpec.name().text());
}

Ast::FunctionInstanceExpr* Context::aAnonymousFunctionExpr(Ast::ChildFunctionDefn& functionDefn, const Ast::CompoundStatement& compoundStatement) {
    aChildFunctionDefn(functionDefn, compoundStatement);
    Ast::ExprList& exprList = addExprList();
    Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(false, functionDefn, false);
    Ast::FunctionInstanceExpr& functionInstanceExpr = _unit.addNode(new Ast::FunctionInstanceExpr(qTypeSpec, functionDefn, exprList));
    return ptr(functionInstanceExpr);
}

Ast::ChildFunctionDefn* Context::aEnterAnonymousFunction(const Ast::Function& function) {
    char namestr[128];
    sprintf(namestr, "_anonymous_%lu", currentTypeSpec().childCount());
    Ast::Token name(0, 0, namestr);
    Ast::ChildFunctionDefn* functionDefn = aEnterChildFunctionDefn(function, name, Ast::DefinitionType::Direct);
    return functionDefn;
}

Ast::ConstantExpr& Context::aConstantExpr(const std::string& type, const Ast::Token& value) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(value, type);
    Ast::ConstantExpr& expr = _unit.addNode(new Ast::ConstantExpr(qTypeSpec, value));
    return expr;
}
