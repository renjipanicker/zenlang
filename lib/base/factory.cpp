#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/factory.hpp"
#include "base/error.hpp"
#include "base/typename.hpp"

/////////////////////////////////////////////////////////////////////////
inline z::Ast::QualifiedTypeSpec& z::Ast::Factory::addQualifiedTypeSpec(const z::Ast::Token& pos, const bool& isConst, const TypeSpec& typeSpec, const bool& isRef, const bool& isStrong) {
    z::Ast::QualifiedTypeSpec& qualifiedTypeSpec = unit().addNode(new z::Ast::QualifiedTypeSpec(pos, isConst, typeSpec, isRef, isStrong));
    return qualifiedTypeSpec;
}

inline const z::Ast::QualifiedTypeSpec& z::Ast::Factory::getQualifiedTypeSpec(const z::Ast::Token& pos, const z::string& name) {
    z::Ast::Token token(filename(), pos.row(), pos.col(), name);
    const z::Ast::TypeSpec& typeSpec = unit().getRootTypeSpec<z::Ast::TypeSpec>(_module.level(), token);
    const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, typeSpec, false, false);
    return qTypeSpec;
}

inline const z::Ast::Expr& z::Ast::Factory::getDefaultValue(const z::Ast::TypeSpec& typeSpec, const z::Ast::Token& name) {
    const z::Ast::TypeSpec* ts = resolveTypedef(typeSpec);

    const z::Ast::Unit::DefaultValueList& list = unit().defaultValueList();
    z::Ast::Unit::DefaultValueList::const_iterator it = list.find(ts);
    if(it != list.end()) {
        return it->second.get();
    }

    const z::Ast::TemplateDefn* td = dynamic_cast<const z::Ast::TemplateDefn*>(ts);
    if(td != 0) {
        const z::string tdName = z::ref(td).name().string() ; // ZenlangNameGenerator().tn(z::ref(td), GenMode::Import); \todo this is incorrect, it will match any type called, say, list.
        if(tdName == "pointer") {
            const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, z::ref(td), false, false);
            z::Ast::ExprList& exprList = addExprList(name);
            const z::Ast::QualifiedTypeSpec& subType = z::ref(td).at(0);
            const z::Ast::Expr& nameExpr = getDefaultValue(subType.typeSpec(), name);
            exprList.addExpr(nameExpr);
            z::Ast::PointerInstanceExpr& expr = unit().addNode(new z::Ast::PointerInstanceExpr(name, qTypeSpec, z::ref(td), z::ref(td), exprList));
            return expr;
        }
        if((tdName == "list") || (tdName == "stack") || (tdName == "queue")) {
            z::Ast::Token value(name.filename(), name.row(), name.col(), "0");
            z::Ast::ConstantNullExpr& expr = aConstantNullExpr(value);
            return expr;
            //const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, z::ref(td), false, false);
            //z::Ast::ListList& llist = unit().addNode(new z::Ast::ListList(name));
            //const z::Ast::QualifiedTypeSpec& qlType = z::ref(td).list().at(0);
            //llist.valueType(qlType);
            //z::Ast::ListExpr& expr = unit().addNode(new z::Ast::ListExpr(name, qTypeSpec, llist));
            //return expr;
        }
        if(tdName == "dict") {
            z::Ast::Token value(name.filename(), name.row(), name.col(), "0");
            z::Ast::ConstantNullExpr& expr = aConstantNullExpr(value);
            return expr;
            //const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, z::ref(td), false, false);
            //z::Ast::DictList& llist = unit().addNode(new z::Ast::DictList(name));
            //const z::Ast::QualifiedTypeSpec& qlType = z::ref(td).list().at(0);
            //const z::Ast::QualifiedTypeSpec& qrType = z::ref(td).list().at(1);
            //llist.keyType(qlType);
            //llist.valueType(qrType);
            //z::Ast::DictExpr& expr = unit().addNode(new z::Ast::DictExpr(name, qTypeSpec, llist));
            //return expr;
        }
        if(tdName == "ptr") {
            z::Ast::Token value(name.filename(), name.row(), name.col(), "0");
            z::Ast::ConstantNullExpr& expr = aConstantNullExpr(value);
            return expr;
        }
    }

    const z::Ast::EnumDefn* ed = dynamic_cast<const z::Ast::EnumDefn*>(ts);
    if(ed != 0) {
        const z::Ast::Scope::List::const_iterator rit = z::ref(ed).list().begin();
        if(rit == z::ref(ed).list().end()) {
            throw z::Exception("NodeFactory", zfmt(typeSpec.name(), "Empty enum type %{s}").arg("s", z::ref(ed).name() ));
        }
        const z::Ast::VariableDefn& vref = rit->get();
        const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, typeSpec, false, false);
        z::Ast::EnumMemberExpr& typeSpecMemberExpr = unit().addNode(new z::Ast::EnumMemberExpr(name, qTypeSpec, typeSpec, vref));
        return typeSpecMemberExpr;
    }

    const z::Ast::StructDecl* sl = dynamic_cast<const z::Ast::StructDecl*>(ts);
    if(sl != 0) {
        z::Ast::Token value(name.filename(), name.row(), name.col(), "0");
        z::Ast::ConstantNullExpr& expr = aConstantNullExpr(value);
        return expr;
    }

    const z::Ast::StructDefn* sd = dynamic_cast<const z::Ast::StructDefn*>(ts);
    if(sd != 0) {
        z::Ast::StructInstanceExpr* expr = aStructInstanceExpr(name, z::ref(sd));
        return z::ref(expr);
    }

    const z::Ast::Function* fd = dynamic_cast<const z::Ast::Function*>(ts);
    if(fd != 0) {
        z::Ast::ExprList& exprList = addExprList(name);
        z::Ast::FunctionInstanceExpr* expr = aFunctionInstanceExpr(name, z::ref(fd), exprList);
        return z::ref(expr);
    }

    throw z::Exception("", zfmt(name, "No default value for type %{s}").arg("s", z::ref(ts).name() ));
}

inline const z::Ast::Expr& z::Ast::Factory::convertExprToExpectedTypeSpec(const z::Ast::Token& pos, const z::Ast::Expr& initExpr) {
    // if it is null type, do nothing
    const z::Ast::ConstantNullExpr* cne = dynamic_cast<const z::Ast::ConstantNullExpr*>(z::ptr(initExpr));
    if(cne != 0) {
        return initExpr;
    }

    // check if lhs is a pointer to rhs, if so auto-convert
    const z::Ast::TemplateDefn* ts = unit().isPointerToExprExpected(initExpr);
    if(ts) {
        z::Ast::PointerInstanceExpr* expr = aPointerInstanceExpr(pos, initExpr);
        return z::ref(expr);
    }

    const z::Ast::QualifiedTypeSpec* qts = unit().getExpectedTypeSpecIfAny();
    if(qts) {
        // if expected type is z::ptr, no checking
        const z::Ast::TemplateDefn* td = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(z::ref(qts).typeSpec()));
        if((td) && (z::ref(td).name().string() == "ptr")) {
            return initExpr;
        }

        // if expected type is a struct-decl, no checking
        const z::Ast::StructDecl* sd = dynamic_cast<const z::Ast::StructDecl*>(z::ptr(z::ref(qts).typeSpec()));
        if(sd) {
            // check if rhs typespec is a null-expr
            const z::Ast::ConstantNullExpr* cne = dynamic_cast<const z::Ast::ConstantNullExpr*>(z::ptr(initExpr));
            if(cne) {
                return initExpr;
            }
        }

        // check if rhs is a pointer to lhs, if so, auto-convert
        const z::Ast::TemplateDefn* templateDefn = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(initExpr.qTypeSpec().typeSpec()));
        if((templateDefn) && (z::ref(templateDefn).name().string() == "pointer")) {
            const z::Ast::QualifiedTypeSpec& rhsQts = z::ref(templateDefn).at(0);
            Unit::CoercionResult::T mode = Unit::CoercionResult::None;
            unit().canCoerceX(z::ref(qts), rhsQts, mode);
            if(mode == Unit::CoercionResult::Rhs) {
                const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(qts).isConst(), z::ref(qts).typeSpec(), true, false);
                z::Ast::TypecastExpr& typecastExpr = unit().addNode(new z::Ast::DynamicTypecastExpr(pos, qTypeSpec, qTypeSpec, initExpr));
                return typecastExpr;
            }
        }

        // check if initExpr can be converted to expected type, if any
        Unit::CoercionResult::T mode = Unit::CoercionResult::None;
        const z::Ast::QualifiedTypeSpec* cqts = unit().canCoerceX(z::ref(qts), initExpr.qTypeSpec(), mode);
        if(mode == Unit::CoercionResult::Lhs) {
            if(z::ptr(z::ref(cqts).typeSpec()) == z::ptr(initExpr.qTypeSpec().typeSpec())) {
                return initExpr;
            }
            const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(cqts).isConst(), z::ref(qts).typeSpec(), false, false);
            z::Ast::TypecastExpr& typecastExpr = unit().addNode(new z::Ast::StaticTypecastExpr(pos, qTypeSpec, qTypeSpec, initExpr));
            return typecastExpr;
        }
        throw z::Exception("", zfmt(pos, "Cannot convert expression from '%{f}' to '%{t}' (%{m})")
                           .arg("f", ZenlangNameGenerator().qtn(initExpr.qTypeSpec()) )
                           .arg("t", ZenlangNameGenerator().qtn(z::ref(qts)) )
                           .arg("m", mode )
                           );
    }

    return initExpr;
}

inline z::Ast::Scope& z::Ast::Factory::addScope(const z::Ast::Token& pos, const z::Ast::ScopeType::T& type) {
    z::Ast::Scope& scope = unit().addNode(new z::Ast::Scope(pos, type));
    return scope;
}

inline z::Ast::ExprList& z::Ast::Factory::addExprList(const z::Ast::Token& pos) {
    z::Ast::ExprList& exprList = unit().addNode(new z::Ast::ExprList(pos));
    return exprList;
}

inline z::Ast::TemplateDefn& z::Ast::Factory::createTemplateDefn(const z::Ast::Token& pos, const z::string& name, const z::Ast::TemplateTypePartList& list) {
    z::Ast::Token token(pos.filename(), pos.row(), pos.col(), name);
    const z::Ast::TemplateDecl& templateDecl = unit().getRootTypeSpec<z::Ast::TemplateDecl>(_module.level(), token);
    z::Ast::TemplateDefn& templateDefn = unit().addNode(new z::Ast::TemplateDefn(unit().anonymousNS(), token, z::Ast::DefinitionType::Final, templateDecl, list));
    unit().addAnonymous(templateDefn);
    return templateDefn;
}

inline const z::Ast::FunctionRetn& z::Ast::Factory::getFunctionRetn(const z::Ast::Token& pos, const z::Ast::Function& function) {
    /// \todo use FunctionBaseIterator here
    const z::Ast::Function* base = z::ptr(function);
    while(base != 0) {
        const z::Ast::ChildFunctionDefn* childFunctionDefn = dynamic_cast<const z::Ast::ChildFunctionDefn*>(base);
        if(childFunctionDefn == 0)
            break;
        base = z::ptr(z::ref(childFunctionDefn).base());
    }
    if(base != 0) {
        const z::Ast::FunctionRetn* functionRetn = z::ref(base).hasChild<const z::Ast::FunctionRetn>("_Out");
        if(functionRetn != 0) {
            return z::ref(functionRetn);
        }
    }

    throw z::Exception("NodeFactory", zfmt(pos, "Unknown function return type %{s}").arg("s", function.name() ));
}

inline const z::Ast::QualifiedTypeSpec& z::Ast::Factory::getFunctionReturnType(const z::Ast::Token& pos, const z::Ast::Function& function) {
    if(function.sig().outScope().isTuple()) {
        const z::Ast::FunctionRetn& functionRetn = getFunctionRetn(pos, function);
        z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, functionRetn, false, false);
        return qTypeSpec;
    }
    return function.sig().out().front().qTypeSpec();
}

inline z::Ast::VariableDefn& z::Ast::Factory::addVariableDefn(const z::Ast::QualifiedTypeSpec& qualifiedTypeSpec, const z::Ast::Token& name) {
    const z::Ast::Expr& initExpr = getDefaultValue(qualifiedTypeSpec.typeSpec(), name);
    z::Ast::VariableDefn& variableDef = unit().addNode(new z::Ast::VariableDefn(qualifiedTypeSpec, name, initExpr));
    return variableDef;
}

inline const z::Ast::TemplateDefn& z::Ast::Factory::getTemplateDefn(const z::Ast::Token& name, const z::Ast::Expr& expr, const z::string& cname, const z::Ast::TemplateDefn::size_type& len) {
    const z::Ast::TypeSpec& typeSpec = expr.qTypeSpec().typeSpec();
    if(typeSpec.name().string() != cname) {
        throw z::Exception("NodeFactory", zfmt(name, "Expression is not of %{c} type: %{t} (1)").arg("c", cname).arg("t", typeSpec.name() ) );
    }
    const z::Ast::TemplateDefn* templateDefn = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(typeSpec));
    if(templateDefn == 0) {
        throw z::Exception("NodeFactory", zfmt(name, "Expression is not of %{c} type: %{t} (2)").arg("c", cname).arg("t", typeSpec.name() ) );
    }
    if(z::ref(templateDefn).list().size() != len) {
        throw z::Exception("NodeFactory", zfmt(name, "Expression is not of %{c} type: %{t} (3)").arg("c", cname).arg("t", typeSpec.name() ) );
    }
    return z::ref(templateDefn);
}

inline z::Ast::RootFunctionDecl& z::Ast::Factory::addRootFunctionDecl(const z::Ast::TypeSpec& parent, const z::Ast::FunctionSig& functionSig, const z::Ast::DefinitionType::T& defType, const z::Ast::ClosureRef& cref) {
    const z::Ast::Token& name = functionSig.name();
    z::Ast::RootFunctionDecl& functionDecl = unit().addNode(new z::Ast::RootFunctionDecl(parent, name, defType, functionSig, z::ref(cref.xref), z::ref(cref.iref)));
    z::Ast::Token token1(name.filename(), name.row(), name.col(), "_Out");
    z::Ast::FunctionRetn& functionRetn = unit().addNode(new z::Ast::FunctionRetn(functionDecl, token1, functionSig.outScope()));
    functionDecl.addChild(functionRetn);
    return functionDecl;
}

inline z::Ast::ChildFunctionDecl& z::Ast::Factory::addChildFunctionDecl(const z::Ast::TypeSpec& parent, const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType, const z::Ast::TypeSpec& base, const z::Ast::ClosureRef& cref) {
    const z::Ast::Function* function = dynamic_cast<const z::Ast::Function*>(z::ptr(base));
    if(function == 0) {
        throw z::Exception("NodeFactory", zfmt(name, "Base type is not a function '%{s}'").arg("s", base.name() ));
    }

    z::Ast::ChildFunctionDecl& functionDecl = unit().addNode(new z::Ast::ChildFunctionDecl(parent, name, defType, z::ref(function).sig(), z::ref(cref.xref), z::ref(cref.iref), z::ref(function)));
    z::Ast::Token token1(name.filename(), name.row(), name.col(), "_Out");
    z::Ast::FunctionRetn& functionRetn = unit().addNode(new z::Ast::FunctionRetn(functionDecl, token1, z::ref(function).sig().outScope()));
    functionDecl.addChild(functionRetn);
    return functionDecl;
}

inline z::Ast::ValueInstanceExpr& z::Ast::Factory::getValueInstanceExpr(const z::Ast::Token& pos, const z::Ast::QualifiedTypeSpec& qTypeSpec, const z::Ast::TemplateDefn& srcTemplateDefn, const z::Ast::TemplateDefn& templateDefn, const z::Ast::ExprList& exprList) {
    const z::Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, qTypeSpec.isConst(), qTypeSpec.typeSpec(), true, false);
    z::Ast::ValueInstanceExpr& valueInstanceExpr = unit().addNode(new z::Ast::ValueInstanceExpr(pos, typeSpec, srcTemplateDefn, templateDefn, exprList));
    return valueInstanceExpr;
}

////////////////////////////////////////////////////////////
z::Ast::Factory::Factory(z::Ast::Module& module) : _module(module), _lastToken(_module.filename(), 0, 0, "") {
    z::Ast::Root& rootTypeSpec = unit().getRootNamespace(_module.level());
    unit().enterTypeSpec(rootTypeSpec);
}

z::Ast::Factory::~Factory() {
    z::Ast::Root& rootTypeSpec = unit().getRootNamespace(_module.level());
    unit().leaveTypeSpec(rootTypeSpec);
}

////////////////////////////////////////////////////////////
void z::Ast::Factory::aUnitStatementList(const z::Ast::EnterNamespaceStatement& nss) {
    z::Ast::LeaveNamespaceStatement& lns = unit().addNode(new z::Ast::LeaveNamespaceStatement(getToken(), nss));
    _module.addGlobalStatement(lns);
    unit().leaveNamespace();
}

z::Ast::Module::Level_t z::Ast::Factory::aImportStatement(const z::Ast::Token& pos, const z::Ast::AccessType::T& accessType, const z::Ast::HeaderType::T& headerType, const z::Ast::DefinitionType::T& defType, z::Ast::NamespaceList& list, z::string& filename) {
    z::Ast::ImportStatement& statement = unit().addNode(new z::Ast::ImportStatement(pos, accessType, headerType, defType, list));
    _module.addGlobalStatement(statement);

    if(statement.defType() != z::Ast::DefinitionType::Native) {
        filename = "";
        z::string sep = "";
        for(z::Ast::NamespaceList::List::const_iterator it = statement.list().begin(); it != statement.list().end(); ++it) {
            const z::Ast::Token& name = it->get().name();
            filename += sep;
            filename += name.string();
            sep = "/";
        }
        filename += ".ipp";
        return _module.level() + 1;
    }
    return 0;
}

z::Ast::NamespaceList* z::Ast::Factory::aImportNamespaceList(z::Ast::NamespaceList& list, const z::Ast::Token &name) {
    z::Ast::Namespace& ns = unit().addNode(new z::Ast::Namespace(unit().currentTypeSpec(), name));
    list.addNamespace(ns);
    return z::ptr(list);
}

z::Ast::NamespaceList* z::Ast::Factory::aImportNamespaceList(const z::Ast::Token& name) {
    z::Ast::NamespaceList& list = unit().addNode(new z::Ast::NamespaceList(name));
    return aImportNamespaceList(list, name);
}

z::Ast::EnterNamespaceStatement* z::Ast::Factory::aNamespaceStatement(const z::Ast::Token& pos, z::Ast::NamespaceList& list) {
    z::Ast::EnterNamespaceStatement& statement = unit().addNode(new z::Ast::EnterNamespaceStatement(pos, list));
    _module.addGlobalStatement(statement);
    return z::ptr(statement);
}

z::Ast::EnterNamespaceStatement* z::Ast::Factory::aNamespaceStatement() {
    z::Ast::NamespaceList& list = unit().addNode(new z::Ast::NamespaceList(getToken()));
    return aNamespaceStatement(getToken(), list);
}

inline z::Ast::Namespace& z::Ast::Factory::getUnitNamespace(const z::Ast::Token& name) {
    if(_module.level() == 0) {
        z::Ast::Namespace& ns = unit().addNode(new z::Ast::Namespace(unit().currentTypeSpec(), name));
        unit().currentTypeSpec().addChild(ns);
        return ns;
    }

    z::Ast::Namespace* cns = unit().importNS().hasChild<z::Ast::Namespace>(name.string());
    if(cns) {
        return z::ref(cns);
    }

    z::Ast::Namespace& ns = unit().addNode(new z::Ast::Namespace(unit().currentTypeSpec(), name));
    unit().currentTypeSpec().addChild(ns);
    return ns;
}

z::Ast::NamespaceList* z::Ast::Factory::aUnitNamespaceList(z::Ast::NamespaceList& list, const z::Ast::Token& name) {
    z::Ast::Namespace& ns = getUnitNamespace(name);
    unit().enterTypeSpec(ns);
    if(_module.level() == 0) {
        unit().addNamespacePart(name);
    }
    unit().addNamespace(ns);
    list.addNamespace(ns);
    return z::ptr(list);
}

z::Ast::NamespaceList* z::Ast::Factory::aUnitNamespaceList(const z::Ast::Token& name) {
    z::Ast::NamespaceList& list = unit().addNode(new z::Ast::NamespaceList(name));
    return aUnitNamespaceList(list, name);
}

z::Ast::Statement* z::Ast::Factory::aGlobalStatement(z::Ast::Statement& statement) {
    _module.addGlobalStatement(statement);
    return z::ptr(statement);
}

z::Ast::Statement* z::Ast::Factory::aGlobalTypeSpecStatement(const z::Ast::AccessType::T& accessType, z::Ast::UserDefinedTypeSpec& typeSpec){
    typeSpec.accessType(accessType);
    z::Ast::UserDefinedTypeSpecStatement* statement = aUserDefinedTypeSpecStatement(typeSpec);
    return aGlobalStatement(z::ref(statement));
}

void z::Ast::Factory::aGlobalCoerceStatement(z::Ast::CoerceList& list) {
    unit().addCoercionList(list);
}

z::Ast::CoerceList* z::Ast::Factory::aCoerceList(z::Ast::CoerceList& list, const z::Ast::TypeSpec& typeSpec) {
    list.addTypeSpec(typeSpec);
    return z::ptr(list);
}

z::Ast::CoerceList* z::Ast::Factory::aCoerceList(const z::Ast::TypeSpec& typeSpec) {
    z::Ast::CoerceList& list = unit().addNode(new z::Ast::CoerceList(typeSpec.pos()));
    list.addTypeSpec(typeSpec);
    return z::ptr(list);
}

void z::Ast::Factory::aGlobalDefaultStatement(const z::Ast::TypeSpec& typeSpec, const z::Ast::Expr& expr) {
    unit().addDefaultValue(typeSpec, expr);
}

inline void z::Ast::Factory::setDefaultDummyValue(const z::Ast::Token& name, z::Ast::TypeSpec& typeSpec) {
    z::Ast::CoerceList& list = unit().addNode(new z::Ast::CoerceList(name));
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(name, "void");
    list.addTypeSpec(qTypeSpec.typeSpec());
    list.addTypeSpec(typeSpec);
    unit().addCoercionList(list);

    z::Ast::ConstantNullExpr& expr = unit().addNode(new z::Ast::ConstantNullExpr(name, qTypeSpec));
    unit().addDefaultValue(typeSpec, expr);
}

z::Ast::TypedefDecl* z::Ast::Factory::aTypedefDecl(const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType) {
    z::Ast::TypedefDecl& typedefDefn = unit().addNode(new z::Ast::TypedefDecl(unit().currentTypeSpec(), name, defType));
    unit().currentTypeSpec().addChild(typedefDefn);
    // if this is a native-typedef-decl, add dummy coercion and default-value = (void)0
    // unless this is the native-typedef-decl for void itself
    if((defType == z::Ast::DefinitionType::Native) && (name.string() != "void")) {
        setDefaultDummyValue(name, typedefDefn);
    }
    return z::ptr(typedefDefn);
}

z::Ast::TypedefDefn* z::Ast::Factory::aTypedefDefn(const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType, const z::Ast::QualifiedTypeSpec& qTypeSpec) {
    z::Ast::TypedefDefn& typedefDefn = unit().addNode(new z::Ast::TypedefDefn(unit().currentTypeSpec(), name, defType, qTypeSpec));
    unit().currentTypeSpec().addChild(typedefDefn);
    return z::ptr(typedefDefn);
}

z::Ast::TemplatePartList* z::Ast::Factory::aTemplatePartList(z::Ast::TemplatePartList& list, const z::Ast::Token& name) {
    list.addPart(name);
    return z::ptr(list);
}

z::Ast::TemplatePartList* z::Ast::Factory::aTemplatePartList(const z::Ast::Token& name) {
    z::Ast::TemplatePartList& list = unit().addNode(new z::Ast::TemplatePartList(name));
    list.addPart(name);
    return z::ptr(list);
}

z::Ast::TemplateDecl* z::Ast::Factory::aTemplateDecl(const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType, const z::Ast::TemplatePartList& list) {
    z::Ast::TemplateDecl& templateDefn = unit().addNode(new z::Ast::TemplateDecl(unit().currentTypeSpec(), name, defType, list));
    unit().currentTypeSpec().addChild(templateDefn);
    return z::ptr(templateDefn);
}

z::Ast::EnumDefn* z::Ast::Factory::aEnumDefn(const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType, const z::Ast::Scope& list) {
    z::Ast::EnumDefn& enumDefn = unit().addNode(new z::Ast::EnumDefn(unit().currentTypeSpec(), name, defType, list));
    unit().currentTypeSpec().addChild(enumDefn);
    return z::ptr(enumDefn);
}

z::Ast::EnumDecl* z::Ast::Factory::aEnumDecl(const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType) {
    z::Ast::EnumDecl& enumDecl = unit().addNode(new z::Ast::EnumDecl(unit().currentTypeSpec(), name, defType));
    unit().currentTypeSpec().addChild(enumDecl);
    return z::ptr(enumDecl);
}

z::Ast::Scope* z::Ast::Factory::aEnumMemberDefnList(z::Ast::Scope& list, const z::Ast::VariableDefn& variableDefn) {
    list.addVariableDef(variableDefn);
    return z::ptr(list);
}

z::Ast::Scope* z::Ast::Factory::aEnumMemberDefnListEmpty(const z::Ast::Token& pos) {
    z::Ast::Scope& list = addScope(pos, z::Ast::ScopeType::Member);
    return z::ptr(list);
}

z::Ast::VariableDefn* z::Ast::Factory::aEnumMemberDefn(const z::Ast::Token& name) {
    z::Ast::Token value(name.filename(), name.row(), name.col(), "#");
    const z::Ast::ConstantIntExpr& initExpr = aConstantIntExpr(value);
    z::Ast::VariableDefn& variableDefn = unit().addNode(new z::Ast::VariableDefn(initExpr.qTypeSpec(), name, initExpr));
    return z::ptr(variableDefn);
}

z::Ast::VariableDefn* z::Ast::Factory::aEnumMemberDefn(const z::Ast::Token& name, const z::Ast::Expr& initExpr) {
    z::Ast::VariableDefn& variableDefn = unit().addNode(new z::Ast::VariableDefn(initExpr.qTypeSpec(), name, initExpr));
    return z::ptr(variableDefn);
}

z::Ast::StructDecl* z::Ast::Factory::aStructDecl(const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType) {
    z::Ast::StructDecl& structDecl = unit().addNode(new z::Ast::StructDecl(unit().currentTypeSpec(), name, defType));
    unit().currentTypeSpec().addChild(structDecl);

    // if this is a native struct decl, add int=>struct-decl coercion and default-value = 0
    if(defType == z::Ast::DefinitionType::Native) {
        setDefaultDummyValue(name, structDecl);
    }
    return z::ptr(structDecl);
}

z::Ast::RootStructDefn* z::Ast::Factory::aLeaveRootStructDefn(z::Ast::RootStructDefn& structDefn) {
    unit().leaveTypeSpec(structDefn);
    z::Ast::StructInitStatement& statement = unit().addNode(new z::Ast::StructInitStatement(structDefn.pos(), structDefn));
    structDefn.block().addStatement(statement);
    return z::ptr(structDefn);
}

z::Ast::ChildStructDefn* z::Ast::Factory::aLeaveChildStructDefn(z::Ast::ChildStructDefn& structDefn) {
    unit().leaveTypeSpec(structDefn);
    z::Ast::StructInitStatement& statement = unit().addNode(new z::Ast::StructInitStatement(structDefn.pos(), structDefn));
    structDefn.block().addStatement(statement);
    return z::ptr(structDefn);
}

z::Ast::RootStructDefn* z::Ast::Factory::aEnterRootStructDefn(const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType) {
    z::Ast::Scope& list = addScope(name, z::Ast::ScopeType::Member);
    z::Ast::CompoundStatement& block = unit().addNode(new z::Ast::CompoundStatement(name));
    z::Ast::RootStructDefn& structDefn = unit().addNode(new z::Ast::RootStructDefn(unit().currentTypeSpec(), name, defType, list, block));
    unit().currentTypeSpec().addChild(structDefn);
    unit().enterTypeSpec(structDefn);
    return z::ptr(structDefn);
}

z::Ast::ChildStructDefn* z::Ast::Factory::aEnterChildStructDefn(const z::Ast::Token& name, const z::Ast::StructDefn& base, const z::Ast::DefinitionType::T& defType) {
    //@if(base.defType() == z::Ast::DefinitionType::Final) {
    //    throw z::Exception("NodeFactory", zfmt(name, "Base struct is not abstract '%{s}'").arg("s", base.name() ));
    //}
    z::Ast::Scope& list = addScope(name, z::Ast::ScopeType::Member);
    z::Ast::CompoundStatement& block = unit().addNode(new z::Ast::CompoundStatement(name));
    z::Ast::ChildStructDefn& structDefn = unit().addNode(new z::Ast::ChildStructDefn(unit().currentTypeSpec(), base, name, defType, list, block));
    unit().currentTypeSpec().addChild(structDefn);
    unit().enterTypeSpec(structDefn);
    return z::ptr(structDefn);
}

void z::Ast::Factory::aStructMemberVariableDefn(const z::Ast::VariableDefn& vdef) {
    z::Ast::StructDefn& sd = unit().getCurrentStructDefn(vdef.name());
    sd.addVariable(vdef);
    z::Ast::StructMemberVariableStatement& statement = unit().addNode(new z::Ast::StructMemberVariableStatement(vdef.pos(), sd, vdef));
    sd.block().addStatement(statement);
}

void z::Ast::Factory::aStructMemberTypeDefn(z::Ast::UserDefinedTypeSpec& typeSpec) {
    z::Ast::StructDefn& sd = unit().getCurrentStructDefn(typeSpec.name());
    typeSpec.accessType(z::Ast::AccessType::Parent);
    z::Ast::Statement* statement = aUserDefinedTypeSpecStatement(typeSpec);
    sd.block().addStatement(z::ref(statement));
}

void z::Ast::Factory::aStructMemberPropertyDefn(z::Ast::PropertyDecl& typeSpec) {
    aStructMemberTypeDefn(typeSpec);
    z::Ast::StructDefn& sd = unit().getCurrentStructDefn(typeSpec.name());
    sd.addProperty(typeSpec);
}

z::Ast::PropertyDeclRW* z::Ast::Factory::aStructPropertyDeclRW(const z::Ast::Token& pos, const z::Ast::QualifiedTypeSpec& propertyType, const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType) {
    unused(pos);
    z::Ast::PropertyDeclRW& structPropertyDecl = unit().addNode(new z::Ast::PropertyDeclRW(unit().currentTypeSpec(), name, defType, propertyType));
    unit().currentTypeSpec().addChild(structPropertyDecl);
    return z::ptr(structPropertyDecl);
}

z::Ast::PropertyDeclRO* z::Ast::Factory::aStructPropertyDeclRO(const z::Ast::Token& pos, const z::Ast::QualifiedTypeSpec& propertyType, const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType) {
    unused(pos);
    z::Ast::PropertyDeclRO& structPropertyDecl = unit().addNode(new z::Ast::PropertyDeclRO(unit().currentTypeSpec(), name, defType, propertyType));
    unit().currentTypeSpec().addChild(structPropertyDecl);
    return z::ptr(structPropertyDecl);
}

z::Ast::RoutineDecl* z::Ast::Factory::aRoutineDecl(const z::Ast::QualifiedTypeSpec& outType, const z::Ast::Token& name, z::Ast::Scope& in, const z::Ast::DefinitionType::T& defType) {
    z::Ast::RoutineDecl& routineDecl = unit().addNode(new z::Ast::RoutineDecl(unit().currentTypeSpec(), outType, name, in, defType));
    unit().currentTypeSpec().addChild(routineDecl);
    return z::ptr(routineDecl);
}

z::Ast::RoutineDecl* z::Ast::Factory::aVarArgRoutineDecl(const z::Ast::QualifiedTypeSpec& outType, const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType) {
    z::Ast::Scope& in = addScope(name, z::Ast::ScopeType::VarArg);
    z::Ast::RoutineDecl& routineDecl = unit().addNode(new z::Ast::RoutineDecl(unit().currentTypeSpec(), outType, name, in, defType));
    unit().currentTypeSpec().addChild(routineDecl);
    return z::ptr(routineDecl);
}

z::Ast::RoutineDefn* z::Ast::Factory::aRoutineDefn(z::Ast::RoutineDefn& routineDefn, const z::Ast::CompoundStatement& block) {
    routineDefn.setBlock(block);
    unit().leaveScope(routineDefn.inScope());
    unit().leaveTypeSpec(routineDefn);
    unit().addBody(unit().addNode(new z::Ast::RoutineBody(block.pos(), routineDefn, block)));
    return z::ptr(routineDefn);
}

z::Ast::RoutineDefn* z::Ast::Factory::aEnterRoutineDefn(const z::Ast::QualifiedTypeSpec& outType, const z::Ast::Token& name, z::Ast::Scope& in, const z::Ast::DefinitionType::T& defType) {
    z::Ast::RoutineDefn& routineDefn = unit().addNode(new z::Ast::RoutineDefn(unit().currentTypeSpec(), outType, name, in, defType));
    unit().currentTypeSpec().addChild(routineDefn);
    unit().enterScope(in);
    unit().enterTypeSpec(routineDefn);
    return z::ptr(routineDefn);
}

z::Ast::RootFunctionDecl* z::Ast::Factory::aRootFunctionDecl(const z::Ast::FunctionSig& functionSig, const z::Ast::DefinitionType::T& defType, const z::Ast::ClosureRef& cref) {
    z::Ast::RootFunctionDecl& functionDecl = addRootFunctionDecl(unit().currentTypeSpec(), functionSig, defType, cref);
    unit().currentTypeSpec().addChild(functionDecl);
    return z::ptr(functionDecl);
}

z::Ast::ChildFunctionDecl* z::Ast::Factory::aChildFunctionDecl(const z::Ast::TypeSpec& base, const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType, const z::Ast::ClosureRef& cref) {
    z::Ast::ChildFunctionDecl& functionDecl = addChildFunctionDecl(unit().currentTypeSpec(), name, defType, base, cref);
    unit().currentTypeSpec().addChild(functionDecl);
    return z::ptr(functionDecl);
}

z::Ast::RootFunctionDefn* z::Ast::Factory::aRootFunctionDefn(z::Ast::RootFunctionDefn& functionDefn, const z::Ast::CompoundStatement& block) {
    functionDefn.setBlock(block);
    unit().leaveScope(functionDefn.sig().inScope());
    unit().leaveScope(functionDefn.irefScope());
    unit().leaveScope(functionDefn.xrefScope());
    unit().leaveTypeSpec(functionDefn);
    unit().addBody(unit().addNode(new z::Ast::FunctionBody(block.pos(), functionDefn, block)));
    return z::ptr(functionDefn);
}

z::Ast::RootFunctionDefn* z::Ast::Factory::aEnterRootFunctionDefn(const z::Ast::FunctionSig& functionSig, const z::Ast::DefinitionType::T& defType, const z::Ast::ClosureRef& cref) {
    const z::Ast::Token& name = functionSig.name();
    z::Ast::RootFunctionDefn& functionDefn = unit().addNode(new z::Ast::RootFunctionDefn(unit().currentTypeSpec(), name, defType, functionSig, z::ref(cref.xref), z::ref(cref.iref)));
    unit().currentTypeSpec().addChild(functionDefn);
    unit().enterScope(functionDefn.xrefScope());
    unit().enterScope(functionDefn.irefScope());
    unit().enterScope(functionSig.inScope());
    unit().enterTypeSpec(functionDefn);

    z::Ast::Token token1(name.filename(), name.row(), name.col(), "_Out");
    z::Ast::FunctionRetn& functionRetn = unit().addNode(new z::Ast::FunctionRetn(functionDefn, token1, functionSig.outScope()));
    functionDefn.addChild(functionRetn);

    return z::ptr(functionDefn);
}

z::Ast::ChildFunctionDefn* z::Ast::Factory::aChildFunctionDefn(z::Ast::ChildFunctionDefn& functionDefn, const z::Ast::CompoundStatement& block) {
    functionDefn.setBlock(block);
    unit().leaveScope(functionDefn.sig().inScope());
    unit().leaveScope(functionDefn.irefScope());
    unit().leaveScope(functionDefn.xrefScope());
    unit().leaveTypeSpec(functionDefn);
    unit().addBody(unit().addNode(new z::Ast::FunctionBody(block.pos(), functionDefn, block)));
    return z::ptr(functionDefn);
}

inline z::Ast::ChildFunctionDefn& z::Ast::Factory::createChildFunctionDefn(z::Ast::TypeSpec& parent, const z::Ast::Function& base, const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType, const z::Ast::ClosureRef& cref) {
    if(base.defType() == z::Ast::DefinitionType::Final) {
        throw z::Exception("NodeFactory", zfmt(name, "Base struct is not abstract '%{s}'").arg("s", base.name() ));
    }

    z::Ast::ChildFunctionDefn& functionDefn = unit().addNode(new z::Ast::ChildFunctionDefn(parent, name, defType, base.sig(), z::ref(cref.xref), z::ref(cref.iref), base));
    parent.addChild(functionDefn);
    unit().enterScope(functionDefn.xrefScope());
    unit().enterScope(functionDefn.irefScope());
    unit().enterScope(base.sig().inScope());
    unit().enterTypeSpec(functionDefn);
    return functionDefn;
}

z::Ast::ChildFunctionDefn* z::Ast::Factory::aEnterChildFunctionDefn(const z::Ast::TypeSpec& base, const z::Ast::Token& name, const z::Ast::DefinitionType::T& defType, const z::Ast::ClosureRef& cref) {
    const z::Ast::Function* function = dynamic_cast<const z::Ast::Function*>(z::ptr(base));
    if(function == 0) {
        throw z::Exception("NodeFactory", zfmt(name, "Base type is not a function '%{s}'").arg("s", base.name() ));
    }
    z::Ast::ChildFunctionDefn& functionDefn = createChildFunctionDefn(unit().currentTypeSpec(), z::ref(function), name, defType, cref);
    return z::ptr(functionDefn);
}

z::Ast::EventDecl* z::Ast::Factory::aEventDecl(const z::Ast::Token& pos, const z::Ast::VariableDefn& in, const z::Ast::DefinitionType::T& eventDefType, const z::Ast::FunctionSig& functionSig, const z::Ast::DefinitionType::T& handlerDefType) {
    const z::Ast::Token& name = functionSig.name();

    z::Ast::Token eventName(pos.filename(), pos.row(), pos.col(), name.string());
    z::Ast::EventDecl& eventDef = unit().addNode(new z::Ast::EventDecl(unit().currentTypeSpec(), eventName, functionSig, in, eventDefType));
    unit().currentTypeSpec().addChild(eventDef);

    z::Ast::Token handlerName(pos.filename(), pos.row(), pos.col(), "Handler");
    z::Ast::FunctionSig* handlerSig = aFunctionSig(functionSig.outScope(), handlerName, functionSig.inScope());
    z::Ast::Scope& xref = addScope(getToken(), z::Ast::ScopeType::XRef);
    z::Ast::Scope& iref = addScope(getToken(), z::Ast::ScopeType::IRef);
    z::Ast::ClosureRef cref = aClosureList(xref, iref);
    z::Ast::RootFunctionDecl& funDecl = addRootFunctionDecl(eventDef, z::ref(handlerSig), handlerDefType, cref);
    eventDef.setHandler(funDecl);

    return z::ptr(eventDef);
}

z::Ast::FunctionSig* z::Ast::Factory::aFunctionSig(const z::Ast::Scope& out, const z::Ast::Token& name, z::Ast::Scope& in) {
    z::Ast::FunctionSig& functionSig = unit().addNode(new z::Ast::FunctionSig(out, name, in));
    return z::ptr(functionSig);
}

z::Ast::FunctionSig* z::Ast::Factory::aFunctionSig(const z::Ast::QualifiedTypeSpec& typeSpec, const z::Ast::Token& name, z::Ast::Scope& in) {
    z::Ast::Scope& out = addScope(name, z::Ast::ScopeType::Param);
    out.isTuple(false);

    z::Ast::Token oname(name.filename(), name.row(), name.col(), "_out");
    z::Ast::VariableDefn& vdef = addVariableDefn(typeSpec, oname);
    out.addVariableDef(vdef);

    return aFunctionSig(out, name, in);
}

z::Ast::ClosureRef z::Ast::Factory::aClosureList(z::Ast::Scope& xref, z::Ast::Scope& iref) {
    assert(xref.type() == z::Ast::ScopeType::XRef);
    assert(iref.type() == z::Ast::ScopeType::IRef);
    z::Ast::ClosureRef cref;
    cref.xref = z::ptr(xref);
    cref.iref = z::ptr(iref);
    return cref;
}

z::Ast::ClosureRef z::Ast::Factory::aClosureList(z::Ast::Scope& xref) {
    z::Ast::Scope& iref = addScope(getToken(), z::Ast::ScopeType::IRef);
    return aClosureList(xref, iref);
}

z::Ast::ClosureRef z::Ast::Factory::aClosureList() {
    z::Ast::Scope& xref = addScope(getToken(), z::Ast::ScopeType::XRef);
    z::Ast::Scope& iref = addScope(getToken(), z::Ast::ScopeType::IRef);
    return aClosureList(xref, iref);
}

z::Ast::Scope* z::Ast::Factory::aInParamsList(z::Ast::Scope& scope) {
    return z::ptr(scope);
}

z::Ast::Scope* z::Ast::Factory::aParamsList(z::Ast::Scope& scope) {
    return z::ptr(scope);
}

z::Ast::Scope* z::Ast::Factory::aParamsList(z::Ast::Scope& scope, const z::Ast::Scope& posParam) {
    scope.posParam(posParam);
    return aParamsList(scope);
}

z::Ast::Scope* z::Ast::Factory::aParam(z::Ast::Scope& list, const z::Ast::VariableDefn& variableDefn) {
    list.addVariableDef(variableDefn);
    return z::ptr(list);
}

z::Ast::Scope* z::Ast::Factory::aParam(const z::Ast::VariableDefn& variableDefn, const ScopeType::T& type) {
    z::Ast::Scope& list = addScope(variableDefn.pos(), type);
    return aParam(list, variableDefn);
}

z::Ast::Scope* z::Ast::Factory::aParam(const z::Ast::ScopeType::T& type) {
    z::Ast::Scope& list = addScope(getToken(), type);
    return z::ptr(list);
}

z::Ast::VariableDefn* z::Ast::Factory::aVariableDefn(const z::Ast::QualifiedTypeSpec& qualifiedTypeSpec, const z::Ast::Token& name, const z::Ast::Expr& initExpr) {
    const z::Ast::Expr& expr = convertExprToExpectedTypeSpec(name, initExpr);
    z::Ast::VariableDefn& variableDef = unit().addNode(new z::Ast::VariableDefn(qualifiedTypeSpec, name, expr));
    unit().popExpectedTypeSpecOrAuto(name, Unit::ExpectedTypeSpec::etAssignment);
    return z::ptr(variableDef);
}

z::Ast::VariableDefn* z::Ast::Factory::aVariableDefn(const z::Ast::QualifiedTypeSpec& qualifiedTypeSpec, const z::Ast::Token& name) {
    const z::Ast::Expr& initExpr = getDefaultValue(qualifiedTypeSpec.typeSpec(), name);
    return aVariableDefn(qualifiedTypeSpec, name, initExpr);
}

z::Ast::VariableDefn* z::Ast::Factory::aVariableDefn(const z::Ast::Token& name, const z::Ast::Expr& initExpr) {
    const z::Ast::QualifiedTypeSpec& qualifiedTypeSpec = initExpr.qTypeSpec();
    return aVariableDefn(qualifiedTypeSpec, name, initExpr);
}

const z::Ast::QualifiedTypeSpec* z::Ast::Factory::aQualifiedVariableDefn(const z::Ast::QualifiedTypeSpec& qTypeSpec) {
    unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etAssignment, qTypeSpec);
    return z::ptr(qTypeSpec);
}

void z::Ast::Factory::aAutoQualifiedVariableDefn() {
    unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etAuto);
}

z::Ast::QualifiedTypeSpec* z::Ast::Factory::aQualifiedTypeSpec(const z::Ast::Token& pos, const bool& isConst, const z::Ast::TypeSpec& typeSpec, const bool& isRef, const bool& isStrong) {
    z::Ast::QualifiedTypeSpec& qualifiedTypeSpec = addQualifiedTypeSpec(pos, isConst, typeSpec, isRef, isStrong);
    return z::ptr(qualifiedTypeSpec);
}

z::Ast::QualifiedTypeSpec* z::Ast::Factory::aQualifiedTypeSpec(const bool& isConst, const z::Ast::TypeSpec& typeSpec, const bool& isRef, const bool& isStrong) {
    return aQualifiedTypeSpec(getToken(), isConst, typeSpec, isRef, isStrong);
}

const z::Ast::TemplateDecl* z::Ast::Factory::aTemplateTypeSpec(const z::Ast::TypeSpec& parent, const z::Ast::Token& name) {
    return unit().setCurrentChildTypeRef<z::Ast::TemplateDecl>(parent, name, "template");
}

const z::Ast::TemplateDecl* z::Ast::Factory::aTemplateTypeSpec(const z::Ast::Token& name) {
    return unit().setCurrentRootTypeRef<z::Ast::TemplateDecl>(_module.level(), name);
}

const z::Ast::TemplateDecl* z::Ast::Factory::aTemplateTypeSpec(const z::Ast::TemplateDecl& templateDecl) {
    return unit().resetCurrentTypeRef<z::Ast::TemplateDecl>(templateDecl);
}

const z::Ast::StructDefn* z::Ast::Factory::aStructTypeSpec(const z::Ast::TypeSpec& parent, const z::Ast::Token& name) {
    return unit().setCurrentChildTypeRef<z::Ast::StructDefn>(parent, name, "struct");
}

const z::Ast::StructDefn* z::Ast::Factory::aStructTypeSpec(const z::Ast::Token& name) {
    return unit().setCurrentRootTypeRef<z::Ast::StructDefn>(_module.level(), name);
}

const z::Ast::StructDefn* z::Ast::Factory::aStructTypeSpec(const z::Ast::StructDefn& structDefn) {
    return unit().resetCurrentTypeRef<z::Ast::StructDefn>(structDefn);
}

const z::Ast::Routine* z::Ast::Factory::aRoutineTypeSpec(const z::Ast::TypeSpec& parent, const z::Ast::Token& name) {
    return unit().setCurrentChildTypeRef<z::Ast::Routine>(parent, name, "routine");
}

const z::Ast::Routine* z::Ast::Factory::aRoutineTypeSpec(const z::Ast::Token& name) {
    return unit().setCurrentRootTypeRef<z::Ast::Routine>(_module.level(), name);
}

const z::Ast::Routine* z::Ast::Factory::aRoutineTypeSpec(const z::Ast::Routine& routine) {
    return unit().resetCurrentTypeRef<z::Ast::Routine>(routine);
}

const z::Ast::Function* z::Ast::Factory::aFunctionTypeSpec(const z::Ast::TypeSpec& parent, const z::Ast::Token& name) {
    return unit().setCurrentChildTypeRef<z::Ast::Function>(parent, name, "function");
}

const z::Ast::Function* z::Ast::Factory::aFunctionTypeSpec(const z::Ast::Token& name) {
    return unit().setCurrentRootTypeRef<z::Ast::Function>(_module.level(), name);
}

const z::Ast::Function* z::Ast::Factory::aFunctionTypeSpec(const z::Ast::Function& function) {
    return unit().resetCurrentTypeRef<z::Ast::Function>(function);
}

const z::Ast::EventDecl* z::Ast::Factory::aEventTypeSpec(const z::Ast::TypeSpec& parent, const z::Ast::Token& name) {
    return unit().setCurrentChildTypeRef<z::Ast::EventDecl>(parent, name, "event");
}

const z::Ast::EventDecl* z::Ast::Factory::aEventTypeSpec(const z::Ast::Token& name) {
    return unit().setCurrentRootTypeRef<z::Ast::EventDecl>(_module.level(), name);
}

const z::Ast::EventDecl* z::Ast::Factory::aEventTypeSpec(const z::Ast::EventDecl& event) {
    return unit().resetCurrentTypeRef<z::Ast::EventDecl>(event);
}

const z::Ast::TypeSpec* z::Ast::Factory::aOtherTypeSpec(const z::Ast::TypeSpec& parent, const z::Ast::Token& name) {
    return unit().setCurrentChildTypeRef<z::Ast::TypeSpec>(parent, name, "parent");
}

const z::Ast::TypeSpec* z::Ast::Factory::aOtherTypeSpec(const z::Ast::Token& name) {
    return unit().setCurrentRootTypeRef<z::Ast::TypeSpec>(_module.level(), name);
}

const z::Ast::TypeSpec* z::Ast::Factory::aTypeSpec(const z::Ast::TypeSpec& TypeSpec) {
    return unit().resetCurrentTypeRef<z::Ast::TypeSpec>(TypeSpec);
}

const z::Ast::TemplateDefn* z::Ast::Factory::aTemplateDefnTypeSpec(const z::Ast::TemplateDecl& typeSpec, const z::Ast::TemplateTypePartList& list) {
    z::Ast::TemplateDefn& templateDefn = unit().addNode(new z::Ast::TemplateDefn(unit().anonymousNS(), typeSpec.name(), z::Ast::DefinitionType::Final, typeSpec, list));
    unit().addAnonymous(templateDefn);
    return z::ptr(templateDefn);
}

z::Ast::TemplateTypePartList* z::Ast::Factory::aTemplateTypePartList(z::Ast::TemplateTypePartList& list, const z::Ast::QualifiedTypeSpec& qTypeSpec) {
    list.addType(qTypeSpec);
    return z::ptr(list);
}

z::Ast::TemplateTypePartList* z::Ast::Factory::aTemplateTypePartList(const z::Ast::QualifiedTypeSpec& qTypeSpec) {
    z::Ast::TemplateTypePartList& list = unit().addNode(new z::Ast::TemplateTypePartList(getToken()));
    return aTemplateTypePartList(list, qTypeSpec);
}

z::Ast::UserDefinedTypeSpecStatement* z::Ast::Factory::aUserDefinedTypeSpecStatement(const z::Ast::UserDefinedTypeSpec& typeSpec) {
    z::Ast::UserDefinedTypeSpecStatement& userDefinedTypeSpecStatement = unit().addNode(new z::Ast::UserDefinedTypeSpecStatement(typeSpec.pos(), typeSpec));
    return z::ptr(userDefinedTypeSpecStatement);
}

z::Ast::EmptyStatement* z::Ast::Factory::aEmptyStatement(const z::Ast::Token& pos) {
    z::Ast::EmptyStatement& emptyStatement = unit().addNode(new z::Ast::EmptyStatement(pos));
    return z::ptr(emptyStatement);
}

z::Ast::AutoStatement* z::Ast::Factory::aAutoStatement(const z::Ast::VariableDefn& defn) {
    z::Ast::AutoStatement& defnStatement = unit().addNode(new z::Ast::AutoStatement(defn.pos(), defn));
    unit().currentScope().addVariableDef(defn);
    return z::ptr(defnStatement);
}

z::Ast::ExprStatement* z::Ast::Factory::aExprStatement(const z::Ast::Expr& expr) {
    z::Ast::ExprStatement& exprStatement = unit().addNode(new z::Ast::ExprStatement(expr.pos(), expr));
    return z::ptr(exprStatement);
}

z::Ast::PrintStatement* z::Ast::Factory::aPrintStatement(const z::Ast::Token& pos, const z::Ast::Expr& expr) {
    z::Ast::PrintStatement& printStatement = unit().addNode(new z::Ast::PrintStatement(pos, expr));
    return z::ptr(printStatement);
}

z::Ast::IfStatement* z::Ast::Factory::aIfStatement(const z::Ast::Token& pos, const z::Ast::Expr& expr, const z::Ast::CompoundStatement& tblock) {
    z::Ast::IfStatement& ifStatement = unit().addNode(new z::Ast::IfStatement(pos, expr, tblock));
    return z::ptr(ifStatement);
}

z::Ast::IfElseStatement* z::Ast::Factory::aIfElseStatement(const z::Ast::Token& pos, const z::Ast::Expr& expr, const z::Ast::CompoundStatement& tblock, const z::Ast::CompoundStatement& fblock) {
    z::Ast::IfElseStatement& ifElseStatement = unit().addNode(new z::Ast::IfElseStatement(pos, expr, tblock, fblock));
    return z::ptr(ifElseStatement);
}

z::Ast::WhileStatement* z::Ast::Factory::aWhileStatement(const z::Ast::Token& pos, const z::Ast::Expr& expr, const z::Ast::CompoundStatement& block) {
    z::Ast::WhileStatement& whileStatement = unit().addNode(new z::Ast::WhileStatement(pos, expr, block));
    return z::ptr(whileStatement);
}

z::Ast::DoWhileStatement* z::Ast::Factory::aDoWhileStatement(const z::Ast::Token& pos, const z::Ast::Expr& expr, const z::Ast::CompoundStatement& block) {
    z::Ast::DoWhileStatement& doWhileStatement = unit().addNode(new z::Ast::DoWhileStatement(pos, expr, block));
    return z::ptr(doWhileStatement);
}

z::Ast::ForStatement* z::Ast::Factory::aForStatement(const z::Ast::Token& pos, const z::Ast::Expr& init, const z::Ast::Expr& expr, const z::Ast::Expr& incr, const z::Ast::CompoundStatement& block) {
    z::Ast::ForExprStatement& forStatement = unit().addNode(new z::Ast::ForExprStatement(pos, init, expr, incr, block));
    return z::ptr(forStatement);
}

z::Ast::ForStatement* z::Ast::Factory::aForStatement(const z::Ast::Token& pos, const z::Ast::VariableDefn& init, const z::Ast::Expr& expr, const z::Ast::Expr& incr, const z::Ast::CompoundStatement& block) {
    z::Ast::ForInitStatement& forStatement = unit().addNode(new z::Ast::ForInitStatement(pos, init, expr, incr, block));
    unit().leaveScope();
    return z::ptr(forStatement);
}

const z::Ast::VariableDefn* z::Ast::Factory::aEnterForInit(const z::Ast::VariableDefn& init) {
    z::Ast::Scope& scope = addScope(init.pos(), z::Ast::ScopeType::Local);
    scope.addVariableDef(init);
    unit().enterScope(scope);
    return z::ptr(init);
}

z::Ast::ForeachStatement* z::Ast::Factory::aForeachStatement(z::Ast::ForeachStatement& statement, const z::Ast::CompoundStatement& block) {
    statement.setBlock(block);
    unit().leaveScope();
    return z::ptr(statement);
}

z::Ast::ForeachStatement* z::Ast::Factory::aEnterForeachInit(const z::Ast::Token& valName, const z::Ast::Expr& expr) {
    if(ZenlangNameGenerator().tn(expr.qTypeSpec().typeSpec()) == "string") {
        const z::Ast::QualifiedTypeSpec& valTypeSpec = getQualifiedTypeSpec(valName, "char");
        const z::Ast::VariableDefn& valDef = addVariableDefn(valTypeSpec, valName);
        z::Ast::Scope& scope = addScope(valName, z::Ast::ScopeType::Local);
        scope.addVariableDef(valDef);
        unit().enterScope(scope);
        z::Ast::ForeachStringStatement& foreachStatement = unit().addNode(new z::Ast::ForeachStringStatement(valName, valDef, expr));
        return z::ptr(foreachStatement);
    }

    const z::Ast::TemplateDefn& templateDefn = getTemplateDefn(valName, expr, "list", 1);
    const z::Ast::QualifiedTypeSpec& valTypeSpec = addQualifiedTypeSpec(valName, expr.qTypeSpec().isConst(), templateDefn.at(0).typeSpec(), true, false);
    const z::Ast::VariableDefn& valDef = addVariableDefn(valTypeSpec, valName);
    z::Ast::Scope& scope = addScope(valName, z::Ast::ScopeType::Local);
    scope.addVariableDef(valDef);
    unit().enterScope(scope);
    z::Ast::ForeachListStatement& foreachStatement = unit().addNode(new z::Ast::ForeachListStatement(valName, valDef, expr));
    return z::ptr(foreachStatement);
}

z::Ast::ForeachDictStatement* z::Ast::Factory::aEnterForeachInit(const z::Ast::Token& keyName, const z::Ast::Token& valName, const z::Ast::Expr& expr) {
    const z::Ast::TemplateDefn& templateDefn = getTemplateDefn(valName, expr, "dict", 2);
    const z::Ast::QualifiedTypeSpec& keyTypeSpec = addQualifiedTypeSpec(keyName, true, templateDefn.at(0).typeSpec(), true, false);
    const z::Ast::QualifiedTypeSpec& valTypeSpec = addQualifiedTypeSpec(keyName, expr.qTypeSpec().isConst(), templateDefn.at(1).typeSpec(), true, false);
    const z::Ast::VariableDefn& keyDef = addVariableDefn(keyTypeSpec, keyName);
    const z::Ast::VariableDefn& valDef = addVariableDefn(valTypeSpec, valName);
    z::Ast::Scope& scope = addScope(keyName, z::Ast::ScopeType::Local);
    scope.addVariableDef(keyDef);
    scope.addVariableDef(valDef);
    unit().enterScope(scope);

    z::Ast::ForeachDictStatement& foreachStatement = unit().addNode(new z::Ast::ForeachDictStatement(keyName, keyDef, valDef, expr));
    return z::ptr(foreachStatement);
}

z::Ast::SwitchValueStatement* z::Ast::Factory::aSwitchStatement(const z::Ast::Token& pos, const z::Ast::Expr& expr, const z::Ast::CompoundStatement& list) {
    z::Ast::SwitchValueStatement& switchStatement = unit().addNode(new z::Ast::SwitchValueStatement(pos, expr, list));
    return z::ptr(switchStatement);
}

z::Ast::SwitchExprStatement* z::Ast::Factory::aSwitchStatement(const z::Ast::Token& pos, const z::Ast::CompoundStatement& list) {
    z::Ast::SwitchExprStatement& switchStatement = unit().addNode(new z::Ast::SwitchExprStatement(pos, list));
    return z::ptr(switchStatement);
}

z::Ast::CompoundStatement* z::Ast::Factory::aCaseList(z::Ast::CompoundStatement& list, const z::Ast::CaseStatement& stmt) {
    list.addStatement(stmt);
    return z::ptr(list);
}

z::Ast::CompoundStatement* z::Ast::Factory::aCaseList(const z::Ast::CaseStatement& stmt) {
    z::Ast::CompoundStatement& list = unit().addNode(new z::Ast::CompoundStatement(stmt.pos()));
    list.addStatement(stmt);
    return z::ptr(list);
}

z::Ast::CaseStatement* z::Ast::Factory::aCaseStatement(const z::Ast::Token& pos, const z::Ast::Expr& expr, const z::Ast::CompoundStatement& block) {
    z::Ast::CaseExprStatement& caseStatement = unit().addNode(new z::Ast::CaseExprStatement(pos, expr, block));
    return z::ptr(caseStatement);
}

z::Ast::CaseStatement* z::Ast::Factory::aCaseStatement(const z::Ast::Token& pos, const z::Ast::CompoundStatement& block) {
    z::Ast::CaseDefaultStatement& caseStatement = unit().addNode(new z::Ast::CaseDefaultStatement(pos, block));
    return z::ptr(caseStatement);
}

z::Ast::BreakStatement* z::Ast::Factory::aBreakStatement(const z::Ast::Token& pos) {
    z::Ast::BreakStatement& breakStatement = unit().addNode(new z::Ast::BreakStatement(pos));
    return z::ptr(breakStatement);
}

z::Ast::ContinueStatement* z::Ast::Factory::aContinueStatement(const z::Ast::Token& pos) {
    z::Ast::ContinueStatement& continueStatement = unit().addNode(new z::Ast::ContinueStatement(pos));
    return z::ptr(continueStatement);
}

z::Ast::AddEventHandlerStatement* z::Ast::Factory::aAddEventHandlerStatement(const z::Ast::Token& pos, const z::Ast::EventDecl& event, const z::Ast::Expr& source, z::Ast::FunctionTypeInstanceExpr& functor) {
    z::Ast::AddEventHandlerStatement& addEventHandlerStatement = unit().addNode(new z::Ast::AddEventHandlerStatement(pos, event, source, functor));
    unit().popExpectedTypeSpec(pos, Unit::ExpectedTypeSpec::etEventHandler);
    return z::ptr(addEventHandlerStatement);
}

const z::Ast::EventDecl* z::Ast::Factory::aEnterAddEventHandler(const z::Ast::EventDecl& eventDecl) {
    z::Ast::QualifiedTypeSpec& qts = addQualifiedTypeSpec(getToken(), false, eventDecl.handler(), false, false);
    unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etEventHandler, qts);
    return z::ptr(eventDecl);
}

z::Ast::RoutineReturnStatement* z::Ast::Factory::aRoutineReturnStatement(const z::Ast::Token& pos) {
    z::Ast::ExprList& exprList = addExprList(pos);
    z::Ast::RoutineReturnStatement& returnStatement = unit().addNode(new z::Ast::RoutineReturnStatement(pos, exprList));
    return z::ptr(returnStatement);
}

z::Ast::RoutineReturnStatement* z::Ast::Factory::aRoutineReturnStatement(const z::Ast::Token& pos, const z::Ast::Expr& expr) {
    z::Ast::ExprList& exprList = addExprList(pos);
    exprList.addExpr(expr);
    z::Ast::RoutineReturnStatement& returnStatement = unit().addNode(new z::Ast::RoutineReturnStatement(pos, exprList));
    return z::ptr(returnStatement);
}

z::Ast::FunctionReturnStatement* z::Ast::Factory::aFunctionReturnStatement(const z::Ast::Token& pos, const z::Ast::ExprList& exprList) {
    const z::Ast::FunctionSig* sig = 0;
    const z::Ast::RootFunctionDefn* rfd = 0;
    const z::Ast::ChildFunctionDefn* cfd = 0;
    for(z::Ast::Unit::TypeSpecStack::const_reverse_iterator it = unit().typeSpecStack().rbegin(); it != unit().typeSpecStack().rend(); ++it) {
        const z::Ast::TypeSpec& ts = it->get();
        if((rfd = dynamic_cast<const z::Ast::RootFunctionDefn*>(z::ptr(ts))) != 0) {
            sig = z::ptr(z::ref(rfd).sig());
            break;
        }
        if((cfd = dynamic_cast<const z::Ast::ChildFunctionDefn*>(z::ptr(ts))) != 0) {
            sig = z::ptr(z::ref(cfd).sig());
            break;
        }
    }
    z::Ast::FunctionReturnStatement& returnStatement = unit().addNode(new z::Ast::FunctionReturnStatement(pos, exprList, z::ref(sig)));
    return z::ptr(returnStatement);
}

z::Ast::ExitStatement* z::Ast::Factory::aExitStatement(const z::Ast::Token& pos, const z::Ast::Expr& expr) {
    z::Ast::ExitStatement& exitStatement = unit().addNode(new z::Ast::ExitStatement(pos, expr));
    return z::ptr(exitStatement);
}

z::Ast::CompoundStatement* z::Ast::Factory::aStatementList(z::Ast::CompoundStatement& list, const z::Ast::Statement& statement) {
    list.addStatement(statement);
    return z::ptr(list);
}

z::Ast::CompoundStatement* z::Ast::Factory::aStatementList() {
    z::Ast::CompoundStatement& statement = unit().addNode(new z::Ast::CompoundStatement(getToken()));
    return z::ptr(statement);
}

void z::Ast::Factory::aEnterCompoundStatement(const z::Ast::Token& pos) {
    z::Ast::Scope& scope = addScope(pos, z::Ast::ScopeType::Local);
    unit().enterScope(scope);
}

void z::Ast::Factory::aLeaveCompoundStatement() {
    unit().leaveScope();
}

void z::Ast::Factory::aEnterFunctionBlock(const z::Ast::Token& pos) {
    unit().pushExpectedTypeSpec(z::Ast::Unit::ExpectedTypeSpec::etNone);
    return aEnterCompoundStatement(pos);
}

void z::Ast::Factory::aLeaveFunctionBlock() {
    unit().popExpectedTypeSpec(getToken(), z::Ast::Unit::ExpectedTypeSpec::etNone);
    return aLeaveCompoundStatement();
}

z::Ast::ExprList* z::Ast::Factory::aExprList(z::Ast::ExprList& list, const z::Ast::Expr& expr) {
    list.addExpr(expr);
    return z::ptr(list);
}

z::Ast::ExprList* z::Ast::Factory::aExprList(const z::Ast::Expr& expr) {
    z::Ast::ExprList& list = addExprList(expr.pos());
    return aExprList(list, expr);
}

z::Ast::ExprList* z::Ast::Factory::aExprList() {
    z::Ast::ExprList& list = addExprList(getToken());
    return z::ptr(list);
}

z::Ast::TernaryOpExpr* z::Ast::Factory::aConditionalExpr(const z::Ast::Token& op1, const z::Ast::Token& op2, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs1, const z::Ast::Expr& rhs2) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = unit().coerce(op2, rhs1.qTypeSpec(), rhs2.qTypeSpec());
    z::Ast::ConditionalExpr& expr = unit().addNode(new z::Ast::ConditionalExpr(qTypeSpec, op1, op2, lhs, rhs1, rhs2));
    return z::ptr(expr);
}

template <typename T>
inline z::Ast::Expr& z::Ast::Factory::createBooleanExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(op, "bool");
    T& expr = unit().addNode(new T(qTypeSpec, op, lhs, rhs));
    return expr;
}

z::Ast::Expr& z::Ast::Factory::aBooleanAndExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBooleanExpr<z::Ast::BooleanAndExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBooleanOrExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBooleanExpr<z::Ast::BooleanOrExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBooleanEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBooleanExpr<z::Ast::BooleanEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBooleanNotEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBooleanExpr<z::Ast::BooleanNotEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBooleanLessThanExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBooleanExpr<z::Ast::BooleanLessThanExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBooleanGreaterThanExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBooleanExpr<z::Ast::BooleanGreaterThanExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBooleanLessThanOrEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBooleanExpr<z::Ast::BooleanLessThanOrEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBooleanGreaterThanOrEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBooleanExpr<z::Ast::BooleanGreaterThanOrEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBooleanHasExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBooleanExpr<z::Ast::BooleanHasExpr>(op, lhs, rhs);
}

template <typename T>
inline z::Ast::Expr& z::Ast::Factory::createBinaryExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = unit().coerce(op, lhs.qTypeSpec(), rhs.qTypeSpec());
    T& expr = unit().addNode(new T(qTypeSpec, op, lhs, rhs));
    return expr;
}

z::Ast::Expr& z::Ast::Factory::aBinaryAssignEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    const z::Ast::IndexExpr* indexExpr = dynamic_cast<const z::Ast::IndexExpr*>(z::ptr(lhs));
    if(indexExpr) {
        const z::Ast::QualifiedTypeSpec& qTypeSpec = unit().coerce(op, lhs.qTypeSpec(), rhs.qTypeSpec());
        z::Ast::SetIndexExpr& expr = unit().addNode(new z::Ast::SetIndexExpr(op, qTypeSpec, z::ref(indexExpr), rhs));
        return expr;
    }
    return createBinaryExpr<z::Ast::BinaryAssignEqualExpr>(op, lhs, rhs);
}

template <typename T>
inline z::Ast::Expr& z::Ast::Factory::createBinaryOpExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    const z::Ast::IndexExpr* indexExpr = dynamic_cast<const z::Ast::IndexExpr*>(z::ptr(lhs));
    if(indexExpr) {
        throw z::Exception("NodeFactory", zfmt(op, "Operator '%{s}' on index expression not implemented").arg("s", op ));
    }
    return createBinaryExpr<T>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryPlusEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryOpExpr<z::Ast::BinaryPlusEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryMinusEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryOpExpr<z::Ast::BinaryMinusEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryTimesEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryOpExpr<z::Ast::BinaryTimesEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryDivideEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryOpExpr<z::Ast::BinaryDivideEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryModEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryOpExpr<z::Ast::BinaryModEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryBitwiseAndEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryOpExpr<z::Ast::BinaryBitwiseAndEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryBitwiseOrEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryOpExpr<z::Ast::BinaryBitwiseOrEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryBitwiseXorEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryOpExpr<z::Ast::BinaryBitwiseXorEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryShiftLeftEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryOpExpr<z::Ast::BinaryShiftLeftEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryShiftRightEqualExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryOpExpr<z::Ast::BinaryShiftRightEqualExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryPlusExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryExpr<z::Ast::BinaryPlusExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryMinusExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryExpr<z::Ast::BinaryMinusExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryTimesExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryExpr<z::Ast::BinaryTimesExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryDivideExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryExpr<z::Ast::BinaryDivideExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryModExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryExpr<z::Ast::BinaryModExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryBitwiseAndExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryExpr<z::Ast::BinaryBitwiseAndExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryBitwiseOrExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryExpr<z::Ast::BinaryBitwiseOrExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryBitwiseXorExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryExpr<z::Ast::BinaryBitwiseXorExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryShiftLeftExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryExpr<z::Ast::BinaryShiftLeftExpr>(op, lhs, rhs);
}

z::Ast::Expr& z::Ast::Factory::aBinaryShiftRightExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs, const z::Ast::Expr& rhs) {
    return createBinaryExpr<z::Ast::BinaryShiftRightExpr>(op, lhs, rhs);
}

template <typename T>
inline T& z::Ast::Factory::createPostfixExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = lhs.qTypeSpec();
    T& expr = unit().addNode(new T(qTypeSpec, op, lhs));
    return expr;
}

z::Ast::PostfixIncExpr& z::Ast::Factory::aPostfixIncExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs) {
    return createPostfixExpr<z::Ast::PostfixIncExpr>(op, lhs);
}

z::Ast::PostfixDecExpr& z::Ast::Factory::aPostfixDecExpr(const z::Ast::Token& op, const z::Ast::Expr& lhs) {
    return createPostfixExpr<z::Ast::PostfixDecExpr>(op, lhs);
}

template <typename T>
inline T& z::Ast::Factory::createPrefixExpr(const z::Ast::Token& op, const z::Ast::Expr& rhs) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = rhs.qTypeSpec();
    T& expr = unit().addNode(new T(qTypeSpec, op, rhs));
    return expr;
}

z::Ast::PrefixNotExpr& z::Ast::Factory::aPrefixNotExpr(const z::Ast::Token& op, const z::Ast::Expr& rhs) {
    return createPrefixExpr<z::Ast::PrefixNotExpr>(op, rhs);
}

z::Ast::PrefixPlusExpr& z::Ast::Factory::aPrefixPlusExpr(const z::Ast::Token& op, const z::Ast::Expr& rhs) {
    return createPrefixExpr<z::Ast::PrefixPlusExpr>(op, rhs);
}

z::Ast::PrefixMinusExpr& z::Ast::Factory::aPrefixMinusExpr(const z::Ast::Token& op, const z::Ast::Expr& rhs) {
    return createPrefixExpr<z::Ast::PrefixMinusExpr>(op, rhs);
}

z::Ast::PrefixIncExpr& z::Ast::Factory::aPrefixIncExpr(const z::Ast::Token& op, const z::Ast::Expr& rhs) {
    return createPrefixExpr<z::Ast::PrefixIncExpr>(op, rhs);
}

z::Ast::PrefixDecExpr& z::Ast::Factory::aPrefixDecExpr(const z::Ast::Token& op, const z::Ast::Expr& rhs) {
    return createPrefixExpr<z::Ast::PrefixDecExpr>(op, rhs);
}

z::Ast::PrefixBitwiseNotExpr& z::Ast::Factory::aPrefixBitwiseNotExpr(const z::Ast::Token& op, const z::Ast::Expr& rhs) {
    return createPrefixExpr<z::Ast::PrefixBitwiseNotExpr>(op, rhs);
}

z::Ast::ListExpr* z::Ast::Factory::aListExpr(const z::Ast::Token& pos, const z::Ast::ListList& list) {
    unit().popExpectedTypeSpecOrAuto(pos, Unit::ExpectedTypeSpec::etListVal);
    z::Ast::TemplateTypePartList& tlist = unit().addNode(new z::Ast::TemplateTypePartList(pos));
    tlist.addType(list.valueType());
    z::Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "list", tlist);
    const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false, false);

    z::Ast::ListExpr& expr = unit().addNode(new z::Ast::ListExpr(pos, qTypeSpec, list));
    return z::ptr(expr);
}

z::Ast::ListList* z::Ast::Factory::aListList(const z::Ast::Token& pos, z::Ast::ListList& list, const z::Ast::ListItem& item) {
    list.addItem(item);
    const z::Ast::QualifiedTypeSpec& qValueTypeSpec = unit().coerce(pos, list.valueType(), item.valueExpr().qTypeSpec());
    list.valueType(qValueTypeSpec);
    return z::ptr(list);
}

z::Ast::ListList* z::Ast::Factory::aListList(const z::Ast::Token& pos, const z::Ast::ListItem& item) {
    z::Ast::ListList& list = unit().addNode(new z::Ast::ListList(pos));
    list.addItem(item);
    const z::Ast::QualifiedTypeSpec& valType = unit().getExpectedTypeSpec(pos, z::ptr(item.valueExpr().qTypeSpec()));
    list.valueType(valType);
    return z::ptr(list);
}

z::Ast::ListList* z::Ast::Factory::aListList(const z::Ast::Token& pos, const z::Ast::QualifiedTypeSpec& qTypeSpec) {
    z::Ast::ListList& list = unit().addNode(new z::Ast::ListList(pos));
    const z::Ast::QualifiedTypeSpec& valType = unit().getExpectedTypeSpec(pos, z::ptr(qTypeSpec));
    list.dValueType(qTypeSpec);
    list.valueType(valType);
    return z::ptr(list);
}

z::Ast::ListList* z::Ast::Factory::aListList(const z::Ast::Token& pos) {
    z::Ast::ListList& list = unit().addNode(new z::Ast::ListList(pos));
    const z::Ast::QualifiedTypeSpec& valType = unit().getExpectedTypeSpec(pos, 0);
    list.valueType(valType);
    return z::ptr(list);
}

z::Ast::ListItem* z::Ast::Factory::aListItem(const z::Ast::Expr& valueExpr) {
    const z::Ast::Expr& expr = convertExprToExpectedTypeSpec(valueExpr.pos(), valueExpr);
    z::Ast::ListItem& item = unit().addNode(new z::Ast::ListItem(valueExpr.pos(), expr));
//    popExpectedTypeSpec(valueExpr.pos(), ExpectedTypeSpec::etListVal);
    return z::ptr(item);
}

z::Ast::DictExpr* z::Ast::Factory::aDictExpr(const z::Ast::Token& pos, const z::Ast::DictList& list) {
    unit().popExpectedTypeSpecOrAuto(pos, Unit::ExpectedTypeSpec::etDictKey);

    z::Ast::TemplateTypePartList& tlist = unit().addNode(new z::Ast::TemplateTypePartList(pos));
    tlist.addType(list.keyType());
    tlist.addType(list.valueType());
    z::Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "dict", tlist);

    const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false, false);

    z::Ast::DictExpr& expr = unit().addNode(new z::Ast::DictExpr(pos, qTypeSpec, list));
    return z::ptr(expr);
}

z::Ast::DictList* z::Ast::Factory::aDictList(const z::Ast::Token& pos, z::Ast::DictList& list, const z::Ast::DictItem& item) {
    list.addItem(item);
    const z::Ast::QualifiedTypeSpec& keyType = unit().coerce(pos, list.keyType(), item.keyExpr().qTypeSpec());
    const z::Ast::QualifiedTypeSpec& valType = unit().coerce(pos, list.valueType(), item.valueExpr().qTypeSpec());

    list.keyType(keyType);
    list.valueType(valType);
    return z::ptr(list);
}

z::Ast::DictList* z::Ast::Factory::aDictList(const z::Ast::Token& pos, const z::Ast::DictItem& item) {
    z::Ast::DictList& list = unit().addNode(new z::Ast::DictList(pos));
    list.addItem(item);
    list.keyType(item.keyExpr().qTypeSpec());
    list.valueType(item.valueExpr().qTypeSpec());
    return z::ptr(list);
}

z::Ast::DictList* z::Ast::Factory::aDictList(const z::Ast::Token& pos, const z::Ast::QualifiedTypeSpec& qKeyTypeSpec, const z::Ast::QualifiedTypeSpec& qValueTypeSpec) {
    z::Ast::DictList& list = unit().addNode(new z::Ast::DictList(pos));
    list.dKeyType(qKeyTypeSpec);
    list.dValueType(qValueTypeSpec);
    list.keyType(qKeyTypeSpec);
    list.valueType(qValueTypeSpec);
    return z::ptr(list);
}

/// The sequence of calls in this function is important.
inline const z::Ast::Expr& z::Ast::Factory::switchDictKeyValue(const z::Ast::Token& pos, const z::Ast::Unit::ExpectedTypeSpec::Type& popType, const z::Ast::Unit::ExpectedTypeSpec::Type& pushType, const z::Ast::TemplateDefn::size_type& idx, const z::Ast::Expr& initExpr) {
    const z::Ast::Expr& expr = convertExprToExpectedTypeSpec(pos, initExpr);
    bool isExpected = unit().popExpectedTypeSpecOrAuto(pos, popType);
    const z::Ast::TemplateDefn* td0 = unit().isEnteringList();

    if(isExpected) {
        if((td0) && (z::ref(td0).name().string() == "dict")) {
            const z::Ast::QualifiedTypeSpec& keyType = z::ref(td0).at(idx);
            unit().pushExpectedTypeSpec(pushType, keyType);
        } else {
            assert(false);
        }
    } else {
        unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etAuto);
    }
    return expr;
}

z::Ast::DictItem* z::Ast::Factory::aDictItem(const z::Ast::Token& pos, const z::Ast::Expr& keyExpr, const z::Ast::Expr& valueExpr) {
    const z::Ast::Expr& expr = switchDictKeyValue(pos, Unit::ExpectedTypeSpec::etDictVal, Unit::ExpectedTypeSpec::etDictKey, 0, valueExpr);
    z::Ast::DictItem& item = unit().addNode(new z::Ast::DictItem(pos, keyExpr, expr));
    return z::ptr(item);
}

const z::Ast::Expr* z::Ast::Factory::aDictKey(const z::Ast::Expr& keyExpr) {
    const z::Ast::Expr& expr = switchDictKeyValue(keyExpr.pos(), Unit::ExpectedTypeSpec::etDictKey, Unit::ExpectedTypeSpec::etDictVal, 1, keyExpr);
    return z::ptr(expr);
}

const z::Ast::Token& z::Ast::Factory::aEnterList(const z::Ast::Token& pos) {
    const z::Ast::TemplateDefn* td0 = unit().isEnteringList();
    if(td0) {
        if(z::ref(td0).name().string() == "list") {
            const z::Ast::QualifiedTypeSpec& valType = z::ref(td0).at(0);
            unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etListVal, valType);
        } else if(z::ref(td0).name().string() == "dict") {
            const z::Ast::QualifiedTypeSpec& keyType = z::ref(td0).at(0);
            unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etDictKey, keyType);
        } else {
            assert(false);
        }
    } else {
        unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etAuto);
    }

    return pos;
}

z::Ast::FormatExpr* z::Ast::Factory::aFormatExpr(const z::Ast::Token& pos, const z::Ast::Expr& stringExpr, const z::Ast::DictExpr& dictExpr) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "string");
    z::Ast::FormatExpr& formatExpr = unit().addNode(new z::Ast::FormatExpr(pos, qTypeSpec, stringExpr, dictExpr));
    return z::ptr(formatExpr);
}

z::Ast::RoutineCallExpr* z::Ast::Factory::aRoutineCallExpr(const z::Ast::Token& pos, const z::Ast::Routine& routine, const z::Ast::ExprList& exprList) {
    unit().popCallArgList(pos, routine.inScope());
    const z::Ast::QualifiedTypeSpec& qTypeSpec = routine.outType();
    z::Ast::RoutineCallExpr& routineCallExpr = unit().addNode(new z::Ast::RoutineCallExpr(pos, qTypeSpec, routine, exprList));
    return z::ptr(routineCallExpr);
}

const z::Ast::Routine* z::Ast::Factory::aEnterRoutineCall(const z::Ast::Routine& routine) {
    unit().pushCallArgList(routine.inScope());
    return z::ptr(routine);
}

z::Ast::FunctorCallExpr* z::Ast::Factory::aFunctorCallExpr(const z::Ast::Token& pos, const z::Ast::Expr& expr, const z::Ast::ExprList& exprList) {
    const z::Ast::Function* function = dynamic_cast<const z::Ast::Function*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if(function == 0) {
        throw z::Exception("NodeFactory", zfmt(pos, "Unknown functor being called '%{s}'").arg("s", expr.qTypeSpec().typeSpec().name() ));
    }

    unit().popCallArgList(pos, z::ref(function).sig().inScope());

    const z::Ast::QualifiedTypeSpec& qTypeSpec = getFunctionReturnType(pos, z::ref(function));
    z::Ast::FunctorCallExpr& functorCallExpr = unit().addNode(new z::Ast::FunctorCallExpr(pos, qTypeSpec, expr, exprList));
    return z::ptr(functorCallExpr);
}

z::Ast::Expr* z::Ast::Factory::aEnterFunctorCall(z::Ast::Expr& expr) {
    const z::Ast::Function* function = dynamic_cast<const z::Ast::Function*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if(function == 0) {
        throw z::Exception("NodeFactory", zfmt(expr.pos(), "Unknown functor being called '%{s}'").arg("s", expr.qTypeSpec().typeSpec().name() ));
    }

    unit().pushCallArgList(z::ref(function).sig().inScope());
    return z::ptr(expr);
}

z::Ast::Expr* z::Ast::Factory::aEnterFunctorCall(const z::Ast::Token& name) {
    z::Ast::VariableRefExpr* expr = aVariableRefExpr(name);
    return aEnterFunctorCall(z::ref(expr));
}

z::Ast::Expr* z::Ast::Factory::aEnterFunctorCall(const z::Ast::Function& function) {
    z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(getToken(), false, function, false, false);
    z::Ast::ExprList& exprList = addExprList(getToken());
    z::Ast::FunctionInstanceExpr& expr = unit().addNode(new z::Ast::FunctionInstanceExpr(getToken(), qTypeSpec, function, exprList));
    return aEnterFunctorCall(expr);
}

z::Ast::ExprList* z::Ast::Factory::aCallArgList(const z::Ast::Token& pos, z::Ast::ExprList& list, const z::Ast::Expr& expr) {
    const z::Ast::Expr& argExpr = convertExprToExpectedTypeSpec(pos, expr);
    list.addExpr(argExpr);
    unit().popCallArg(pos);
    return z::ptr(list);
}

z::Ast::ExprList* z::Ast::Factory::aCallArgList(const z::Ast::Expr& expr) {
    z::Ast::ExprList& list = addExprList(getToken());
    return aCallArgList(getToken(), list, expr);
}

z::Ast::ExprList* z::Ast::Factory::aCallArgList() {
    z::Ast::ExprList& list = addExprList(getToken());
    return z::ptr(list);
}

z::Ast::RunExpr* z::Ast::Factory::aRunExpr(const z::Ast::Token& pos, const z::Ast::FunctorCallExpr& callExpr) {
    const z::Ast::Function* function = dynamic_cast<const z::Ast::Function*>(z::ptr(callExpr.expr().qTypeSpec().typeSpec()));
    if(function != 0) {
        z::Ast::QualifiedTypeSpec& qRetTypeSpec = addQualifiedTypeSpec(pos, false, z::ref(function), false, false);

        z::Ast::TemplateTypePartList& list = unit().addNode(new z::Ast::TemplateTypePartList(pos));
        list.addType(qRetTypeSpec);
        z::Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "future", list);
        const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, true, false);

        z::Ast::RunExpr& runExpr = unit().addNode(new z::Ast::RunExpr(pos, qTypeSpec, callExpr));
        return z::ptr(runExpr);
    }

    throw z::Exception("NodeFactory", zfmt(pos, "Unknown functor in run expression '%{s}'").arg("s", ZenlangNameGenerator().qtn(callExpr.expr().qTypeSpec())));
}

z::Ast::OrderedExpr* z::Ast::Factory::aOrderedExpr(const z::Ast::Token& pos, const z::Ast::Expr& innerExpr) {
    z::Ast::OrderedExpr& expr = unit().addNode(new z::Ast::OrderedExpr(pos, innerExpr.qTypeSpec(), innerExpr));
    return z::ptr(expr);
}

z::Ast::IndexExpr* z::Ast::Factory::aIndexExpr(const z::Ast::Token& pos, const z::Ast::Expr& expr, const z::Ast::Expr& index) {
    const z::Ast::TypeSpec* listTypeSpec = resolveTypedef(expr.qTypeSpec().typeSpec());
    if(z::ref(listTypeSpec).name().string() == "data") {
        const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "ubyte");
        z::Ast::IndexExpr& indexExpr = unit().addNode(new z::Ast::IndexExpr(pos, qTypeSpec, expr, index));
        return z::ptr(indexExpr);
    }

    if(z::ref(listTypeSpec).name().string() == "widget") {
        const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "widget");
        z::Ast::IndexExpr& indexExpr = unit().addNode(new z::Ast::IndexExpr(pos, qTypeSpec, expr, index));
        return z::ptr(indexExpr);
    }

    const z::Ast::TemplateDefn* td = dynamic_cast<const z::Ast::TemplateDefn*>(listTypeSpec);
    if(td) {
        if(z::ref(td).name().string() == "list") {
            const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(td).at(0).isConst(), z::ref(td).at(0).typeSpec(), false, false);
            z::Ast::IndexExpr& indexExpr = unit().addNode(new z::Ast::IndexExpr(pos, qTypeSpec, expr, index));
            return z::ptr(indexExpr);
        }

        if(z::ref(td).name().string() == "dict") {
            const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(td).at(1).isConst(), z::ref(td).at(1).typeSpec(), true, false);
            z::Ast::IndexExpr& indexExpr = unit().addNode(new z::Ast::IndexExpr(pos, qTypeSpec, expr, index));
            return z::ptr(indexExpr);
        }
    }

    const z::Ast::StructDefn* sd = dynamic_cast<const z::Ast::StructDefn*>(listTypeSpec);
    if(sd) {
        const z::Ast::Routine* routine = z::ref(sd).hasChild<const z::Ast::Routine>("at");
        if(routine) {
            const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(routine).outType().isConst(), z::ref(routine).outType().typeSpec(), true, false);
            z::Ast::IndexExpr& indexExpr = unit().addNode(new z::Ast::IndexExpr(pos, qTypeSpec, expr, index));
            return z::ptr(indexExpr);
        }
    }

    throw z::Exception("NodeFactory", zfmt(pos, "'%{s}' is not an indexable type").arg("s", ZenlangNameGenerator().tn(expr.qTypeSpec().typeSpec()) ));
}

z::Ast::SpliceExpr* z::Ast::Factory::aSpliceExpr(const z::Ast::Token& pos, const z::Ast::Expr& expr, const z::Ast::Expr& from, const z::Ast::Expr& to) {
    const z::Ast::TypeSpec* listTypeSpec = resolveTypedef(expr.qTypeSpec().typeSpec());

    if((listTypeSpec) && (z::ref(listTypeSpec).name().string() == "data")) {
        const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, expr.qTypeSpec().typeSpec(), false, false);
        z::Ast::SpliceExpr& spliceExpr = unit().addNode(new z::Ast::SpliceExpr(pos, qTypeSpec, expr, from, to));
        return z::ptr(spliceExpr);
    }

    const z::Ast::TemplateDefn* td = dynamic_cast<const z::Ast::TemplateDefn*>(listTypeSpec);
    if((td) && (z::ref(td).name().string() == "list")) {
        const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(td).at(0).isConst(), expr.qTypeSpec().typeSpec(), false, false);
        z::Ast::SpliceExpr& spliceExpr = unit().addNode(new z::Ast::SpliceExpr(pos, qTypeSpec, expr, from, to));
        return z::ptr(spliceExpr);
    }

    throw z::Exception("NodeFactory", zfmt(pos, "'%{s}' is not an splicable type").arg("s", ZenlangNameGenerator().qtn(expr.qTypeSpec()) ));
}

z::Ast::SizeofTypeExpr* z::Ast::Factory::aSizeofTypeExpr(const z::Ast::Token& pos, const z::Ast::QualifiedTypeSpec& typeSpec) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "size");
    z::Ast::SizeofTypeExpr& expr = unit().addNode(new z::Ast::SizeofTypeExpr(pos, qTypeSpec, typeSpec));
    return z::ptr(expr);
}

z::Ast::SizeofExprExpr* z::Ast::Factory::aSizeofExprExpr(const z::Ast::Token& pos, const z::Ast::Expr& expr) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "size");
    z::Ast::SizeofExprExpr& sizeofExpr = unit().addNode(new z::Ast::SizeofExprExpr(pos, qTypeSpec, expr));
    return z::ptr(sizeofExpr);
}

z::Ast::TypeofTypeExpr* z::Ast::Factory::aTypeofTypeExpr(const z::Ast::Token& pos, const z::Ast::QualifiedTypeSpec& typeSpec) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "type");
    z::Ast::TypeofTypeExpr& typeofExpr = unit().addNode(new z::Ast::TypeofTypeExpr(pos, qTypeSpec, typeSpec));
    return z::ptr(typeofExpr);
}

z::Ast::TypeofExprExpr* z::Ast::Factory::aTypeofExprExpr(const z::Ast::Token& pos, const z::Ast::Expr& expr) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "type");
    z::Ast::TypeofExprExpr& typeofExpr = unit().addNode(new z::Ast::TypeofExprExpr(pos, qTypeSpec, expr));
    return z::ptr(typeofExpr);
}

z::Ast::TypecastExpr* z::Ast::Factory::aTypecastExpr(const z::Ast::Token& pos, const z::Ast::QualifiedTypeSpec& qTypeSpec, const z::Ast::Expr& expr) {
    /// \todo check if canCoerce
    const z::Ast::TemplateDefn* subType = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if((subType) && (z::ref(subType).name().string() == "pointer")) {
        const z::Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, qTypeSpec.isConst(), qTypeSpec.typeSpec(), true, false);
        z::Ast::TypecastExpr& typecastExpr = unit().addNode(new z::Ast::DynamicTypecastExpr(pos, qTypeSpec, typeSpec, expr));
        return z::ptr(typecastExpr);
    }

    z::Ast::TypecastExpr& typecastExpr = unit().addNode(new z::Ast::StaticTypecastExpr(pos, qTypeSpec, qTypeSpec, expr));
    return z::ptr(typecastExpr);
}

z::Ast::PointerInstanceExpr* z::Ast::Factory::aPointerInstanceExpr(const z::Ast::Token& pos, const z::Ast::Expr& expr) {
    z::Ast::TemplateTypePartList& list = unit().addNode(new z::Ast::TemplateTypePartList(pos));
    const z::Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, expr.qTypeSpec().isConst(), expr.qTypeSpec().typeSpec(), true, false);
    list.addType(typeSpec);
    z::Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "pointer", list);
    const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false, false);

    z::Ast::ExprList& exprList = addExprList(pos);
    exprList.addExpr(expr);

    z::Ast::PointerInstanceExpr& pointerExpr = unit().addNode(new z::Ast::PointerInstanceExpr(pos, qTypeSpec, templateDefn, templateDefn, exprList));
    return z::ptr(pointerExpr);
}

z::Ast::TemplateDefnInstanceExpr* z::Ast::Factory::aValueInstanceExpr(const z::Ast::Token& pos, const z::Ast::Expr& expr) {
    const z::Ast::TemplateDefn* templateDefn = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if(templateDefn) {
        if(z::ref(templateDefn).name().string() == "pointer") {
            z::Ast::ExprList& exprList = addExprList(pos);
            exprList.addExpr(expr);
            z::Ast::ValueInstanceExpr& valueInstanceExpr = getValueInstanceExpr(pos, z::ref(templateDefn).at(0), z::ref(templateDefn), z::ref(templateDefn), exprList);
            return z::ptr(valueInstanceExpr);
        }
        if(z::ref(templateDefn).name().string() == "ptr") {
            z::Ast::ExprList& exprList = addExprList(pos);
            exprList.addExpr(expr);
            Ast::DeRefInstanceExpr& drefex = unit().addNode(new z::Ast::DeRefInstanceExpr(pos, z::ref(templateDefn).at(0), z::ref(templateDefn), z::ref(templateDefn), exprList));
            return z::ptr(drefex);
        }
    }

    throw z::Exception("NodeFactory", zfmt(pos, "Expression is not a pointer to  '%{s}'").arg("s", expr.qTypeSpec().typeSpec().name() ));
}

z::Ast::TemplateDefnInstanceExpr* z::Ast::Factory::aTemplateDefnInstanceExpr(const z::Ast::Token& pos, const z::Ast::TemplateDefn& templateDefn, const z::Ast::ExprList& exprList) {
    z::string name = templateDefn.name().string();
    if(name == "pointer") {
        const z::Ast::QualifiedTypeSpec& newTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn.at(0).typeSpec(), true, false);
        z::Ast::TemplateTypePartList& list = unit().addNode(new z::Ast::TemplateTypePartList(pos));
        list.addType(newTypeSpec);
        z::Ast::TemplateDefn& newTemplateDefn = createTemplateDefn(pos, "pointer", list);
        const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, newTemplateDefn, false, false);

        z::Ast::PointerInstanceExpr& expr = unit().addNode(new z::Ast::PointerInstanceExpr(pos, qTypeSpec, templateDefn, newTemplateDefn, exprList));
        return z::ptr(expr);
    }

    if(name == "value") {
        const z::Ast::QualifiedTypeSpec& qTypeSpec = templateDefn.at(0);
        const z::Ast::Expr& expr = exprList.at(0);
        const z::Ast::TemplateDefn* subType = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(expr.qTypeSpec().typeSpec()));
        if((subType == 0) || (z::ref(subType).name().string() != "pointer")) {
            throw z::Exception("NodeFactory", zfmt(pos, "Expression is not a pointer to '%{s}'").arg("s", qTypeSpec.typeSpec().name() ));
        }

        z::Ast::ValueInstanceExpr& valueInstanceExpr = getValueInstanceExpr(pos, qTypeSpec, templateDefn, templateDefn, exprList);
        return z::ptr(valueInstanceExpr);
    }

    if(name == "raw") {
        const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn.at(0).typeSpec(), false, false);
        z::Ast::RawDataInstanceExpr& expr = unit().addNode(new z::Ast::RawDataInstanceExpr(pos, qTypeSpec, templateDefn, templateDefn, exprList));
        return z::ptr(expr);
    }

    throw z::Exception("NodeFactory", zfmt(pos, "Invalid template instantiation '%{s}'").arg("s", templateDefn.name() ));
}

z::Ast::VariableRefExpr* z::Ast::Factory::aVariableRefExpr(const z::Ast::Token& name) {
    z::Ast::RefType::T refType = z::Ast::RefType::Local;
    const z::Ast::VariableDefn* vref = unit().getVariableDef(name, refType);
    if(vref == 0) {
        throw z::Exception("NodeFactory", zfmt(name, "Variable not found: '%{s}'").arg("s", name ));
    }
    // create vref expression
    z::Ast::VariableRefExpr& vrefExpr = unit().addNode(new z::Ast::VariableRefExpr(name, z::ref(vref).qTypeSpec(), z::ref(vref), refType));
    return z::ptr(vrefExpr);
}

namespace zz {
    struct FindMemberVarItem {
        inline FindMemberVarItem(const z::Ast::TemplateDefn* ptd, const z::Ast::QualifiedTypeSpec* pqts) : td(ptd), qts(pqts) {}
        const z::Ast::TemplateDefn* td;
        const z::Ast::QualifiedTypeSpec* qts;
    };
}

z::Ast::MemberExpr* z::Ast::Factory::aMemberVariableExpr(const z::Ast::Expr& exprx, const z::Ast::Token& name) {
    const z::Ast::TypeSpec& typeSpec = exprx.qTypeSpec().typeSpec();

    z::stack<zz::FindMemberVarItem> tss;
    const z::Ast::QualifiedTypeSpec* qpts = z::ptr(exprx.qTypeSpec());
    while(qpts != 0) {
        const z::Ast::TypeSpec& ts = z::ref(qpts).typeSpec();
        const z::Ast::TypeSpec* pts = z::ptr(ts);
        const z::Ast::TemplateDefn* td = dynamic_cast<const z::Ast::TemplateDefn*>(pts);
        if(td == 0) {
            break;
        }
        if(z::ref(td).name().string() != "ptr") {
            break;
        }

        tss.push(zz::FindMemberVarItem(td, qpts));
        const z::Ast::QualifiedTypeSpec& qcts = z::ref(td).at(0);
        qpts = z::ptr(qcts);
    }

    assert(qpts != 0);
    const Ast::Expr* expr1 = z::ptr(exprx);
    while(tss.size() > 0) {
        zz::FindMemberVarItem it = tss.pop();
        assert(z::ref(it.td).name().string() == "ptr");
        z::Ast::ExprList& exprList = addExprList(name);
        exprList.addExpr(z::ref(expr1));
        const Ast::DeRefInstanceExpr& drefex = unit().addNode(new z::Ast::DeRefInstanceExpr(name, z::ref(it.qts), z::ref(it.td), z::ref(it.td), exprList));
        expr1 = z::ptr(drefex);
    }

    const Ast::Expr& expr = z::ref(expr1);
    const z::Ast::TypeSpec& ts = z::ref(qpts).typeSpec();
    const z::Ast::StructDefn* structDefn = dynamic_cast<const z::Ast::StructDefn*>(z::ptr(ts));
    if(structDefn != 0) {
        for(StructBaseIterator sbi(structDefn); sbi.hasNext(); sbi.next()) {
            const z::Ast::VariableDefn* vref = unit().hasMember(sbi.get().scope(), name);
            if(vref) {
                const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, expr.qTypeSpec().isConst(), z::ref(vref).qTypeSpec().typeSpec(), true, false);
                z::Ast::MemberVariableExpr& vdefExpr = unit().addNode(new z::Ast::MemberVariableExpr(name, qTypeSpec, expr, z::ref(vref)));
                return z::ptr(vdefExpr);
            }

            for(z::Ast::StructDefn::PropertyList::const_iterator it = sbi.get().propertyList().begin(); it != sbi.get().propertyList().end(); ++it) {
                const z::Ast::PropertyDecl& pref = it->get();
                if(pref.name().string() == name.string()) {
                    const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, expr.qTypeSpec().isConst(), pref.qTypeSpec().typeSpec(), true, false);
                    z::Ast::MemberPropertyExpr& vdefExpr = unit().addNode(new z::Ast::MemberPropertyExpr(name, qTypeSpec, expr, pref));
                    return z::ptr(vdefExpr);
                }
            }
        }

        throw z::Exception("NodeFactory", zfmt(name, "'%{s}' is not a member of struct '%{t}'")
                           .arg("s", name )
                           .arg("t", ZenlangNameGenerator().tn(typeSpec) )
                           );
    }

    const z::Ast::FunctionRetn* functionRetn = dynamic_cast<const z::Ast::FunctionRetn*>(z::ptr(ts));
    if(functionRetn != 0) {
        const z::Ast::VariableDefn* vref = unit().hasMember(z::ref(functionRetn).outScope(), name);
        if(vref) {
            z::Ast::MemberVariableExpr& vdefExpr = unit().addNode(new z::Ast::MemberVariableExpr(name, z::ref(vref).qTypeSpec(), expr, z::ref(vref)));
            return z::ptr(vdefExpr);
        }
        throw z::Exception("NodeFactory", zfmt(name, "'%{s}' is not a member of function: '%{t}'")
                           .arg("s", name)
                           .arg("t", ZenlangNameGenerator().tn(typeSpec))
                           );
    }

    throw z::Exception("NodeFactory", zfmt(name, "Not an aggregate expression type '%{s}' (looking for member '%{t}')")
                       .arg("s", typeSpec.name())
                       .arg("t", name)
                       );
}

z::Ast::TypeSpecMemberExpr* z::Ast::Factory::aTypeSpecMemberExpr(const z::Ast::TypeSpec& typeSpec, const z::Ast::Token& name) {
    const z::Ast::EnumDefn* enumDefn = dynamic_cast<const z::Ast::EnumDefn*>(z::ptr(typeSpec));
    if(enumDefn != 0) {
        const z::Ast::VariableDefn* vref = unit().hasMember(z::ref(enumDefn).scope(), name);
        if(vref == 0) {
            throw z::Exception("NodeFactory", zfmt(name, "'%{s}' is not a member of type %{t}")
                               .arg("s", name)
                               .arg("t", typeSpec.name())
                               );
        }
        const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, typeSpec, false, false);
        z::Ast::EnumMemberExpr& typeSpecMemberExpr = unit().addNode(new z::Ast::EnumMemberExpr(name, qTypeSpec, typeSpec, z::ref(vref)));
        return z::ptr(typeSpecMemberExpr);
    }

    const z::Ast::StructDefn* structDefn = dynamic_cast<const z::Ast::StructDefn*>(z::ptr(typeSpec));
    if(structDefn != 0) {
        const z::Ast::VariableDefn* vref = unit().hasMember(z::ref(structDefn).scope(), name);
        if(vref == 0) {
            throw z::Exception("NodeFactory", zfmt(name, "'%{s}' is not a member of type %{t}")
                               .arg("s", name)
                               .arg("t", typeSpec.name())
                               );
        }
        z::Ast::StructMemberExpr& typeSpecMemberExpr = unit().addNode(new z::Ast::StructMemberExpr(name, z::ref(vref).qTypeSpec(), typeSpec, z::ref(vref)));
        return z::ptr(typeSpecMemberExpr);
    }

    throw z::Exception("NodeFactory", zfmt(name, "Not an aggregate type '%{s}' (looking for member %{t})")
                       .arg("s", typeSpec.name())
                       .arg("t", name)
                       );
}

z::Ast::StructInstanceExpr* z::Ast::Factory::aStructInstanceExpr(const z::Ast::Token& pos, const z::Ast::StructDefn& structDefn, const z::Ast::StructInitPartList& list) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, structDefn, false, false);
    z::Ast::StructInstanceExpr& structInstanceExpr = unit().addNode(new z::Ast::StructInstanceExpr(pos, qTypeSpec, structDefn, list));
    return z::ptr(structInstanceExpr);
}

z::Ast::StructInstanceExpr* z::Ast::Factory::aStructInstanceExpr(const z::Ast::Token& pos, const z::Ast::StructDefn& structDefn) {
    z::Ast::StructInitPartList& list = unit().addNode(new z::Ast::StructInitPartList(pos));
    return aStructInstanceExpr(pos, structDefn, list);
}

z::Ast::Expr* z::Ast::Factory::aAutoStructInstanceExpr(const z::Ast::Token& pos, const z::Ast::StructDefn& structDefn, const z::Ast::StructInitPartList& list) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, structDefn, false, false);
    z::Ast::StructInstanceExpr& structInstanceExpr = unit().addNode(new z::Ast::StructInstanceExpr(pos, qTypeSpec, structDefn, list));
    return z::ptr(structInstanceExpr);
}

z::Ast::Expr* z::Ast::Factory::aAutoStructInstanceExpr(const z::Ast::Token& pos, const z::Ast::StructDefn& structDefn) {
    z::Ast::StructInitPartList& list = unit().addNode(new z::Ast::StructInitPartList(pos));
    return aAutoStructInstanceExpr(pos, structDefn, list);
}

const z::Ast::StructDefn* z::Ast::Factory::aEnterStructInstanceExpr(const z::Ast::StructDefn& structDefn) {
    unit().pushStructInit(structDefn);
    return z::ptr(structDefn);
}

const z::Ast::StructDefn* z::Ast::Factory::aEnterAutoStructInstanceExpr(const z::Ast::Token& pos) {
    const z::Ast::StructDefn* sd = unit().isStructExpected();
    if(sd) {
        return aEnterStructInstanceExpr(z::ref(sd));
    }
    sd = unit().isPointerToStructExpected();
    if(sd) {
        return aEnterStructInstanceExpr(z::ref(sd));
    }
    throw z::Exception("NodeFactory", zfmt(pos, "No struct type expected") );
}

void z::Ast::Factory::aLeaveStructInstanceExpr() {
    unit().popStructInit();
}

const z::Ast::VariableDefn* z::Ast::Factory::aEnterStructInitPart(const z::Ast::Token& name) {
    const z::Ast::StructDefn* structDefn = unit().structInit();
    if(structDefn == 0) {
        throw z::Exception("NodeFactory", zfmt(name, "Internal error initializing struct-member") );
    }

    for(StructBaseIterator sbi(structDefn); sbi.hasNext(); sbi.next()) {
        for(z::Ast::Scope::List::const_iterator it = sbi.get().list().begin(); it != sbi.get().list().end(); ++it) {
            const z::Ast::VariableDefn& vdef = it->get();
            if(vdef.name().string() == name.string()) {
                unit().pushExpectedTypeSpec(Unit::ExpectedTypeSpec::etStructInit, vdef.qTypeSpec());
                return z::ptr(vdef);
            }
        }
    }

    throw z::Exception("NodeFactory", zfmt(name, "member %{c} not found in struct %{t} (1)")
                       .arg("c", name)
                       .arg("t", ZenlangNameGenerator().tn(z::ref(structDefn)))
                       );
}

void z::Ast::Factory::aLeaveStructInitPart(const z::Ast::Token& pos) {
    unused(pos);
}

z::Ast::StructInitPartList* z::Ast::Factory::aStructInitPartList(z::Ast::StructInitPartList& list, const z::Ast::StructInitPart& part) {
    list.addPart(part);
    return z::ptr(list);
}

z::Ast::StructInitPartList* z::Ast::Factory::aStructInitPartList(const z::Ast::StructInitPart& part) {
    z::Ast::StructInitPartList& list = unit().addNode(new z::Ast::StructInitPartList(getToken()));
    list.addPart(part);
    return z::ptr(list);
}

z::Ast::StructInitPart* z::Ast::Factory::aStructInitPart(const z::Ast::Token& pos, const z::Ast::VariableDefn& vdef, const z::Ast::Expr& initExpr) {
    const z::Ast::Expr& expr = convertExprToExpectedTypeSpec(pos, initExpr);
    unit().popExpectedTypeSpec(pos, Unit::ExpectedTypeSpec::etStructInit);
    z::Ast::StructInitPart& part = unit().addNode(new z::Ast::StructInitPart(pos, vdef, expr));
    return z::ptr(part);
}

z::Ast::FunctionInstanceExpr* z::Ast::Factory::aFunctionInstanceExpr(const z::Ast::Token& pos, const z::Ast::TypeSpec& typeSpec, const z::Ast::ExprList& exprList) {
    const z::Ast::Function* function = dynamic_cast<const z::Ast::Function*>(z::ptr(typeSpec));
    if(function != 0) {
        z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, z::ref(function), false, false);
        z::Ast::FunctionInstanceExpr& functionInstanceExpr = unit().addNode(new z::Ast::FunctionInstanceExpr(pos, qTypeSpec, z::ref(function), exprList));
        return z::ptr(functionInstanceExpr);
    }

    throw z::Exception("NodeFactory", zfmt(pos, "Not a function type %{s}").arg("s", typeSpec.name() ));
}

z::Ast::AnonymousFunctionExpr* z::Ast::Factory::aAnonymousFunctionExpr(z::Ast::ChildFunctionDefn& functionDefn, const z::Ast::CompoundStatement& compoundStatement) {
    aChildFunctionDefn(functionDefn, compoundStatement);
    z::Ast::ExprList& exprList = addExprList(getToken());
    z::Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(getToken(), false, functionDefn, false, false);
    z::Ast::AnonymousFunctionExpr& functionInstanceExpr = unit().addNode(new z::Ast::AnonymousFunctionExpr(getToken(), qTypeSpec, functionDefn, exprList));
    return z::ptr(functionInstanceExpr);
}

z::Ast::ChildFunctionDefn* z::Ast::Factory::aEnterAnonymousFunction(const z::Ast::Function& function, const Ast::ClosureRef& cref) {
    z::string ns = z::string("_anonymous_%{i}").arg("i", unit().uniqueIdx());
    z::Ast::Token name(filename(), getToken().row(), getToken().col(), ns);

    z::Ast::TypeSpec* ts = 0;
    for(Unit::TypeSpecStack::reverse_iterator it = unit().typeSpecStack().rbegin(); it != unit().typeSpecStack().rend(); ++it) {
        ts = z::ptr(it->get());
        if(dynamic_cast<z::Ast::Namespace*>(ts) != 0)
            break;
        if(dynamic_cast<z::Ast::Root*>(ts) != 0)
            break;
    }

    if(ts == 0) {
        throw z::Exception("NodeFactory", zfmt(name, "Internal error: Unable to find parent for anonymous function  %{s}").arg("s", ZenlangNameGenerator().tn(function) ));
    }

    z::Ast::ChildFunctionDefn& functionDefn = createChildFunctionDefn(z::ref(ts), function, name, z::Ast::DefinitionType::Final, cref);
    z::Ast::Statement* statement = aGlobalTypeSpecStatement(z::Ast::AccessType::Private, functionDefn);
    unused(statement);
    return z::ptr(functionDefn);
}

z::Ast::ChildFunctionDefn* z::Ast::Factory::aEnterAutoAnonymousFunction(const z::Ast::Token& pos, const Ast::ClosureRef& cref) {
    const z::Ast::Function* function = unit().isFunctionExpected();
    if(function == 0) {
        throw z::Exception("NodeFactory", zfmt(pos, "Internal error: no function type expected") );
    }
    return aEnterAnonymousFunction(z::ref(function), cref);
}

z::Ast::ChildFunctionDefn* z::Ast::Factory::aEnterAnonymousFunctionExpr(const z::Ast::Function& function) {
    z::Ast::ClosureRef cref = aClosureList();
    return aEnterAnonymousFunction(function, cref);
}

z::Ast::ConstantNullExpr& z::Ast::Factory::aConstantNullExpr(const z::Ast::Token& token) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "void");
    z::Ast::ConstantNullExpr& expr = unit().addNode(new z::Ast::ConstantNullExpr(token, qTypeSpec));
    return expr;
}

z::Ast::ConstantFloatExpr& z::Ast::Factory::aConstantFloatExpr(const z::Ast::Token& token) {
    float value = token.string().to<float>();
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "float");
    z::Ast::ConstantFloatExpr& expr = unit().addNode(new z::Ast::ConstantFloatExpr(token, qTypeSpec, value));
    return expr;
}

z::Ast::ConstantDoubleExpr& z::Ast::Factory::aConstantDoubleExpr(const z::Ast::Token& token) {
    double value = token.string().to<double>();
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "double");
    z::Ast::ConstantDoubleExpr& expr = unit().addNode(new z::Ast::ConstantDoubleExpr(token, qTypeSpec, value));
    return expr;
}

z::Ast::ConstantBooleanExpr& z::Ast::Factory::aConstantBooleanExpr(const z::Ast::Token& token) {
    const bool value = (token.string() == "true")?true:false;
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "bool");
    z::Ast::ConstantBooleanExpr& expr = unit().addNode(new z::Ast::ConstantBooleanExpr(token, qTypeSpec, value));
    return expr;
}

z::Ast::ConstantStringExpr& z::Ast::Factory::aConstantStringExpr(const z::Ast::Token& token) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "string");
    z::Ast::ConstantStringExpr& expr = unit().addNode(new z::Ast::ConstantStringExpr(token, qTypeSpec, token.string()));
    return expr;
}

z::Ast::ConstantCharExpr& z::Ast::Factory::aConstantCharExpr(const z::Ast::Token& token) {
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "char");
    z::Ast::ConstantCharExpr& expr = unit().addNode(new z::Ast::ConstantCharExpr(token, qTypeSpec, token.string()));
    return expr;
}

z::Ast::ConstantLongExpr& z::Ast::Factory::aConstantLongExpr(const z::Ast::Token& token) {
    int64_t value = token.string().to<int64_t>();
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "long");
    z::Ast::ConstantLongExpr& expr = unit().addNode(new z::Ast::ConstantLongExpr(token, qTypeSpec, value));
    return expr;
}

z::Ast::ConstantIntExpr& z::Ast::Factory::aConstantIntExpr(const z::Ast::Token& token) {
    int32_t value = token.string().to<int32_t>();
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "int");
    z::Ast::ConstantIntExpr& expr = unit().addNode(new z::Ast::ConstantIntExpr(token, qTypeSpec, value));
    return expr;
}

z::Ast::ConstantShortExpr& z::Ast::Factory::aConstantShortExpr(const z::Ast::Token& token) {
    int16_t value = token.string().to<int16_t>();
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "short");
    z::Ast::ConstantShortExpr& expr = unit().addNode(new z::Ast::ConstantShortExpr(token, qTypeSpec, value));
    return expr;
}

z::Ast::ConstantByteExpr& z::Ast::Factory::aConstantByteExpr(const z::Ast::Token& token) {
    int8_t value = token.string().to<int8_t>();
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "byte");
    z::Ast::ConstantByteExpr& expr = unit().addNode(new z::Ast::ConstantByteExpr(token, qTypeSpec, value));
    return expr;
}

z::Ast::ConstantUnLongExpr& z::Ast::Factory::aConstantUnLongExpr(const z::Ast::Token& token) {
    uint64_t value = token.string().to<uint64_t>();
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "ulong");
    z::Ast::ConstantUnLongExpr& expr = unit().addNode(new z::Ast::ConstantUnLongExpr(token, qTypeSpec, value));
    return expr;
}

z::Ast::ConstantUnIntExpr& z::Ast::Factory::aConstantUnIntExpr(const z::Ast::Token& token) {
    uint32_t value = token.string().to<uint32_t>();
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "uint");
    z::Ast::ConstantUnIntExpr& expr = unit().addNode(new z::Ast::ConstantUnIntExpr(token, qTypeSpec, value));
    return expr;
}

z::Ast::ConstantUnShortExpr& z::Ast::Factory::aConstantUnShortExpr(const z::Ast::Token& token) {
    uint16_t value = token.string().to<uint16_t>();
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "ushort");
    z::Ast::ConstantUnShortExpr& expr = unit().addNode(new z::Ast::ConstantUnShortExpr(token, qTypeSpec, value));
    return expr;
}

z::Ast::ConstantUnByteExpr& z::Ast::Factory::aConstantUnByteExpr(const z::Ast::Token& token) {
    uint8_t value = token.string().to<uint8_t>();
    const z::Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "ubyte");
    z::Ast::ConstantUnByteExpr& expr = unit().addNode(new z::Ast::ConstantUnByteExpr(token, qTypeSpec, value));
    return expr;
}
