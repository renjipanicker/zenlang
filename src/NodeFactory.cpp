#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "NodeFactory.hpp"
#include "error.hpp"
#include "typename.hpp"
#include "compiler.hpp"

/////////////////////////////////////////////////////////////////////////
inline Ast::QualifiedTypeSpec& Ast::NodeFactory::addQualifiedTypeSpec(const Ast::Token& pos, const bool& isConst, const TypeSpec& typeSpec, const bool& isRef) {
    Ast::QualifiedTypeSpec& qualifiedTypeSpec = unit().addNode(new Ast::QualifiedTypeSpec(pos, isConst, typeSpec, isRef));
    return qualifiedTypeSpec;
}

inline const Ast::QualifiedTypeSpec& Ast::NodeFactory::getQualifiedTypeSpec(const Ast::Token& pos, const z::string& name) {
    Ast::Token token(filename(), pos.row(), pos.col(), name);
    const Ast::TypeSpec& typeSpec = unit().getRootTypeSpec<Ast::TypeSpec>(_module.level(), token);
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, typeSpec, false);
    return qTypeSpec;
}

inline const Ast::Expr& Ast::NodeFactory::getDefaultValue(const Ast::TypeSpec& typeSpec, const Ast::Token& name) {
    const Ast::TypeSpec* ts = resolveTypedef(typeSpec);

//    trace("getDef: %s %lu, unit(%lu)\n", ZenlangNameGenerator().tn(z::ref(ts), GenMode::Import).c_str(), (unsigned long)ts, z::pad(_unit));
    const Ast::Unit::DefaultValueList& list = unit().defaultValueList();
    Ast::Unit::DefaultValueList::const_iterator it = list.find(ts);
    if(it != list.end()) {
        return it->second.get();
    }

    const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(ts);
    if(td != 0) {
        const z::string tdName = z::ref(td).name().string() ; // ZenlangNameGenerator().tn(z::ref(td), GenMode::Import); \todo this is incorrect, it will match any type called, say, list.
        if(tdName == "pointer") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, z::ref(td), false);
            Ast::ExprList& exprList = addExprList(name);
            const Ast::QualifiedTypeSpec& subType = z::ref(td).at(0);
            const Ast::Expr& nameExpr = getDefaultValue(subType.typeSpec(), name);
            exprList.addExpr(nameExpr);
            Ast::PointerInstanceExpr& expr = unit().addNode(new Ast::PointerInstanceExpr(name, qTypeSpec, z::ref(td), z::ref(td), exprList));
            return expr;
        }
        if(tdName == "list") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, z::ref(td), false);
            Ast::ListList& llist = unit().addNode(new Ast::ListList(name));
            const Ast::QualifiedTypeSpec& qlType = z::ref(td).list().at(0);
            llist.valueType(qlType);
            Ast::ListExpr& expr = unit().addNode(new Ast::ListExpr(name, qTypeSpec, llist));
            return expr;
        }
        if(tdName == "dict") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, z::ref(td), false);
            Ast::DictList& llist = unit().addNode(new Ast::DictList(name));
            const Ast::QualifiedTypeSpec& qlType = z::ref(td).list().at(0);
            const Ast::QualifiedTypeSpec& qrType = z::ref(td).list().at(1);
            llist.keyType(qlType);
            llist.valueType(qrType);
            Ast::DictExpr& expr = unit().addNode(new Ast::DictExpr(name, qTypeSpec, llist));
            return expr;
        }
        if(tdName == "ptr") {
            Ast::Token value(name.filename(), name.row(), name.col(), "0");
            Ast::ConstantIntExpr& expr = aConstantIntExpr(value);
            return expr;
        }
    }

    const Ast::EnumDefn* ed = dynamic_cast<const Ast::EnumDefn*>(ts);
    if(ed != 0) {
        const Ast::Scope::List::const_iterator rit = z::ref(ed).list().begin();
        if(rit == z::ref(ed).list().end()) {
            throw z::Exception("NodeFactory", zfmt(typeSpec.name(), "Empty enum type %{s}").add("s", z::ref(ed).name() ));
        }
        const Ast::VariableDefn& vref = rit->get();
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, typeSpec, false);
        Ast::EnumMemberExpr& typeSpecMemberExpr = unit().addNode(new Ast::EnumMemberExpr(name, qTypeSpec, typeSpec, vref));
        return typeSpecMemberExpr;
    }

    const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(ts);
    if(sd != 0) {
        Ast::StructInstanceExpr* expr = aStructInstanceExpr(name, z::ref(sd));
        return z::ref(expr);
    }

    const Ast::Function* fd = dynamic_cast<const Ast::Function*>(ts);
    if(fd != 0) {
        Ast::ExprList& exprList = addExprList(name);
        Ast::FunctionInstanceExpr* expr = aFunctionInstanceExpr(name, z::ref(fd), exprList);
        return z::ref(expr);
    }

    throw z::Exception("NodeFactory", zfmt(name, "No default value for type %{s}").add("s", z::ref(ts).name() ));
}

inline const Ast::Expr& Ast::NodeFactory::convertExprToExpectedTypeSpec(const Ast::Token& pos, const Ast::Expr& initExpr) {
    // check if lhs is a pointer to rhs, if so auto-convert
    const Ast::TemplateDefn* ts = unit().isPointerToExprExpected(initExpr);
    if(ts) {
        Ast::PointerInstanceExpr* expr = aPointerInstanceExpr(pos, initExpr);
        return z::ref(expr);
    }

    const Ast::QualifiedTypeSpec* qts = unit().getExpectedTypeSpecIfAny();
    if(qts) {
        // if expected type is z::ptr, no checking
        const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(z::ref(qts).typeSpec()));
        if((td) && (z::ref(td).name().string() == "ptr")) {
            return initExpr;
        }

        // check if rhs is a pointer to lhs, if so, auto-convert
        const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(initExpr.qTypeSpec().typeSpec()));
        if((templateDefn) && (z::ref(templateDefn).name().string() == "pointer")) {
            const Ast::QualifiedTypeSpec& rhsQts = z::ref(templateDefn).at(0);
            Unit::CoercionResult::T mode = Unit::CoercionResult::None;
            unit().canCoerceX(z::ref(qts), rhsQts, mode);
            if(mode == Unit::CoercionResult::Rhs) {
                const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(qts).isConst(), z::ref(qts).typeSpec(), true);
                Ast::TypecastExpr& typecastExpr = unit().addNode(new Ast::DynamicTypecastExpr(pos, qTypeSpec, qTypeSpec, initExpr));
                return typecastExpr;
            }
        }

        // check if initExpr can be converted to expected type, if any
        Unit::CoercionResult::T mode = Unit::CoercionResult::None;
        const Ast::QualifiedTypeSpec* cqts = unit().canCoerceX(z::ref(qts), initExpr.qTypeSpec(), mode);
        if(mode != Unit::CoercionResult::Lhs) {
            throw z::Exception("NodeFactory", zfmt(pos, "Cannot convert expression from '%{f}' to '%{t}' (%{m})")
                               .add("f", ZenlangNameGenerator().qtn(initExpr.qTypeSpec()) )
                               .add("t", ZenlangNameGenerator().qtn(z::ref(qts)) )
                               .add("m", mode )
                               );
        }
        if(z::ptr(z::ref(cqts).typeSpec()) != z::ptr(initExpr.qTypeSpec().typeSpec())) {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(cqts).isConst(), z::ref(qts).typeSpec(), false);
            Ast::TypecastExpr& typecastExpr = unit().addNode(new Ast::StaticTypecastExpr(pos, qTypeSpec, qTypeSpec, initExpr));
            return typecastExpr;
        }
    }

    return initExpr;
}

inline Ast::Scope& Ast::NodeFactory::addScope(const Ast::Token& pos, const Ast::ScopeType::T& type) {
    Ast::Scope& scope = unit().addNode(new Ast::Scope(pos, type));
    return scope;
}

inline Ast::Scope& Ast::NodeFactory::addScopeWithSig(const Ast::Token& pos, const Ast::ScopeType::T& type, const Ast::FunctionSig& sig) {
    unused(sig);
    Ast::Scope& scope = addScope(pos, type);
//@    for(Ast::Scope::List::const_iterator it = sig.xref().begin(); it != sig.xref().end(); ++it) {
//        const Ast::VariableDefn& vdef = it->get();
//        Ast::VariableDefn& cvdef = unit().addNode(vdef.clone());
//        scope.addVariableDef(cvdef);
//    }
    return scope;
}

inline Ast::ExprList& Ast::NodeFactory::addExprList(const Ast::Token& pos) {
    Ast::ExprList& exprList = unit().addNode(new Ast::ExprList(pos));
    return exprList;
}

inline Ast::TemplateDefn& Ast::NodeFactory::createTemplateDefn(const Ast::Token& pos, const z::string& name, const Ast::TemplateTypePartList& list) {
    Ast::Token token(pos.filename(), pos.row(), pos.col(), name);
    const Ast::TemplateDecl& templateDecl = unit().getRootTypeSpec<Ast::TemplateDecl>(_module.level(), token);
    Ast::TemplateDefn& templateDefn = unit().addNode(new Ast::TemplateDefn(unit().anonymousNS(), token, Ast::DefinitionType::Final, templateDecl, list));
    unit().addAnonymous(templateDefn);
    return templateDefn;
}

inline const Ast::FunctionRetn& Ast::NodeFactory::getFunctionRetn(const Ast::Token& pos, const Ast::Function& function) {
    /// \todo use FunctionBaseIterator here
    const Ast::Function* base = z::ptr(function);
    while(base != 0) {
        const Ast::ChildFunctionDefn* childFunctionDefn = dynamic_cast<const Ast::ChildFunctionDefn*>(base);
        if(childFunctionDefn == 0)
            break;
        base = z::ptr(z::ref(childFunctionDefn).base());
    }
    if(base != 0) {
        const Ast::FunctionRetn* functionRetn = z::ref(base).hasChild<const Ast::FunctionRetn>("_Out");
        if(functionRetn != 0) {
            return z::ref(functionRetn);
        }
    }

    throw z::Exception("NodeFactory", zfmt(pos, "Unknown function return type %{s}").add("s", function.name() ));
}

inline const Ast::QualifiedTypeSpec& Ast::NodeFactory::getFunctionReturnType(const Ast::Token& pos, const Ast::Function& function) {
    if(function.sig().outScope().isTuple()) {
        const Ast::FunctionRetn& functionRetn = getFunctionRetn(pos, function);
        Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, functionRetn, false);
        return qTypeSpec;
    }
    return function.sig().out().front().qTypeSpec();
}

inline Ast::VariableDefn& Ast::NodeFactory::addVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name) {
    const Ast::Expr& initExpr = getDefaultValue(qualifiedTypeSpec.typeSpec(), name);
    Ast::VariableDefn& variableDef = unit().addNode(new Ast::VariableDefn(qualifiedTypeSpec, name, initExpr));
    return variableDef;
}

inline const Ast::TemplateDefn& Ast::NodeFactory::getTemplateDefn(const Ast::Token& name, const Ast::Expr& expr, const z::string& cname, const Ast::TemplateDefn::size_type& len) {
    const Ast::TypeSpec& typeSpec = expr.qTypeSpec().typeSpec();
    if(typeSpec.name().string() != cname) {
        throw z::Exception("NodeFactory", zfmt(name, "Expression is not of %{c} type: %{t} (1)").add("c", cname).add("t", typeSpec.name() ) );
    }
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(typeSpec));
    if(templateDefn == 0) {
        throw z::Exception("NodeFactory", zfmt(name, "Expression is not of %{c} type: %{t} (2)").add("c", cname).add("t", typeSpec.name() ) );
    }
    if(z::ref(templateDefn).list().size() != len) {
        throw z::Exception("NodeFactory", zfmt(name, "Expression is not of %{c} type: %{t} (3)").add("c", cname).add("t", typeSpec.name() ) );
    }
    return z::ref(templateDefn);
}

inline Ast::RootFunctionDecl& Ast::NodeFactory::addRootFunctionDecl(const Ast::TypeSpec& parent, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType, const Ast::ClosureRef& cref) {
    const Ast::Token& name = functionSig.name();
    Ast::RootFunctionDecl& functionDecl = unit().addNode(new Ast::RootFunctionDecl(parent, name, defType, functionSig, z::ref(cref.xref), z::ref(cref.iref)));
    Ast::Token token1(name.filename(), name.row(), name.col(), "_Out");
    Ast::FunctionRetn& functionRetn = unit().addNode(new Ast::FunctionRetn(functionDecl, token1, functionSig.outScope()));
    functionDecl.addChild(functionRetn);
    return functionDecl;
}

inline Ast::ChildFunctionDecl& Ast::NodeFactory::addChildFunctionDecl(const Ast::TypeSpec& parent, const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::TypeSpec& base, const Ast::ClosureRef& cref) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(z::ptr(base));
    if(function == 0) {
        throw z::Exception("NodeFactory", zfmt(name, "Base type is not a function '%{s}'").add("s", base.name() ));
    }

    Ast::ChildFunctionDecl& functionDecl = unit().addNode(new Ast::ChildFunctionDecl(parent, name, defType, z::ref(function).sig(), z::ref(cref.xref), z::ref(cref.iref), z::ref(function)));
    Ast::Token token1(name.filename(), name.row(), name.col(), "_Out");
    Ast::FunctionRetn& functionRetn = unit().addNode(new Ast::FunctionRetn(functionDecl, token1, z::ref(function).sig().outScope()));
    functionDecl.addChild(functionRetn);
    return functionDecl;
}

inline Ast::ValueInstanceExpr& Ast::NodeFactory::getValueInstanceExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec, const Ast::TemplateDefn& srcTemplateDefn, const Ast::TemplateDefn& templateDefn, const Ast::ExprList& exprList) {
    const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, qTypeSpec.isConst(), qTypeSpec.typeSpec(), true);
    Ast::ValueInstanceExpr& valueInstanceExpr = unit().addNode(new Ast::ValueInstanceExpr(pos, typeSpec, srcTemplateDefn, templateDefn, exprList));
    return valueInstanceExpr;
}

////////////////////////////////////////////////////////////
Ast::NodeFactory::NodeFactory(Ast::Module& module, Compiler& compiler)
    : _module(module), _compiler(compiler), _lastToken(_module.filename(), 0, 0, "") {
    Ast::Root& rootTypeSpec = unit().getRootNamespace(_module.level());
    unit().enterTypeSpec(rootTypeSpec);
}

Ast::NodeFactory::~NodeFactory() {
    Ast::Root& rootTypeSpec = unit().getRootNamespace(_module.level());
    unit().leaveTypeSpec(rootTypeSpec);
}

////////////////////////////////////////////////////////////
void Ast::NodeFactory::aUnitStatementList(const Ast::EnterNamespaceStatement& nss) {
    Ast::LeaveNamespaceStatement& lns = unit().addNode(new Ast::LeaveNamespaceStatement(getToken(), nss));
//@    if(_module.level() == 0) {
        _module.addGlobalStatement(lns);
//    }
    unit().leaveNamespace();
}

void Ast::NodeFactory::aImportStatement(const Ast::Token& pos, const Ast::AccessType::T& accessType, const Ast::HeaderType::T& headerType, const Ast::DefinitionType::T& defType, Ast::NamespaceList& list) {
    Ast::ImportStatement& statement = unit().addNode(new Ast::ImportStatement(pos, accessType, headerType, defType, list));
    _module.addGlobalStatement(statement);

    if(statement.defType() != Ast::DefinitionType::Native) {
        z::string filename;
        z::string sep = "";
        for(Ast::NamespaceList::List::const_iterator it = statement.list().begin(); it != statement.list().end(); ++it) {
            const Ast::Token& name = it->get().name();
            filename += sep;
            filename += name.string();
            sep = "/";
        }
        filename += ".ipp";
        Ast::Module module(unit(), filename, _module.level() + 1);
        _compiler.import(module);
    }
}

Ast::NamespaceList* Ast::NodeFactory::aImportNamespaceList(Ast::NamespaceList& list, const Ast::Token &name) {
    Ast::Namespace& ns = unit().addNode(new Ast::Namespace(unit().currentTypeSpec(), name));
    list.addNamespace(ns);
    return z::ptr(list);
}

Ast::NamespaceList* Ast::NodeFactory::aImportNamespaceList(const Ast::Token& name) {
    Ast::NamespaceList& list = unit().addNode(new Ast::NamespaceList(name));
    return aImportNamespaceList(list, name);
}

Ast::EnterNamespaceStatement* Ast::NodeFactory::aNamespaceStatement(const Ast::Token& pos, Ast::NamespaceList& list) {
    Ast::EnterNamespaceStatement& statement = unit().addNode(new Ast::EnterNamespaceStatement(pos, list));
//@    if(_module.level() == 0) {
        _module.addGlobalStatement(statement);
//    }
    return z::ptr(statement);
}

Ast::EnterNamespaceStatement* Ast::NodeFactory::aNamespaceStatement() {
    Ast::NamespaceList& list = unit().addNode(new Ast::NamespaceList(getToken()));
    return aNamespaceStatement(getToken(), list);
}

inline Ast::Namespace& Ast::NodeFactory::getUnitNamespace(const Ast::Token& name) {
    if(_module.level() == 0) {
        Ast::Namespace& ns = unit().addNode(new Ast::Namespace(unit().currentTypeSpec(), name));
        unit().currentTypeSpec().addChild(ns);
        return ns;
    }

    Ast::Namespace* cns = unit().importNS().hasChild<Ast::Namespace>(name.string());
    if(cns) {
        return z::ref(cns);
    }

    Ast::Namespace& ns = unit().addNode(new Ast::Namespace(unit().currentTypeSpec(), name));
    unit().currentTypeSpec().addChild(ns);
    return ns;
}

Ast::NamespaceList* Ast::NodeFactory::aUnitNamespaceList(Ast::NamespaceList& list, const Ast::Token& name) {
    Ast::Namespace& ns = getUnitNamespace(name);
    unit().enterTypeSpec(ns);
    if(_module.level() == 0) {
        unit().addNamespacePart(name);
    }
    unit().addNamespace(ns);
    list.addNamespace(ns);
    return z::ptr(list);
}

Ast::NamespaceList* Ast::NodeFactory::aUnitNamespaceList(const Ast::Token& name) {
    Ast::NamespaceList& list = unit().addNode(new Ast::NamespaceList(name));
    return aUnitNamespaceList(list, name);
}

Ast::Statement* Ast::NodeFactory::aGlobalStatement(Ast::Statement& statement) {
//@    if(_module.level() == 0) {
        _module.addGlobalStatement(statement);
//    }
    return z::ptr(statement);
}

Ast::Statement* Ast::NodeFactory::aGlobalTypeSpecStatement(const Ast::AccessType::T& accessType, Ast::UserDefinedTypeSpec& typeSpec){
    typeSpec.accessType(accessType);
    Ast::UserDefinedTypeSpecStatement* statement = aUserDefinedTypeSpecStatement(typeSpec);
    return aGlobalStatement(z::ref(statement));
}

void Ast::NodeFactory::aGlobalCoerceStatement(Ast::CoerceList& list) {
    unit().addCoercionList(list);
}

Ast::CoerceList* Ast::NodeFactory::aCoerceList(Ast::CoerceList& list, const Ast::TypeSpec& typeSpec) {
    list.addTypeSpec(typeSpec);
    return z::ptr(list);
}

Ast::CoerceList* Ast::NodeFactory::aCoerceList(const Ast::TypeSpec& typeSpec) {
    Ast::CoerceList& list = unit().addNode(new Ast::CoerceList(typeSpec.pos()));
    list.addTypeSpec(typeSpec);
    return z::ptr(list);
}

void Ast::NodeFactory::aGlobalDefaultStatement(const Ast::TypeSpec& typeSpec, const Ast::Expr& expr) {
//    trace("addDef: %s %lu unit(%lu)\n", ZenlangNameGenerator().tn(typeSpec, GenMode::Import).c_str(), z::pad(typeSpec), z::pad(_unit));
    unit().addDefaultValue(typeSpec, expr);
}

inline void Ast::NodeFactory::setDefaultDummyValue(const Ast::Token& name, Ast::TypeSpec& typeSpec) {
    Ast::CoerceList& list = unit().addNode(new Ast::CoerceList(name));
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(name, "void");
    list.addTypeSpec(qTypeSpec.typeSpec());
    list.addTypeSpec(typeSpec);
    unit().addCoercionList(list);

    Ast::ConstantIntExpr& expr = unit().addNode(new Ast::ConstantIntExpr(name, qTypeSpec, 0));
    unit().addDefaultValue(typeSpec, expr);
}

Ast::TypedefDecl* Ast::NodeFactory::aTypedefDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::TypedefDecl& typedefDefn = unit().addNode(new Ast::TypedefDecl(unit().currentTypeSpec(), name, defType));
    unit().currentTypeSpec().addChild(typedefDefn);
    // if this is a native-typedef-decl, add dummy coercion and default-value = (void)0
    // unless this is the native-typedef-decl for void itself
    if((defType == Ast::DefinitionType::Native) && (name.string() != "void")) {
        setDefaultDummyValue(name, typedefDefn);
    }
    return z::ptr(typedefDefn);
}

Ast::TypedefDefn* Ast::NodeFactory::aTypedefDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::QualifiedTypeSpec& qTypeSpec) {
    Ast::TypedefDefn& typedefDefn = unit().addNode(new Ast::TypedefDefn(unit().currentTypeSpec(), name, defType, qTypeSpec));
    unit().currentTypeSpec().addChild(typedefDefn);
    return z::ptr(typedefDefn);
}

Ast::TemplatePartList* Ast::NodeFactory::aTemplatePartList(Ast::TemplatePartList& list, const Ast::Token& name) {
    list.addPart(name);
    return z::ptr(list);
}

Ast::TemplatePartList* Ast::NodeFactory::aTemplatePartList(const Ast::Token& name) {
    Ast::TemplatePartList& list = unit().addNode(new Ast::TemplatePartList(name));
    list.addPart(name);
    return z::ptr(list);
}

Ast::TemplateDecl* Ast::NodeFactory::aTemplateDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::TemplatePartList& list) {
    Ast::TemplateDecl& templateDefn = unit().addNode(new Ast::TemplateDecl(unit().currentTypeSpec(), name, defType, list));
    unit().currentTypeSpec().addChild(templateDefn);
    return z::ptr(templateDefn);
}

Ast::EnumDefn* Ast::NodeFactory::aEnumDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list) {
    Ast::EnumDefn& enumDefn = unit().addNode(new Ast::EnumDefn(unit().currentTypeSpec(), name, defType, list));
    unit().currentTypeSpec().addChild(enumDefn);
    return z::ptr(enumDefn);
}

Ast::EnumDecl* Ast::NodeFactory::aEnumDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::EnumDecl& enumDecl = unit().addNode(new Ast::EnumDecl(unit().currentTypeSpec(), name, defType));
    unit().currentTypeSpec().addChild(enumDecl);
    return z::ptr(enumDecl);
}

Ast::Scope* Ast::NodeFactory::aEnumMemberDefnList(Ast::Scope& list, const Ast::VariableDefn& variableDefn) {
    list.addVariableDef(variableDefn);
    return z::ptr(list);
}

Ast::Scope* Ast::NodeFactory::aEnumMemberDefnListEmpty(const Ast::Token& pos) {
    Ast::Scope& list = addScope(pos, Ast::ScopeType::Member);
    return z::ptr(list);
}

Ast::VariableDefn* Ast::NodeFactory::aEnumMemberDefn(const Ast::Token& name) {
    Ast::Token value(name.filename(), name.row(), name.col(), "#");
    const Ast::ConstantIntExpr& initExpr = aConstantIntExpr(value);
    Ast::VariableDefn& variableDefn = unit().addNode(new Ast::VariableDefn(initExpr.qTypeSpec(), name, initExpr));
    return z::ptr(variableDefn);
}

Ast::VariableDefn* Ast::NodeFactory::aEnumMemberDefn(const Ast::Token& name, const Ast::Expr& initExpr) {
    Ast::VariableDefn& variableDefn = unit().addNode(new Ast::VariableDefn(initExpr.qTypeSpec(), name, initExpr));
    return z::ptr(variableDefn);
}

Ast::StructDecl* Ast::NodeFactory::aStructDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::StructDecl& structDecl = unit().addNode(new Ast::StructDecl(unit().currentTypeSpec(), name, defType));
    unit().currentTypeSpec().addChild(structDecl);

    // if this is a native struct decl, add int=>struct-decl coercion and default-value = 0
    if(defType == Ast::DefinitionType::Native) {
        setDefaultDummyValue(name, structDecl);
    }
    return z::ptr(structDecl);
}

Ast::RootStructDefn* Ast::NodeFactory::aLeaveRootStructDefn(Ast::RootStructDefn& structDefn) {
    unit().leaveTypeSpec(structDefn);
    Ast::StructInitStatement& statement = unit().addNode(new Ast::StructInitStatement(structDefn.pos(), structDefn));
    structDefn.block().addStatement(statement);
    return z::ptr(structDefn);
}

Ast::ChildStructDefn* Ast::NodeFactory::aLeaveChildStructDefn(Ast::ChildStructDefn& structDefn) {
    unit().leaveTypeSpec(structDefn);
    Ast::StructInitStatement& statement = unit().addNode(new Ast::StructInitStatement(structDefn.pos(), structDefn));
    structDefn.block().addStatement(statement);
    return z::ptr(structDefn);
}

Ast::RootStructDefn* Ast::NodeFactory::aEnterRootStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& list = addScope(name, Ast::ScopeType::Member);
    Ast::CompoundStatement& block = unit().addNode(new Ast::CompoundStatement(name));
    Ast::RootStructDefn& structDefn = unit().addNode(new Ast::RootStructDefn(unit().currentTypeSpec(), name, defType, list, block));
    unit().currentTypeSpec().addChild(structDefn);
    unit().enterTypeSpec(structDefn);
    return z::ptr(structDefn);
}

Ast::ChildStructDefn* Ast::NodeFactory::aEnterChildStructDefn(const Ast::Token& name, const Ast::StructDefn& base, const Ast::DefinitionType::T& defType) {
    if(base.defType() == Ast::DefinitionType::Final) {
        throw z::Exception("NodeFactory", zfmt(name, "Base struct is not abstract '%{s}'").add("s", base.name() ));
    }
    Ast::Scope& list = addScope(name, Ast::ScopeType::Member);
    Ast::CompoundStatement& block = unit().addNode(new Ast::CompoundStatement(name));
    Ast::ChildStructDefn& structDefn = unit().addNode(new Ast::ChildStructDefn(unit().currentTypeSpec(), base, name, defType, list, block));
    unit().currentTypeSpec().addChild(structDefn);
    unit().enterTypeSpec(structDefn);
    return z::ptr(structDefn);
}

void Ast::NodeFactory::aStructMemberVariableDefn(const Ast::VariableDefn& vdef) {
    Ast::StructDefn& sd = unit().getCurrentStructDefn(vdef.name());
    sd.addVariable(vdef);
    Ast::StructMemberVariableStatement& statement = unit().addNode(new Ast::StructMemberVariableStatement(vdef.pos(), sd, vdef));
    sd.block().addStatement(statement);
}

void Ast::NodeFactory::aStructMemberTypeDefn(Ast::UserDefinedTypeSpec& typeSpec) {
    Ast::StructDefn& sd = unit().getCurrentStructDefn(typeSpec.name());
    typeSpec.accessType(Ast::AccessType::Parent);
    Ast::Statement* statement = aUserDefinedTypeSpecStatement(typeSpec);
    sd.block().addStatement(z::ref(statement));
}

void Ast::NodeFactory::aStructMemberPropertyDefn(Ast::PropertyDecl& typeSpec) {
    aStructMemberTypeDefn(typeSpec);
    Ast::StructDefn& sd = unit().getCurrentStructDefn(typeSpec.name());
    sd.addProperty(typeSpec);
}

Ast::PropertyDeclRW* Ast::NodeFactory::aStructPropertyDeclRW(const Ast::Token& pos, const Ast::QualifiedTypeSpec& propertyType, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    unused(pos);
    Ast::PropertyDeclRW& structPropertyDecl = unit().addNode(new Ast::PropertyDeclRW(unit().currentTypeSpec(), name, defType, propertyType));
    unit().currentTypeSpec().addChild(structPropertyDecl);
    return z::ptr(structPropertyDecl);
}

Ast::PropertyDeclRO* Ast::NodeFactory::aStructPropertyDeclRO(const Ast::Token& pos, const Ast::QualifiedTypeSpec& propertyType, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    unused(pos);
    Ast::PropertyDeclRO& structPropertyDecl = unit().addNode(new Ast::PropertyDeclRO(unit().currentTypeSpec(), name, defType, propertyType));
    unit().currentTypeSpec().addChild(structPropertyDecl);
    return z::ptr(structPropertyDecl);
}

Ast::RoutineDecl* Ast::NodeFactory::aRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const Ast::DefinitionType::T& defType) {
    Ast::RoutineDecl& routineDecl = unit().addNode(new Ast::RoutineDecl(unit().currentTypeSpec(), outType, name, in, defType));
    unit().currentTypeSpec().addChild(routineDecl);
    return z::ptr(routineDecl);
}

Ast::RoutineDecl* Ast::NodeFactory::aVarArgRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& in = addScope(name, Ast::ScopeType::VarArg);
    Ast::RoutineDecl& routineDecl = unit().addNode(new Ast::RoutineDecl(unit().currentTypeSpec(), outType, name, in, defType));
    unit().currentTypeSpec().addChild(routineDecl);
    return z::ptr(routineDecl);
}

Ast::RoutineDefn* Ast::NodeFactory::aRoutineDefn(Ast::RoutineDefn& routineDefn, const Ast::CompoundStatement& block) {
    routineDefn.setBlock(block);
    unit().leaveScope(routineDefn.inScope());
    unit().leaveTypeSpec(routineDefn);
    unit().addBody(unit().addNode(new Ast::RoutineBody(block.pos(), routineDefn, block)));
    return z::ptr(routineDefn);
}

Ast::RoutineDefn* Ast::NodeFactory::aEnterRoutineDefn(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const Ast::DefinitionType::T& defType) {
    Ast::RoutineDefn& routineDefn = unit().addNode(new Ast::RoutineDefn(unit().currentTypeSpec(), outType, name, in, defType));
    unit().currentTypeSpec().addChild(routineDefn);
    unit().enterScope(in);
    unit().enterTypeSpec(routineDefn);
    return z::ptr(routineDefn);
}

Ast::RootFunctionDecl* Ast::NodeFactory::aRootFunctionDecl(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType, const Ast::ClosureRef& cref) {
    Ast::RootFunctionDecl& functionDecl = addRootFunctionDecl(unit().currentTypeSpec(), functionSig, defType, cref);
    unit().currentTypeSpec().addChild(functionDecl);
    return z::ptr(functionDecl);
}

Ast::ChildFunctionDecl* Ast::NodeFactory::aChildFunctionDecl(const Ast::TypeSpec& base, const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::ClosureRef& cref) {
    Ast::ChildFunctionDecl& functionDecl = addChildFunctionDecl(unit().currentTypeSpec(), name, defType, base, cref);
    unit().currentTypeSpec().addChild(functionDecl);
    return z::ptr(functionDecl);
}

Ast::RootFunctionDefn* Ast::NodeFactory::aRootFunctionDefn(Ast::RootFunctionDefn& functionDefn, const Ast::CompoundStatement& block) {
    functionDefn.setBlock(block);
    unit().leaveScope(functionDefn.sig().inScope());
    unit().leaveScope(functionDefn.xrefScope());
    unit().leaveTypeSpec(functionDefn);
    unit().addBody(unit().addNode(new Ast::FunctionBody(block.pos(), functionDefn, block)));
    return z::ptr(functionDefn);
}

Ast::RootFunctionDefn* Ast::NodeFactory::aEnterRootFunctionDefn(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType, const Ast::ClosureRef& cref) {
    const Ast::Token& name = functionSig.name();
    Ast::RootFunctionDefn& functionDefn = unit().addNode(new Ast::RootFunctionDefn(unit().currentTypeSpec(), name, defType, functionSig, z::ref(cref.xref), z::ref(cref.iref)));
    unit().currentTypeSpec().addChild(functionDefn);
    unit().enterScope(functionDefn.xrefScope());
    unit().enterScope(functionSig.inScope());
    unit().enterTypeSpec(functionDefn);

    Ast::Token token1(name.filename(), name.row(), name.col(), "_Out");
    Ast::FunctionRetn& functionRetn = unit().addNode(new Ast::FunctionRetn(functionDefn, token1, functionSig.outScope()));
    functionDefn.addChild(functionRetn);

    return z::ptr(functionDefn);
}

Ast::ChildFunctionDefn* Ast::NodeFactory::aChildFunctionDefn(Ast::ChildFunctionDefn& functionDefn, const Ast::CompoundStatement& block) {
    functionDefn.setBlock(block);
    unit().leaveScope(functionDefn.sig().inScope());
    unit().leaveScope(functionDefn.xrefScope());
    unit().leaveTypeSpec(functionDefn);
    unit().addBody(unit().addNode(new Ast::FunctionBody(block.pos(), functionDefn, block)));
    return z::ptr(functionDefn);
}

inline Ast::ChildFunctionDefn& Ast::NodeFactory::createChildFunctionDefn(Ast::TypeSpec& parent, const Ast::Function& base, const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::ClosureRef& cref) {
    if(base.defType() == Ast::DefinitionType::Final) {
        throw z::Exception("NodeFactory", zfmt(name, "Base struct is not abstract '%{s}'").add("s", base.name() ));
    }

    Ast::ChildFunctionDefn& functionDefn = unit().addNode(new Ast::ChildFunctionDefn(parent, name, defType, base.sig(), z::ref(cref.xref), z::ref(cref.iref), base));
    parent.addChild(functionDefn);
    unit().enterScope(functionDefn.xrefScope());
    unit().enterScope(base.sig().inScope());
    unit().enterTypeSpec(functionDefn);
    return functionDefn;
}

Ast::ChildFunctionDefn* Ast::NodeFactory::aEnterChildFunctionDefn(const Ast::TypeSpec& base, const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::ClosureRef& cref) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(z::ptr(base));
    if(function == 0) {
        throw z::Exception("NodeFactory", zfmt(name, "Base type is not a function '%{s}'").add("s", base.name() ));
    }
    Ast::ChildFunctionDefn& functionDefn = createChildFunctionDefn(unit().currentTypeSpec(), z::ref(function), name, defType, cref);
    return z::ptr(functionDefn);
}

Ast::EventDecl* Ast::NodeFactory::aEventDecl(const Ast::Token& pos, const Ast::VariableDefn& in, const Ast::DefinitionType::T& eventDefType, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& handlerDefType) {
    const Ast::Token& name = functionSig.name();

    Ast::Token eventName(pos.filename(), pos.row(), pos.col(), name.string());
    Ast::EventDecl& eventDef = unit().addNode(new Ast::EventDecl(unit().currentTypeSpec(), eventName, functionSig, in, eventDefType));
    unit().currentTypeSpec().addChild(eventDef);

    Ast::Token handlerName(pos.filename(), pos.row(), pos.col(), "Handler");
    Ast::FunctionSig* handlerSig = aFunctionSig(functionSig.outScope(), handlerName, functionSig.inScope());
    Ast::Scope& xref = addScope(getToken(), Ast::ScopeType::XRef);
    Ast::Scope& iref = addScope(getToken(), Ast::ScopeType::XRef);
    Ast::ClosureRef cref = aClosureList(xref, iref);
    Ast::RootFunctionDecl& funDecl = addRootFunctionDecl(eventDef, z::ref(handlerSig), handlerDefType, cref);
    eventDef.setHandler(funDecl);

    Ast::TemplateTypePartList& list = unit().addNode(new Ast::TemplateTypePartList(pos));
    Ast::QualifiedTypeSpec& qFunTypeSpec = addQualifiedTypeSpec(pos, false, funDecl, false);
    list.addType(qFunTypeSpec);
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "pointer", list);
    const Ast::QualifiedTypeSpec& qFunctorTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);

    Ast::Token hVarName(pos.filename(), pos.row(), pos.col(), "handler");
    Ast::VariableDefn& vdef = addVariableDefn(qFunctorTypeSpec, hVarName);

    Ast::Scope& outAdd = addScope(pos, Ast::ScopeType::Param);
    Ast::Token oname(name.filename(), name.row(), name.col(), "_out");
    const Ast::QualifiedTypeSpec& voidTypeSpec = getQualifiedTypeSpec(pos, "void");
    Ast::VariableDefn& voidVdef = addVariableDefn(voidTypeSpec, oname);
    outAdd.addVariableDef(voidVdef);
    outAdd.isTuple(false);

    Ast::Scope& inAdd  = addScope(pos, Ast::ScopeType::Param);
    Ast::Token nameAdd(pos.filename(), pos.row(), pos.col(), "Add");
    Ast::FunctionSig* addSig = aFunctionSig(outAdd, nameAdd, inAdd);
    Ast::RootFunctionDecl& addDecl = addRootFunctionDecl(eventDef, z::ref(addSig), eventDefType, cref);
    eventDef.setAddFunction(addDecl);

    inAdd.addVariableDef(in);
    inAdd.addVariableDef(vdef);

    return z::ptr(eventDef);
}

Ast::FunctionSig* Ast::NodeFactory::aFunctionSig(const Ast::Scope& out, const Ast::Token& name, Ast::Scope& in) {
    Ast::FunctionSig& functionSig = unit().addNode(new Ast::FunctionSig(out, name, in));
    return z::ptr(functionSig);
}

Ast::FunctionSig* Ast::NodeFactory::aFunctionSig(const Ast::QualifiedTypeSpec& typeSpec, const Ast::Token& name, Ast::Scope& in) {
    Ast::Scope& out = addScope(name, Ast::ScopeType::Param);
    out.isTuple(false);

    Ast::Token oname(name.filename(), name.row(), name.col(), "_out");
    Ast::VariableDefn& vdef = addVariableDefn(typeSpec, oname);
    out.addVariableDef(vdef);

    return aFunctionSig(out, name, in);
}

Ast::ClosureRef Ast::NodeFactory::aClosureList(Ast::Scope& xref, Ast::Scope& iref) {
    Ast::ClosureRef cref;
    cref.xref = z::ptr(xref);
    cref.iref = z::ptr(iref);
    return cref;
}

Ast::ClosureRef Ast::NodeFactory::aClosureList(Ast::Scope& xref) {
    Ast::Scope& iref = addScope(getToken(), Ast::ScopeType::XRef);
    return aClosureList(xref, iref);
}

Ast::ClosureRef Ast::NodeFactory::aClosureList() {
    Ast::Scope& xref = addScope(getToken(), Ast::ScopeType::XRef);
    Ast::Scope& iref = addScope(getToken(), Ast::ScopeType::XRef);
    return aClosureList(xref, iref);
}

Ast::Scope* Ast::NodeFactory::aInParamsList(Ast::Scope& scope) {
    return z::ptr(scope);
}

Ast::Scope* Ast::NodeFactory::aParamsList(Ast::Scope& scope) {
    return z::ptr(scope);
}

Ast::Scope* Ast::NodeFactory::aParamsList(Ast::Scope& scope, const Ast::Scope& posParam) {
    scope.posParam(posParam);
    return aParamsList(scope);
}

Ast::Scope* Ast::NodeFactory::aParam(Ast::Scope& list, const Ast::VariableDefn& variableDefn) {
    list.addVariableDef(variableDefn);
    return z::ptr(list);
}

Ast::Scope* Ast::NodeFactory::aParam(const Ast::VariableDefn& variableDefn) {
    Ast::Scope& list = addScope(variableDefn.pos(), Ast::ScopeType::Param);
    return aParam(list, variableDefn);
}

Ast::Scope* Ast::NodeFactory::aParam(const Ast::ScopeType::T& type) {
    Ast::Scope& list = addScope(getToken(), type);
    return z::ptr(list);
}

Ast::VariableDefn* Ast::NodeFactory::aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name, const Ast::Expr& initExpr) {
    const Ast::Expr& expr = convertExprToExpectedTypeSpec(name, initExpr);
    Ast::VariableDefn& variableDef = unit().addNode(new Ast::VariableDefn(qualifiedTypeSpec, name, expr));
    unit().popExpectedTypeSpecOrAuto(name, Unit::ExpectedTypeSpec::etAssignment);
    return z::ptr(variableDef);
}

Ast::VariableDefn* Ast::NodeFactory::aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name) {
    const Ast::Expr& initExpr = getDefaultValue(qualifiedTypeSpec.typeSpec(), name);
    return aVariableDefn(qualifiedTypeSpec, name, initExpr);
}

Ast::VariableDefn* Ast::NodeFactory::aVariableDefn(const Ast::Token& name, const Ast::Expr& initExpr) {
    const Ast::QualifiedTypeSpec& qualifiedTypeSpec = initExpr.qTypeSpec();
    return aVariableDefn(qualifiedTypeSpec, name, initExpr);
}

const Ast::QualifiedTypeSpec* Ast::NodeFactory::aQualifiedVariableDefn(const Ast::QualifiedTypeSpec& qTypeSpec) {
    unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etAssignment, qTypeSpec);
    return z::ptr(qTypeSpec);
}

void Ast::NodeFactory::aAutoQualifiedVariableDefn() {
    unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etAuto);
}

Ast::QualifiedTypeSpec* Ast::NodeFactory::aQualifiedTypeSpec(const Ast::Token& pos, const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef) {
    Ast::QualifiedTypeSpec& qualifiedTypeSpec = addQualifiedTypeSpec(pos, isConst, typeSpec, isRef);
    return z::ptr(qualifiedTypeSpec);
}

Ast::QualifiedTypeSpec* Ast::NodeFactory::aQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef) {
    return aQualifiedTypeSpec(getToken(), isConst, typeSpec, isRef);
}

const Ast::TemplateDecl* Ast::NodeFactory::aTemplateTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return unit().setCurrentChildTypeRef<Ast::TemplateDecl>(parent, name, "template");
}

const Ast::TemplateDecl* Ast::NodeFactory::aTemplateTypeSpec(const Ast::Token& name) {
    return unit().setCurrentRootTypeRef<Ast::TemplateDecl>(_module.level(), name);
}

const Ast::TemplateDecl* Ast::NodeFactory::aTemplateTypeSpec(const Ast::TemplateDecl& templateDecl) {
    return unit().resetCurrentTypeRef<Ast::TemplateDecl>(templateDecl);
}

const Ast::StructDefn* Ast::NodeFactory::aStructTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return unit().setCurrentChildTypeRef<Ast::StructDefn>(parent, name, "struct");
}

const Ast::StructDefn* Ast::NodeFactory::aStructTypeSpec(const Ast::Token& name) {
    return unit().setCurrentRootTypeRef<Ast::StructDefn>(_module.level(), name);
}

const Ast::StructDefn* Ast::NodeFactory::aStructTypeSpec(const Ast::StructDefn& structDefn) {
    return unit().resetCurrentTypeRef<Ast::StructDefn>(structDefn);
}

const Ast::Routine* Ast::NodeFactory::aRoutineTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return unit().setCurrentChildTypeRef<Ast::Routine>(parent, name, "routine");
}

const Ast::Routine* Ast::NodeFactory::aRoutineTypeSpec(const Ast::Token& name) {
    return unit().setCurrentRootTypeRef<Ast::Routine>(_module.level(), name);
}

const Ast::Routine* Ast::NodeFactory::aRoutineTypeSpec(const Ast::Routine& routine) {
    return unit().resetCurrentTypeRef<Ast::Routine>(routine);
}

const Ast::Function* Ast::NodeFactory::aFunctionTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return unit().setCurrentChildTypeRef<Ast::Function>(parent, name, "function");
}

const Ast::Function* Ast::NodeFactory::aFunctionTypeSpec(const Ast::Token& name) {
    return unit().setCurrentRootTypeRef<Ast::Function>(_module.level(), name);
}

const Ast::Function* Ast::NodeFactory::aFunctionTypeSpec(const Ast::Function& function) {
    return unit().resetCurrentTypeRef<Ast::Function>(function);
}

const Ast::EventDecl* Ast::NodeFactory::aEventTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return unit().setCurrentChildTypeRef<Ast::EventDecl>(parent, name, "event");
}

const Ast::EventDecl* Ast::NodeFactory::aEventTypeSpec(const Ast::Token& name) {
    return unit().setCurrentRootTypeRef<Ast::EventDecl>(_module.level(), name);
}

const Ast::EventDecl* Ast::NodeFactory::aEventTypeSpec(const Ast::EventDecl& event) {
    return unit().resetCurrentTypeRef<Ast::EventDecl>(event);
}

const Ast::TypeSpec* Ast::NodeFactory::aOtherTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return unit().setCurrentChildTypeRef<Ast::TypeSpec>(parent, name, "parent");
}

const Ast::TypeSpec* Ast::NodeFactory::aOtherTypeSpec(const Ast::Token& name) {
    return unit().setCurrentRootTypeRef<Ast::TypeSpec>(_module.level(), name);
}

const Ast::TypeSpec* Ast::NodeFactory::aTypeSpec(const Ast::TypeSpec& TypeSpec) {
    return unit().resetCurrentTypeRef<Ast::TypeSpec>(TypeSpec);
}

const Ast::TemplateDefn* Ast::NodeFactory::aTemplateDefnTypeSpec(const Ast::TemplateDecl& typeSpec, const Ast::TemplateTypePartList& list) {
    Ast::TemplateDefn& templateDefn = unit().addNode(new Ast::TemplateDefn(unit().anonymousNS(), typeSpec.name(), Ast::DefinitionType::Final, typeSpec, list));
    unit().addAnonymous(templateDefn);
    return z::ptr(templateDefn);
}

Ast::TemplateTypePartList* Ast::NodeFactory::aTemplateTypePartList(Ast::TemplateTypePartList& list, const Ast::QualifiedTypeSpec& qTypeSpec) {
    list.addType(qTypeSpec);
    return z::ptr(list);
}

Ast::TemplateTypePartList* Ast::NodeFactory::aTemplateTypePartList(const Ast::QualifiedTypeSpec& qTypeSpec) {
    Ast::TemplateTypePartList& list = unit().addNode(new Ast::TemplateTypePartList(getToken()));
    return aTemplateTypePartList(list, qTypeSpec);
}

Ast::UserDefinedTypeSpecStatement* Ast::NodeFactory::aUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec) {
    Ast::UserDefinedTypeSpecStatement& userDefinedTypeSpecStatement = unit().addNode(new Ast::UserDefinedTypeSpecStatement(typeSpec.pos(), typeSpec));
    return z::ptr(userDefinedTypeSpecStatement);
}

Ast::EmptyStatement* Ast::NodeFactory::aEmptyStatement(const Ast::Token& pos) {
    Ast::EmptyStatement& emptyStatement = unit().addNode(new Ast::EmptyStatement(pos));
    return z::ptr(emptyStatement);
}

Ast::AutoStatement* Ast::NodeFactory::aAutoStatement(const Ast::VariableDefn& defn) {
    Ast::AutoStatement& defnStatement = unit().addNode(new Ast::AutoStatement(defn.pos(), defn));
    unit().currentScope().addVariableDef(defn);
    return z::ptr(defnStatement);
}

Ast::ExprStatement* Ast::NodeFactory::aExprStatement(const Ast::Expr& expr) {
    Ast::ExprStatement& exprStatement = unit().addNode(new Ast::ExprStatement(expr.pos(), expr));
    return z::ptr(exprStatement);
}

Ast::PrintStatement* Ast::NodeFactory::aPrintStatement(const Ast::Token& pos, const Ast::Expr& expr) {
    Ast::PrintStatement& printStatement = unit().addNode(new Ast::PrintStatement(pos, expr));
    return z::ptr(printStatement);
}

Ast::IfStatement* Ast::NodeFactory::aIfStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& tblock) {
    Ast::IfStatement& ifStatement = unit().addNode(new Ast::IfStatement(pos, expr, tblock));
    return z::ptr(ifStatement);
}

Ast::IfElseStatement* Ast::NodeFactory::aIfElseStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& tblock, const Ast::CompoundStatement& fblock) {
    Ast::IfElseStatement& ifElseStatement = unit().addNode(new Ast::IfElseStatement(pos, expr, tblock, fblock));
    return z::ptr(ifElseStatement);
}

Ast::WhileStatement* Ast::NodeFactory::aWhileStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& block) {
    Ast::WhileStatement& whileStatement = unit().addNode(new Ast::WhileStatement(pos, expr, block));
    return z::ptr(whileStatement);
}

Ast::DoWhileStatement* Ast::NodeFactory::aDoWhileStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& block) {
    Ast::DoWhileStatement& doWhileStatement = unit().addNode(new Ast::DoWhileStatement(pos, expr, block));
    return z::ptr(doWhileStatement);
}

Ast::ForStatement* Ast::NodeFactory::aForStatement(const Ast::Token& pos, const Ast::Expr& init, const Ast::Expr& expr, const Ast::Expr& incr, const Ast::CompoundStatement& block) {
    Ast::ForExprStatement& forStatement = unit().addNode(new Ast::ForExprStatement(pos, init, expr, incr, block));
    return z::ptr(forStatement);
}

Ast::ForStatement* Ast::NodeFactory::aForStatement(const Ast::Token& pos, const Ast::VariableDefn& init, const Ast::Expr& expr, const Ast::Expr& incr, const Ast::CompoundStatement& block) {
    Ast::ForInitStatement& forStatement = unit().addNode(new Ast::ForInitStatement(pos, init, expr, incr, block));
    unit().leaveScope();
    return z::ptr(forStatement);
}

const Ast::VariableDefn* Ast::NodeFactory::aEnterForInit(const Ast::VariableDefn& init) {
    Ast::Scope& scope = addScope(init.pos(), Ast::ScopeType::Local);
    scope.addVariableDef(init);
    unit().enterScope(scope);
    return z::ptr(init);
}

Ast::ForeachStatement* Ast::NodeFactory::aForeachStatement(Ast::ForeachStatement& statement, const Ast::CompoundStatement& block) {
    statement.setBlock(block);
    unit().leaveScope();
    return z::ptr(statement);
}

Ast::ForeachStatement* Ast::NodeFactory::aEnterForeachInit(const Ast::Token& valName, const Ast::Expr& expr) {
    if(ZenlangNameGenerator().tn(expr.qTypeSpec().typeSpec()) == "string") {
        const Ast::QualifiedTypeSpec& valTypeSpec = getQualifiedTypeSpec(valName, "char");
        const Ast::VariableDefn& valDef = addVariableDefn(valTypeSpec, valName);
        Ast::Scope& scope = addScope(valName, Ast::ScopeType::Local);
        scope.addVariableDef(valDef);
        unit().enterScope(scope);
        Ast::ForeachStringStatement& foreachStatement = unit().addNode(new Ast::ForeachStringStatement(valName, valDef, expr));
        return z::ptr(foreachStatement);
    }

    const Ast::TemplateDefn& templateDefn = getTemplateDefn(valName, expr, "list", 1);
    const Ast::QualifiedTypeSpec& valTypeSpec = addQualifiedTypeSpec(valName, expr.qTypeSpec().isConst(), templateDefn.at(0).typeSpec(), true);
    const Ast::VariableDefn& valDef = addVariableDefn(valTypeSpec, valName);
    Ast::Scope& scope = addScope(valName, Ast::ScopeType::Local);
    scope.addVariableDef(valDef);
    unit().enterScope(scope);
    Ast::ForeachListStatement& foreachStatement = unit().addNode(new Ast::ForeachListStatement(valName, valDef, expr));
    return z::ptr(foreachStatement);
}

Ast::ForeachDictStatement* Ast::NodeFactory::aEnterForeachInit(const Ast::Token& keyName, const Ast::Token& valName, const Ast::Expr& expr) {
    const Ast::TemplateDefn& templateDefn = getTemplateDefn(valName, expr, "dict", 2);
    const Ast::QualifiedTypeSpec& keyTypeSpec = addQualifiedTypeSpec(keyName, true, templateDefn.at(0).typeSpec(), true);
    const Ast::QualifiedTypeSpec& valTypeSpec = addQualifiedTypeSpec(keyName, expr.qTypeSpec().isConst(), templateDefn.at(1).typeSpec(), true);
    const Ast::VariableDefn& keyDef = addVariableDefn(keyTypeSpec, keyName);
    const Ast::VariableDefn& valDef = addVariableDefn(valTypeSpec, valName);
    Ast::Scope& scope = addScope(keyName, Ast::ScopeType::Local);
    scope.addVariableDef(keyDef);
    scope.addVariableDef(valDef);
    unit().enterScope(scope);

    Ast::ForeachDictStatement& foreachStatement = unit().addNode(new Ast::ForeachDictStatement(keyName, keyDef, valDef, expr));
    return z::ptr(foreachStatement);
}

Ast::SwitchValueStatement* Ast::NodeFactory::aSwitchStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& list) {
    Ast::SwitchValueStatement& switchStatement = unit().addNode(new Ast::SwitchValueStatement(pos, expr, list));
    return z::ptr(switchStatement);
}

Ast::SwitchExprStatement* Ast::NodeFactory::aSwitchStatement(const Ast::Token& pos, const Ast::CompoundStatement& list) {
    Ast::SwitchExprStatement& switchStatement = unit().addNode(new Ast::SwitchExprStatement(pos, list));
    return z::ptr(switchStatement);
}

Ast::CompoundStatement* Ast::NodeFactory::aCaseList(Ast::CompoundStatement& list, const Ast::CaseStatement& stmt) {
    list.addStatement(stmt);
    return z::ptr(list);
}

Ast::CompoundStatement* Ast::NodeFactory::aCaseList(const Ast::CaseStatement& stmt) {
    Ast::CompoundStatement& list = unit().addNode(new Ast::CompoundStatement(stmt.pos()));
    list.addStatement(stmt);
    return z::ptr(list);
}

Ast::CaseStatement* Ast::NodeFactory::aCaseStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& block) {
    Ast::CaseExprStatement& caseStatement = unit().addNode(new Ast::CaseExprStatement(pos, expr, block));
    return z::ptr(caseStatement);
}

Ast::CaseStatement* Ast::NodeFactory::aCaseStatement(const Ast::Token& pos, const Ast::CompoundStatement& block) {
    Ast::CaseDefaultStatement& caseStatement = unit().addNode(new Ast::CaseDefaultStatement(pos, block));
    return z::ptr(caseStatement);
}

Ast::BreakStatement* Ast::NodeFactory::aBreakStatement(const Ast::Token& pos) {
    Ast::BreakStatement& breakStatement = unit().addNode(new Ast::BreakStatement(pos));
    return z::ptr(breakStatement);
}

Ast::ContinueStatement* Ast::NodeFactory::aContinueStatement(const Ast::Token& pos) {
    Ast::ContinueStatement& continueStatement = unit().addNode(new Ast::ContinueStatement(pos));
    return z::ptr(continueStatement);
}

Ast::AddEventHandlerStatement* Ast::NodeFactory::aAddEventHandlerStatement(const Ast::Token& pos, const Ast::EventDecl& event, const Ast::Expr& source, Ast::FunctionTypeInstanceExpr& functor) {
    Ast::AddEventHandlerStatement& addEventHandlerStatement = unit().addNode(new Ast::AddEventHandlerStatement(pos, event, source, functor));
    unit().popExpectedTypeSpec(pos, Unit::ExpectedTypeSpec::etEventHandler);
    return z::ptr(addEventHandlerStatement);
}

const Ast::EventDecl* Ast::NodeFactory::aEnterAddEventHandler(const Ast::EventDecl& eventDecl) {
    Ast::QualifiedTypeSpec& qts = addQualifiedTypeSpec(getToken(), false, eventDecl.handler(), false);
    unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etEventHandler, qts);
    return z::ptr(eventDecl);
}

Ast::RoutineReturnStatement* Ast::NodeFactory::aRoutineReturnStatement(const Ast::Token& pos) {
    Ast::ExprList& exprList = addExprList(pos);
    Ast::RoutineReturnStatement& returnStatement = unit().addNode(new Ast::RoutineReturnStatement(pos, exprList));
    return z::ptr(returnStatement);
}

Ast::RoutineReturnStatement* Ast::NodeFactory::aRoutineReturnStatement(const Ast::Token& pos, const Ast::Expr& expr) {
    Ast::ExprList& exprList = addExprList(pos);
    exprList.addExpr(expr);
    Ast::RoutineReturnStatement& returnStatement = unit().addNode(new Ast::RoutineReturnStatement(pos, exprList));
    return z::ptr(returnStatement);
}

Ast::FunctionReturnStatement* Ast::NodeFactory::aFunctionReturnStatement(const Ast::Token& pos, const Ast::ExprList& exprList) {
    const Ast::FunctionSig* sig = 0;
    const Ast::RootFunctionDefn* rfd = 0;
    const Ast::ChildFunctionDefn* cfd = 0;
    for(Ast::Unit::TypeSpecStack::const_reverse_iterator it = unit().typeSpecStack().rbegin(); it != unit().typeSpecStack().rend(); ++it) {
        const Ast::TypeSpec& ts = it->get();
        if((rfd = dynamic_cast<const Ast::RootFunctionDefn*>(z::ptr(ts))) != 0) {
            sig = z::ptr(z::ref(rfd).sig());
            break;
        }
        if((cfd = dynamic_cast<const Ast::ChildFunctionDefn*>(z::ptr(ts))) != 0) {
            sig = z::ptr(z::ref(cfd).sig());
            break;
        }
    }
    Ast::FunctionReturnStatement& returnStatement = unit().addNode(new Ast::FunctionReturnStatement(pos, exprList, z::ref(sig)));
    return z::ptr(returnStatement);
}

Ast::CompoundStatement* Ast::NodeFactory::aStatementList(Ast::CompoundStatement& list, const Ast::Statement& statement) {
    list.addStatement(statement);
    return z::ptr(list);
}

Ast::CompoundStatement* Ast::NodeFactory::aStatementList() {
    Ast::CompoundStatement& statement = unit().addNode(new Ast::CompoundStatement(getToken()));
    return z::ptr(statement);
}

void Ast::NodeFactory::aEnterCompoundStatement(const Ast::Token& pos) {
    Ast::Scope& scope = addScope(pos, Ast::ScopeType::Local);
    unit().enterScope(scope);
}

void Ast::NodeFactory::aLeaveCompoundStatement() {
    unit().leaveScope();
}

Ast::ExprList* Ast::NodeFactory::aExprList(Ast::ExprList& list, const Ast::Expr& expr) {
    list.addExpr(expr);
    return z::ptr(list);
}

Ast::ExprList* Ast::NodeFactory::aExprList(const Ast::Expr& expr) {
    Ast::ExprList& list = addExprList(expr.pos());
    return aExprList(list, expr);
}

Ast::ExprList* Ast::NodeFactory::aExprList() {
    Ast::ExprList& list = addExprList(getToken());
    return z::ptr(list);
}

Ast::TernaryOpExpr* Ast::NodeFactory::aConditionalExpr(const Ast::Token& op1, const Ast::Token& op2, const Ast::Expr& lhs, const Ast::Expr& rhs1, const Ast::Expr& rhs2) {
    const Ast::QualifiedTypeSpec& qTypeSpec = unit().coerce(op2, rhs1.qTypeSpec(), rhs2.qTypeSpec());
    Ast::ConditionalExpr& expr = unit().addNode(new Ast::ConditionalExpr(qTypeSpec, op1, op2, lhs, rhs1, rhs2));
    return z::ptr(expr);
}

template <typename T>
inline Ast::Expr& Ast::NodeFactory::createBooleanExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(op, "bool");
    T& expr = unit().addNode(new T(qTypeSpec, op, lhs, rhs));
    return expr;
}

Ast::Expr& Ast::NodeFactory::aBooleanAndExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBooleanExpr<Ast::BooleanAndExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBooleanOrExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBooleanExpr<Ast::BooleanOrExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBooleanEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBooleanExpr<Ast::BooleanEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBooleanNotEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBooleanExpr<Ast::BooleanNotEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBooleanLessThanExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBooleanExpr<Ast::BooleanLessThanExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBooleanGreaterThanExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBooleanExpr<Ast::BooleanGreaterThanExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBooleanLessThanOrEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBooleanExpr<Ast::BooleanLessThanOrEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBooleanGreaterThanOrEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBooleanExpr<Ast::BooleanGreaterThanOrEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBooleanHasExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBooleanExpr<Ast::BooleanHasExpr>(op, lhs, rhs);
}

template <typename T>
inline Ast::Expr& Ast::NodeFactory::createBinaryExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    const Ast::QualifiedTypeSpec& qTypeSpec = unit().coerce(op, lhs.qTypeSpec(), rhs.qTypeSpec());
    T& expr = unit().addNode(new T(qTypeSpec, op, lhs, rhs));
    return expr;
}

Ast::Expr& Ast::NodeFactory::aBinaryAssignEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    const Ast::IndexExpr* indexExpr = dynamic_cast<const Ast::IndexExpr*>(z::ptr(lhs));
    if(indexExpr) {
        const Ast::QualifiedTypeSpec& qTypeSpec = unit().coerce(op, lhs.qTypeSpec(), rhs.qTypeSpec());
        Ast::SetIndexExpr& expr = unit().addNode(new Ast::SetIndexExpr(op, qTypeSpec, z::ref(indexExpr), rhs));
        return expr;
    }
    return createBinaryExpr<Ast::BinaryAssignEqualExpr>(op, lhs, rhs);
}

template <typename T>
inline Ast::Expr& Ast::NodeFactory::createBinaryOpExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    const Ast::IndexExpr* indexExpr = dynamic_cast<const Ast::IndexExpr*>(z::ptr(lhs));
    if(indexExpr) {
        throw z::Exception("NodeFactory", zfmt(op, "Operator '%{s}' on index expression not implemented").add("s", op ));
    }
    return createBinaryExpr<T>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryPlusEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryOpExpr<Ast::BinaryPlusEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryMinusEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryOpExpr<Ast::BinaryMinusEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryTimesEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryOpExpr<Ast::BinaryTimesEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryDivideEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryOpExpr<Ast::BinaryDivideEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryModEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryOpExpr<Ast::BinaryModEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryBitwiseAndEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryOpExpr<Ast::BinaryBitwiseAndEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryBitwiseOrEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryOpExpr<Ast::BinaryBitwiseOrEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryBitwiseXorEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryOpExpr<Ast::BinaryBitwiseXorEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryShiftLeftEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryOpExpr<Ast::BinaryShiftLeftEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryShiftRightEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryOpExpr<Ast::BinaryShiftRightEqualExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryPlusExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryExpr<Ast::BinaryPlusExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryMinusExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryExpr<Ast::BinaryMinusExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryTimesExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryExpr<Ast::BinaryTimesExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryDivideExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryExpr<Ast::BinaryDivideExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryModExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryExpr<Ast::BinaryModExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryBitwiseAndExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryExpr<Ast::BinaryBitwiseAndExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryBitwiseOrExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryExpr<Ast::BinaryBitwiseOrExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryBitwiseXorExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryExpr<Ast::BinaryBitwiseXorExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryShiftLeftExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryExpr<Ast::BinaryShiftLeftExpr>(op, lhs, rhs);
}

Ast::Expr& Ast::NodeFactory::aBinaryShiftRightExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    return createBinaryExpr<Ast::BinaryShiftRightExpr>(op, lhs, rhs);
}

template <typename T>
inline T& Ast::NodeFactory::createPostfixExpr(const Ast::Token& op, const Ast::Expr& lhs) {
    const Ast::QualifiedTypeSpec& qTypeSpec = lhs.qTypeSpec();
    T& expr = unit().addNode(new T(qTypeSpec, op, lhs));
    return expr;
}

Ast::PostfixIncExpr& Ast::NodeFactory::aPostfixIncExpr(const Ast::Token& op, const Ast::Expr& lhs) {
    return createPostfixExpr<Ast::PostfixIncExpr>(op, lhs);
}

Ast::PostfixDecExpr& Ast::NodeFactory::aPostfixDecExpr(const Ast::Token& op, const Ast::Expr& lhs) {
    return createPostfixExpr<Ast::PostfixDecExpr>(op, lhs);
}

template <typename T>
inline T& Ast::NodeFactory::createPrefixExpr(const Ast::Token& op, const Ast::Expr& rhs) {
    const Ast::QualifiedTypeSpec& qTypeSpec = rhs.qTypeSpec();
    T& expr = unit().addNode(new T(qTypeSpec, op, rhs));
    return expr;
}

Ast::PrefixNotExpr& Ast::NodeFactory::aPrefixNotExpr(const Ast::Token& op, const Ast::Expr& rhs) {
    return createPrefixExpr<Ast::PrefixNotExpr>(op, rhs);
}

Ast::PrefixPlusExpr& Ast::NodeFactory::aPrefixPlusExpr(const Ast::Token& op, const Ast::Expr& rhs) {
    return createPrefixExpr<Ast::PrefixPlusExpr>(op, rhs);
}

Ast::PrefixMinusExpr& Ast::NodeFactory::aPrefixMinusExpr(const Ast::Token& op, const Ast::Expr& rhs) {
    return createPrefixExpr<Ast::PrefixMinusExpr>(op, rhs);
}

Ast::PrefixIncExpr& Ast::NodeFactory::aPrefixIncExpr(const Ast::Token& op, const Ast::Expr& rhs) {
    return createPrefixExpr<Ast::PrefixIncExpr>(op, rhs);
}

Ast::PrefixDecExpr& Ast::NodeFactory::aPrefixDecExpr(const Ast::Token& op, const Ast::Expr& rhs) {
    return createPrefixExpr<Ast::PrefixDecExpr>(op, rhs);
}

Ast::PrefixBitwiseNotExpr& Ast::NodeFactory::aPrefixBitwiseNotExpr(const Ast::Token& op, const Ast::Expr& rhs) {
    return createPrefixExpr<Ast::PrefixBitwiseNotExpr>(op, rhs);
}

Ast::ListExpr* Ast::NodeFactory::aListExpr(const Ast::Token& pos, const Ast::ListList& list) {
    unit().popExpectedTypeSpecOrAuto(pos, Unit::ExpectedTypeSpec::etListVal);
    Ast::TemplateTypePartList& tlist = unit().addNode(new Ast::TemplateTypePartList(pos));
    tlist.addType(list.valueType());
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "list", tlist);
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);

    Ast::ListExpr& expr = unit().addNode(new Ast::ListExpr(pos, qTypeSpec, list));
    return z::ptr(expr);
}

Ast::ListList* Ast::NodeFactory::aListList(const Ast::Token& pos, Ast::ListList& list, const Ast::ListItem& item) {
    list.addItem(item);
    const Ast::QualifiedTypeSpec& qValueTypeSpec = unit().coerce(pos, list.valueType(), item.valueExpr().qTypeSpec());
    list.valueType(qValueTypeSpec);
    return z::ptr(list);
}

Ast::ListList* Ast::NodeFactory::aListList(const Ast::Token& pos, const Ast::ListItem& item) {
    Ast::ListList& list = unit().addNode(new Ast::ListList(pos));
    list.addItem(item);
    const Ast::QualifiedTypeSpec& valType = unit().getExpectedTypeSpec(pos, z::ptr(item.valueExpr().qTypeSpec()));
    list.valueType(valType);
    return z::ptr(list);
}

Ast::ListList* Ast::NodeFactory::aListList(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec) {
    Ast::ListList& list = unit().addNode(new Ast::ListList(pos));
    const Ast::QualifiedTypeSpec& valType = unit().getExpectedTypeSpec(pos, z::ptr(qTypeSpec));
    list.dValueType(qTypeSpec);
    list.valueType(valType);
    return z::ptr(list);
}

Ast::ListList* Ast::NodeFactory::aListList(const Ast::Token& pos) {
    Ast::ListList& list = unit().addNode(new Ast::ListList(pos));
    const Ast::QualifiedTypeSpec& valType = unit().getExpectedTypeSpec(pos, 0);
    list.valueType(valType);
    return z::ptr(list);
}

Ast::ListItem* Ast::NodeFactory::aListItem(const Ast::Expr& valueExpr) {
    const Ast::Expr& expr = convertExprToExpectedTypeSpec(valueExpr.pos(), valueExpr);
    Ast::ListItem& item = unit().addNode(new Ast::ListItem(valueExpr.pos(), expr));
//    popExpectedTypeSpec(valueExpr.pos(), ExpectedTypeSpec::etListVal);
    return z::ptr(item);
}

Ast::DictExpr* Ast::NodeFactory::aDictExpr(const Ast::Token& pos, const Ast::DictList& list) {
    unit().popExpectedTypeSpecOrAuto(pos, Unit::ExpectedTypeSpec::etDictKey);

    Ast::TemplateTypePartList& tlist = unit().addNode(new Ast::TemplateTypePartList(pos));
    tlist.addType(list.keyType());
    tlist.addType(list.valueType());
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "dict", tlist);

    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);

    Ast::DictExpr& expr = unit().addNode(new Ast::DictExpr(pos, qTypeSpec, list));
    return z::ptr(expr);
}

Ast::DictList* Ast::NodeFactory::aDictList(const Ast::Token& pos, Ast::DictList& list, const Ast::DictItem& item) {
    list.addItem(item);
    const Ast::QualifiedTypeSpec& keyType = unit().coerce(pos, list.keyType(), item.keyExpr().qTypeSpec());
    const Ast::QualifiedTypeSpec& valType = unit().coerce(pos, list.valueType(), item.valueExpr().qTypeSpec());

    list.keyType(keyType);
    list.valueType(valType);
    return z::ptr(list);
}

Ast::DictList* Ast::NodeFactory::aDictList(const Ast::Token& pos, const Ast::DictItem& item) {
    Ast::DictList& list = unit().addNode(new Ast::DictList(pos));
    list.addItem(item);
    list.keyType(item.keyExpr().qTypeSpec());
    list.valueType(item.valueExpr().qTypeSpec());
    return z::ptr(list);
}

Ast::DictList* Ast::NodeFactory::aDictList(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qKeyTypeSpec, const Ast::QualifiedTypeSpec& qValueTypeSpec) {
    Ast::DictList& list = unit().addNode(new Ast::DictList(pos));
    list.dKeyType(qKeyTypeSpec);
    list.dValueType(qValueTypeSpec);
    list.keyType(qKeyTypeSpec);
    list.valueType(qValueTypeSpec);
    return z::ptr(list);
}

/// The sequence of calls in this function is important.
inline const Ast::Expr& Ast::NodeFactory::switchDictKeyValue(const Ast::Token& pos, const Ast::Unit::ExpectedTypeSpec::Type& popType, const Ast::Unit::ExpectedTypeSpec::Type& pushType, const Ast::TemplateDefn::size_type& idx, const Ast::Expr& initExpr) {
    const Ast::Expr& expr = convertExprToExpectedTypeSpec(pos, initExpr);
    bool isExpected = unit().popExpectedTypeSpecOrAuto(pos, popType);
    const Ast::TemplateDefn* td0 = unit().isEnteringList();

    if(isExpected) {
        if((td0) && (z::ref(td0).name().string() == "dict")) {
            const Ast::QualifiedTypeSpec& keyType = z::ref(td0).at(idx);
            unit().pushExpectedTypeSpec(pushType, keyType);
        } else {
            assert(false);
        }
    } else {
        unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etAuto);
    }
    return expr;
}

Ast::DictItem* Ast::NodeFactory::aDictItem(const Ast::Token& pos, const Ast::Expr& keyExpr, const Ast::Expr& valueExpr) {
    const Ast::Expr& expr = switchDictKeyValue(pos, Unit::ExpectedTypeSpec::etDictVal, Unit::ExpectedTypeSpec::etDictKey, 0, valueExpr);
    Ast::DictItem& item = unit().addNode(new Ast::DictItem(pos, keyExpr, expr));
    return z::ptr(item);
}

const Ast::Expr* Ast::NodeFactory::aDictKey(const Ast::Expr& keyExpr) {
    const Ast::Expr& expr = switchDictKeyValue(keyExpr.pos(), Unit::ExpectedTypeSpec::etDictKey, Unit::ExpectedTypeSpec::etDictVal, 1, keyExpr);
    return z::ptr(expr);
}

const Ast::Token& Ast::NodeFactory::aEnterList(const Ast::Token& pos) {
    const Ast::TemplateDefn* td0 = unit().isEnteringList();
    if(td0) {
        if(z::ref(td0).name().string() == "list") {
            const Ast::QualifiedTypeSpec& valType = z::ref(td0).at(0);
            unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etListVal, valType);
        } else if(z::ref(td0).name().string() == "dict") {
            const Ast::QualifiedTypeSpec& keyType = z::ref(td0).at(0);
            unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etDictKey, keyType);
        } else {
            assert(false);
        }
    } else {
        unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etAuto);
    }

    return pos;
}

Ast::FormatExpr* Ast::NodeFactory::aFormatExpr(const Ast::Token& pos, const Ast::Expr& stringExpr, const Ast::DictExpr& dictExpr) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "string");
    Ast::FormatExpr& formatExpr = unit().addNode(new Ast::FormatExpr(pos, qTypeSpec, stringExpr, dictExpr));
    return z::ptr(formatExpr);
}

Ast::RoutineCallExpr* Ast::NodeFactory::aRoutineCallExpr(const Ast::Token& pos, const Ast::Routine& routine, const Ast::ExprList& exprList) {
    unit().popCallArgList(pos, routine.inScope());
    const Ast::QualifiedTypeSpec& qTypeSpec = routine.outType();
    Ast::RoutineCallExpr& routineCallExpr = unit().addNode(new Ast::RoutineCallExpr(pos, qTypeSpec, routine, exprList));
    return z::ptr(routineCallExpr);
}

const Ast::Routine* Ast::NodeFactory::aEnterRoutineCall(const Ast::Routine& routine) {
    unit().pushCallArgList(routine.inScope());
    return z::ptr(routine);
}

Ast::FunctorCallExpr* Ast::NodeFactory::aFunctorCallExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::ExprList& exprList) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if(function == 0) {
        throw z::Exception("NodeFactory", zfmt(pos, "Unknown functor being called '%{s}'").add("s", expr.qTypeSpec().typeSpec().name() ));
    }

    unit().popCallArgList(pos, z::ref(function).sig().inScope());

    const Ast::QualifiedTypeSpec& qTypeSpec = getFunctionReturnType(pos, z::ref(function));
    Ast::FunctorCallExpr& functorCallExpr = unit().addNode(new Ast::FunctorCallExpr(pos, qTypeSpec, expr, exprList));
    return z::ptr(functorCallExpr);
}

Ast::Expr* Ast::NodeFactory::aEnterFunctorCall(Ast::Expr& expr) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if(function == 0) {
        throw z::Exception("NodeFactory", zfmt(expr.pos(), "Unknown functor being called '%{s}'").add("s", expr.qTypeSpec().typeSpec().name() ));
    }

    unit().pushCallArgList(z::ref(function).sig().inScope());
    return z::ptr(expr);
}

Ast::Expr* Ast::NodeFactory::aEnterFunctorCall(const Ast::Token& name) {
    Ast::VariableRefExpr* expr = aVariableRefExpr(name);
    return aEnterFunctorCall(z::ref(expr));
}

Ast::Expr* Ast::NodeFactory::aEnterFunctorCall(const Ast::Function& function) {
    Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(getToken(), false, function, false);
    Ast::ExprList& exprList = addExprList(getToken());
    Ast::FunctionInstanceExpr& expr = unit().addNode(new Ast::FunctionInstanceExpr(getToken(), qTypeSpec, function, exprList));
    return aEnterFunctorCall(expr);
}

Ast::ExprList* Ast::NodeFactory::aCallArgList(const Ast::Token& pos, Ast::ExprList& list, const Ast::Expr& expr) {
    const Ast::Expr& argExpr = convertExprToExpectedTypeSpec(pos, expr);
    list.addExpr(argExpr);
    unit().popCallArg(pos);
    return z::ptr(list);
}

Ast::ExprList* Ast::NodeFactory::aCallArgList(const Ast::Expr& expr) {
    Ast::ExprList& list = addExprList(getToken());
    return aCallArgList(getToken(), list, expr);
}

Ast::ExprList* Ast::NodeFactory::aCallArgList() {
    Ast::ExprList& list = addExprList(getToken());
    return z::ptr(list);
}

Ast::RunExpr* Ast::NodeFactory::aRunExpr(const Ast::Token& pos, const Ast::FunctorCallExpr& callExpr) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(z::ptr(callExpr.expr().qTypeSpec().typeSpec()));
    if(function != 0) {
        Ast::QualifiedTypeSpec& qRetTypeSpec = addQualifiedTypeSpec(pos, false, z::ref(function), false);

        Ast::TemplateTypePartList& list = unit().addNode(new Ast::TemplateTypePartList(pos));
        list.addType(qRetTypeSpec);
        Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "future", list);
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);

        Ast::RunExpr& runExpr = unit().addNode(new Ast::RunExpr(pos, qTypeSpec, callExpr));
        return z::ptr(runExpr);
    }
    throw z::Exception("NodeFactory", zfmt(pos, "Unknown functor in run expression '%{s}'")
                       .add("s", ZenlangNameGenerator().qtn(callExpr.expr().qTypeSpec()) )
                       );
}

Ast::OrderedExpr* Ast::NodeFactory::aOrderedExpr(const Ast::Token& pos, const Ast::Expr& innerExpr) {
    Ast::OrderedExpr& expr = unit().addNode(new Ast::OrderedExpr(pos, innerExpr.qTypeSpec(), innerExpr));
    return z::ptr(expr);
}

Ast::IndexExpr* Ast::NodeFactory::aIndexExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::Expr& index) {
    const Ast::TypeSpec* listTypeSpec = resolveTypedef(expr.qTypeSpec().typeSpec());
    const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(listTypeSpec);
    if(td) {
        if(z::ref(td).name().string() == "list") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(td).at(0).isConst(), z::ref(td).at(0).typeSpec(), false);
            Ast::IndexExpr& indexExpr = unit().addNode(new Ast::IndexExpr(pos, qTypeSpec, expr, index));
            return z::ptr(indexExpr);
        }

        if(z::ref(td).name().string() == "dict") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(td).at(1).isConst(), z::ref(td).at(1).typeSpec(), true);
            Ast::IndexExpr& indexExpr = unit().addNode(new Ast::IndexExpr(pos, qTypeSpec, expr, index));
            return z::ptr(indexExpr);
        }
    }

    const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(listTypeSpec);
    if(sd) {
        const Ast::Routine* routine = z::ref(sd).hasChild<const Ast::Routine>("at");
        if(routine) {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(routine).outType().isConst(), z::ref(routine).outType().typeSpec(), true);
            Ast::IndexExpr& indexExpr = unit().addNode(new Ast::IndexExpr(pos, qTypeSpec, expr, index));
            return z::ptr(indexExpr);
        }
    }

    throw z::Exception("NodeFactory", zfmt(pos, "'%{s}' is not an indexable type").add("s", ZenlangNameGenerator().qtn(expr.qTypeSpec()) ));
}

Ast::SpliceExpr* Ast::NodeFactory::aSpliceExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::Expr& from, const Ast::Expr& to) {
    const Ast::TypeSpec* listTypeSpec = resolveTypedef(expr.qTypeSpec().typeSpec());
    const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(listTypeSpec);
    if((td) && (z::ref(td).name().string() == "list")) {
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(td).at(0).isConst(), expr.qTypeSpec().typeSpec(), false);
        Ast::SpliceExpr& spliceExpr = unit().addNode(new Ast::SpliceExpr(pos, qTypeSpec, expr, from, to));
        return z::ptr(spliceExpr);
    }

    throw z::Exception("NodeFactory", zfmt(pos, "'%{s}' is not an splicable type").add("s", ZenlangNameGenerator().qtn(expr.qTypeSpec()) ));
}

Ast::TypeofTypeExpr* Ast::NodeFactory::aTypeofTypeExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& typeSpec) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "type");
    Ast::TypeofTypeExpr& typeofExpr = unit().addNode(new Ast::TypeofTypeExpr(pos, qTypeSpec, typeSpec));
    return z::ptr(typeofExpr);
}

Ast::TypeofExprExpr* Ast::NodeFactory::aTypeofExprExpr(const Ast::Token& pos, const Ast::Expr& expr) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "type");
    Ast::TypeofExprExpr& typeofExpr = unit().addNode(new Ast::TypeofExprExpr(pos, qTypeSpec, expr));
    return z::ptr(typeofExpr);
}

Ast::TypecastExpr* Ast::NodeFactory::aTypecastExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr) {
    /// \todo check if canCoerce
    const Ast::TemplateDefn* subType = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if((subType) && (z::ref(subType).name().string() == "pointer")) {
        const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, qTypeSpec.isConst(), qTypeSpec.typeSpec(), true);
        Ast::TypecastExpr& typecastExpr = unit().addNode(new Ast::DynamicTypecastExpr(pos, qTypeSpec, typeSpec, expr));
        return z::ptr(typecastExpr);
    }

    Ast::TypecastExpr& typecastExpr = unit().addNode(new Ast::StaticTypecastExpr(pos, qTypeSpec, qTypeSpec, expr));
    return z::ptr(typecastExpr);
}

Ast::PointerInstanceExpr* Ast::NodeFactory::aPointerInstanceExpr(const Ast::Token& pos, const Ast::Expr& expr) {
    Ast::TemplateTypePartList& list = unit().addNode(new Ast::TemplateTypePartList(pos));
    const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, expr.qTypeSpec().isConst(), expr.qTypeSpec().typeSpec(), true);
    list.addType(typeSpec);
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "pointer", list);

    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);
    Ast::ExprList& exprList = addExprList(pos);
    exprList.addExpr(expr);

    Ast::PointerInstanceExpr& pointerExpr = unit().addNode(new Ast::PointerInstanceExpr(pos, qTypeSpec, templateDefn, templateDefn, exprList));
    return z::ptr(pointerExpr);
}

Ast::ValueInstanceExpr* Ast::NodeFactory::aValueInstanceExpr(const Ast::Token& pos, const Ast::Expr& expr) {
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if(templateDefn) {
        if(z::ref(templateDefn).name().string() == "pointer") {
            Ast::ExprList& exprList = addExprList(pos);
            exprList.addExpr(expr);
            Ast::ValueInstanceExpr& valueInstanceExpr = getValueInstanceExpr(pos, z::ref(templateDefn).at(0), z::ref(templateDefn), z::ref(templateDefn), exprList);
            return z::ptr(valueInstanceExpr);
        }
    }

    throw z::Exception("NodeFactory", zfmt(pos, "Expression is not a pointer to  '%{s}'").add("s", expr.qTypeSpec().typeSpec().name() ));
}

Ast::TemplateDefnInstanceExpr* Ast::NodeFactory::aTemplateDefnInstanceExpr(const Ast::Token& pos, const Ast::TemplateDefn& templateDefn, const Ast::ExprList& exprList) {
    z::string name = templateDefn.name().string();
    if(name == "pointer") {

        Ast::TemplateTypePartList& list = unit().addNode(new Ast::TemplateTypePartList(pos));
        const Ast::QualifiedTypeSpec& newTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn.at(0).typeSpec(), true);
        list.addType(newTypeSpec);
        Ast::TemplateDefn& newTemplateDefn = createTemplateDefn(pos, "pointer", list);
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, newTemplateDefn, false);

        Ast::PointerInstanceExpr& expr = unit().addNode(new Ast::PointerInstanceExpr(pos, qTypeSpec, templateDefn, newTemplateDefn, exprList));
        return z::ptr(expr);
    }

    if(name == "value") {
//@        Ast::ValueInstanceExpr& valueInstanceExpr = getValueInstanceExpr(pos, templateDefn, templateDefn, exprList.at(0));
//        return z::ptr(valueInstanceExpr);
        const Ast::QualifiedTypeSpec& qTypeSpec = templateDefn.at(0);
        const Ast::Expr& expr = exprList.at(0);
        const Ast::TemplateDefn* subType = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(expr.qTypeSpec().typeSpec()));
        if((subType == 0) || (z::ref(subType).name().string() != "pointer")) {
            throw z::Exception("NodeFactory", zfmt(pos, "Expression is not a pointer to '%{s}'").add("s", qTypeSpec.typeSpec().name() ));
        }

        Ast::ValueInstanceExpr& valueInstanceExpr = getValueInstanceExpr(pos, qTypeSpec, templateDefn, templateDefn, exprList);
        return z::ptr(valueInstanceExpr);
//@        return addValueInstanceExpr(pos, templateDefn, templateDefn.at(0), exprList.at(0));
    }

    throw z::Exception("NodeFactory", zfmt(pos, "Invalid template instantiation '%{s}'").add("s", templateDefn.name() ));
}

Ast::VariableRefExpr* Ast::NodeFactory::aVariableRefExpr(const Ast::Token& name) {
    Ast::RefType::T refType = Ast::RefType::Local;
    const Ast::VariableDefn* vref = unit().getVariableDef(name, refType);
    if(vref == 0) {
        throw z::Exception("NodeFactory", zfmt(name, "Variable not found: '%{s}'").add("s", name ));
    }

    // create vref expression
    Ast::VariableRefExpr& vrefExpr = unit().addNode(new Ast::VariableRefExpr(name, z::ref(vref).qTypeSpec(), z::ref(vref), refType));
    return z::ptr(vrefExpr);
}

Ast::MemberExpr* Ast::NodeFactory::aMemberVariableExpr(const Ast::Expr& expr, const Ast::Token& name) {
    const Ast::TypeSpec& typeSpec = expr.qTypeSpec().typeSpec();

    const Ast::StructDefn* structDefn = dynamic_cast<const Ast::StructDefn*>(z::ptr(typeSpec));
    if(structDefn != 0) {
        for(StructBaseIterator sbi(structDefn); sbi.hasNext(); sbi.next()) {
            const Ast::VariableDefn* vref = unit().hasMember(sbi.get().scope(), name);
            if(vref) {
                const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, expr.qTypeSpec().isConst(), z::ref(vref).qTypeSpec().typeSpec(), true);
                Ast::MemberVariableExpr& vdefExpr = unit().addNode(new Ast::MemberVariableExpr(name, qTypeSpec, expr, z::ref(vref)));
                return z::ptr(vdefExpr);
            }

            for(Ast::StructDefn::PropertyList::const_iterator it = sbi.get().propertyList().begin(); it != sbi.get().propertyList().end(); ++it) {
                const Ast::PropertyDecl& pref = it->get();
                if(pref.name().string() == name.string()) {
                    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, expr.qTypeSpec().isConst(), pref.qTypeSpec().typeSpec(), true);
                    Ast::MemberPropertyExpr& vdefExpr = unit().addNode(new Ast::MemberPropertyExpr(name, qTypeSpec, expr, pref));
                    return z::ptr(vdefExpr);
                }
            }
        }

        throw z::Exception("NodeFactory", zfmt(name, "'%{s}' is not a member of struct '%{t}'")
                           .add("s", name )
                           .add("t", ZenlangNameGenerator().tn(typeSpec) )
                           );
    }

    const Ast::FunctionRetn* functionRetn = dynamic_cast<const Ast::FunctionRetn*>(z::ptr(typeSpec));
    if(functionRetn != 0) {
        const Ast::VariableDefn* vref = unit().hasMember(z::ref(functionRetn).outScope(), name);
        if(vref) {
            Ast::MemberVariableExpr& vdefExpr = unit().addNode(new Ast::MemberVariableExpr(name, z::ref(vref).qTypeSpec(), expr, z::ref(vref)));
            return z::ptr(vdefExpr);
        }
        throw z::Exception("NodeFactory", zfmt(name, "'%{s}' is not a member of function: '%{t}'")
                           .add("s", name)
                           .add("t", ZenlangNameGenerator().tn(typeSpec))
                           );
    }

    throw z::Exception("NodeFactory", zfmt(name, "Not an aggregate expression type '%{s}' (looking for member '%{t}')")
                       .add("s", typeSpec.name())
                       .add("t", name)
                       );
}

Ast::TypeSpecMemberExpr* Ast::NodeFactory::aTypeSpecMemberExpr(const Ast::TypeSpec& typeSpec, const Ast::Token& name) {
    const Ast::EnumDefn* enumDefn = dynamic_cast<const Ast::EnumDefn*>(z::ptr(typeSpec));
    if(enumDefn != 0) {
        const Ast::VariableDefn* vref = unit().hasMember(z::ref(enumDefn).scope(), name);
        if(vref == 0) {
            throw z::Exception("NodeFactory", zfmt(name, "'%{s}' is not a member of type %{t}")
                               .add("s", name)
                               .add("t", typeSpec.name())
                               );
        }
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, typeSpec, false);
        Ast::EnumMemberExpr& typeSpecMemberExpr = unit().addNode(new Ast::EnumMemberExpr(name, qTypeSpec, typeSpec, z::ref(vref)));
        return z::ptr(typeSpecMemberExpr);
    }

    const Ast::StructDefn* structDefn = dynamic_cast<const Ast::StructDefn*>(z::ptr(typeSpec));
    if(structDefn != 0) {
        const Ast::VariableDefn* vref = unit().hasMember(z::ref(structDefn).scope(), name);
        if(vref == 0) {
            throw z::Exception("NodeFactory", zfmt(name, "'%{s}' is not a member of type %{t}")
                               .add("s", name)
                               .add("t", typeSpec.name())
                               );
        }
        Ast::StructMemberExpr& typeSpecMemberExpr = unit().addNode(new Ast::StructMemberExpr(name, z::ref(vref).qTypeSpec(), typeSpec, z::ref(vref)));
        return z::ptr(typeSpecMemberExpr);
    }

    throw z::Exception("NodeFactory", zfmt(name, "Not an aggregate type '%{s}' (looking for member %{t})")
                       .add("s", typeSpec.name())
                       .add("t", name)
                       );
}

Ast::StructInstanceExpr* Ast::NodeFactory::aStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn, const Ast::StructInitPartList& list) {
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, structDefn, false);
    Ast::StructInstanceExpr& structInstanceExpr = unit().addNode(new Ast::StructInstanceExpr(pos, qTypeSpec, structDefn, list));
    return z::ptr(structInstanceExpr);
}

Ast::StructInstanceExpr* Ast::NodeFactory::aStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn) {
    Ast::StructInitPartList& list = unit().addNode(new Ast::StructInitPartList(pos));
    return aStructInstanceExpr(pos, structDefn, list);
}

Ast::Expr* Ast::NodeFactory::aAutoStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn, const Ast::StructInitPartList& list) {
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, structDefn, false);
    Ast::StructInstanceExpr& structInstanceExpr = unit().addNode(new Ast::StructInstanceExpr(pos, qTypeSpec, structDefn, list));
    return z::ptr(structInstanceExpr);
}

Ast::Expr* Ast::NodeFactory::aAutoStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn) {
    Ast::StructInitPartList& list = unit().addNode(new Ast::StructInitPartList(pos));
    return aAutoStructInstanceExpr(pos, structDefn, list);
}

const Ast::StructDefn* Ast::NodeFactory::aEnterStructInstanceExpr(const Ast::StructDefn& structDefn) {
    unit().pushStructInit(structDefn);
    return z::ptr(structDefn);
}

const Ast::StructDefn* Ast::NodeFactory::aEnterAutoStructInstanceExpr(const Ast::Token& pos) {
    const Ast::StructDefn* sd = unit().isStructExpected();
    if(sd) {
        return aEnterStructInstanceExpr(z::ref(sd));
    }
    sd = unit().isPointerToStructExpected();
    if(sd) {
        return aEnterStructInstanceExpr(z::ref(sd));
    }
    throw z::Exception("NodeFactory", zfmt(pos, "No struct type expected") );
}

void Ast::NodeFactory::aLeaveStructInstanceExpr() {
    unit().popStructInit();
}

const Ast::VariableDefn* Ast::NodeFactory::aEnterStructInitPart(const Ast::Token& name) {
    const Ast::StructDefn* structDefn = unit().structInit();
    if(structDefn == 0) {
        throw z::Exception("NodeFactory", zfmt(name, "Internal error initializing struct-member") );
    }

    for(StructBaseIterator sbi(structDefn); sbi.hasNext(); sbi.next()) {
        for(Ast::Scope::List::const_iterator it = sbi.get().list().begin(); it != sbi.get().list().end(); ++it) {
            const Ast::VariableDefn& vdef = it->get();
            if(vdef.name().string() == name.string()) {
                unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etStructInit, vdef.qTypeSpec());
                return z::ptr(vdef);
            }
        }
    }

    throw z::Exception("NodeFactory", zfmt(name, "member %{c} not found in struct %{t} (1)")
                       .add("c", name)
                       .add("t", ZenlangNameGenerator().tn(z::ref(structDefn)))
                       );
}

void Ast::NodeFactory::aLeaveStructInitPart(const Ast::Token& pos) {
    unused(pos);
}

Ast::StructInitPartList* Ast::NodeFactory::aStructInitPartList(Ast::StructInitPartList& list, const Ast::StructInitPart& part) {
    list.addPart(part);
    return z::ptr(list);
}

Ast::StructInitPartList* Ast::NodeFactory::aStructInitPartList(const Ast::StructInitPart& part) {
    Ast::StructInitPartList& list = unit().addNode(new Ast::StructInitPartList(getToken()));
    list.addPart(part);
    return z::ptr(list);
}

Ast::StructInitPart* Ast::NodeFactory::aStructInitPart(const Ast::Token& pos, const Ast::VariableDefn& vdef, const Ast::Expr& initExpr) {
    const Ast::Expr& expr = convertExprToExpectedTypeSpec(pos, initExpr);
    unit().popExpectedTypeSpec(pos, Unit::ExpectedTypeSpec::etStructInit);
    Ast::StructInitPart& part = unit().addNode(new Ast::StructInitPart(pos, vdef, expr));
    return z::ptr(part);
}

Ast::FunctionInstanceExpr* Ast::NodeFactory::aFunctionInstanceExpr(const Ast::Token& pos, const Ast::TypeSpec& typeSpec, const Ast::ExprList& exprList) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(z::ptr(typeSpec));
    if(function != 0) {
        Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, z::ref(function), false);
        Ast::FunctionInstanceExpr& functionInstanceExpr = unit().addNode(new Ast::FunctionInstanceExpr(pos, qTypeSpec, z::ref(function), exprList));
        return z::ptr(functionInstanceExpr);
    }

    throw z::Exception("NodeFactory", zfmt(pos, "Not a function type %{s}").add("s", typeSpec.name() ));
}

Ast::AnonymousFunctionExpr* Ast::NodeFactory::aAnonymousFunctionExpr(Ast::ChildFunctionDefn& functionDefn, const Ast::CompoundStatement& compoundStatement) {
    aChildFunctionDefn(functionDefn, compoundStatement);
    Ast::ExprList& exprList = addExprList(getToken());
    Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(getToken(), false, functionDefn, false);
    Ast::AnonymousFunctionExpr& functionInstanceExpr = unit().addNode(new Ast::AnonymousFunctionExpr(getToken(), qTypeSpec, functionDefn, exprList));
    return z::ptr(functionInstanceExpr);
}

Ast::ChildFunctionDefn* Ast::NodeFactory::aEnterAnonymousFunction(const Ast::Function& function) {
    char namestr[128];
    sprintf(namestr, "_anonymous_%lu", unit().uniqueIdx());
    Ast::Token name(filename(), getToken().row(), getToken().col(), namestr);

    Ast::TypeSpec* ts = 0;
    for(Unit::TypeSpecStack::reverse_iterator it = unit().typeSpecStack().rbegin(); it != unit().typeSpecStack().rend(); ++it) {
        ts = z::ptr(it->get());
        if(dynamic_cast<Ast::Namespace*>(ts) != 0)
            break;
        if(dynamic_cast<Ast::Root*>(ts) != 0)
            break;
    }

    if(ts == 0) {
        throw z::Exception("NodeFactory", zfmt(name, "Internal error: Unable to find parent for anonymous function  %{s}").add("s", ZenlangNameGenerator().tn(function) ));
    }

    Ast::Scope& xref = addScope(name, Ast::ScopeType::XRef);
    Ast::Scope& iref = addScope(name, Ast::ScopeType::XRef);
    Ast::ClosureRef cref = aClosureList(xref, iref);
    Ast::ChildFunctionDefn& functionDefn = createChildFunctionDefn(z::ref(ts), function, name, Ast::DefinitionType::Final, cref);
    Ast::Statement* statement = aGlobalTypeSpecStatement(Ast::AccessType::Private, functionDefn);
    unused(statement);
    return z::ptr(functionDefn);
}

Ast::ChildFunctionDefn* Ast::NodeFactory::aEnterAutoAnonymousFunction(const Ast::Token& pos) {
    const Ast::Function* function = unit().isFunctionExpected();
    if(function == 0) {
        throw z::Exception("NodeFactory", zfmt(pos, "Internal error: no function type expected") );
    }
    return aEnterAnonymousFunction(z::ref(function));
}

Ast::ConstantNullExpr& Ast::NodeFactory::aConstantNullExpr(const Ast::Token& token) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "void");
    Ast::ConstantNullExpr& expr = unit().addNode(new Ast::ConstantNullExpr(token, qTypeSpec));
    return expr;
}

Ast::ConstantFloatExpr& Ast::NodeFactory::aConstantFloatExpr(const Ast::Token& token) {
    float value = token.string().to<float>();
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "float");
    Ast::ConstantFloatExpr& expr = unit().addNode(new Ast::ConstantFloatExpr(token, qTypeSpec, value));
    return expr;
}

Ast::ConstantDoubleExpr& Ast::NodeFactory::aConstantDoubleExpr(const Ast::Token& token) {
    double value = token.string().to<double>();
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "double");
    Ast::ConstantDoubleExpr& expr = unit().addNode(new Ast::ConstantDoubleExpr(token, qTypeSpec, value));
    return expr;
}

Ast::ConstantBooleanExpr& Ast::NodeFactory::aConstantBooleanExpr(const Ast::Token& token) {
    const bool value = (token.string() == "true")?true:false;
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "bool");
    Ast::ConstantBooleanExpr& expr = unit().addNode(new Ast::ConstantBooleanExpr(token, qTypeSpec, value));
    return expr;
}

Ast::ConstantStringExpr& Ast::NodeFactory::aConstantStringExpr(const Ast::Token& token) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "string");
    Ast::ConstantStringExpr& expr = unit().addNode(new Ast::ConstantStringExpr(token, qTypeSpec, token.string()));
    return expr;
}

Ast::ConstantCharExpr& Ast::NodeFactory::aConstantCharExpr(const Ast::Token& token) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "char");
    Ast::ConstantCharExpr& expr = unit().addNode(new Ast::ConstantCharExpr(token, qTypeSpec, token.string()));
    return expr;
}

Ast::ConstantLongExpr& Ast::NodeFactory::aConstantLongExpr(const Ast::Token& token) {
    long value = token.string().to<long>();
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "long");
    Ast::ConstantLongExpr& expr = unit().addNode(new Ast::ConstantLongExpr(token, qTypeSpec, value));
    return expr;
}

Ast::ConstantIntExpr& Ast::NodeFactory::aConstantIntExpr(const Ast::Token& token) {
    int value = token.string().to<int>();
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "int");
    Ast::ConstantIntExpr& expr = unit().addNode(new Ast::ConstantIntExpr(token, qTypeSpec, value));
    return expr;
}

Ast::ConstantShortExpr& Ast::NodeFactory::aConstantShortExpr(const Ast::Token& token) {
    int value = token.string().to<int>();
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "short");
    Ast::ConstantShortExpr& expr = unit().addNode(new Ast::ConstantShortExpr(token, qTypeSpec, value));
    return expr;
}
