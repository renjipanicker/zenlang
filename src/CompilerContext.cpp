#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "NodeFactory.hpp"
#include "typename.hpp"
#include "compiler.hpp"

Ast::Context::Context(const std::string& filename)
    : _filename(filename), _statementVisitor(0), _rootNS("*root*"), _importNS("*import*"), _currentTypeRef(0), _currentImportedTypeRef(0) {
}

Ast::Context::~Context() {
    assert(_expectedTypeSpecStack.size() == 0);
    assert(_typeSpecStack.size() == 0);
}

const Ast::QualifiedTypeSpec* Ast::Context::canCoerceX(const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs, CoercionResult::T& mode) const {
    const Ast::TypeSpec& lts = resolveTypedefR(lhs.typeSpec());
    const Ast::TypeSpec& rts = resolveTypedefR(rhs.typeSpec());

    mode = CoercionResult::None;
    if(z::ptr(lts) == z::ptr(rts)) {
        mode = CoercionResult::Lhs;
        return z::ptr(lhs);
    }

    for(Ast::Context::CoerceListList::const_iterator it = coercionList().begin(); it != coercionList().end(); ++it) {
        const Ast::CoerceList& coerceList = z::ref(*it);
        int lidx = -1;
        int ridx = -1;
        int cidx = 0;
        for(Ast::CoerceList::List::const_iterator cit = coerceList.list().begin(); cit != coerceList.list().end(); ++cit, ++cidx) {
            const Ast::TypeSpec& typeSpec = z::ref(*cit);
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

inline const Ast::QualifiedTypeSpec* Ast::Context::canCoerce(const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs) const {
    CoercionResult::T mode = CoercionResult::None;
    return canCoerceX(lhs, rhs, mode);
}
const Ast::QualifiedTypeSpec& Ast::Context::coerce(const Ast::Token& pos, const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs) {
    const Ast::QualifiedTypeSpec* val = canCoerce(lhs, rhs);
    if(!val) {
        throw z::Exception("%s Cannot coerce '%s' and '%s'\n",
                        err(_filename, pos).c_str(),
                        getQualifiedTypeSpecName(lhs, GenMode::Import).c_str(),
                        getQualifiedTypeSpecName(rhs, GenMode::Import).c_str());
    }
    return z::ref(val);
}

Ast::Scope& Ast::Context::enterScope(Ast::Scope& scope) {
    _scopeStack.push_back(z::ptr(scope));
    return scope;
}

Ast::Scope& Ast::Context::leaveScope() {
    Ast::Scope* s = _scopeStack.back();
    assert(_scopeStack.size() > 0);
    _scopeStack.pop_back();
    return z::ref(s);
}

Ast::Scope& Ast::Context::leaveScope(Ast::Scope& scope) {
    Ast::Scope& s = leaveScope();
    assert(z::ptr(s) == z::ptr(scope));
    return s;
}

Ast::Scope& Ast::Context::currentScope() {
    assert(_scopeStack.size() > 0);
    Ast::Scope* scope = _scopeStack.back();
    return z::ref(scope);
}

const Ast::VariableDefn* Ast::Context::hasMember(const Ast::Scope& scope, const Ast::Token& name) const {
    for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
        const Ast::VariableDefn& vref = z::ref(*it);
        if(vref.name().string() == name.string())
            return z::ptr(vref);
    }
    return 0;
}

const Ast::VariableDefn* Ast::Context::getVariableDef(const std::string& filename, const Ast::Token& name, Ast::RefType::T& refType) const {
    refType = Ast::RefType::Local;
    typedef std::list<Ast::Scope*> ScopeList;
    ScopeList scopeList;

    for(Context::ScopeStack::const_reverse_iterator it = _scopeStack.rbegin(); it != _scopeStack.rend(); ++it) {
        Ast::Scope& scope = z::ref(*it);

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
                    case Ast::ScopeType::Global:
                        throw z::Exception("%s Internal error: Invalid vref %s: Param-Global\n", err(filename, name).c_str(), name.text());
                    case Ast::ScopeType::Member:
                        throw z::Exception("%s Internal error: Invalid vref %s: Param-Member\n", err(filename, name).c_str(), name.text());
                    case Ast::ScopeType::XRef:
                        refType = Ast::RefType::XRef;
                        break;
                    case Ast::ScopeType::Param:
                    case Ast::ScopeType::VarArg:
                        throw z::Exception("%s Internal error: Invalid vref %s: Param-Param\n", err(filename, name).c_str(), name.text());
                    case Ast::ScopeType::Local:
                        refType = Ast::RefType::XRef;
                        break;
                }
                break;
            case Ast::RefType::Local:
                switch(scope.type()) {
                    case Ast::ScopeType::Global:
                        throw z::Exception("%s Internal error: Invalid vref %s: Local-Global\n", err(filename, name).c_str(), name.text());
                    case Ast::ScopeType::Member:
                        throw z::Exception("%s Internal error: Invalid vref %s: Local-Member\n", err(filename, name).c_str(), name.text());
                    case Ast::ScopeType::XRef:
                        throw z::Exception("%s Internal error: Invalid vref %s: Local-XRef\n", err(filename, name).c_str(), name.text());
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
                        const Ast::VariableDefn* xref = *xit;
                        if(vref == xref) {
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

void Ast::Context::leaveNamespace() {
    while(_namespaceStack.size() > 0) {
        Ast::Namespace* ns = _namespaceStack.back();
        leaveTypeSpec(z::ref(ns));
        _namespaceStack.pop_back();
    }
}

Ast::Root& Ast::Context::getRootNamespace(const int& level) {
    return (level == 0)?_rootNS:_importNS;
}

inline const Ast::TypeSpec* Ast::Context::hasImportRootTypeSpec(const int& level, const Ast::Token& name) const {
    if(level == 0) {
        const Ast::TypeSpec* typeSpec = _importNS.hasChild<const Ast::TypeSpec>(name.string());
        if(typeSpec)
            return typeSpec;
    }

    return 0;
}

const Ast::TypeSpec* Ast::Context::currentTypeRefHasChild(const Ast::Token& name) const {
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

inline const Ast::TypeSpec* Ast::Context::findTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const {
    const Ast::TypeSpec* child = parent.hasChild<const Ast::TypeSpec>(name.string());
    if(child)
        return child;
    const Ast::ChildTypeSpec* parentx = dynamic_cast<const Ast::ChildTypeSpec*>(z::ptr(parent));
    if(!parentx)
        return 0;
    return findTypeSpec(z::ref(parentx).parent(), name);
}

Ast::TypeSpec& Ast::Context::currentTypeSpec() const {
    assert(_typeSpecStack.size() > 0);
    return z::ref(_typeSpecStack.back());
}

Ast::TypeSpec& Ast::Context::enterTypeSpec(Ast::TypeSpec& typeSpec) {
    _typeSpecStack.push_back(z::ptr(typeSpec));
    return z::ref(_typeSpecStack.back());
}

Ast::TypeSpec& Ast::Context::leaveTypeSpec(Ast::TypeSpec& typeSpec) {
    Ast::TypeSpec* ct = _typeSpecStack.back();
    assert(ct == z::ptr(typeSpec));
    _typeSpecStack.pop_back();
    return z::ref(ct);
}

const Ast::TypeSpec* Ast::Context::hasRootTypeSpec(const int& level, const Ast::Token& name) const {
    const Ast::TypeSpec* typeSpec = findTypeSpec(currentTypeSpec(), name);
    if(typeSpec)
        return typeSpec;

    return hasImportRootTypeSpec(level, name);
}

Ast::StructDefn& Ast::Context::getCurrentStructDefn(const Ast::Token& pos) {
    Ast::TypeSpec& ts = currentTypeSpec();
    Ast::StructDefn* sd = dynamic_cast<Ast::StructDefn*>(z::ptr(ts));
    if(sd == 0) {
        throw z::Exception("%s Internal error: not a struct type'%s'\n", err(_filename, pos).c_str(), ts.name().text());
    }
    return z::ref(sd);
}

inline std::string Ast::Context::getExpectedTypeName(const Ast::Context::ExpectedTypeSpec::Type& exType) {
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
    throw z::Exception("Internal error: Unknown Expected Type '%d'\n", exType);
}

inline Ast::Context::ExpectedTypeSpec::Type Ast::Context::getExpectedType(const Ast::Token& pos) const {
    if(_expectedTypeSpecStack.size() == 0) {
        throw z::Exception("%s Empty expected type stack\n", err(_filename, pos).c_str());
    }

    return _expectedTypeSpecStack.back().type();
}

void Ast::Context::pushExpectedTypeSpec(const ExpectedTypeSpec::Type& type, const Ast::QualifiedTypeSpec& qTypeSpec) {
    _expectedTypeSpecStack.push_back(ExpectedTypeSpec(type, z::ptr(qTypeSpec)));
}

void Ast::Context::pushExpectedTypeSpec(const ExpectedTypeSpec::Type& type) {
    _expectedTypeSpecStack.push_back(ExpectedTypeSpec(type));
}

void Ast::Context::popExpectedTypeSpec(const Ast::Token& pos, const ExpectedTypeSpec::Type& type) {
    if(type != getExpectedType(pos)) {
        throw z::Exception("%s Internal error: Invalid expected type popped. Popping %s, got %s\n",
                        err(_filename, pos).c_str(),
                        getExpectedTypeName(type).c_str(),
                        getExpectedTypeName(_expectedTypeSpecStack.back().type()).c_str());
    }

    _expectedTypeSpecStack.pop_back();
}

bool Ast::Context::popExpectedTypeSpecOrAuto(const Ast::Token& pos, const ExpectedTypeSpec::Type& type) {
    if(getExpectedType(pos) == ExpectedTypeSpec::etAuto) {
        popExpectedTypeSpec(pos, ExpectedTypeSpec::etAuto);
        return false;
    }
    popExpectedTypeSpec(pos, type);
    return true;
}

inline const Ast::Context::ExpectedTypeSpec& Ast::Context::getExpectedTypeList(const Ast::Token& pos) const {
    if(_expectedTypeSpecStack.size() == 0) {
        throw z::Exception("%s Empty expected type stack\n", err(_filename, pos).c_str());
    }

    const ExpectedTypeSpec& exl = _expectedTypeSpecStack.back();
    return exl;
}

const Ast::QualifiedTypeSpec* Ast::Context::getExpectedTypeSpecIfAny() const {
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

const Ast::QualifiedTypeSpec& Ast::Context::getExpectedTypeSpec(const Ast::QualifiedTypeSpec* qTypeSpec) const {
    const Ast::QualifiedTypeSpec* ts = getExpectedTypeSpecIfAny();
    if(ts == 0) {
        return z::ref(qTypeSpec);
    }
    return z::ref(ts);
}

inline const Ast::QualifiedTypeSpec& Ast::Context::getExpectedTypeSpecEx(const Ast::Token& pos) const {
    const Ast::QualifiedTypeSpec* ts = getExpectedTypeSpecIfAny();
    if(ts == 0) {
        throw z::Exception("%s Empty expected type stack\n", err(_filename, pos).c_str());
    }
    return z::ref(ts);
}

inline const Ast::TemplateDefn* Ast::Context::isEnteringTemplate() const {
    const Ast::QualifiedTypeSpec* qts = getExpectedTypeSpecIfAny();
    if(qts == 0) {
        return 0;
    }
    const Ast::TypeSpec& ts = z::ref(qts).typeSpec();
    const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(ts));
    return td;
}

const Ast::TemplateDefn* Ast::Context::isEnteringList() const {
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

const Ast::StructDefn* Ast::Context::isStructExpected() const {
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

const Ast::Function* Ast::Context::isFunctionExpected() const {
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

const Ast::TemplateDefn* Ast::Context::isPointerExpected() const {
    const Ast::TemplateDefn* td = isEnteringTemplate();
    if(td) {
        if(z::ref(td).name().string() == "pointer") {
            return td;
        }
    }
    return 0;
}

const Ast::TemplateDefn* Ast::Context::isPointerToExprExpected(const Ast::Expr& expr) const {
    const Ast::TemplateDefn* ts = isPointerExpected();
    if(ts) {
        const Ast::QualifiedTypeSpec& innerQts = z::ref(ts).at(0);

        Context::CoercionResult::T mode = Context::CoercionResult::None;
        const Ast::QualifiedTypeSpec* qts = canCoerceX(innerQts, expr.qTypeSpec(), mode);
        if(qts && (mode == Context::CoercionResult::Lhs)) {
            return ts;
        }
    }
    return 0;
}

const Ast::StructDefn* Ast::Context::isPointerToStructExpected() const {
    const Ast::TemplateDefn* td = isPointerExpected();
    if(td) {
        const Ast::QualifiedTypeSpec& valType = z::ref(td).at(0);
        const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(z::ptr(valType.typeSpec()));
        return sd;
    }
    return 0;
}

const Ast::StructDefn* Ast::Context::isListOfStructExpected() const {
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

inline const Ast::TypeSpec* Ast::Context::isListOfPointerExpected() const {
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

const Ast::StructDefn* Ast::Context::isListOfPointerToStructExpected() const {
    const Ast::TypeSpec* ts = isListOfPointerExpected();
    if(!ts)
        return 0;

    const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(ts);
    return sd;
}

void Ast::Context::pushCallArgList(const Ast::Scope& in) {
    if(in.type() == Ast::ScopeType::VarArg) {
        pushExpectedTypeSpec(ExpectedTypeSpec::etVarArg);
    } else {
        for(Ast::Scope::List::const_reverse_iterator it = in.list().rbegin(); it != in.list().rend(); ++it) {
            const Ast::VariableDefn& def = z::ref(*it);
            pushExpectedTypeSpec(ExpectedTypeSpec::etCallArg, def.qTypeSpec());
        }
    }
}

void Ast::Context::popCallArgList(const Ast::Token& pos, const Ast::Scope& in) {
    if(in.type() == Ast::ScopeType::VarArg) {
        popExpectedTypeSpec(pos, ExpectedTypeSpec::etVarArg);
    }
}

void Ast::Context::popCallArg(const Ast::Token& pos) {
    if(getExpectedType(pos) != ExpectedTypeSpec::etVarArg) {
        popExpectedTypeSpec(pos, ExpectedTypeSpec::etCallArg);
    }
}
