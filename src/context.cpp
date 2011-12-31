#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "context.hpp"
#include "error.hpp"
#include "typename.hpp"

inline std::string Context::getExpectedTypeName(const Context::ExpectedTypeSpec::Type& exType) {
    switch(exType) {
        case Context::ExpectedTypeSpec::etNone:
            return "etNone";
        case Context::ExpectedTypeSpec::etAuto:
            return "etAuto";
        case Context::ExpectedTypeSpec::etCallArg:
            return "etCallArg";
        case Context::ExpectedTypeSpec::etListVal:
            return "etListVal";
        case Context::ExpectedTypeSpec::etDictKey:
            return "etDictKey";
        case Context::ExpectedTypeSpec::etDictVal:
            return "etDictVal";
        case Context::ExpectedTypeSpec::etAssignment:
            return "etAssignment";
        case Context::ExpectedTypeSpec::etEventHandler:
            return "etEventHandler";
        case Context::ExpectedTypeSpec::etStructInit:
            return "etStructInit";
    }
    throw Exception("Internal error: Unknown Expected Type '%d'\n", exType);
}

template <typename DefnT, typename ChildT>
struct BaseIterator {
    inline BaseIterator(const DefnT* defn) : _defn(defn) {}
    inline bool hasNext() const {return (_defn != 0);}
    inline const DefnT& get() const {return ref(_defn);}
    inline void next() {
        const ChildT* csd = dynamic_cast<const ChildT*>(_defn);
        if(csd) {
            _defn = ptr(ref(csd).base());
        } else {
            _defn = 0;
        }
    }

private:
    const DefnT* _defn;
};

struct StructBaseIterator : public BaseIterator<Ast::StructDefn, Ast::ChildStructDefn> {
    inline StructBaseIterator(const Ast::StructDefn* defn) : BaseIterator(defn) {}
};

struct FunctionBaseIterator : public BaseIterator<Ast::Function, Ast::ChildFunctionDefn> {
    inline FunctionBaseIterator(const Ast::FunctionDefn* defn) : BaseIterator(defn) {}
};

inline Ast::Root& Context::getRootNamespace() const {
    return (_level == 0)?_unit.rootNS():_unit.importNS();
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

const Ast::TypeSpec* Context::currentTypeRefHasChild(const Ast::Token& name) const {
    if(_currentTypeRef == 0)
        return 0;
    const Ast::TypeSpec* td = ref(_currentTypeRef).hasChild<const Ast::TypeSpec>(name.string());
    if(td)
        return td;

    if(_currentImportedTypeRef) {
        const Ast::TypeSpec* itd = ref(_currentImportedTypeRef).hasChild<const Ast::TypeSpec>(name.string());
        if(itd) {
            return itd;
        }
    }
    return 0;
}

inline const Ast::TypeSpec* Context::findTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const {
    const Ast::TypeSpec* child = parent.hasChild<const Ast::TypeSpec>(name.string());
    if(child)
        return child;
    const Ast::ChildTypeSpec* parentx = dynamic_cast<const Ast::ChildTypeSpec*>(ptr(parent));
    if(!parentx)
        return 0;
    return findTypeSpec(ref(parentx).parent(), name);
}

inline const Ast::TypeSpec* Context::hasImportRootTypeSpec(const Ast::Token& name) const {
    if(_level == 0) {
        const Ast::TypeSpec* typeSpec = _unit.importNS().hasChild<const Ast::TypeSpec>(name.string());
        if(typeSpec)
            return typeSpec;
    }

    return 0;
}

const Ast::TypeSpec* Context::hasRootTypeSpec(const Ast::Token& name) const {
    const Ast::TypeSpec* typeSpec = findTypeSpec(currentTypeSpec(), name);
    if(typeSpec)
        return typeSpec;

    return hasImportRootTypeSpec(name);
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

template <typename T>
inline const T* Context::setCurrentRootTypeRef(const Ast::Token& name) {
    const T& td = getRootTypeSpec<T>(name);
    _currentTypeRef = ptr(td);
    _currentImportedTypeRef = hasImportRootTypeSpec(name);
    return ptr(td);
}

template <typename T>
inline const T* Context::setCurrentChildTypeRef(const Ast::TypeSpec& parent, const Ast::Token& name, const std::string& extype) {
    if(ptr(parent) != _currentTypeRef) {
        throw Exception("%s Internal error: %s parent mismatch '%s'\n", err(_filename, name).c_str(), extype.c_str(), name.text());
    }
    const T* td = ref(_currentTypeRef).hasChild<const T>(name.string());
    if(td) {
        _currentTypeRef = td;
        if(_currentImportedTypeRef) {
            const T* itd = ref(_currentImportedTypeRef).hasChild<const T>(name.string());
            if(itd) {
                _currentImportedTypeRef = itd;
            } else {
                _currentImportedTypeRef = 0;
            }
        }
        return td;
    }

    if(_currentImportedTypeRef) {
        const T* itd = ref(_currentImportedTypeRef).hasChild<const T>(name.string());
        if(itd) {
            _currentImportedTypeRef = 0;
            _currentTypeRef = itd;
            return itd;
        } else {
            _currentImportedTypeRef = 0;
        }
    }

    throw Exception("%s %s type expected '%s'\n", err(_filename, name).c_str(), extype.c_str(), name.text());
}

template <typename T>
inline const T* Context::resetCurrentTypeRef(const T& typeSpec) {
    _currentTypeRef = 0;
    return ptr(typeSpec);
}

inline Ast::QualifiedTypeSpec& Context::addQualifiedTypeSpec(const Ast::Token& pos, const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef) {
    Ast::QualifiedTypeSpec& qualifiedTypeSpec = _unit.addNode(new Ast::QualifiedTypeSpec(pos, isConst, typeSpec, isRef));
    return qualifiedTypeSpec;
}

inline const Ast::QualifiedTypeSpec& Context::getQualifiedTypeSpec(const Ast::Token& pos, const std::string& name) {
    Ast::Token token(pos.row(), pos.col(), name);
    const Ast::TypeSpec& typeSpec = getRootTypeSpec<Ast::TypeSpec>(token);
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, typeSpec, false);
    return qTypeSpec;
}

inline Ast::Scope& Context::addScope(const Ast::Token& pos, const Ast::ScopeType::T& type) {
    Ast::Scope& scope = _unit.addNode(new Ast::Scope(pos, type));
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

inline const Ast::VariableDefn* Context::hasMember(const Ast::Scope& scope, const Ast::Token& name) {
    for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
        const Ast::VariableDefn& vref = ref(*it);
        if(vref.name().string() == name.string())
            return ptr(vref);
    }
    return 0;
}

inline Ast::ExprList& Context::addExprList(const Ast::Token& pos) {
    Ast::ExprList& exprList = _unit.addNode(new Ast::ExprList(pos));
    return exprList;
}

inline const Ast::QualifiedTypeSpec* Context::canCoerceX(const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs, CoercionResult::T& mode) const {
    const Ast::TypeSpec& lts = resolveTypedefR(lhs.typeSpec());
    const Ast::TypeSpec& rts = resolveTypedefR(rhs.typeSpec());

    mode = CoercionResult::None;
    if(ptr(lts) == ptr(rts)) {
        mode = CoercionResult::Lhs;
        return ptr(lhs);
    }

    for(Ast::Unit::CoerceListList::const_iterator it = _unit.coercionList().begin(); it != _unit.coercionList().end(); ++it) {
        const Ast::CoerceList& coerceList = ref(*it);
        int lidx = -1;
        int ridx = -1;
        int cidx = 0;
        for(Ast::CoerceList::List::const_iterator cit = coerceList.list().begin(); cit != coerceList.list().end(); ++cit, ++cidx) {
            const Ast::TypeSpec& typeSpec = ref(*cit);
            if(ptr(typeSpec) == ptr(lts)) {
                lidx = cidx;
            }
            if(ptr(typeSpec) == ptr(rts)) {
                ridx = cidx;
            }
        }
        if((lidx >= 0) && (ridx >= 0)) {
            if(lidx >= ridx) {
                mode = CoercionResult::Lhs;
                return ptr(lhs);
            }
            mode = CoercionResult::Rhs;
            return ptr(rhs);
        }
    }

    const Ast::StructDefn* lsd = dynamic_cast<const Ast::StructDefn*>(ptr(lts));
    const Ast::StructDefn* rsd = dynamic_cast<const Ast::StructDefn*>(ptr(rts));
    if((lsd != 0) && (rsd != 0)) {
        for(StructBaseIterator sbi(lsd); sbi.hasNext(); sbi.next()) {
            if(ptr(sbi.get()) == rsd) {
                mode = CoercionResult::Rhs;
                return ptr(rhs);
            }
        }
        for(StructBaseIterator sbi(rsd); sbi.hasNext(); sbi.next()) {
            if(ptr(sbi.get()) == lsd) {
                mode = CoercionResult::Lhs;
                return ptr(lhs);
            }
        }
    }

    const Ast::FunctionDefn* lfd = dynamic_cast<const Ast::FunctionDefn*>(ptr(lts));
    const Ast::FunctionDefn* rfd = dynamic_cast<const Ast::FunctionDefn*>(ptr(rts));
    if((lfd != 0) && (rfd != 0)) {
        for(FunctionBaseIterator fbi(lfd); fbi.hasNext(); fbi.next()) {
            if(ptr(fbi.get()) == rfd) {
                mode = CoercionResult::Rhs;
                return ptr(rhs);
            }
        }
        for(FunctionBaseIterator fbi(rfd); fbi.hasNext(); fbi.next()) {
            if(ptr(fbi.get()) == lfd) {
                mode = CoercionResult::Lhs;
                return ptr(lhs);
            }
        }
    }

    const Ast::TemplateDefn* ltd = dynamic_cast<const Ast::TemplateDefn*>(ptr(lts));
    const Ast::TemplateDefn* rtd = dynamic_cast<const Ast::TemplateDefn*>(ptr(rts));
    if((ltd != 0) && (rtd != 0)) {
        if(ref(ltd).name().string() == ref(rtd).name().string()) {
            const Ast::QualifiedTypeSpec& lSubType = ref(ltd).at(0);
            const Ast::QualifiedTypeSpec& rSubType = ref(rtd).at(0);
            CoercionResult::T imode = CoercionResult::None;
            const Ast::QualifiedTypeSpec* val = canCoerceX(lSubType, rSubType, imode);
            unused(val);
            if(imode == CoercionResult::Lhs) {
                mode = CoercionResult::Lhs;
                return ptr(lhs);
            } else if (imode == CoercionResult::Rhs) {
                mode = CoercionResult::Rhs;
                return ptr(rhs);
            }
        }
    }

    mode = CoercionResult::None;
    return 0;
}

inline const Ast::QualifiedTypeSpec* Context::canCoerce(const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs) const {
    CoercionResult::T mode = CoercionResult::None;
    return canCoerceX(lhs, rhs, mode);
}
inline const Ast::QualifiedTypeSpec& Context::coerce(const Ast::Token& pos, const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs) {
    const Ast::QualifiedTypeSpec* val = canCoerce(lhs, rhs);
    if(!val) {
        throw Exception("%s Cannot coerce '%s' and '%s'\n",
                        err(_filename, pos).c_str(),
                        getQualifiedTypeSpecName(lhs, GenMode::Import).c_str(),
                        getQualifiedTypeSpecName(rhs, GenMode::Import).c_str());
    }
    return ref(val);
}

inline const Ast::Expr& Context::getDefaultValue(const Ast::TypeSpec& typeSpec, const Ast::Token& name) {
    const Ast::TypeSpec* ts = resolveTypedef(typeSpec);

    const Ast::Unit::DefaultValueList& list = _unit.defaultValueList();
    Ast::Unit::DefaultValueList::const_iterator it = list.find(ts);
    if(it != list.end()) {
        return ref(it->second);
    }

    const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(ts);
    if(td != 0) {
        const std::string tdName = ref(td).name().string() ; // getTypeSpecName(ref(td), GenMode::Import); \todo this is incorrect, it will match any type called, say, list.
        if(tdName == "pointer") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, ref(td), false);
            Ast::ExprList& exprList = addExprList(name);
            const Ast::QualifiedTypeSpec& subType = ref(td).at(0);
            const Ast::Expr& nameExpr = getDefaultValue(subType.typeSpec(), name);
            exprList.addExpr(nameExpr);
            Ast::PointerInstanceExpr& expr = _unit.addNode(new Ast::PointerInstanceExpr(name, qTypeSpec, ref(td), exprList));
            return expr;
        }
        if(tdName == "list") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, ref(td), false);
            Ast::ListList& llist = _unit.addNode(new Ast::ListList(name));
            const Ast::QualifiedTypeSpec* qlType = ref(td).list().at(0);
            llist.valueType(ref(qlType));
            Ast::ListExpr& expr = _unit.addNode(new Ast::ListExpr(name, qTypeSpec, llist));
            return expr;
        }
        if(tdName == "dict") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, ref(td), false);
            Ast::DictList& llist = _unit.addNode(new Ast::DictList(name));
            const Ast::QualifiedTypeSpec* qlType = ref(td).list().at(0);
            const Ast::QualifiedTypeSpec* qrType = ref(td).list().at(1);
            llist.keyType(ref(qlType));
            llist.valueType(ref(qrType));
            Ast::DictExpr& expr = _unit.addNode(new Ast::DictExpr(name, qTypeSpec, llist));
            return expr;
        }
        if(tdName == "ptr") {
            Ast::Token value(name.row(), name.col(), "0");
            Ast::ConstantExpr& expr = aConstantExpr("int", value);
            return expr;
        }
    }

    const Ast::EnumDefn* ed = dynamic_cast<const Ast::EnumDefn*>(ts);
    if(ed != 0) {
        const Ast::Scope::List::const_iterator rit = ref(ed).list().begin();
        if(rit == ref(ed).list().end()) {
            throw Exception("%s empty enum type '%s'\n", err(_filename, typeSpec.name()).c_str(), ref(ed).name().text());
        }
        const Ast::VariableDefn& vref = ref(*rit);
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, typeSpec, false);
        Ast::EnumMemberExpr& typeSpecMemberExpr = _unit.addNode(new Ast::EnumMemberExpr(name, qTypeSpec, typeSpec, vref));
        return typeSpecMemberExpr;
    }

    const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(ts);
    if(sd != 0) {
        Ast::StructInstanceExpr* expr = aStructInstanceExpr(name, ref(sd));
        return ref(expr);
    }

    const Ast::Function* fd = dynamic_cast<const Ast::Function*>(ts);
    if(fd != 0) {
        Ast::ExprList& exprList = addExprList(name);
        Ast::FunctionInstanceExpr* expr = aFunctionInstanceExpr(name, ref(fd), exprList);
        return ref(expr);
    }

    throw Exception("%s No default value for type '%s'\n", err(_filename, name).c_str(), ref(ts).name().text());
}

inline Ast::TemplateDefn& Context::createTemplateDefn(const Ast::Token& pos, const std::string& name) {
    Ast::Token token(pos.row(), pos.col(), name);
    const Ast::TemplateDecl& templateDecl = getRootTypeSpec<Ast::TemplateDecl>(token);
    Ast::TemplateDefn& templateDefn = _unit.addNode(new Ast::TemplateDefn(currentTypeSpec(), token, Ast::DefinitionType::Final, templateDecl));
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

inline const Ast::QualifiedTypeSpec& Context::getFunctionReturnType(const Ast::Token& pos, const Ast::Function& function) {
    if(function.sig().outScope().isTuple()) {
        const Ast::FunctionRetn& functionRetn = getFunctionRetn(pos, function);
        Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, functionRetn, false);
        return qTypeSpec;
    }
    return ref(function.sig().out().front()).qTypeSpec();
}

inline Ast::StructDefn& Context::getCurrentStructDefn(const Ast::Token& pos) {
    Ast::TypeSpec& ts = currentTypeSpec();
    Ast::StructDefn* sd = dynamic_cast<Ast::StructDefn*>(ptr(ts));
    if(sd == 0) {
        throw Exception("%s Internal error: not a struct type'%s'\n", err(_filename, pos).c_str(), ts.name().text());
    }
    return ref(sd);
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
    Ast::Scope& xref = addScope(name, Ast::ScopeType::XRef);
    Ast::FunctionDecl& functionDecl = _unit.addNode(new Ast::FunctionDecl(parent, name, defType, functionSig, xref));
    Ast::Token token1(name.row(), name.col(), "_Out");
    Ast::FunctionRetn& functionRetn = _unit.addNode(new Ast::FunctionRetn(functionDecl, token1, functionSig.outScope()));
    functionDecl.addChild(functionRetn);
    return functionDecl;
}

inline Ast::ValueInstanceExpr& Context::getValueInstanceExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec, const Ast::TemplateDefn& templateDefn, const Ast::Expr& expr) {
    unused(pos);
    Ast::ExprList& exprList = addExprList(pos);
    exprList.addExpr(expr);

    const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, qTypeSpec.isConst(), qTypeSpec.typeSpec(), true);
    Ast::ValueInstanceExpr& valueInstanceExpr = _unit.addNode(new Ast::ValueInstanceExpr(pos, typeSpec, templateDefn, exprList));
    return valueInstanceExpr;
}

inline void Context::pushExpectedTypeSpec(const ExpectedTypeSpec::Type& type, const Ast::QualifiedTypeSpec& qTypeSpec) {
    _expectedTypeSpecStack.push_back(ExpectedTypeSpec(type, ptr(qTypeSpec)));
}

inline void Context::pushExpectedTypeSpec(const ExpectedTypeSpec::Type& type) {
    _expectedTypeSpecStack.push_back(ExpectedTypeSpec(type));
}

inline void Context::popExpectedTypeSpec(const Ast::Token& pos, const ExpectedTypeSpec::Type& type) {
    if(_expectedTypeSpecStack.size() == 0) {
        throw Exception("%s Internal error: Empty expected type stack\n", err(_filename, pos).c_str());
    }
    if(type != _expectedTypeSpecStack.back().type()) {
        throw Exception("%s Internal error: Invalid expected type popped. Popping %s, got %s\n",
                        err(_filename, pos).c_str(),
                        getExpectedTypeName(type).c_str(),
                        getExpectedTypeName(_expectedTypeSpecStack.back().type()).c_str());
    }

    _expectedTypeSpecStack.pop_back();
}

inline bool Context::popExpectedTypeSpecOrAuto(const Ast::Token& pos, const ExpectedTypeSpec::Type& type) {
    if(getExpectedType() == ExpectedTypeSpec::etAuto) {
        popExpectedTypeSpec(pos, ExpectedTypeSpec::etAuto);
        return false;
    }
    popExpectedTypeSpec(pos, type);
    return true;
}

inline Context::ExpectedTypeSpec::Type Context::getExpectedType() const {
    if(_expectedTypeSpecStack.size() == 0) {
        return ExpectedTypeSpec::etNone;
    }

    return _expectedTypeSpecStack.back().type();
}

inline const Context::ExpectedTypeSpec& Context::getExpectedTypeList(const Ast::Token& pos) const {
    if(_expectedTypeSpecStack.size() == 0) {
        throw Exception("%s Empty expected type stack\n", err(_filename, pos).c_str());
    }

    const ExpectedTypeSpec& exl = _expectedTypeSpecStack.back();
    return exl;
}

inline const Ast::QualifiedTypeSpec* Context::getExpectedTypeSpecIfAny(const size_t& idx) const {
    assert(idx == 0);
    if(_expectedTypeSpecStack.size() == 0) {
        return 0;
    }

    const ExpectedTypeSpec& exl = _expectedTypeSpecStack.back();
    if(!exl.hasTypeSpec()) {
        return 0;
    }

    const Ast::QualifiedTypeSpec& ts = exl.typeSpec();
    return ptr(ts);
}

inline const Ast::QualifiedTypeSpec& Context::getExpectedTypeSpec(const Ast::QualifiedTypeSpec* qTypeSpec, const size_t& idx) const {
    assert(idx == 0);
    const Ast::QualifiedTypeSpec* ts = getExpectedTypeSpecIfAny(idx);
    if(ts == 0) {
        return ref(qTypeSpec);
    }
    return ref(ts);
}

inline const Ast::QualifiedTypeSpec& Context::getExpectedTypeSpecEx(const Ast::Token& pos, const size_t& idx) const {
    assert(idx == 0);
    const Ast::QualifiedTypeSpec* ts = getExpectedTypeSpecIfAny(idx);
    if(ts == 0) {
        throw Exception("%s Empty expected type stack\n", err(_filename, pos).c_str());
    }
    return ref(ts);
}

inline const Ast::TemplateDefn* Context::isEnteringTemplate(const size_t& idx) const {
    assert(idx == 0);
    const Ast::QualifiedTypeSpec* qts = getExpectedTypeSpecIfAny(idx);
    if(qts == 0) {
        return 0;
    }
    const Ast::TypeSpec& ts = ref(qts).typeSpec();
    const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(ptr(ts));
    return td;
}

inline const Ast::TemplateDefn* Context::isEnteringList(const size_t& idx) const {
    assert(idx == 0);
    const Ast::TemplateDefn* td = isEnteringTemplate(idx);
    if(td) {
        if(ref(td).name().string() == "list") {
            return td;
        }
        if(ref(td).name().string() == "dict") {
            return td;
        }
    }
    return 0;
}

const Ast::StructDefn* Context::isStructExpected(const size_t& idx) const {
    assert(idx == 0);
    const Ast::QualifiedTypeSpec* qts = getExpectedTypeSpecIfAny(idx);
    if(qts == 0) {
        return 0;
    }

    const Ast::TypeSpec& ts = ref(qts).typeSpec();
    const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(ptr(ts));
    if(sd == 0) {
        return 0;
    }

    return sd;
}

const Ast::Function* Context::isFunctionExpected(const size_t& idx) const {
    assert(idx == 0);
    const Ast::QualifiedTypeSpec* qts = getExpectedTypeSpecIfAny(idx);
    if(qts == 0) {
        return 0;
    }

    const Ast::TypeSpec& ts = ref(qts).typeSpec();
    const Ast::Function* ed = dynamic_cast<const Ast::Function*>(ptr(ts));
    if(ed == 0) {
        return 0;
    }

    return ed;
}

const Ast::TemplateDefn* Context::isPointerExpected(const size_t& idx) const {
    assert(idx == 0);
    const Ast::TemplateDefn* td = isEnteringTemplate(idx);
    if(td) {
        if(ref(td).name().string() == "pointer") {
            return td;
        }
    }
    return 0;
}

const Ast::TemplateDefn* Context::isPointerToExprExpected(const size_t& idx, const Ast::Expr& expr) const {
    assert(idx == 0);
    const Ast::TemplateDefn* ts = isPointerExpected(idx);
    if(ts) {
        const Ast::QualifiedTypeSpec& innerQts = ref(ts).at(0);

        CoercionResult::T mode = CoercionResult::None;
        const Ast::QualifiedTypeSpec* qts = canCoerceX(innerQts, expr.qTypeSpec(), mode);
        if(qts && (mode == CoercionResult::Lhs)) {
            return ts;
        }
    }
    return 0;
}

const Ast::StructDefn* Context::isPointerToStructExpected(const size_t& idx) const {
    assert(idx == 0);
    const Ast::TemplateDefn* td = isPointerExpected(idx);
    if(td) {
        const Ast::QualifiedTypeSpec& valType = ref(td).at(0);
        const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(ptr(valType.typeSpec()));
        return sd;
    }
    return 0;
}

const Ast::StructDefn* Context::isListOfStructExpected(const size_t& idx) const {
    assert(idx == 0);
    const Ast::TemplateDefn* td = isEnteringList(idx);
    const Ast::StructDefn* sd = 0;
    if(td) {
        if(ref(td).name().string() == "list") {
            const Ast::QualifiedTypeSpec& valType = ref(td).at(0);
            sd = dynamic_cast<const Ast::StructDefn*>(ptr(valType.typeSpec()));
        } else if(ref(td).name().string() == "dict") {
            const Ast::QualifiedTypeSpec& valType = ref(td).at(1);
            sd = dynamic_cast<const Ast::StructDefn*>(ptr(valType.typeSpec()));
        } else {
            assert(false);
        }
    }
    return sd;
}

inline const Ast::TypeSpec* Context::isListOfPointerExpected(const size_t& idx) const {
    assert(idx == 0);
    const Ast::TemplateDefn* td = isEnteringList(idx);
    if(td) {
        if(ref(td).name().string() == "list") {
            const Ast::QualifiedTypeSpec& innerType = ref(td).at(0);
            const Ast::TemplateDefn* td1 = dynamic_cast<const Ast::TemplateDefn*>(ptr(innerType.typeSpec()));
            if(td1) {
                if(ref(td1).name().string() == "pointer") {
                    const Ast::QualifiedTypeSpec& valType = ref(td1).at(0);
                    return ptr(valType.typeSpec());
                }
            }
        } else if(ref(td).name().string() == "dict") {
            const Ast::QualifiedTypeSpec& innerType = ref(td).at(1);
            const Ast::TemplateDefn* td1 = dynamic_cast<const Ast::TemplateDefn*>(ptr(innerType.typeSpec()));
            if(td1) {
                if(ref(td1).name().string() == "pointer") {
                    const Ast::QualifiedTypeSpec& valType = ref(td1).at(0);
                    return ptr(valType.typeSpec());
                }
            }
        } else {
            assert(false);
        }
    }
    return 0;
}

const Ast::StructDefn* Context::isListOfPointerToStructExpected(const size_t& idx) const {
    assert(idx == 0);
    const Ast::TypeSpec* ts = isListOfPointerExpected(idx);
    if(!ts)
        return 0;

    const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(ts);
    return sd;
}

inline const Ast::Expr& Context::convertExprToExpectedTypeSpec(const Ast::Token& pos, const size_t& idx, const Ast::Expr& initExpr) {
    assert(idx == 0);
    // check if lhs is a pointer to rhs, if so auto-convert
    const Ast::TemplateDefn* ts = isPointerToExprExpected(idx, initExpr);
    if(ts) {
        Ast::PointerInstanceExpr* expr = aPointerInstanceExpr(pos, initExpr);
        return ref(expr);
    }

    const Ast::QualifiedTypeSpec* qts = getExpectedTypeSpecIfAny(idx);
    if(qts) {
        // if expected type is ptr, no checking
        const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(ptr(ref(qts).typeSpec()));
        if((td) && (ref(td).name().string() == "ptr")) {
            return initExpr;
        }

        // check if rhs is a pointer to lhs, if so, auto-convert
        const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(ptr(initExpr.qTypeSpec().typeSpec()));
        if((templateDefn) && (ref(templateDefn).name().string() == "pointer")) {
            const Ast::QualifiedTypeSpec& rhsQts = ref(templateDefn).at(0);
            CoercionResult::T mode = CoercionResult::None;
            canCoerceX(ref(qts), rhsQts, mode);
            if(mode == CoercionResult::Rhs) {
                const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, ref(qts).isConst(), ref(qts).typeSpec(), true);
                Ast::TypecastExpr& typecastExpr = _unit.addNode(new Ast::DynamicTypecastExpr(pos, typeSpec, initExpr));
                return typecastExpr;
            }
        }

        // check if initExpr can be converted to expected type, if any
        CoercionResult::T mode = CoercionResult::None;
        const Ast::QualifiedTypeSpec* cqts = canCoerceX(ref(qts), initExpr.qTypeSpec(), mode);
        if(mode != CoercionResult::Lhs) {
            throw Exception("%s Cannot convert expression %lu from '%s' to '%s' (%d)\n",
                            err(_filename, pos).c_str(),
                            idx,
                            getQualifiedTypeSpecName(initExpr.qTypeSpec(), GenMode::Import).c_str(),
                            getQualifiedTypeSpecName(ref(qts), GenMode::Import).c_str(),
                            mode
                            );
        }
        if(ptr(ref(cqts).typeSpec()) != ptr(initExpr.qTypeSpec().typeSpec())) {
            const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, ref(cqts).isConst(), ref(qts).typeSpec(), false);
            Ast::TypecastExpr& typecastExpr = _unit.addNode(new Ast::StaticTypecastExpr(pos, typeSpec, initExpr));
            return typecastExpr;
        }
    }

    return initExpr;
}

inline void Context::pushCallArgList(const Ast::Scope& in) {
    if(in.type() == Ast::ScopeType::VarArg) {
        pushExpectedTypeSpec(ExpectedTypeSpec::etAuto);
    } else {
        for(Ast::Scope::List::const_reverse_iterator it = in.list().rbegin(); it != in.list().rend(); ++it) {
            const Ast::VariableDefn& def = ref(*it);
            pushExpectedTypeSpec(ExpectedTypeSpec::etCallArg, def.qTypeSpec());
        }
    }
}

inline void Context::popCallArgList(const Ast::Token& pos, const Ast::Scope& in) {
    if(in.type() == Ast::ScopeType::VarArg) {
        popExpectedTypeSpec(pos, ExpectedTypeSpec::etAuto);
    }
}

inline void Context::popCallArg(const Ast::Token& pos) {
    if(getExpectedType() != ExpectedTypeSpec::etAuto) {
        popExpectedTypeSpec(pos, ExpectedTypeSpec::etCallArg);
    }
}

////////////////////////////////////////////////////////////
Context::Context(Compiler& compiler, Ast::Unit& unit, const int& level, const std::string& filename) : _compiler(compiler), _unit(unit), _level(level), _filename(filename), _lastToken(0, 0, ""),_currentTypeRef(0), _currentImportedTypeRef(0) {
    Ast::Root& rootTypeSpec = getRootNamespace();
    enterTypeSpec(rootTypeSpec);
}

Context::~Context() {
    assert(_typeSpecStack.size() == 1);
    Ast::Root& rootTypeSpec = getRootNamespace();
    leaveTypeSpec(rootTypeSpec);
    assert(_expectedTypeSpecStack.size() == 0);
}

////////////////////////////////////////////////////////////
void Context::aUnitStatementList(const Ast::EnterNamespaceStatement& nss) {
    Ast::LeaveNamespaceStatement& lns = _unit.addNode(new Ast::LeaveNamespaceStatement(getToken(), nss));
    if(_level == 0) {
        _unit.addStatement(lns);
    }

    while(_namespaceStack.size() > 0) {
        Ast::Namespace* ns = _namespaceStack.back();
        leaveTypeSpec(ref(ns));
        _namespaceStack.pop_back();
    }
}

void Context::aImportStatement(const Ast::Token& pos, const Ast::AccessType::T& accessType, const Ast::HeaderType::T& headerType, const Ast::DefinitionType::T& defType, Ast::NamespaceList& list) {
    Ast::ImportStatement& statement = _unit.addNode(new Ast::ImportStatement(pos, accessType, headerType, defType, list));
    _unit.addStatement(statement);

    if(statement.defType() != Ast::DefinitionType::Native) {
        std::string filename;
        std::string sep = "";
        for(Ast::NamespaceList::List::const_iterator it = statement.list().begin(); it != statement.list().end(); ++it) {
            const Ast::Token& name = ref(*it).name();
            filename += sep;
            filename += name.text();
            sep = "/";
        }
        filename += ".ipp";
        _compiler.import(_unit, filename, _level);
    }
}

Ast::NamespaceList* Context::aImportNamespaceList(Ast::NamespaceList& list, const Ast::Token &name) {
    Ast::Namespace& ns = _unit.addNode(new Ast::Namespace(currentTypeSpec(), name));
    list.addNamespace(ns);
    return ptr(list);
}

Ast::NamespaceList* Context::aImportNamespaceList(const Ast::Token& name) {
    Ast::NamespaceList& list = _unit.addNode(new Ast::NamespaceList(name));
    return aImportNamespaceList(list, name);
}

Ast::EnterNamespaceStatement* Context::aNamespaceStatement(const Ast::Token& pos, Ast::NamespaceList& list) {
    Ast::EnterNamespaceStatement& statement = _unit.addNode(new Ast::EnterNamespaceStatement(pos, list));
    if(_level == 0) {
        _unit.addStatement(statement);
    }
    return ptr(statement);
}

Ast::EnterNamespaceStatement* Context::aNamespaceStatement() {
    Ast::NamespaceList& list = _unit.addNode(new Ast::NamespaceList(getToken()));
    return aNamespaceStatement(getToken(), list);
}

inline Ast::Namespace& Context::getUnitNamespace(const Ast::Token& name) {
    if(_level == 0) {
        Ast::Namespace& ns = _unit.addNode(new Ast::Namespace(currentTypeSpec(), name));
        currentTypeSpec().addChild(ns);
        return ns;
    }

    Ast::Namespace* cns = _unit.importNS().hasChild<Ast::Namespace>(name.string());
    if(cns) {
        return ref(cns);
    }

    Ast::Namespace& ns = _unit.addNode(new Ast::Namespace(currentTypeSpec(), name));
    currentTypeSpec().addChild(ns);
    return ns;
}

Ast::NamespaceList* Context::aUnitNamespaceList(Ast::NamespaceList& list, const Ast::Token& name) {
    Ast::Namespace& ns = getUnitNamespace(name);
    enterTypeSpec(ns);
    if(_level == 0) {
        _unit.addNamespacePart(name);
    }
    _namespaceStack.push_back(ptr(ns));
    list.addNamespace(ns);
    return ptr(list);
}

Ast::NamespaceList* Context::aUnitNamespaceList(const Ast::Token& name) {
    Ast::NamespaceList& list = _unit.addNode(new Ast::NamespaceList(name));
    return aUnitNamespaceList(list, name);
}

Ast::Statement* Context::aGlobalStatement(Ast::Statement& statement) {
    if(_level == 0) {
        _unit.addStatement(statement);
    }
    return ptr(statement);
}

Ast::Statement* Context::aGlobalTypeSpecStatement(const Ast::AccessType::T& accessType, Ast::UserDefinedTypeSpec& typeSpec){
    typeSpec.accessType(accessType);
    Ast::UserDefinedTypeSpecStatement* statement = aUserDefinedTypeSpecStatement(typeSpec);
    return aGlobalStatement(ref(statement));
}

void Context::aGlobalCoerceStatement(Ast::CoerceList& list) {
    _unit.addCoercionList(list);
}

Ast::CoerceList* Context::aCoerceList(Ast::CoerceList& list, const Ast::TypeSpec& typeSpec) {
    list.addTypeSpec(typeSpec);
    return ptr(list);
}

Ast::CoerceList* Context::aCoerceList(const Ast::TypeSpec& typeSpec) {
    Ast::CoerceList& list = _unit.addNode(new Ast::CoerceList(typeSpec.pos()));
    list.addTypeSpec(typeSpec);
    return ptr(list);
}

void Context::aGlobalDefaultStatement(const Ast::TypeSpec& typeSpec, const Ast::Expr& expr) {
    _unit.addDefaultValue(typeSpec, expr);
}

Ast::TypedefDecl* Context::aTypedefDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::TypedefDecl& typedefDefn = _unit.addNode(new Ast::TypedefDecl(currentTypeSpec(), name, defType));
    currentTypeSpec().addChild(typedefDefn);
    return ptr(typedefDefn);
}

Ast::TypedefDefn* Context::aTypedefDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::QualifiedTypeSpec& qTypeSpec) {
    Ast::TypedefDefn& typedefDefn = _unit.addNode(new Ast::TypedefDefn(currentTypeSpec(), name, defType, qTypeSpec));
    currentTypeSpec().addChild(typedefDefn);
    return ptr(typedefDefn);
}

Ast::TemplatePartList* Context::aTemplatePartList(Ast::TemplatePartList& list, const Ast::Token& name) {
    list.addPart(name);
    return ptr(list);
}

Ast::TemplatePartList* Context::aTemplatePartList(const Ast::Token& name) {
    Ast::TemplatePartList& list = _unit.addNode(new Ast::TemplatePartList(name));
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
    Ast::Scope& scope = addScope(name, Ast::ScopeType::Member);
    return aEnumDefn(name, defType, scope);
}

Ast::Scope* Context::aEnumMemberDefnList(Ast::Scope& list, const Ast::VariableDefn& variableDefn) {
    list.addVariableDef(variableDefn);
    return ptr(list);
}

Ast::Scope* Context::aEnumMemberDefnList(const Ast::VariableDefn& variableDefn) {
    Ast::Scope& scope = addScope(variableDefn.pos(), Ast::ScopeType::Member);
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

Ast::StructDecl* Context::aStructDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::StructDecl& structDecl = _unit.addNode(new Ast::StructDecl(currentTypeSpec(), name, defType));
    currentTypeSpec().addChild(structDecl);
    return ptr(structDecl);
}

Ast::RootStructDefn* Context::aLeaveRootStructDefn(Ast::RootStructDefn& structDefn) {
    leaveTypeSpec(structDefn);
    Ast::StructInitStatement& statement = _unit.addNode(new Ast::StructInitStatement(structDefn.pos(), structDefn));
    structDefn.block().addStatement(statement);
    return ptr(structDefn);
}

Ast::ChildStructDefn* Context::aLeaveChildStructDefn(Ast::ChildStructDefn& structDefn) {
    leaveTypeSpec(structDefn);
    Ast::StructInitStatement& statement = _unit.addNode(new Ast::StructInitStatement(structDefn.pos(), structDefn));
    structDefn.block().addStatement(statement);
    return ptr(structDefn);
}

Ast::RootStructDefn* Context::aEnterRootStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& list = addScope(name, Ast::ScopeType::Member);
    Ast::CompoundStatement& block = _unit.addNode(new Ast::CompoundStatement(name));
    Ast::RootStructDefn& structDefn = _unit.addNode(new Ast::RootStructDefn(currentTypeSpec(), name, defType, list, block));
    currentTypeSpec().addChild(structDefn);
    enterTypeSpec(structDefn);
    return ptr(structDefn);
}

Ast::ChildStructDefn* Context::aEnterChildStructDefn(const Ast::Token& name, const Ast::StructDefn& base, const Ast::DefinitionType::T& defType) {
    Ast::Scope& list = addScope(name, Ast::ScopeType::Member);
    Ast::CompoundStatement& block = _unit.addNode(new Ast::CompoundStatement(name));
    Ast::ChildStructDefn& structDefn = _unit.addNode(new Ast::ChildStructDefn(currentTypeSpec(), base, name, defType, list, block));
    currentTypeSpec().addChild(structDefn);
    enterTypeSpec(structDefn);
    return ptr(structDefn);
}

void Context::aStructMemberVariableDefn(const Ast::VariableDefn& vdef) {
    Ast::StructDefn& sd = getCurrentStructDefn(vdef.name());
    sd.addVariable(vdef);
    Ast::StructMemberVariableStatement& statement = _unit.addNode(new Ast::StructMemberVariableStatement(vdef.pos(), sd, vdef));
    sd.block().addStatement(statement);
}

void Context::aStructMemberTypeDefn(Ast::UserDefinedTypeSpec& typeSpec) {
    Ast::StructDefn& sd = getCurrentStructDefn(typeSpec.name());
    typeSpec.accessType(Ast::AccessType::Parent);
    Ast::Statement* statement = aUserDefinedTypeSpecStatement(typeSpec);
    sd.block().addStatement(ref(statement));
}

void Context::aStructMemberPropertyDefn(Ast::PropertyDecl& typeSpec) {
    aStructMemberTypeDefn(typeSpec);
    Ast::StructDefn& sd = getCurrentStructDefn(typeSpec.name());
    sd.addProperty(typeSpec);
}

Ast::PropertyDeclRW* Context::aStructPropertyDeclRW(const Ast::Token& pos, const Ast::QualifiedTypeSpec& propertyType, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::PropertyDeclRW& structPropertyDecl = _unit.addNode(new Ast::PropertyDeclRW(currentTypeSpec(), name, defType, propertyType));
    currentTypeSpec().addChild(structPropertyDecl);
    return ptr(structPropertyDecl);
}

Ast::PropertyDeclRO* Context::aStructPropertyDeclRO(const Ast::Token& pos, const Ast::QualifiedTypeSpec& propertyType, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::PropertyDeclRO& structPropertyDecl = _unit.addNode(new Ast::PropertyDeclRO(currentTypeSpec(), name, defType, propertyType));
    currentTypeSpec().addChild(structPropertyDecl);
    return ptr(structPropertyDecl);
}

Ast::RoutineDecl* Context::aRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const Ast::DefinitionType::T& defType) {
    Ast::RoutineDecl& routineDecl = _unit.addNode(new Ast::RoutineDecl(currentTypeSpec(), outType, name, in, defType));
    currentTypeSpec().addChild(routineDecl);
    return ptr(routineDecl);
}

Ast::RoutineDecl* Context::aVarArgRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& in = addScope(name, Ast::ScopeType::VarArg);
    Ast::RoutineDecl& routineDecl = _unit.addNode(new Ast::RoutineDecl(currentTypeSpec(), outType, name, in, defType));
    currentTypeSpec().addChild(routineDecl);
    return ptr(routineDecl);
}

Ast::RoutineDefn* Context::aRoutineDefn(Ast::RoutineDefn& routineDefn, const Ast::CompoundStatement& block) {
    routineDefn.setBlock(block);
    leaveScope(routineDefn.inScope());
    leaveTypeSpec(routineDefn);
    _unit.addBody(_unit.addNode(new Ast::RoutineBody(block.pos(), routineDefn, block)));
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
    _unit.addBody(_unit.addNode(new Ast::FunctionBody(block.pos(), functionDefn, block)));
    return ptr(functionDefn);
}

Ast::RootFunctionDefn* Context::aEnterRootFunctionDefn(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();
    Ast::Scope& xref = addScope(name, Ast::ScopeType::XRef);
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
    _unit.addBody(_unit.addNode(new Ast::FunctionBody(block.pos(), functionDefn, block)));
    return ptr(functionDefn);
}

inline Ast::ChildFunctionDefn& Context::createChildFunctionDefn(Ast::TypeSpec& parent, const Ast::Function& base, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& xref = addScope(name, Ast::ScopeType::XRef);
    Ast::ChildFunctionDefn& functionDefn = _unit.addNode(new Ast::ChildFunctionDefn(parent, name, defType, base.sig(), xref, base));
    parent.addChild(functionDefn);
    enterScope(functionDefn.xrefScope());
    enterScope(base.sig().inScope());
    enterTypeSpec(functionDefn);
    return functionDefn;
}

Ast::ChildFunctionDefn* Context::aEnterChildFunctionDefn(const Ast::TypeSpec& base, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(ptr(base));
    if(function == 0) {
        throw Exception("%s base type not a function '%s'\n", err(_filename, name).c_str(), base.name().text());
    }
    Ast::ChildFunctionDefn& functionDefn = createChildFunctionDefn(currentTypeSpec(), ref(function), name, defType);
    return ptr(functionDefn);
}

Ast::EventDecl* Context::aEventDecl(const Ast::Token& pos, const Ast::VariableDefn& in, const Ast::DefinitionType::T& eventDefType, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& handlerDefType) {
    const Ast::Token& name = functionSig.name();

    Ast::Token eventName(pos.row(), pos.col(), name.string());
    Ast::EventDecl& eventDef = _unit.addNode(new Ast::EventDecl(currentTypeSpec(), eventName, in, eventDefType));
    currentTypeSpec().addChild(eventDef);

    Ast::Token handlerName(pos.row(), pos.col(), "Handler");
    Ast::FunctionSig* handlerSig = aFunctionSig(functionSig.outScope(), handlerName, functionSig.inScope());
    Ast::FunctionDecl& funDecl = addFunctionDecl(eventDef, ref(handlerSig), handlerDefType);
    eventDef.setHandler(funDecl);

    Ast::QualifiedTypeSpec& qFunTypeSpec = addQualifiedTypeSpec(pos, false, funDecl, false);
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "pointer");
    templateDefn.addType(qFunTypeSpec);
    const Ast::QualifiedTypeSpec& qFunctorTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);

    Ast::Token hVarName(pos.row(), pos.col(), "handler");
    Ast::VariableDefn& vdef = addVariableDefn(qFunctorTypeSpec, hVarName);

    Ast::Scope& outAdd = addScope(pos, Ast::ScopeType::Param);
    Ast::Scope& inAdd  = addScope(pos, Ast::ScopeType::Param);
    Ast::Token nameAdd(pos.row(), pos.col(), "Add");
    Ast::FunctionSig* addSig = aFunctionSig(outAdd, nameAdd, inAdd);
    Ast::FunctionDecl& addDecl = addFunctionDecl(eventDef, ref(addSig), eventDefType);
    eventDef.setAddFunction(addDecl);

    inAdd.addVariableDef(in);
    inAdd.addVariableDef(vdef);

    return ptr(eventDef);
}

Ast::FunctionSig* Context::aFunctionSig(const Ast::Scope& out, const Ast::Token& name, Ast::Scope& in) {
    Ast::FunctionSig& functionSig = _unit.addNode(new Ast::FunctionSig(out, name, in));
    return ptr(functionSig);
}

Ast::FunctionSig* Context::aFunctionSig(const Ast::QualifiedTypeSpec& typeSpec, const Ast::Token& name, Ast::Scope& in) {
    Ast::Scope& out = addScope(name, Ast::ScopeType::Param);
    out.isTuple(false);

    Ast::Token oname(name.row(), name.col(), "_out");
    Ast::VariableDefn& vdef = addVariableDefn(typeSpec, oname);
    out.addVariableDef(vdef);

    return aFunctionSig(out, name, in);
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
    Ast::Scope& list = addScope(variableDefn.pos(), Ast::ScopeType::Param);
    return aParam(list, variableDefn);
}

Ast::Scope* Context::aParam() {
    Ast::Scope& list = addScope(getToken(), Ast::ScopeType::Param);
    return ptr(list);
}

Ast::VariableDefn* Context::aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name, const Ast::Expr& initExpr) {
    const Ast::Expr& expr = convertExprToExpectedTypeSpec(name, 0, initExpr);
    Ast::VariableDefn& variableDef = _unit.addNode(new Ast::VariableDefn(qualifiedTypeSpec, name, expr));
    popExpectedTypeSpecOrAuto(name, ExpectedTypeSpec::etAssignment);
    return ptr(variableDef);
}

Ast::VariableDefn* Context::aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name) {
    const Ast::Expr& initExpr = getDefaultValue(qualifiedTypeSpec.typeSpec(), name);
    return aVariableDefn(qualifiedTypeSpec, name, initExpr);
}

Ast::VariableDefn* Context::aVariableDefn(const Ast::Token& name, const Ast::Expr& initExpr) {
    const Ast::QualifiedTypeSpec& qualifiedTypeSpec = initExpr.qTypeSpec();
    return aVariableDefn(qualifiedTypeSpec, name, initExpr);
}

const Ast::QualifiedTypeSpec* Context::aQualifiedVariableDefn(const Ast::QualifiedTypeSpec& qTypeSpec) {
    pushExpectedTypeSpec(ExpectedTypeSpec::etAssignment, qTypeSpec);
    return ptr(qTypeSpec);
}

void Context::aAutoQualifiedVariableDefn() {
    pushExpectedTypeSpec(ExpectedTypeSpec::etAuto);
}

Ast::QualifiedTypeSpec* Context::aQualifiedTypeSpec(const Ast::Token& pos, const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef) {
    Ast::QualifiedTypeSpec& qualifiedTypeSpec = addQualifiedTypeSpec(pos, isConst, typeSpec, isRef);
    return ptr(qualifiedTypeSpec);
}

Ast::QualifiedTypeSpec* Context::aQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef) {
    return aQualifiedTypeSpec(getToken(), isConst, typeSpec, isRef);
}

const Ast::TemplateDecl* Context::aTemplateTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return setCurrentChildTypeRef<Ast::TemplateDecl>(parent, name, "template");
}

const Ast::TemplateDecl* Context::aTemplateTypeSpec(const Ast::Token& name) {
    return setCurrentRootTypeRef<Ast::TemplateDecl>(name);
}

const Ast::TemplateDecl* Context::aTemplateTypeSpec(const Ast::TemplateDecl& templateDecl) {
    return resetCurrentTypeRef<Ast::TemplateDecl>(templateDecl);
}

const Ast::StructDefn* Context::aStructTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return setCurrentChildTypeRef<Ast::StructDefn>(parent, name, "struct");
}

const Ast::StructDefn* Context::aStructTypeSpec(const Ast::Token& name) {
    return setCurrentRootTypeRef<Ast::StructDefn>(name);
}

const Ast::StructDefn* Context::aStructTypeSpec(const Ast::StructDefn& structDefn) {
    return resetCurrentTypeRef<Ast::StructDefn>(structDefn);
}

const Ast::Routine* Context::aRoutineTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return setCurrentChildTypeRef<Ast::Routine>(parent, name, "routine");
}

const Ast::Routine* Context::aRoutineTypeSpec(const Ast::Token& name) {
    return setCurrentRootTypeRef<Ast::Routine>(name);
}

const Ast::Routine* Context::aRoutineTypeSpec(const Ast::Routine& routine) {
    return resetCurrentTypeRef<Ast::Routine>(routine);
}

const Ast::Function* Context::aFunctionTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return setCurrentChildTypeRef<Ast::Function>(parent, name, "function");
}

const Ast::Function* Context::aFunctionTypeSpec(const Ast::Token& name) {
    return setCurrentRootTypeRef<Ast::Function>(name);
}

const Ast::Function* Context::aFunctionTypeSpec(const Ast::Function& function) {
    return resetCurrentTypeRef<Ast::Function>(function);
}

const Ast::EventDecl* Context::aEventTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return setCurrentChildTypeRef<Ast::EventDecl>(parent, name, "event");
}

const Ast::EventDecl* Context::aEventTypeSpec(const Ast::Token& name) {
    return setCurrentRootTypeRef<Ast::EventDecl>(name);
}

const Ast::EventDecl* Context::aEventTypeSpec(const Ast::EventDecl& event) {
    return resetCurrentTypeRef<Ast::EventDecl>(event);
}

const Ast::TypeSpec* Context::aOtherTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return setCurrentChildTypeRef<Ast::TypeSpec>(parent, name, "parent");
}

const Ast::TypeSpec* Context::aOtherTypeSpec(const Ast::Token& name) {
    return setCurrentRootTypeRef<Ast::TypeSpec>(name);
}

const Ast::TypeSpec* Context::aTypeSpec(const Ast::TypeSpec& TypeSpec) {
    return resetCurrentTypeRef<Ast::TypeSpec>(TypeSpec);
}

const Ast::TemplateDefn* Context::aTemplateDefnTypeSpec(const Ast::TemplateDecl& typeSpec, const Ast::TemplateTypePartList& list) {
    Ast::TemplateDefn& templateDefn = _unit.addNode(new Ast::TemplateDefn(currentTypeSpec(), typeSpec.name(), Ast::DefinitionType::Final, typeSpec));
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
    Ast::TemplateTypePartList& list = _unit.addNode(new Ast::TemplateTypePartList(getToken()));
    return aTemplateTypePartList(list, qTypeSpec);
}

Ast::UserDefinedTypeSpecStatement* Context::aUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec) {
    Ast::UserDefinedTypeSpecStatement& userDefinedTypeSpecStatement = _unit.addNode(new Ast::UserDefinedTypeSpecStatement(typeSpec.pos(), typeSpec));
    return ptr(userDefinedTypeSpecStatement);
}

Ast::AutoStatement* Context::aAutoStatement(const Ast::VariableDefn& defn) {
    Ast::AutoStatement& localStatement = _unit.addNode(new Ast::AutoStatement(defn.pos(), defn));
    currentScope().addVariableDef(defn);
    return ptr(localStatement);
}

Ast::ExprStatement* Context::aExprStatement(const Ast::Expr& expr) {
    Ast::ExprStatement& exprStatement = _unit.addNode(new Ast::ExprStatement(expr.pos(), expr));
    return ptr(exprStatement);
}

Ast::PrintStatement* Context::aPrintStatement(const Ast::Token& pos, const Ast::Expr& expr) {
    Ast::PrintStatement& printStatement = _unit.addNode(new Ast::PrintStatement(pos, expr));
    return ptr(printStatement);
}

Ast::IfStatement* Context::aIfStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& tblock) {
    Ast::IfStatement& ifStatement = _unit.addNode(new Ast::IfStatement(pos, expr, tblock));
    return ptr(ifStatement);
}

Ast::IfElseStatement* Context::aIfElseStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& tblock, const Ast::CompoundStatement& fblock) {
    Ast::IfElseStatement& ifElseStatement = _unit.addNode(new Ast::IfElseStatement(pos, expr, tblock, fblock));
    return ptr(ifElseStatement);
}

Ast::WhileStatement* Context::aWhileStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& block) {
    Ast::WhileStatement& whileStatement = _unit.addNode(new Ast::WhileStatement(pos, expr, block));
    return ptr(whileStatement);
}

Ast::DoWhileStatement* Context::aDoWhileStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& block) {
    Ast::DoWhileStatement& doWhileStatement = _unit.addNode(new Ast::DoWhileStatement(pos, expr, block));
    return ptr(doWhileStatement);
}

Ast::ForStatement* Context::aForStatement(const Ast::Token& pos, const Ast::Expr& init, const Ast::Expr& expr, const Ast::Expr& incr, const Ast::CompoundStatement& block) {
    Ast::ForExprStatement& forStatement = _unit.addNode(new Ast::ForExprStatement(pos, init, expr, incr, block));
    return ptr(forStatement);
}

Ast::ForStatement* Context::aForStatement(const Ast::Token& pos, const Ast::VariableDefn& init, const Ast::Expr& expr, const Ast::Expr& incr, const Ast::CompoundStatement& block) {
    Ast::ForInitStatement& forStatement = _unit.addNode(new Ast::ForInitStatement(pos, init, expr, incr, block));
    leaveScope();
    return ptr(forStatement);
}

const Ast::VariableDefn* Context::aEnterForInit(const Ast::VariableDefn& init) {
    Ast::Scope& scope = addScope(init.pos(), Ast::ScopeType::Local);
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
    const Ast::QualifiedTypeSpec& valTypeSpec = addQualifiedTypeSpec(valName, expr.qTypeSpec().isConst(), templateDefn.at(0).typeSpec(), true);
    const Ast::VariableDefn& valDef = addVariableDefn(valTypeSpec, valName);
    Ast::Scope& scope = addScope(valName, Ast::ScopeType::Local);
    scope.addVariableDef(valDef);
    enterScope(scope);

    Ast::ForeachListStatement& foreachStatement = _unit.addNode(new Ast::ForeachListStatement(valName, valDef, expr));
    return ptr(foreachStatement);
}

Ast::ForeachDictStatement* Context::aEnterForeachInit(const Ast::Token& keyName, const Ast::Token& valName, const Ast::Expr& expr) {
    const Ast::TemplateDefn& templateDefn = getTemplateDefn(valName, expr, "dict", 2);
    const Ast::QualifiedTypeSpec& keyTypeSpec = addQualifiedTypeSpec(keyName, true, templateDefn.at(0).typeSpec(), true);
    const Ast::QualifiedTypeSpec& valTypeSpec = addQualifiedTypeSpec(keyName, expr.qTypeSpec().isConst(), templateDefn.at(1).typeSpec(), true);
    const Ast::VariableDefn& keyDef = addVariableDefn(keyTypeSpec, keyName);
    const Ast::VariableDefn& valDef = addVariableDefn(valTypeSpec, valName);
    Ast::Scope& scope = addScope(keyName, Ast::ScopeType::Local);
    scope.addVariableDef(keyDef);
    scope.addVariableDef(valDef);
    enterScope(scope);

    Ast::ForeachDictStatement& foreachStatement = _unit.addNode(new Ast::ForeachDictStatement(keyName, keyDef, valDef, expr));
    return ptr(foreachStatement);
}

Ast::SwitchValueStatement* Context::aSwitchStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& list) {
    Ast::SwitchValueStatement& switchStatement = _unit.addNode(new Ast::SwitchValueStatement(pos, expr, list));
    return ptr(switchStatement);
}

Ast::SwitchExprStatement* Context::aSwitchStatement(const Ast::Token& pos, const Ast::CompoundStatement& list) {
    Ast::SwitchExprStatement& switchStatement = _unit.addNode(new Ast::SwitchExprStatement(pos, list));
    return ptr(switchStatement);
}

Ast::CompoundStatement* Context::aCaseList(Ast::CompoundStatement& list, const Ast::CaseStatement& stmt) {
    list.addStatement(stmt);
    return ptr(list);
}

Ast::CompoundStatement* Context::aCaseList(const Ast::CaseStatement& stmt) {
    Ast::CompoundStatement& list = _unit.addNode(new Ast::CompoundStatement(stmt.pos()));
    list.addStatement(stmt);
    return ptr(list);
}

Ast::CaseStatement* Context::aCaseStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& block) {
    Ast::CaseExprStatement& caseStatement = _unit.addNode(new Ast::CaseExprStatement(pos, expr, block));
    return ptr(caseStatement);
}

Ast::CaseStatement* Context::aCaseStatement(const Ast::Token& pos, const Ast::CompoundStatement& block) {
    Ast::CaseDefaultStatement& caseStatement = _unit.addNode(new Ast::CaseDefaultStatement(pos, block));
    return ptr(caseStatement);
}

Ast::BreakStatement* Context::aBreakStatement(const Ast::Token& pos) {
    Ast::BreakStatement& breakStatement = _unit.addNode(new Ast::BreakStatement(pos));
    return ptr(breakStatement);
}

Ast::ContinueStatement* Context::aContinueStatement(const Ast::Token& pos) {
    Ast::ContinueStatement& continueStatement = _unit.addNode(new Ast::ContinueStatement(pos));
    return ptr(continueStatement);
}

Ast::AddEventHandlerStatement* Context::aAddEventHandlerStatement(const Ast::Token& pos, const Ast::EventDecl& event, const Ast::Expr& source, Ast::FunctionTypeInstanceExpr& functor) {
    Ast::AddEventHandlerStatement& addEventHandlerStatement = _unit.addNode(new Ast::AddEventHandlerStatement(pos, event, source, functor));
    popExpectedTypeSpec(pos, ExpectedTypeSpec::etEventHandler);
    return ptr(addEventHandlerStatement);
}

const Ast::EventDecl* Context::aEnterAddEventHandler(const Ast::EventDecl& eventDecl) {
    Ast::QualifiedTypeSpec& qts = addQualifiedTypeSpec(getToken(), false, eventDecl.handler(), false);
    pushExpectedTypeSpec(ExpectedTypeSpec::etEventHandler, qts);
    return ptr(eventDecl);
}

Ast::RoutineReturnStatement* Context::aRoutineReturnStatement(const Ast::Token& pos) {
    Ast::ExprList& exprList = addExprList(pos);
    Ast::RoutineReturnStatement& returnStatement = _unit.addNode(new Ast::RoutineReturnStatement(pos, exprList));
    return ptr(returnStatement);
}

Ast::RoutineReturnStatement* Context::aRoutineReturnStatement(const Ast::Token& pos, const Ast::Expr& expr) {
    Ast::ExprList& exprList = addExprList(pos);
    exprList.addExpr(expr);
    Ast::RoutineReturnStatement& returnStatement = _unit.addNode(new Ast::RoutineReturnStatement(pos, exprList));
    return ptr(returnStatement);
}

Ast::FunctionReturnStatement* Context::aFunctionReturnStatement(const Ast::Token& pos, const Ast::ExprList& exprList) {
    Ast::FunctionReturnStatement& returnStatement = _unit.addNode(new Ast::FunctionReturnStatement(pos, exprList));
    return ptr(returnStatement);
}

Ast::CompoundStatement* Context::aStatementList() {
    Ast::CompoundStatement& statement = _unit.addNode(new Ast::CompoundStatement(getToken()));
    return ptr(statement);
}

Ast::CompoundStatement* Context::aStatementList(Ast::CompoundStatement& list, const Ast::Statement& statement) {
    list.addStatement(statement);
    return ptr(list);
}

void Context::aEnterCompoundStatement(const Ast::Token& pos) {
    Ast::Scope& scope = addScope(pos, Ast::ScopeType::Local);
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
    Ast::ExprList& list = addExprList(expr.pos());
    return aExprList(list, expr);
}

Ast::ExprList* Context::aExprList() {
    Ast::ExprList& list = addExprList(getToken());
    return ptr(list);
}

Ast::TernaryOpExpr* Context::aTernaryExpr(const Ast::Token& op1, const Ast::Token& op2, const Ast::Expr& lhs, const Ast::Expr& rhs1, const Ast::Expr& rhs2) {
    const Ast::QualifiedTypeSpec& qTypeSpec = coerce(op2, rhs1.qTypeSpec(), rhs2.qTypeSpec());
    Ast::TernaryOpExpr& expr = _unit.addNode(new Ast::TernaryOpExpr(qTypeSpec, op1, op2, lhs, rhs1, rhs2));
    return ptr(expr);
}

Ast::Expr& Context::aBooleanExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(op, "bool");
    Ast::BinaryOpExpr& expr = _unit.addNode(new Ast::BinaryOpExpr(qTypeSpec, op, lhs, rhs));
    return expr;
}

Ast::Expr& Context::aBinaryExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    const Ast::QualifiedTypeSpec& qTypeSpec = coerce(op, lhs.qTypeSpec(), rhs.qTypeSpec());

    // if it is any of the assign-ops (=, +=, *=, etc). Check if last char is '='
    const Ast::IndexExpr* indexExpr = dynamic_cast<const Ast::IndexExpr*>(ptr(lhs));
    if(indexExpr) {
        if(op.string() == "=") {
            Ast::SetIndexExpr& expr = _unit.addNode(new Ast::SetIndexExpr(op, qTypeSpec, ref(indexExpr), rhs));
            return expr;
        } else if(op.string() != "==") {
            if(op.string().at(op.string().size() - 1) == '=') {
                throw Exception("%s Operator '%s' on index expression not implemented\n", err(_filename, op).c_str(), op.text());
            }
        }
    }

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
    popExpectedTypeSpecOrAuto(pos, ExpectedTypeSpec::etListVal);
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "list");
    templateDefn.addType(list.valueType());
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);

    Ast::ListExpr& expr = _unit.addNode(new Ast::ListExpr(pos, qTypeSpec, list));
    return ptr(expr);
}

Ast::ListList* Context::aListList(const Ast::Token& pos, Ast::ListList& list, const Ast::ListItem& item) {
    list.addItem(item);
    const Ast::QualifiedTypeSpec& qValueTypeSpec = coerce(pos, list.valueType(), item.valueExpr().qTypeSpec());
    list.valueType(qValueTypeSpec);
    return ptr(list);
}

Ast::ListList* Context::aListList(const Ast::Token& pos, const Ast::ListItem& item) {
    Ast::ListList& list = _unit.addNode(new Ast::ListList(pos));
    list.addItem(item);
    const Ast::QualifiedTypeSpec& valType = getExpectedTypeSpec(ptr(item.valueExpr().qTypeSpec()), 0);
    list.valueType(valType);
    return ptr(list);
}

Ast::ListList* Context::aListList(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec) {
    Ast::ListList& list = _unit.addNode(new Ast::ListList(pos));
    const Ast::QualifiedTypeSpec& valType = getExpectedTypeSpec(ptr(qTypeSpec), 0);
    list.valueType(valType);
    return ptr(list);
}

Ast::ListList* Context::aListList(const Ast::Token& pos) {
    Ast::ListList& list = _unit.addNode(new Ast::ListList(pos));
    const Ast::QualifiedTypeSpec& valType = getExpectedTypeSpec(0, 0);
    list.valueType(valType);
    return ptr(list);
}

Ast::ListItem* Context::aListItem(const Ast::Expr& valueExpr) {
    const Ast::Expr& expr = convertExprToExpectedTypeSpec(valueExpr.pos(), 0, valueExpr);
    Ast::ListItem& item = _unit.addNode(new Ast::ListItem(valueExpr.pos(), expr));
//    popExpectedTypeSpec(valueExpr.pos(), ExpectedTypeSpec::etListVal);
    return ptr(item);
}

Ast::DictExpr* Context::aDictExpr(const Ast::Token& pos, const Ast::DictList& list) {
    popExpectedTypeSpecOrAuto(pos, ExpectedTypeSpec::etDictKey);
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "dict");
    templateDefn.addType(list.keyType());
    templateDefn.addType(list.valueType());
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);

    Ast::DictExpr& expr = _unit.addNode(new Ast::DictExpr(pos, qTypeSpec, list));
    return ptr(expr);
}

Ast::DictList* Context::aDictList(const Ast::Token& pos, Ast::DictList& list, const Ast::DictItem& item) {
    list.addItem(item);
    const Ast::QualifiedTypeSpec& keyType = coerce(pos, list.keyType(), item.keyExpr().qTypeSpec());
    const Ast::QualifiedTypeSpec& valType = coerce(pos, list.valueType(), item.valueExpr().qTypeSpec());

    list.keyType(keyType);
    list.valueType(valType);
    return ptr(list);
}

Ast::DictList* Context::aDictList(const Ast::Token& pos, const Ast::DictItem& item) {
    Ast::DictList& list = _unit.addNode(new Ast::DictList(pos));
    list.addItem(item);
    list.keyType(item.keyExpr().qTypeSpec());
    list.valueType(item.valueExpr().qTypeSpec());
    return ptr(list);
}

Ast::DictList* Context::aDictList(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qKeyTypeSpec, const Ast::QualifiedTypeSpec& qValueTypeSpec) {
    Ast::DictList& list = _unit.addNode(new Ast::DictList(pos));
    list.keyType(qKeyTypeSpec);
    list.valueType(qValueTypeSpec);
    return ptr(list);
}

/// The sequence of calls in this function is important.
inline const Ast::Expr& Context::switchDictKeyValue(const Ast::Token& pos, const Context::ExpectedTypeSpec::Type& popType, const Context::ExpectedTypeSpec::Type& pushType, const size_t& idx, const Ast::Expr& initExpr) {
    const Ast::Expr& expr = convertExprToExpectedTypeSpec(pos, 0, initExpr);
    bool isExpected = popExpectedTypeSpecOrAuto(pos, popType);
    const Ast::TemplateDefn* td0 = isEnteringList(0);

    if(isExpected) {
        if((td0) && (ref(td0).name().string() == "dict")) {
            const Ast::QualifiedTypeSpec& keyType = ref(td0).at(idx);
            pushExpectedTypeSpec(pushType, keyType);
        } else {
            assert(false);
        }
    } else {
        pushExpectedTypeSpec(ExpectedTypeSpec::etAuto);
    }
    return expr;
}

Ast::DictItem* Context::aDictItem(const Ast::Token& pos, const Ast::Expr& keyExpr, const Ast::Expr& valueExpr) {
    const Ast::Expr& expr = switchDictKeyValue(pos, ExpectedTypeSpec::etDictVal, ExpectedTypeSpec::etDictKey, 0, valueExpr);
    Ast::DictItem& item = _unit.addNode(new Ast::DictItem(pos, keyExpr, expr));
    return ptr(item);
}

const Ast::Expr* Context::aDictKey(const Ast::Expr& keyExpr) {
    const Ast::Expr& expr = switchDictKeyValue(keyExpr.pos(), ExpectedTypeSpec::etDictKey, ExpectedTypeSpec::etDictVal, 1, keyExpr);
    return ptr(expr);
}

const Ast::Token& Context::aEnterList(const Ast::Token& pos) {
    const Ast::TemplateDefn* td0 = isEnteringList(0);
    if(td0) {
        if(ref(td0).name().string() == "list") {
            const Ast::QualifiedTypeSpec& valType = ref(td0).at(0);
            pushExpectedTypeSpec(ExpectedTypeSpec::etListVal, valType);
        } else if(ref(td0).name().string() == "dict") {
            const Ast::QualifiedTypeSpec& keyType = ref(td0).at(0);
            pushExpectedTypeSpec(ExpectedTypeSpec::etDictKey, keyType);
        } else {
            assert(false);
        }
    } else {
        pushExpectedTypeSpec(ExpectedTypeSpec::etAuto);
    }

    return pos;
}

Ast::FormatExpr* Context::aFormatExpr(const Ast::Token& pos, const Ast::Expr& stringExpr, const Ast::DictExpr& dictExpr) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "string");
    Ast::FormatExpr& formatExpr = _unit.addNode(new Ast::FormatExpr(pos, qTypeSpec, stringExpr, dictExpr));
    return ptr(formatExpr);
}

Ast::RoutineCallExpr* Context::aRoutineCallExpr(const Ast::Token& pos, const Ast::Routine& routine, const Ast::ExprList& exprList) {
    popCallArgList(pos, routine.inScope());
    const Ast::QualifiedTypeSpec& qTypeSpec = routine.outType();
    Ast::RoutineCallExpr& routineCallExpr = _unit.addNode(new Ast::RoutineCallExpr(pos, qTypeSpec, routine, exprList));
    return ptr(routineCallExpr);
}

const Ast::Routine* Context::aEnterRoutineCall(const Ast::Routine& routine) {
    pushCallArgList(routine.inScope());
    return ptr(routine);
}

Ast::FunctorCallExpr* Context::aFunctorCallExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::ExprList& exprList) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(ptr(expr.qTypeSpec().typeSpec()));
    if(function == 0) {
        throw Exception("%s Unknown functor being called '%s'\n", err(_filename, pos).c_str(), expr.qTypeSpec().typeSpec().name().text());
    }

    popCallArgList(pos, ref(function).sig().inScope());

    const Ast::QualifiedTypeSpec& qTypeSpec = getFunctionReturnType(pos, ref(function));
    Ast::FunctorCallExpr& functorCallExpr = _unit.addNode(new Ast::FunctorCallExpr(pos, qTypeSpec, expr, exprList));
    return ptr(functorCallExpr);
}

Ast::Expr* Context::aEnterFunctorCall(Ast::Expr& expr) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(ptr(expr.qTypeSpec().typeSpec()));
    if(function == 0) {
        throw Exception("%s Unknown functor being called '%s'\n", err(_filename, expr.pos()).c_str(), expr.qTypeSpec().typeSpec().name().text());
    }

    pushCallArgList(ref(function).sig().inScope());
    return ptr(expr);
}

Ast::Expr* Context::aEnterFunctorCall(const Ast::Token& name) {
    Ast::VariableRefExpr* expr = aVariableRefExpr(name);
    return aEnterFunctorCall(ref(expr));
}

Ast::Expr* Context::aEnterFunctorCall(const Ast::Function& function) {
    Ast::QualifiedTypeSpec& qExprTypeSpec = addQualifiedTypeSpec(getToken(), false, function, false);
    Ast::ExprList& exprList = addExprList(getToken());
    Ast::FunctionInstanceExpr& expr = _unit.addNode(new Ast::FunctionInstanceExpr(getToken(), qExprTypeSpec, function, exprList));
    return aEnterFunctorCall(expr);
}

Ast::ExprList* Context::aCallArgList(const Ast::Token& pos, Ast::ExprList& list, const Ast::Expr& expr) {
    const Ast::Expr& argExpr = convertExprToExpectedTypeSpec(pos, 0, expr);
    list.addExpr(argExpr);
    popCallArg(pos);
    return ptr(list);
}

Ast::ExprList* Context::aCallArgList(const Ast::Expr& expr) {
    Ast::ExprList& list = addExprList(getToken());
    return aCallArgList(getToken(), list, expr);
}

Ast::ExprList* Context::aCallArgList() {
    Ast::ExprList& list = addExprList(getToken());
    return ptr(list);
}

Ast::RunExpr* Context::aRunExpr(const Ast::Token& pos, const Ast::FunctorCallExpr& callExpr) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(ptr(callExpr.expr().qTypeSpec().typeSpec()));
    if(function != 0) {
        Ast::QualifiedTypeSpec& qRetTypeSpec = addQualifiedTypeSpec(pos, false, ref(function), false);

        Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "future");
        templateDefn.addType(qRetTypeSpec);
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);

        Ast::RunExpr& runExpr = _unit.addNode(new Ast::RunExpr(pos, qTypeSpec, callExpr));
        return ptr(runExpr);
    }
    throw Exception("%s Unknown functor in run expression '%s'\n", err(_filename, pos).c_str(), getQualifiedTypeSpecName(callExpr.expr().qTypeSpec(), GenMode::Import).c_str());
}

Ast::OrderedExpr* Context::aOrderedExpr(const Ast::Token& pos, const Ast::Expr& innerExpr) {
    Ast::OrderedExpr& expr = _unit.addNode(new Ast::OrderedExpr(pos, innerExpr.qTypeSpec(), innerExpr));
    return ptr(expr);
}

Ast::IndexExpr* Context::aIndexExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::Expr& index) {
    const Ast::TypeSpec* listTypeSpec = resolveTypedef(expr.qTypeSpec().typeSpec());
    const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(listTypeSpec);
    if(td) {
        if(ref(td).name().string() == "list") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, ref(td).at(0).isConst(), ref(td).at(0).typeSpec(), true);
            Ast::IndexExpr& indexExpr = _unit.addNode(new Ast::IndexExpr(pos, qTypeSpec, expr, index));
            return ptr(indexExpr);
        }

        if(ref(td).name().string() == "dict") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, ref(td).at(1).isConst(), ref(td).at(1).typeSpec(), true);
            Ast::IndexExpr& indexExpr = _unit.addNode(new Ast::IndexExpr(pos, qTypeSpec, expr, index));
            return ptr(indexExpr);
        }
    }

    const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(listTypeSpec);
    if(sd) {
        const Ast::Routine* routine = ref(sd).hasChild<const Ast::Routine>("at");
        if(routine) {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, ref(routine).outType().isConst(), ref(routine).outType().typeSpec(), true);
            Ast::IndexExpr& indexExpr = _unit.addNode(new Ast::IndexExpr(pos, qTypeSpec, expr, index));
            return ptr(indexExpr);
        }
    }

    throw Exception("%s '%s' is not an indexable type\n", err(_filename, pos).c_str(), getQualifiedTypeSpecName(expr.qTypeSpec(), GenMode::Import).c_str());
}

Ast::TypeofTypeExpr* Context::aTypeofTypeExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& typeSpec) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "type");
    Ast::TypeofTypeExpr& typeofExpr = _unit.addNode(new Ast::TypeofTypeExpr(pos, qTypeSpec, typeSpec));
    return ptr(typeofExpr);
}

Ast::TypeofExprExpr* Context::aTypeofExprExpr(const Ast::Token& pos, const Ast::Expr& expr) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "type");
    Ast::TypeofExprExpr& typeofExpr = _unit.addNode(new Ast::TypeofExprExpr(pos, qTypeSpec, expr));
    return ptr(typeofExpr);
}

Ast::TypecastExpr* Context::aTypecastExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr) {
    unused(pos);
    /// \todo check if canCoerce
    const Ast::TemplateDefn* subType = dynamic_cast<const Ast::TemplateDefn*>(ptr(expr.qTypeSpec().typeSpec()));
    if((subType) && (ref(subType).name().string() == "pointer")) {
        const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, qTypeSpec.isConst(), qTypeSpec.typeSpec(), true);
        Ast::TypecastExpr& typecastExpr = _unit.addNode(new Ast::DynamicTypecastExpr(pos, typeSpec, expr));
        return ptr(typecastExpr);
    }

    Ast::TypecastExpr& typecastExpr = _unit.addNode(new Ast::StaticTypecastExpr(pos, qTypeSpec, expr));
    return ptr(typecastExpr);
}

Ast::PointerInstanceExpr* Context::aPointerInstanceExpr(const Ast::Token& pos, const Ast::Expr& expr) {
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "pointer");
    const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, expr.qTypeSpec().isConst(), expr.qTypeSpec().typeSpec(), true);
    templateDefn.addType(typeSpec);

    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);
    Ast::ExprList& exprList = addExprList(pos);
    exprList.addExpr(expr);

    Ast::PointerInstanceExpr& pointerExpr = _unit.addNode(new Ast::PointerInstanceExpr(pos, qTypeSpec, templateDefn, exprList));
    return ptr(pointerExpr);
}

Ast::ValueInstanceExpr* Context::aValueInstanceExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr) {
    const Ast::TemplateDefn* subType = dynamic_cast<const Ast::TemplateDefn*>(ptr(expr.qTypeSpec().typeSpec()));
    if((subType == 0) || (ref(subType).name().string() != "pointer")) {
        throw Exception("%s Expression is not a pointer to %s\n", err(_filename, pos).c_str(), qTypeSpec.typeSpec().name().text());
    }

    Ast::ValueInstanceExpr& valueInstanceExpr = getValueInstanceExpr(pos, qTypeSpec, ref(subType), expr);
    return ptr(valueInstanceExpr);
}

Ast::ValueInstanceExpr* Context::aValueInstanceExpr(const Ast::Token& pos, const Ast::Expr& expr) {
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(ptr(expr.qTypeSpec().typeSpec()));
    if(templateDefn) {
        if(ref(templateDefn).name().string() == "pointer") {
            Ast::ValueInstanceExpr& valueInstanceExpr = getValueInstanceExpr(pos, ref(templateDefn).at(0), ref(templateDefn), expr);
            return ptr(valueInstanceExpr);
        }
    }

    throw Exception("%s Expression is not a pointer to %s\n", err(_filename, pos).c_str(), expr.qTypeSpec().typeSpec().name().text());
}

Ast::TemplateDefnInstanceExpr* Context::aTemplateDefnInstanceExpr(const Ast::Token& pos, const Ast::TemplateDefn& templateDefn, const Ast::ExprList& exprList) {
    std::string name = templateDefn.name().string();
    if(name == "pointer") {
        Ast::TemplateDefn& newTemplateDefn = createTemplateDefn(pos, "pointer");
        const Ast::QualifiedTypeSpec& newTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn.at(0).typeSpec(), true);
        newTemplateDefn.addType(newTypeSpec);

        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, newTemplateDefn, false);
        Ast::PointerInstanceExpr& expr = _unit.addNode(new Ast::PointerInstanceExpr(pos, qTypeSpec, newTemplateDefn, exprList));
        return ptr(expr);
    }

    if(name == "value") {
        return aValueInstanceExpr(pos, templateDefn.at(0), exprList.at(0));
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
                    case Ast::ScopeType::VarArg:
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
                    case Ast::ScopeType::VarArg:
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
            Ast::VariableRefExpr& vrefExpr = _unit.addNode(new Ast::VariableRefExpr(name, ref(vref).qTypeSpec(), ref(vref), refType));
            return ptr(vrefExpr);
        }
    }
    throw Exception("%s Variable not found: '%s'\n", err(_filename, name).c_str(), name.text());
}

Ast::MemberExpr* Context::aMemberVariableExpr(const Ast::Expr& expr, const Ast::Token& name) {
    const Ast::TypeSpec& typeSpec = expr.qTypeSpec().typeSpec();

    const Ast::StructDefn* structDefn = dynamic_cast<const Ast::StructDefn*>(ptr(typeSpec));
    if(structDefn != 0) {
        for(StructBaseIterator sbi(structDefn); sbi.hasNext(); sbi.next()) {
            const Ast::VariableDefn* vref = hasMember(sbi.get().scope(), name);
            if(vref) {
                const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, expr.qTypeSpec().isConst(), ref(vref).qTypeSpec().typeSpec(), true);
                Ast::MemberVariableExpr& vdefExpr = _unit.addNode(new Ast::MemberVariableExpr(name, qTypeSpec, expr, ref(vref)));
                return ptr(vdefExpr);
            }

            for(Ast::StructDefn::PropertyList::const_iterator it = sbi.get().propertyList().begin(); it != sbi.get().propertyList().end(); ++it) {
                const Ast::PropertyDecl& pref = ref(*it);
                if(pref.name().string() == name.string()) {
                    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, expr.qTypeSpec().isConst(), pref.qTypeSpec().typeSpec(), true);
                    Ast::MemberPropertyExpr& vdefExpr = _unit.addNode(new Ast::MemberPropertyExpr(name, qTypeSpec, expr, pref));
                    return ptr(vdefExpr);
                }
            }
        }

        throw Exception("%s '%s' is not a member of struct '%s'\n", err(_filename, name).c_str(), name.text(), getTypeSpecName(typeSpec, GenMode::Import).c_str());
    }

    const Ast::FunctionRetn* functionRetn = dynamic_cast<const Ast::FunctionRetn*>(ptr(typeSpec));
    if(functionRetn != 0) {
        const Ast::VariableDefn* vref = hasMember(ref(functionRetn).outScope(), name);
        if(vref) {
            Ast::MemberVariableExpr& vdefExpr = _unit.addNode(new Ast::MemberVariableExpr(name, ref(vref).qTypeSpec(), expr, ref(vref)));
            return ptr(vdefExpr);
        }
        throw Exception("%s '%s' is not a member of function: '%s'\n", err(_filename, name).c_str(), name.text(), getTypeSpecName(typeSpec, GenMode::Import).c_str());
    }

    throw Exception("%s Not an aggregate expression type '%s' (looking for member %s)\n", err(_filename, name).c_str(), typeSpec.name().text(), name.text());
}

Ast::TypeSpecMemberExpr* Context::aTypeSpecMemberExpr(const Ast::TypeSpec& typeSpec, const Ast::Token& name) {
    const Ast::EnumDefn* enumDefn = dynamic_cast<const Ast::EnumDefn*>(ptr(typeSpec));
    if(enumDefn != 0) {
        const Ast::VariableDefn* vref = hasMember(ref(enumDefn).scope(), name);
        if(vref == 0) {
            throw Exception("%s %s is not a member of type '%s'\n", err(_filename, name).c_str(), name.text(), typeSpec.name().text());
        }
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, typeSpec, false);
        Ast::EnumMemberExpr& typeSpecMemberExpr = _unit.addNode(new Ast::EnumMemberExpr(name, qTypeSpec, typeSpec, ref(vref)));
        return ptr(typeSpecMemberExpr);
    }

    const Ast::StructDefn* structDefn = dynamic_cast<const Ast::StructDefn*>(ptr(typeSpec));
    if(structDefn != 0) {
        const Ast::VariableDefn* vref = hasMember(ref(structDefn).scope(), name);
        if(vref == 0) {
            throw Exception("%s %s is not a member of type '%s'\n", err(_filename, name).c_str(), name.text(), typeSpec.name().text());
        }
        Ast::StructMemberExpr& typeSpecMemberExpr = _unit.addNode(new Ast::StructMemberExpr(name, ref(vref).qTypeSpec(), typeSpec, ref(vref)));
        return ptr(typeSpecMemberExpr);
    }

    throw Exception("%s Not an aggregate type '%s' (looking for member %s)\n", err(_filename, name).c_str(), typeSpec.name().text(), name.text());
}

Ast::StructInstanceExpr* Context::aStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn, const Ast::StructInitPartList& list) {
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, structDefn, false);
    Ast::StructInstanceExpr& structInstanceExpr = _unit.addNode(new Ast::StructInstanceExpr(pos, qTypeSpec, structDefn, list));
    return ptr(structInstanceExpr);
}

Ast::StructInstanceExpr* Context::aStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn) {
    Ast::StructInitPartList& list = _unit.addNode(new Ast::StructInitPartList(pos));
    return aStructInstanceExpr(pos, structDefn, list);
}

Ast::Expr* Context::aAutoStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn, const Ast::StructInitPartList& list) {
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, structDefn, false);
    Ast::StructInstanceExpr& structInstanceExpr = _unit.addNode(new Ast::StructInstanceExpr(pos, qTypeSpec, structDefn, list));
    return ptr(structInstanceExpr);
}

Ast::Expr* Context::aAutoStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn) {
    Ast::StructInitPartList& list = _unit.addNode(new Ast::StructInitPartList(pos));
    return aAutoStructInstanceExpr(pos, structDefn, list);
}

const Ast::StructDefn* Context::aEnterStructInstanceExpr(const Ast::StructDefn& structDefn) {
    _structInitStack.push_back(ptr(structDefn));
    return ptr(structDefn);
}

const Ast::StructDefn* Context::aEnterAutoStructInstanceExpr(const Ast::Token& pos) {
    const Ast::StructDefn* sd = isStructExpected(0);
    if(sd) {
        return aEnterStructInstanceExpr(ref(sd));
    }
    sd = isPointerToStructExpected(0);
    if(sd) {
        return aEnterStructInstanceExpr(ref(sd));
    }
    throw Exception("%s No struct type expected\n", err(_filename, pos).c_str());
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
                pushExpectedTypeSpec(ExpectedTypeSpec::etStructInit, vdef.qTypeSpec());
                return ptr(vdef);
            }
        }
    }

    throw Exception("%s: struct-member '%s' not found in '%s'\n", err(_filename, name).c_str(), name.text(), getTypeSpecName(ref(structDefn), GenMode::Import).c_str());
}

void Context::aLeaveStructInitPart(const Ast::Token& pos) {
}

Ast::StructInitPartList* Context::aStructInitPartList(Ast::StructInitPartList& list, const Ast::StructInitPart& part) {
    list.addPart(part);
    return ptr(list);
}

Ast::StructInitPartList* Context::aStructInitPartList(const Ast::StructInitPart& part) {
    Ast::StructInitPartList& list = _unit.addNode(new Ast::StructInitPartList(getToken()));
    list.addPart(part);
    return ptr(list);
}

Ast::StructInitPart* Context::aStructInitPart(const Ast::Token& pos, const Ast::VariableDefn& vdef, const Ast::Expr& initExpr) {
    const Ast::Expr& expr = convertExprToExpectedTypeSpec(pos, 0, initExpr);
    popExpectedTypeSpec(pos, ExpectedTypeSpec::etStructInit);
    Ast::StructInitPart& part = _unit.addNode(new Ast::StructInitPart(pos, vdef, expr));
    return ptr(part);
}

Ast::FunctionInstanceExpr* Context::aFunctionInstanceExpr(const Ast::Token& pos, const Ast::TypeSpec& typeSpec, const Ast::ExprList& exprList) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(ptr(typeSpec));
    if(function != 0) {
        Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, ref(function), false);
        Ast::FunctionInstanceExpr& functionInstanceExpr = _unit.addNode(new Ast::FunctionInstanceExpr(pos, qTypeSpec, ref(function), exprList));
        return ptr(functionInstanceExpr);
    }

    throw Exception("%s: Not a function type '%s'\n", err(_filename, pos).c_str(), typeSpec.name().text());
}

Ast::AnonymousFunctionExpr* Context::aAnonymousFunctionExpr(Ast::ChildFunctionDefn& functionDefn, const Ast::CompoundStatement& compoundStatement) {
    aChildFunctionDefn(functionDefn, compoundStatement);
    Ast::ExprList& exprList = addExprList(getToken());
    Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(getToken(), false, functionDefn, false);
    Ast::AnonymousFunctionExpr& functionInstanceExpr = _unit.addNode(new Ast::AnonymousFunctionExpr(getToken(), qTypeSpec, functionDefn, exprList));
    return ptr(functionInstanceExpr);
}

Ast::ChildFunctionDefn* Context::aEnterAnonymousFunction(const Ast::Function& function) {
    char namestr[128];
    sprintf(namestr, "_anonymous_%lu", _unit.nodeCount());
    Ast::Token name(getToken().row(), getToken().col(), namestr);

    Ast::TypeSpec* ts = 0;
    for(TypeSpecStack::reverse_iterator it = _typeSpecStack.rbegin(); it != _typeSpecStack.rend(); ++it) {
        ts = *it;
        if(dynamic_cast<Ast::Namespace*>(ts) != 0)
            break;
        if(dynamic_cast<Ast::Root*>(ts) != 0)
            break;
    }

    if(ts == 0) {
        throw Exception("%s: Internal error: Unable to find parent for anonymous function %s\n", err(_filename, name).c_str(), getTypeSpecName(function, GenMode::Import).c_str());
    }

    Ast::ChildFunctionDefn& functionDefn = createChildFunctionDefn(ref(ts), function, name, Ast::DefinitionType::Final);
    Ast::Statement* statement = aGlobalTypeSpecStatement(Ast::AccessType::Private, functionDefn);
    unused(statement);
    return ptr(functionDefn);
}

Ast::ChildFunctionDefn* Context::aEnterAutoAnonymousFunction(const Ast::Token& pos) {
    const Ast::Function* function = isFunctionExpected(0);
    if(function == 0) {
        throw Exception("%s Internal error: no function type expected\n", err(_filename, pos).c_str());
    }
    return aEnterAnonymousFunction(ref(function));
}

Ast::ConstantExpr& Context::aConstantExpr(const std::string& type, const Ast::Token& value) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(value, type);
    Ast::ConstantExpr& expr = _unit.addNode(new Ast::ConstantExpr(qTypeSpec, value));
    return expr;
}
