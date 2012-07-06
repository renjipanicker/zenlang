#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/factory.hpp"
#include "base/typename.hpp"
#include "base/compiler.hpp"

z::Ast::Unit::Unit() : _scopeCallback(0), _currentTypeRef(0), _currentImportedTypeRef(0), _uniqueIdx(0) {
    z::Ast::Root& rootNS = addNode(new z::Ast::Root("*root*"));
    _rootNS.reset(rootNS);

    z::Ast::Root& importNS = addNode(new z::Ast::Root("*import*"));
    _importNS.reset(importNS);

    z::Ast::Root& anonymousNS = addNode(new z::Ast::Root("*anonymous*"));
    _anonymousNS.reset(anonymousNS);
}

z::Ast::Unit::~Unit() {
    for(ExpectedTypeSpecStack::const_iterator it = _expectedTypeSpecStack.begin(); it != _expectedTypeSpecStack.end(); ++it) {
        const ExpectedTypeSpec& et = *it;
        std::cout << ZenlangNameGenerator().qtn(et.typeSpec()) << std::endl;
    }

    z::assert_t(_expectedTypeSpecStack.size() == 0);
    z::assert_t(_typeSpecStack.size() == 0);
}

const z::Ast::QualifiedTypeSpec* z::Ast::Unit::canCoerceX(const z::Ast::QualifiedTypeSpec& lhs, const z::Ast::QualifiedTypeSpec& rhs, CoercionResult::T& mode) const {
    const z::Ast::TypeSpec& lts = resolveTypedefR(lhs.typeSpec());
    const z::Ast::TypeSpec& rts = resolveTypedefR(rhs.typeSpec());

    mode = CoercionResult::None;
    if(z::ptr(lts) == z::ptr(rts)) {
        mode = CoercionResult::Lhs;
        return z::ptr(lhs);
    }

    for(z::Ast::Unit::CoerceListList::const_iterator it = coercionList().begin(); it != coercionList().end(); ++it) {
        const z::Ast::CoerceList& coerceList = it->get();
        int lidx = -1;
        int ridx = -1;
        int cidx = 0;
        for(z::Ast::CoerceList::List::const_iterator cit = coerceList.list().begin(); cit != coerceList.list().end(); ++cit, ++cidx) {
            const z::Ast::TypeSpec& typeSpec = cit->get();
            if(z::ptr(typeSpec) == z::ptr(lts)) {
                lidx = cidx;
            }
            if(z::ptr(typeSpec) == z::ptr(rts)) {
                ridx = cidx;
            }
        }
        if((lidx >= 0) && (ridx >= 0)) {
            if(lidx >= ridx) {
                mode = CoercionResult::Lhs;
                return z::ptr(lhs);
            }
            mode = CoercionResult::Rhs;
            return z::ptr(rhs);
        }
    }

    const z::Ast::StructDefn* lsd = dynamic_cast<const z::Ast::StructDefn*>(z::ptr(lts));
    const z::Ast::StructDefn* rsd = dynamic_cast<const z::Ast::StructDefn*>(z::ptr(rts));
    if((lsd != 0) && (rsd != 0)) {
        for(StructBaseIterator sbi(lsd); sbi.hasNext(); sbi.next()) {
            if(z::ptr(sbi.get()) == rsd) {
                mode = CoercionResult::Rhs;
                return z::ptr(rhs);
            }
        }
        for(StructBaseIterator sbi(rsd); sbi.hasNext(); sbi.next()) {
            if(z::ptr(sbi.get()) == lsd) {
                mode = CoercionResult::Lhs;
                return z::ptr(lhs);
            }
        }
    }

    const z::Ast::FunctionDefn* lfd = dynamic_cast<const z::Ast::FunctionDefn*>(z::ptr(lts));
    const z::Ast::FunctionDefn* rfd = dynamic_cast<const z::Ast::FunctionDefn*>(z::ptr(rts));
    if((lfd != 0) && (rfd != 0)) {
        for(FunctionBaseIterator fbi(lfd); fbi.hasNext(); fbi.next()) {
            if(z::ptr(fbi.get()) == rfd) {
                mode = CoercionResult::Rhs;
                return z::ptr(rhs);
            }
        }
        for(FunctionBaseIterator fbi(rfd); fbi.hasNext(); fbi.next()) {
            if(z::ptr(fbi.get()) == lfd) {
                mode = CoercionResult::Lhs;
                return z::ptr(lhs);
            }
        }
    }

    const z::Ast::TemplateDefn* ltd = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(lts));
    const z::Ast::TemplateDefn* rtd = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(rts));
    if((ltd != 0) && (rtd != 0)) {
        if(z::ref(ltd).name().string() == z::ref(rtd).name().string()) {
            const z::Ast::QualifiedTypeSpec& lSubType = z::ref(ltd).at(0);
            const z::Ast::QualifiedTypeSpec& rSubType = z::ref(rtd).at(0);
            CoercionResult::T imode = CoercionResult::None;
            const z::Ast::QualifiedTypeSpec* val = canCoerceX(lSubType, rSubType, imode);
            z::unused_t(val);
            if(imode == CoercionResult::Lhs) {
                mode = CoercionResult::Lhs;
                return z::ptr(lhs);
            } else if (imode == CoercionResult::Rhs) {
                mode = CoercionResult::Rhs;
                return z::ptr(rhs);
            }
        }
    }

    mode = CoercionResult::None;
    return 0;
}

inline const z::Ast::QualifiedTypeSpec* z::Ast::Unit::canCoerce(const z::Ast::QualifiedTypeSpec& lhs, const z::Ast::QualifiedTypeSpec& rhs) const {
    CoercionResult::T mode = CoercionResult::None;
    return canCoerceX(lhs, rhs, mode);
}
const z::Ast::QualifiedTypeSpec& z::Ast::Unit::coerce(const z::Ast::Token& pos, const z::Ast::QualifiedTypeSpec& lhs, const z::Ast::QualifiedTypeSpec& rhs) {
    const z::Ast::QualifiedTypeSpec* val = canCoerce(lhs, rhs);
    if(!val) {
        throw z::Exception("Unit", zfmt(pos, "Cannot coerce '%{c}'' and '%{t}'")
                           .arg("c", ZenlangNameGenerator().qtn(lhs))
                           .arg("t", ZenlangNameGenerator().qtn(rhs))
                           );
    }
    return z::ref(val);
}

z::Ast::Scope& z::Ast::Unit::enterScope(z::Ast::Scope& scope) {
    _scopeStack.push(scope);
    if(_scopeCallback) {
        z::ref(_scopeCallback).enteringScope(scope);
    }
    return scope;
}

z::Ast::Scope& z::Ast::Unit::enterScope(const z::Ast::Token& pos) {
    z::Ast::Scope& scope = addNode(new z::Ast::Scope(pos, z::Ast::ScopeType::Local));
    return enterScope(scope);
}

void z::Ast::Unit::leaveScope(z::Ast::Scope& scope) {
    z::Ast::Scope& s = _scopeStack.top();
    z::assert_t(z::ptr(s) == z::ptr(scope));
    if(_scopeCallback) {
        z::ref(_scopeCallback).leavingScope(scope);
    }
    _scopeStack.pop();
}

void z::Ast::Unit::leaveScope() {
    z::Ast::Scope& s = _scopeStack.top();
    return leaveScope(s);
}

z::Ast::Scope& z::Ast::Unit::currentScope() {
    return _scopeStack.top();
}

const z::Ast::VariableDefn* z::Ast::Unit::hasMember(const z::Ast::Scope& scope, const z::Ast::Token& name) const {
    for(z::Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
        const z::Ast::VariableDefn& vref = it->get();
        if(vref.name().string() == name.string())
            return z::ptr(vref);
    }
    return 0;
}

const z::Ast::VariableDefn* z::Ast::Unit::getVariableDef(const z::Ast::Token& name, z::Ast::RefType::T& refType) const {
    refType = z::Ast::RefType::Local;
    typedef std::list<z::Ast::Scope*> ScopeList;
    ScopeList scopeList;

    for(Unit::ScopeStack::const_reverse_iterator it = _scopeStack.rbegin(); it != _scopeStack.rend(); ++it) {
        z::Ast::Scope& scope = it->get();

        if(scope.type() == z::Ast::ScopeType::XRef) {
            scopeList.push_back(z::ptr(scope));
        }

        switch(refType) {
            case z::Ast::RefType::Global:
                break;
            case z::Ast::RefType::XRef:
                break;
            case z::Ast::RefType::IRef:
                switch(scope.type()) {
                    case z::Ast::ScopeType::Member:
                        throw z::Exception("Unit", zfmt(name, "Internal error: Invalid vref %{s}: IRef-Member").arg("s", name) );
                    case z::Ast::ScopeType::XRef:
                        refType = z::Ast::RefType::XRef;
                        break;
                    case z::Ast::ScopeType::IRef:
                        throw z::Exception("Unit", zfmt(name, "Internal error: Invalid vref %{s}: IRef-IRef").arg("s", name) );
                    case z::Ast::ScopeType::Param:
                    case z::Ast::ScopeType::VarArg:
                        throw z::Exception("Unit", zfmt(name, "Internal error: Invalid vref %{s}: IRef-Param").arg("s", name) );
                    case z::Ast::ScopeType::Local:
                        refType = z::Ast::RefType::XRef;
                        break;
                }
                break;
            case z::Ast::RefType::Param:
                switch(scope.type()) {
                    case z::Ast::ScopeType::Member:
                        throw z::Exception("Unit", zfmt(name, "Internal error: Invalid vref %{s}: Param-Member").arg("s", name) );
                    case z::Ast::ScopeType::XRef:
                        refType = z::Ast::RefType::XRef;
                        break;
                    case z::Ast::ScopeType::IRef:
                        refType = z::Ast::RefType::IRef;
                        break;
                    case z::Ast::ScopeType::Param:
                    case z::Ast::ScopeType::VarArg:
                        throw z::Exception("Unit", zfmt(name, "Internal error: Invalid vref %{s}: Param-Param").arg("s", name) );
                    case z::Ast::ScopeType::Local:
                        refType = z::Ast::RefType::XRef;
                        break;
                }
                break;
            case z::Ast::RefType::Local:
                switch(scope.type()) {
                    case z::Ast::ScopeType::Member:
                        throw z::Exception("Unit", zfmt(name, "Internal error: Invalid vref %{s}: Local-Member").arg("s", name) );
                    case z::Ast::ScopeType::XRef:
                        throw z::Exception("Unit", zfmt(name, "Internal error: Invalid vref %{s}: Local-XRef").arg("s", name) );
                    case z::Ast::ScopeType::IRef:
                        throw z::Exception("Unit", zfmt(name, "Internal error: Invalid vref %{s}: Local-IRef").arg("s", name) );
                    case z::Ast::ScopeType::Param:
                    case z::Ast::ScopeType::VarArg:
                        refType = z::Ast::RefType::Param;
                        break;
                    case z::Ast::ScopeType::Local:
                        break;
                }
                break;
        }

        const z::Ast::VariableDefn* vref = hasMember(scope, name);
        if(vref != 0) {
            if(refType == z::Ast::RefType::XRef) {
                z::assert_t(scopeList.size() > 0);
                for(ScopeList::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
                    z::Ast::Scope& scope = z::ref(*it);

                    // check if vref already exists in this scope
                    bool found = false;
                    for(z::Ast::Scope::List::const_iterator xit = scope.list().begin(); xit != scope.list().end(); ++xit) {
                        const z::Ast::VariableDefn& xref = xit->get();
                        if(vref == z::ptr(xref)) {
                            found = true;
                            break;
                        }
                    }

                    // if not exists, add it
                    if(!found)
                        scope.addVariableDef(z::ref(vref));
                }
            }

            z::assert_t(vref != 0);
            return vref;
        }
    }
    return 0;
}

void z::Ast::Unit::leaveNamespace() {
    while(_namespaceStack.size() > 0) {
        z::Ast::Namespace& ns = _namespaceStack.top();
        leaveTypeSpec(ns);
        _namespaceStack.pop();
    }
}

z::Ast::Root& z::Ast::Unit::getRootNamespace(const int& level) {
    return (level == 0)?rootNS():importNS();
}

const z::Ast::TypeSpec* z::Ast::Unit::hasImportRootTypeSpec(const int& level, const z::Ast::Token& name) const {
    if(level == 0) {
        const z::Ast::TypeSpec* typeSpec = importNS().hasChild<const z::Ast::TypeSpec>(name.string());
        if(typeSpec)
            return typeSpec;
    }

    return 0;
}

const z::Ast::TypeSpec* z::Ast::Unit::currentTypeRefHasChild(const z::Ast::Token& name) const {
    if(_currentTypeRef == 0)
        return 0;
    const z::Ast::TypeSpec* td = z::ref(_currentTypeRef).hasChild<const z::Ast::TypeSpec>(name.string());
    if(td)
        return td;

    if(_currentImportedTypeRef) {
        const z::Ast::TypeSpec* itd = z::ref(_currentImportedTypeRef).hasChild<const z::Ast::TypeSpec>(name.string());
        if(itd) {
            return itd;
        }
    }
    return 0;
}

inline const z::Ast::TypeSpec* z::Ast::Unit::findTypeSpec(const z::Ast::TypeSpec& parent, const z::Ast::Token& name) const {
    const z::Ast::TypeSpec* child = parent.hasChild<const z::Ast::TypeSpec>(name.string());
    if(child)
        return child;
    const z::Ast::ChildTypeSpec* parentx = dynamic_cast<const z::Ast::ChildTypeSpec*>(z::ptr(parent));
    if(!parentx)
        return 0;
    return findTypeSpec(z::ref(parentx).parent(), name);
}

z::Ast::TypeSpec& z::Ast::Unit::currentTypeSpec() const {
    return _typeSpecStack.top();
}

z::Ast::TypeSpec& z::Ast::Unit::enterTypeSpec(z::Ast::TypeSpec& typeSpec) {
    _typeSpecStack.push(typeSpec);
    return _typeSpecStack.top();
}

z::Ast::TypeSpec& z::Ast::Unit::leaveTypeSpec(z::Ast::TypeSpec& typeSpec) {
    z::Ast::TypeSpec& ct = _typeSpecStack.top();
    z::assert_t(z::ptr(ct) == z::ptr(typeSpec));
    _typeSpecStack.pop();
    return ct;
}

const z::Ast::TypeSpec* z::Ast::Unit::hasRootTypeSpec(const int& level, const z::Ast::Token& name) const {
    const z::Ast::TypeSpec* typeSpec = findTypeSpec(currentTypeSpec(), name);
    if(typeSpec) {
        return typeSpec;
    }

    const z::Ast::TypeSpec* ztypeSpec = importNS().hasChild<const z::Ast::TypeSpec>("z");
    if(ztypeSpec) {
        const z::Ast::TypeSpec* ctypeSpec = z::ref(ztypeSpec).hasChild<const z::Ast::TypeSpec>(name.string());
        if(ctypeSpec) {
            return ctypeSpec;
        }
    }

    return hasImportRootTypeSpec(level, name);
}

z::Ast::StructDefn& z::Ast::Unit::getCurrentStructDefn(const z::Ast::Token& pos) {
    z::Ast::TypeSpec& ts = currentTypeSpec();
    z::Ast::StructDefn* sd = dynamic_cast<z::Ast::StructDefn*>(z::ptr(ts));
    if(sd == 0) {
        throw z::Exception("Unit", zfmt(pos, "Internal error: not a struct type %{s}").arg("s", ts.name()) );
    }
    return z::ref(sd);
}

z::Ast::InterfaceDefn& z::Ast::Unit::getCurrentInterfaceDefn(const z::Ast::Token& pos) {
    z::Ast::TypeSpec& ts = currentTypeSpec();
    z::Ast::InterfaceDefn* sd = dynamic_cast<z::Ast::InterfaceDefn*>(z::ptr(ts));
    if(sd == 0) {
        throw z::Exception("Unit", zfmt(pos, "Internal error: not a Interface type %{s}").arg("s", ts.name()) );
    }
    return z::ref(sd);
}

inline z::string z::Ast::Unit::getExpectedTypeName(const z::Ast::Token& pos, const z::Ast::Unit::ExpectedTypeSpec::Type& exType) {
    switch(exType) {
        case ExpectedTypeSpec::etNone:
            break;
        case ExpectedTypeSpec::etAuto:
            return "etAuto";
        case ExpectedTypeSpec::etVarArg:
            return "etVarArg";
        case ExpectedTypeSpec::etCallArg:
            return "etCallArg";
        case ExpectedTypeSpec::etListVal:
            return "etListVal";
        case ExpectedTypeSpec::etDictKey:
            return "etDictKey";
        case ExpectedTypeSpec::etDictVal:
            return "etDictVal";
        case ExpectedTypeSpec::etAssignment:
            return "etAssignment";
        case ExpectedTypeSpec::etEventHandler:
            return "etEventHandler";
        case ExpectedTypeSpec::etStructInit:
            return "etStructInit";
    }
    throw z::Exception("Unit", zfmt(pos, "Internal error: Unknown Expected Type %{s}").arg("s", exType ));
}

inline z::Ast::Unit::ExpectedTypeSpec::Type z::Ast::Unit::getExpectedType(const z::Ast::Token& pos) const {
    if(_expectedTypeSpecStack.size() == 0) {
        throw z::Exception("Unit", zfmt(pos, "Empty expected type stack(1)"));
    }

    return _expectedTypeSpecStack.back().type();
}

void z::Ast::Unit::pushExpectedTypeSpec(const ExpectedTypeSpec::Type& type, const z::Ast::QualifiedTypeSpec& qTypeSpec) {
    _expectedTypeSpecStack.push_back(ExpectedTypeSpec(type, qTypeSpec));
}

void z::Ast::Unit::pushExpectedTypeSpec(const ExpectedTypeSpec::Type& type) {
    _expectedTypeSpecStack.push_back(ExpectedTypeSpec(type));
}

void z::Ast::Unit::popExpectedTypeSpec(const z::Ast::Token& pos, const ExpectedTypeSpec::Type& type) {
    if(type != getExpectedType(pos)) {
        throw z::Exception("Unit", zfmt(pos, "Internal error: Invalid expected type popped. Popping %{s} got %{t}")
                           .arg("s", getExpectedTypeName(pos, type))
                           .arg("t", getExpectedTypeName(pos, _expectedTypeSpecStack.back().type()))
                           );
    }

    _expectedTypeSpecStack.pop_back();
}

bool z::Ast::Unit::popExpectedTypeSpecOrAuto(const z::Ast::Token& pos, const ExpectedTypeSpec::Type& type) {
    if(getExpectedType(pos) == ExpectedTypeSpec::etAuto) {
        popExpectedTypeSpec(pos, ExpectedTypeSpec::etAuto);
        return false;
    }
    popExpectedTypeSpec(pos, type);
    return true;
}

inline const z::Ast::Unit::ExpectedTypeSpec& z::Ast::Unit::getExpectedTypeList(const z::Ast::Token& pos) const {
    if(_expectedTypeSpecStack.size() == 0) {
        throw z::Exception("Unit", zfmt(pos, "Empty expected type stack(2)"));
    }

    const ExpectedTypeSpec& exl = _expectedTypeSpecStack.back();
    return exl;
}

const z::Ast::QualifiedTypeSpec* z::Ast::Unit::getExpectedTypeSpecIfAny() const {
    if(_expectedTypeSpecStack.size() == 0) {
        return 0;
    }

    const ExpectedTypeSpec& exl = _expectedTypeSpecStack.back();
    if(!exl.hasTypeSpec()) {
        return 0;
    }

    const z::Ast::QualifiedTypeSpec& ts = exl.typeSpec();
    return z::ptr(ts);
}

const z::Ast::QualifiedTypeSpec& z::Ast::Unit::getExpectedTypeSpec(const z::Ast::Token& pos, const z::Ast::QualifiedTypeSpec* qTypeSpec) const {
    const z::Ast::QualifiedTypeSpec* ts = getExpectedTypeSpecIfAny();
    if(ts == 0) {
        if(qTypeSpec == 0) {
            throw z::Exception("Unit", zfmt(pos, "Empty expected type stack(3)"));
        }
        return z::ref(qTypeSpec);
    }
    return z::ref(ts);
}

inline const z::Ast::QualifiedTypeSpec& z::Ast::Unit::getExpectedTypeSpecEx(const z::Ast::Token& pos) const {
    const z::Ast::QualifiedTypeSpec* ts = getExpectedTypeSpecIfAny();
    if(ts == 0) {
        throw z::Exception("Unit", zfmt(pos, "Empty expected type stack(4)"));
    }
    return z::ref(ts);
}

inline const z::Ast::TemplateDefn* z::Ast::Unit::isEnteringTemplate() const {
    const z::Ast::QualifiedTypeSpec* qts = getExpectedTypeSpecIfAny();
    if(qts == 0) {
        return 0;
    }
    const z::Ast::TypeSpec& ts = z::ref(qts).typeSpec();
    const z::Ast::TemplateDefn* td = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(ts));
    return td;
}

const z::Ast::TemplateDefn* z::Ast::Unit::isEnteringList() const {
    const z::Ast::TemplateDefn* td = isEnteringTemplate();
    if(td) {
        if(z::ref(td).name().string() == "list") {
            return td;
        }
        if(z::ref(td).name().string() == "dict") {
            return td;
        }
    }
    return 0;
}

const z::Ast::StructDefn* z::Ast::Unit::isStructExpected() const {
    const z::Ast::QualifiedTypeSpec* qts = getExpectedTypeSpecIfAny();
    if(qts == 0) {
        return 0;
    }

    const z::Ast::TypeSpec& ts = z::ref(qts).typeSpec();
    const z::Ast::StructDefn* sd = dynamic_cast<const z::Ast::StructDefn*>(z::ptr(ts));
    if(sd == 0) {
        return 0;
    }

    return sd;
}

const z::Ast::Function* z::Ast::Unit::isFunctionExpected() const {
    const z::Ast::QualifiedTypeSpec* qts = getExpectedTypeSpecIfAny();
    if(qts == 0) {
        return 0;
    }

    const z::Ast::TypeSpec& ts = z::ref(qts).typeSpec();
    const z::Ast::Function* ed = dynamic_cast<const z::Ast::Function*>(z::ptr(ts));
    if(ed == 0) {
        return 0;
    }

    return ed;
}

const z::Ast::TemplateDefn* z::Ast::Unit::isPointerExpected() const {
    const z::Ast::TemplateDefn* td = isEnteringTemplate();
    if(td) {
        if(z::ref(td).name().string() == "pointer") {
            return td;
        }
    }
    return 0;
}

const z::Ast::TemplateDefn* z::Ast::Unit::isPointerToExprExpected(const z::Ast::Expr& expr) const {
    const z::Ast::TemplateDefn* ts = isPointerExpected();
    if(ts) {
        const z::Ast::QualifiedTypeSpec& innerQts = z::ref(ts).at(0);

        Unit::CoercionResult::T mode = Unit::CoercionResult::None;
        const z::Ast::QualifiedTypeSpec* qts = canCoerceX(innerQts, expr.qTypeSpec(), mode);
        if(qts && (mode == Unit::CoercionResult::Lhs)) {
            return ts;
        }
    }
    return 0;
}

const z::Ast::StructDefn* z::Ast::Unit::isPointerToStructExpected() const {
    const z::Ast::TemplateDefn* td = isPointerExpected();
    if(td) {
        const z::Ast::QualifiedTypeSpec& valType = z::ref(td).at(0);
        const z::Ast::StructDefn* sd = dynamic_cast<const z::Ast::StructDefn*>(z::ptr(valType.typeSpec()));
        return sd;
    }
    return 0;
}

const z::Ast::StructDefn* z::Ast::Unit::isListOfStructExpected() const {
    const z::Ast::TemplateDefn* td = isEnteringList();
    const z::Ast::StructDefn* sd = 0;
    if(td) {
        if(z::ref(td).name().string() == "list") {
            const z::Ast::QualifiedTypeSpec& valType = z::ref(td).at(0);
            sd = dynamic_cast<const z::Ast::StructDefn*>(z::ptr(valType.typeSpec()));
        } else if(z::ref(td).name().string() == "dict") {
            const z::Ast::QualifiedTypeSpec& valType = z::ref(td).at(1);
            sd = dynamic_cast<const z::Ast::StructDefn*>(z::ptr(valType.typeSpec()));
        } else {
            z::assert_t(false);
        }
    }
    return sd;
}

inline const z::Ast::TypeSpec* z::Ast::Unit::isListOfPointerExpected() const {
    const z::Ast::TemplateDefn* td = isEnteringList();
    if(td) {
        if(z::ref(td).name().string() == "list") {
            const z::Ast::QualifiedTypeSpec& innerType = z::ref(td).at(0);
            const z::Ast::TemplateDefn* td1 = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(innerType.typeSpec()));
            if(td1) {
                if(z::ref(td1).name().string() == "pointer") {
                    const z::Ast::QualifiedTypeSpec& valType = z::ref(td1).at(0);
                    return z::ptr(valType.typeSpec());
                }
            }
        } else if(z::ref(td).name().string() == "dict") {
            const z::Ast::QualifiedTypeSpec& innerType = z::ref(td).at(1);
            const z::Ast::TemplateDefn* td1 = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(innerType.typeSpec()));
            if(td1) {
                if(z::ref(td1).name().string() == "pointer") {
                    const z::Ast::QualifiedTypeSpec& valType = z::ref(td1).at(0);
                    return z::ptr(valType.typeSpec());
                }
            }
        } else {
            z::assert_t(false);
        }
    }
    return 0;
}

const z::Ast::StructDefn* z::Ast::Unit::isListOfPointerToStructExpected() const {
    const z::Ast::TypeSpec* ts = isListOfPointerExpected();
    if(!ts)
        return 0;

    const z::Ast::StructDefn* sd = dynamic_cast<const z::Ast::StructDefn*>(ts);
    return sd;
}

void z::Ast::Unit::pushCallArgList(const z::Ast::Scope& in) {
    if(in.type() == z::Ast::ScopeType::VarArg) {
        pushExpectedTypeSpec(ExpectedTypeSpec::etVarArg);
    } else {
        for(z::Ast::Scope::List::const_reverse_iterator it = in.list().rbegin(); it != in.list().rend(); ++it) {
            const z::Ast::VariableDefn& def = it->get();
            pushExpectedTypeSpec(ExpectedTypeSpec::etCallArg, def.qTypeSpec());
        }
    }
}

void z::Ast::Unit::popCallArgList(const z::Ast::Token& pos, const z::Ast::Scope& in) {
    if(in.type() == z::Ast::ScopeType::VarArg) {
        popExpectedTypeSpec(pos, ExpectedTypeSpec::etVarArg);
    }
}

void z::Ast::Unit::popCallArg(const z::Ast::Token& pos) {
    if(getExpectedType(pos) != ExpectedTypeSpec::etVarArg) {
        popExpectedTypeSpec(pos, ExpectedTypeSpec::etCallArg);
    }
}
