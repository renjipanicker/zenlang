#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "base/factory.hpp"
#include "base/typename.hpp"
#include "base/compiler.hpp"

Ast::Unit::Unit() : _scopeCallback(0), _currentTypeRef(0), _currentImportedTypeRef(0), _uniqueIdx(0) {
    Ast::Root& rootNS = addNode(new Ast::Root("*root*"));
    _rootNS.reset(rootNS);

    Ast::Root& importNS = addNode(new Ast::Root("*import*"));
    _importNS.reset(importNS);

    Ast::Root& anonymousNS = addNode(new Ast::Root("*anonymous*"));
    _anonymousNS.reset(anonymousNS);
}

Ast::Unit::~Unit() {
    for(ExpectedTypeSpecStack::const_iterator it = _expectedTypeSpecStack.begin(); it != _expectedTypeSpecStack.end(); ++it) {
        const ExpectedTypeSpec& et = *it;
        std::cout << ZenlangNameGenerator().qtn(et.typeSpec()) << std::endl;
    }

    assert(_expectedTypeSpecStack.size() == 0);
    assert(_typeSpecStack.size() == 0);
}

const Ast::QualifiedTypeSpec* Ast::Unit::canCoerceX(const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs, CoercionResult::T& mode) const {
    const Ast::TypeSpec& lts = resolveTypedefR(lhs.typeSpec());
    const Ast::TypeSpec& rts = resolveTypedefR(rhs.typeSpec());

    mode = CoercionResult::None;
    if(z::ptr(lts) == z::ptr(rts)) {
        mode = CoercionResult::Lhs;
        return z::ptr(lhs);
    }

    for(Ast::Unit::CoerceListList::const_iterator it = coercionList().begin(); it != coercionList().end(); ++it) {
        const Ast::CoerceList& coerceList = it->get();
        int lidx = -1;
        int ridx = -1;
        int cidx = 0;
        for(Ast::CoerceList::List::const_iterator cit = coerceList.list().begin(); cit != coerceList.list().end(); ++cit, ++cidx) {
            const Ast::TypeSpec& typeSpec = cit->get();
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

    const Ast::StructDefn* lsd = dynamic_cast<const Ast::StructDefn*>(z::ptr(lts));
    const Ast::StructDefn* rsd = dynamic_cast<const Ast::StructDefn*>(z::ptr(rts));
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

    const Ast::FunctionDefn* lfd = dynamic_cast<const Ast::FunctionDefn*>(z::ptr(lts));
    const Ast::FunctionDefn* rfd = dynamic_cast<const Ast::FunctionDefn*>(z::ptr(rts));
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

    const Ast::TemplateDefn* ltd = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(lts));
    const Ast::TemplateDefn* rtd = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(rts));
    if((ltd != 0) && (rtd != 0)) {
        if(z::ref(ltd).name().string() == z::ref(rtd).name().string()) {
            const Ast::QualifiedTypeSpec& lSubType = z::ref(ltd).at(0);
            const Ast::QualifiedTypeSpec& rSubType = z::ref(rtd).at(0);
            CoercionResult::T imode = CoercionResult::None;
            const Ast::QualifiedTypeSpec* val = canCoerceX(lSubType, rSubType, imode);
            unused(val);
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

inline const Ast::QualifiedTypeSpec* Ast::Unit::canCoerce(const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs) const {
    CoercionResult::T mode = CoercionResult::None;
    return canCoerceX(lhs, rhs, mode);
}
const Ast::QualifiedTypeSpec& Ast::Unit::coerce(const Ast::Token& pos, const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs) {
    const Ast::QualifiedTypeSpec* val = canCoerce(lhs, rhs);
    if(!val) {
        throw z::Exception("Unit", zfmt(pos, "Cannot coerce '%{c}'' and '%{t}'")
                           .arg("c", ZenlangNameGenerator().qtn(lhs))
                           .arg("t", ZenlangNameGenerator().qtn(rhs))
                           );
    }
    return z::ref(val);
}

Ast::Scope& Ast::Unit::enterScope(Ast::Scope& scope) {
    _scopeStack.push(scope);
    if(_scopeCallback) {
        z::ref(_scopeCallback).enteringScope(scope);
    }
    return scope;
}

Ast::Scope& Ast::Unit::enterScope(const Ast::Token& pos) {
    Ast::Scope& scope = addNode(new Ast::Scope(pos, Ast::ScopeType::Local));
    return enterScope(scope);
}

void Ast::Unit::leaveScope(Ast::Scope& scope) {
    Ast::Scope& s = _scopeStack.top();
    assert(z::ptr(s) == z::ptr(scope));
    if(_scopeCallback) {
        z::ref(_scopeCallback).leavingScope(scope);
    }
    _scopeStack.pop();
}

void Ast::Unit::leaveScope() {
    Ast::Scope& s = _scopeStack.top();
    return leaveScope(s);
}

Ast::Scope& Ast::Unit::currentScope() {
    return _scopeStack.top();
}

const Ast::VariableDefn* Ast::Unit::hasMember(const Ast::Scope& scope, const Ast::Token& name) const {
    for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
        const Ast::VariableDefn& vref = it->get();
        if(vref.name().string() == name.string())
            return z::ptr(vref);
    }
    return 0;
}

const Ast::VariableDefn* Ast::Unit::getVariableDef(const Ast::Token& name, Ast::RefType::T& refType) const {
    refType = Ast::RefType::Local;
    typedef std::list<Ast::Scope*> ScopeList;
    ScopeList scopeList;

    for(Unit::ScopeStack::const_reverse_iterator it = _scopeStack.rbegin(); it != _scopeStack.rend(); ++it) {
        Ast::Scope& scope = it->get();

        if(scope.type() == Ast::ScopeType::XRef) {
            scopeList.push_back(z::ptr(scope));
        }

        switch(refType) {
            case Ast::RefType::Global:
                break;
            case Ast::RefType::XRef:
                break;
            case Ast::RefType::Param:
                switch(scope.type()) {
                    case Ast::ScopeType::Member:
                        throw z::Exception("Unit", zfmt(name, "Internal error: Invalid vref %{s}: Param-Member").arg("s", name) );
                    case Ast::ScopeType::XRef:
                        refType = Ast::RefType::XRef;
                        break;
                    case Ast::ScopeType::Param:
                    case Ast::ScopeType::VarArg:
                        throw z::Exception("Unit", zfmt(name, "Internal error: Invalid vref %{s}: Param-Param").arg("s", name) );
                    case Ast::ScopeType::Local:
                        refType = Ast::RefType::XRef;
                        break;
                }
                break;
            case Ast::RefType::Local:
                switch(scope.type()) {
                    case Ast::ScopeType::Member:
                        throw z::Exception("Unit", zfmt(name, "Internal error: Invalid vref %{s}: Local-Member").arg("s", name) );
                    case Ast::ScopeType::XRef:
                        throw z::Exception("Unit", zfmt(name, "Internal error: Invalid vref %{s}: Local-XRef").arg("s", name) );
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
                    Ast::Scope& scope = z::ref(*it);

                    // check if vref already exists in this scope
                    bool found = false;
                    for(Ast::Scope::List::const_iterator xit = scope.list().begin(); xit != scope.list().end(); ++xit) {
                        const Ast::VariableDefn& xref = xit->get();
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

            assert(vref);
            return vref;
        }
    }
    return 0;
}

void Ast::Unit::leaveNamespace() {
    while(_namespaceStack.size() > 0) {
        Ast::Namespace& ns = _namespaceStack.top();
        leaveTypeSpec(ns);
        _namespaceStack.pop();
    }
}

Ast::Root& Ast::Unit::getRootNamespace(const int& level) {
    return (level == 0)?rootNS():importNS();
}

const Ast::TypeSpec* Ast::Unit::hasImportRootTypeSpec(const int& level, const Ast::Token& name) const {
    if(level == 0) {
        const Ast::TypeSpec* typeSpec = importNS().hasChild<const Ast::TypeSpec>(name.string());
        if(typeSpec)
            return typeSpec;
    }

    return 0;
}

const Ast::TypeSpec* Ast::Unit::currentTypeRefHasChild(const Ast::Token& name) const {
    if(_currentTypeRef == 0)
        return 0;
    const Ast::TypeSpec* td = z::ref(_currentTypeRef).hasChild<const Ast::TypeSpec>(name.string());
    if(td)
        return td;

    if(_currentImportedTypeRef) {
        const Ast::TypeSpec* itd = z::ref(_currentImportedTypeRef).hasChild<const Ast::TypeSpec>(name.string());
        if(itd) {
            return itd;
        }
    }
    return 0;
}

inline const Ast::TypeSpec* Ast::Unit::findTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const {
    const Ast::TypeSpec* child = parent.hasChild<const Ast::TypeSpec>(name.string());
    if(child)
        return child;
    const Ast::ChildTypeSpec* parentx = dynamic_cast<const Ast::ChildTypeSpec*>(z::ptr(parent));
    if(!parentx)
        return 0;
    return findTypeSpec(z::ref(parentx).parent(), name);
}

Ast::TypeSpec& Ast::Unit::currentTypeSpec() const {
    return _typeSpecStack.top();
}

Ast::TypeSpec& Ast::Unit::enterTypeSpec(Ast::TypeSpec& typeSpec) {
    _typeSpecStack.push(typeSpec);
    return _typeSpecStack.top();
}

Ast::TypeSpec& Ast::Unit::leaveTypeSpec(Ast::TypeSpec& typeSpec) {
    Ast::TypeSpec& ct = _typeSpecStack.top();
    assert(z::ptr(ct) == z::ptr(typeSpec));
    _typeSpecStack.pop();
    return ct;
}

const Ast::TypeSpec* Ast::Unit::hasRootTypeSpec(const int& level, const Ast::Token& name) const {
    const Ast::TypeSpec* typeSpec = findTypeSpec(currentTypeSpec(), name);
    if(typeSpec)
        return typeSpec;

    return hasImportRootTypeSpec(level, name);
}

Ast::StructDefn& Ast::Unit::getCurrentStructDefn(const Ast::Token& pos) {
    Ast::TypeSpec& ts = currentTypeSpec();
    Ast::StructDefn* sd = dynamic_cast<Ast::StructDefn*>(z::ptr(ts));
    if(sd == 0) {
        throw z::Exception("Unit", zfmt(pos, "Internal error: not a struct type %{s}").arg("s", ts.name()) );
    }
    return z::ref(sd);
}

inline z::string Ast::Unit::getExpectedTypeName(const Ast::Token& pos, const Ast::Unit::ExpectedTypeSpec::Type& exType) {
    switch(exType) {
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

inline Ast::Unit::ExpectedTypeSpec::Type Ast::Unit::getExpectedType(const Ast::Token& pos) const {
    if(_expectedTypeSpecStack.size() == 0) {
        throw z::Exception("Unit", zfmt(pos, "Empty expected type stack(1)"));
    }

    return _expectedTypeSpecStack.back().type();
}

void Ast::Unit::pushExpectedTypeSpec(const ExpectedTypeSpec::Type& type, const Ast::QualifiedTypeSpec& qTypeSpec) {
    _expectedTypeSpecStack.push_back(ExpectedTypeSpec(type, qTypeSpec));
}

void Ast::Unit::pushExpectedTypeSpec(const ExpectedTypeSpec::Type& type) {
    _expectedTypeSpecStack.push_back(ExpectedTypeSpec(type));
}

void Ast::Unit::popExpectedTypeSpec(const Ast::Token& pos, const ExpectedTypeSpec::Type& type) {
    if(type != getExpectedType(pos)) {
        throw z::Exception("Unit", zfmt(pos, "Internal error: Invalid expected type popped. Popping %{s} got %{t}")
                           .arg("s", getExpectedTypeName(pos, type))
                           .arg("t", getExpectedTypeName(pos, _expectedTypeSpecStack.back().type()))
                           );
    }

    _expectedTypeSpecStack.pop_back();
}

bool Ast::Unit::popExpectedTypeSpecOrAuto(const Ast::Token& pos, const ExpectedTypeSpec::Type& type) {
    if(getExpectedType(pos) == ExpectedTypeSpec::etAuto) {
        popExpectedTypeSpec(pos, ExpectedTypeSpec::etAuto);
        return false;
    }
    popExpectedTypeSpec(pos, type);
    return true;
}

inline const Ast::Unit::ExpectedTypeSpec& Ast::Unit::getExpectedTypeList(const Ast::Token& pos) const {
    if(_expectedTypeSpecStack.size() == 0) {
        throw z::Exception("Unit", zfmt(pos, "Empty expected type stack(2)"));
    }

    const ExpectedTypeSpec& exl = _expectedTypeSpecStack.back();
    return exl;
}

const Ast::QualifiedTypeSpec* Ast::Unit::getExpectedTypeSpecIfAny() const {
    if(_expectedTypeSpecStack.size() == 0) {
        return 0;
    }

    const ExpectedTypeSpec& exl = _expectedTypeSpecStack.back();
    if(!exl.hasTypeSpec()) {
        return 0;
    }

    const Ast::QualifiedTypeSpec& ts = exl.typeSpec();
    return z::ptr(ts);
}

const Ast::QualifiedTypeSpec& Ast::Unit::getExpectedTypeSpec(const Ast::Token& pos, const Ast::QualifiedTypeSpec* qTypeSpec) const {
    const Ast::QualifiedTypeSpec* ts = getExpectedTypeSpecIfAny();
    if(ts == 0) {
        if(qTypeSpec == 0) {
            throw z::Exception("Unit", zfmt(pos, "Empty expected type stack(3)"));
        }
        return z::ref(qTypeSpec);
    }
    return z::ref(ts);
}

inline const Ast::QualifiedTypeSpec& Ast::Unit::getExpectedTypeSpecEx(const Ast::Token& pos) const {
    const Ast::QualifiedTypeSpec* ts = getExpectedTypeSpecIfAny();
    if(ts == 0) {
        throw z::Exception("Unit", zfmt(pos, "Empty expected type stack(4)"));
    }
    return z::ref(ts);
}

inline const Ast::TemplateDefn* Ast::Unit::isEnteringTemplate() const {
    const Ast::QualifiedTypeSpec* qts = getExpectedTypeSpecIfAny();
    if(qts == 0) {
        return 0;
    }
    const Ast::TypeSpec& ts = z::ref(qts).typeSpec();
    const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(ts));
    return td;
}

const Ast::TemplateDefn* Ast::Unit::isEnteringList() const {
    const Ast::TemplateDefn* td = isEnteringTemplate();
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

const Ast::StructDefn* Ast::Unit::isStructExpected() const {
    const Ast::QualifiedTypeSpec* qts = getExpectedTypeSpecIfAny();
    if(qts == 0) {
        return 0;
    }

    const Ast::TypeSpec& ts = z::ref(qts).typeSpec();
    const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(z::ptr(ts));
    if(sd == 0) {
        return 0;
    }

    return sd;
}

const Ast::Function* Ast::Unit::isFunctionExpected() const {
    const Ast::QualifiedTypeSpec* qts = getExpectedTypeSpecIfAny();
    if(qts == 0) {
        return 0;
    }

    const Ast::TypeSpec& ts = z::ref(qts).typeSpec();
    const Ast::Function* ed = dynamic_cast<const Ast::Function*>(z::ptr(ts));
    if(ed == 0) {
        return 0;
    }

    return ed;
}

const Ast::TemplateDefn* Ast::Unit::isPointerExpected() const {
    const Ast::TemplateDefn* td = isEnteringTemplate();
    if(td) {
        if(z::ref(td).name().string() == "pointer") {
            return td;
        }
    }
    return 0;
}

const Ast::TemplateDefn* Ast::Unit::isPointerToExprExpected(const Ast::Expr& expr) const {
    const Ast::TemplateDefn* ts = isPointerExpected();
    if(ts) {
        const Ast::QualifiedTypeSpec& innerQts = z::ref(ts).at(0);

        Unit::CoercionResult::T mode = Unit::CoercionResult::None;
        const Ast::QualifiedTypeSpec* qts = canCoerceX(innerQts, expr.qTypeSpec(), mode);
        if(qts && (mode == Unit::CoercionResult::Lhs)) {
            return ts;
        }
    }
    return 0;
}

const Ast::StructDefn* Ast::Unit::isPointerToStructExpected() const {
    const Ast::TemplateDefn* td = isPointerExpected();
    if(td) {
        const Ast::QualifiedTypeSpec& valType = z::ref(td).at(0);
        const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(z::ptr(valType.typeSpec()));
        return sd;
    }
    return 0;
}

const Ast::StructDefn* Ast::Unit::isListOfStructExpected() const {
    const Ast::TemplateDefn* td = isEnteringList();
    const Ast::StructDefn* sd = 0;
    if(td) {
        if(z::ref(td).name().string() == "list") {
            const Ast::QualifiedTypeSpec& valType = z::ref(td).at(0);
            sd = dynamic_cast<const Ast::StructDefn*>(z::ptr(valType.typeSpec()));
        } else if(z::ref(td).name().string() == "dict") {
            const Ast::QualifiedTypeSpec& valType = z::ref(td).at(1);
            sd = dynamic_cast<const Ast::StructDefn*>(z::ptr(valType.typeSpec()));
        } else {
            assert(false);
        }
    }
    return sd;
}

inline const Ast::TypeSpec* Ast::Unit::isListOfPointerExpected() const {
    const Ast::TemplateDefn* td = isEnteringList();
    if(td) {
        if(z::ref(td).name().string() == "list") {
            const Ast::QualifiedTypeSpec& innerType = z::ref(td).at(0);
            const Ast::TemplateDefn* td1 = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(innerType.typeSpec()));
            if(td1) {
                if(z::ref(td1).name().string() == "pointer") {
                    const Ast::QualifiedTypeSpec& valType = z::ref(td1).at(0);
                    return z::ptr(valType.typeSpec());
                }
            }
        } else if(z::ref(td).name().string() == "dict") {
            const Ast::QualifiedTypeSpec& innerType = z::ref(td).at(1);
            const Ast::TemplateDefn* td1 = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(innerType.typeSpec()));
            if(td1) {
                if(z::ref(td1).name().string() == "pointer") {
                    const Ast::QualifiedTypeSpec& valType = z::ref(td1).at(0);
                    return z::ptr(valType.typeSpec());
                }
            }
        } else {
            assert(false);
        }
    }
    return 0;
}

const Ast::StructDefn* Ast::Unit::isListOfPointerToStructExpected() const {
    const Ast::TypeSpec* ts = isListOfPointerExpected();
    if(!ts)
        return 0;

    const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(ts);
    return sd;
}

void Ast::Unit::pushCallArgList(const Ast::Scope& in) {
    if(in.type() == Ast::ScopeType::VarArg) {
        pushExpectedTypeSpec(ExpectedTypeSpec::etVarArg);
    } else {
        for(Ast::Scope::List::const_reverse_iterator it = in.list().rbegin(); it != in.list().rend(); ++it) {
            const Ast::VariableDefn& def = it->get();
            pushExpectedTypeSpec(ExpectedTypeSpec::etCallArg, def.qTypeSpec());
        }
    }
}

void Ast::Unit::popCallArgList(const Ast::Token& pos, const Ast::Scope& in) {
    if(in.type() == Ast::ScopeType::VarArg) {
        popExpectedTypeSpec(pos, ExpectedTypeSpec::etVarArg);
    }
}

void Ast::Unit::popCallArg(const Ast::Token& pos) {
    if(getExpectedType(pos) != ExpectedTypeSpec::etVarArg) {
        popExpectedTypeSpec(pos, ExpectedTypeSpec::etCallArg);
    }
}
