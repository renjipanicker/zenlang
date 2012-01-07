#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "NodeFactory.hpp"
#include "error.hpp"
#include "typename.hpp"
#include "compiler.hpp"

/////////////////////////////////////////////////////////////////////////
inline Ast::QualifiedTypeSpec& Ast::NodeFactory::addQualifiedTypeSpec(const Ast::Token& pos, const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef) {
    Ast::QualifiedTypeSpec& qualifiedTypeSpec = addUnitNode(new Ast::QualifiedTypeSpec(pos, isConst, typeSpec, isRef));
    return qualifiedTypeSpec;
}

inline const Ast::QualifiedTypeSpec& Ast::NodeFactory::getQualifiedTypeSpec(const Ast::Token& pos, const std::string& name) {
    Ast::Token token(pos.row(), pos.col(), name);
    const Ast::TypeSpec& typeSpec = _ctx.getRootTypeSpec<Ast::TypeSpec>(token);
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, typeSpec, false);
    return qTypeSpec;
}

inline const Ast::Expr& Ast::NodeFactory::getDefaultValue(const Ast::TypeSpec& typeSpec, const Ast::Token& name) {
    const Ast::TypeSpec* ts = resolveTypedef(typeSpec);

    const Ast::Unit::DefaultValueList& list = _module.unit().defaultValueList();
    Ast::Unit::DefaultValueList::const_iterator it = list.find(ts);
    if(it != list.end()) {
        return z::ref(it->second);
    }

    const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(ts);
    if(td != 0) {
        const std::string tdName = z::ref(td).name().string() ; // getTypeSpecName(z::ref(td), GenMode::Import); \todo this is incorrect, it will match any type called, say, list.
        if(tdName == "pointer") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, z::ref(td), false);
            Ast::ExprList& exprList = addExprList(name);
            const Ast::QualifiedTypeSpec& subType = z::ref(td).at(0);
            const Ast::Expr& nameExpr = getDefaultValue(subType.typeSpec(), name);
            exprList.addExpr(nameExpr);
            Ast::PointerInstanceExpr& expr = addUnitNode(new Ast::PointerInstanceExpr(name, qTypeSpec, z::ref(td), exprList));
            return expr;
        }
        if(tdName == "list") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, z::ref(td), false);
            Ast::ListList& llist = addUnitNode(new Ast::ListList(name));
            const Ast::QualifiedTypeSpec* qlType = z::ref(td).list().at(0);
            llist.valueType(z::ref(qlType));
            Ast::ListExpr& expr = addUnitNode(new Ast::ListExpr(name, qTypeSpec, llist));
            return expr;
        }
        if(tdName == "dict") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, z::ref(td), false);
            Ast::DictList& llist = addUnitNode(new Ast::DictList(name));
            const Ast::QualifiedTypeSpec* qlType = z::ref(td).list().at(0);
            const Ast::QualifiedTypeSpec* qrType = z::ref(td).list().at(1);
            llist.keyType(z::ref(qlType));
            llist.valueType(z::ref(qrType));
            Ast::DictExpr& expr = addUnitNode(new Ast::DictExpr(name, qTypeSpec, llist));
            return expr;
        }
        if(tdName == "ptr") {
            Ast::Token value(name.row(), name.col(), "0");
            Ast::ConstantIntExpr& expr = aConstantIntExpr(value);
            return expr;
        }
    }

    const Ast::EnumDefn* ed = dynamic_cast<const Ast::EnumDefn*>(ts);
    if(ed != 0) {
        const Ast::Scope::List::const_iterator rit = z::ref(ed).list().begin();
        if(rit == z::ref(ed).list().end()) {
            throw z::Exception("%s empty enum type '%s'\n", err(_ctx.filename(), typeSpec.name()).c_str(), z::ref(ed).name().text());
        }
        const Ast::VariableDefn& vref = z::ref(*rit);
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, typeSpec, false);
        Ast::EnumMemberExpr& typeSpecMemberExpr = addUnitNode(new Ast::EnumMemberExpr(name, qTypeSpec, typeSpec, vref));
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

    throw z::Exception("%s No default value for type '%s'\n", err(_ctx.filename(), name).c_str(), z::ref(ts).name().text());
}

inline const Ast::Expr& Ast::NodeFactory::convertExprToExpectedTypeSpec(const Ast::Token& pos, const Ast::Expr& initExpr) {
    // check if lhs is a pointer to rhs, if so auto-convert
    const Ast::TemplateDefn* ts = _ctx.isPointerToExprExpected(initExpr);
    if(ts) {
        Ast::PointerInstanceExpr* expr = aPointerInstanceExpr(pos, initExpr);
        return z::ref(expr);
    }

    const Ast::QualifiedTypeSpec* qts = _ctx.getExpectedTypeSpecIfAny();
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
            Context::CoercionResult::T mode = Context::CoercionResult::None;
            _ctx.canCoerceX(z::ref(qts), rhsQts, mode);
            if(mode == Context::CoercionResult::Rhs) {
                const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, z::ref(qts).isConst(), z::ref(qts).typeSpec(), true);
                Ast::TypecastExpr& typecastExpr = addUnitNode(new Ast::DynamicTypecastExpr(pos, typeSpec, initExpr));
                return typecastExpr;
            }
        }

        // check if initExpr can be converted to expected type, if any
        Context::CoercionResult::T mode = Context::CoercionResult::None;
        const Ast::QualifiedTypeSpec* cqts = _ctx.canCoerceX(z::ref(qts), initExpr.qTypeSpec(), mode);
        if(mode != Context::CoercionResult::Lhs) {
            throw z::Exception("%s Cannot convert expression from '%s' to '%s' (%d)\n",
                            err(_ctx.filename(), pos).c_str(),
                            getQualifiedTypeSpecName(initExpr.qTypeSpec(), GenMode::Import).c_str(),
                            getQualifiedTypeSpecName(z::ref(qts), GenMode::Import).c_str(),
                            mode
                            );
        }
        if(z::ptr(z::ref(cqts).typeSpec()) != z::ptr(initExpr.qTypeSpec().typeSpec())) {
            const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, z::ref(cqts).isConst(), z::ref(qts).typeSpec(), false);
            Ast::TypecastExpr& typecastExpr = addUnitNode(new Ast::StaticTypecastExpr(pos, typeSpec, initExpr));
            return typecastExpr;
        }
    }

    return initExpr;
}

inline Ast::Scope& Ast::NodeFactory::addScope(const Ast::Token& pos, const Ast::ScopeType::T& type) {
    Ast::Scope& scope = addUnitNode(new Ast::Scope(pos, type));
    return scope;
}

inline Ast::ExprList& Ast::NodeFactory::addExprList(const Ast::Token& pos) {
    Ast::ExprList& exprList = addUnitNode(new Ast::ExprList(pos));
    return exprList;
}

inline Ast::TemplateDefn& Ast::NodeFactory::createTemplateDefn(const Ast::Token& pos, const std::string& name) {
    Ast::Token token(pos.row(), pos.col(), name);
    const Ast::TemplateDecl& templateDecl = _ctx.getRootTypeSpec<Ast::TemplateDecl>(token);
    Ast::TemplateDefn& templateDefn = addUnitNode(new Ast::TemplateDefn(_ctx.currentTypeSpec(), token, Ast::DefinitionType::Final, templateDecl));
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
    throw z::Exception("%s Unknown function return type '%s'\n", err(_ctx.filename(), pos).c_str(), function.name().text());
}

inline const Ast::QualifiedTypeSpec& Ast::NodeFactory::getFunctionReturnType(const Ast::Token& pos, const Ast::Function& function) {
    if(function.sig().outScope().isTuple()) {
        const Ast::FunctionRetn& functionRetn = getFunctionRetn(pos, function);
        Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, functionRetn, false);
        return qTypeSpec;
    }
    return z::ref(function.sig().out().front()).qTypeSpec();
}

inline Ast::VariableDefn& Ast::NodeFactory::addVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name) {
    const Ast::Expr& initExpr = getDefaultValue(qualifiedTypeSpec.typeSpec(), name);
    Ast::VariableDefn& variableDef = addUnitNode(new Ast::VariableDefn(qualifiedTypeSpec, name, initExpr));
    return variableDef;
}

inline const Ast::TemplateDefn& Ast::NodeFactory::getTemplateDefn(const Ast::Token& name, const Ast::Expr& expr, const std::string& cname, const size_t& len) {
    const Ast::TypeSpec& typeSpec = expr.qTypeSpec().typeSpec();
    if(typeSpec.name().string() != cname) {
        throw z::Exception("%s Expression is not of %s type: %s (1)\n", err(_ctx.filename(), name).c_str(), cname.c_str(), typeSpec.name().text());
    }
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(typeSpec));
    if(templateDefn == 0) {
        throw z::Exception("%s Expression is not of %s type: %s (2)\n", err(_ctx.filename(), name).c_str(), cname.c_str(), typeSpec.name().text());
    }
    if(z::ref(templateDefn).list().size() != len) {
        throw z::Exception("%s Expression is not of %s type: %s (3)\n", err(_ctx.filename(), name).c_str(), cname.c_str(), typeSpec.name().text());
    }
    return z::ref(templateDefn);
}

inline Ast::FunctionDecl& Ast::NodeFactory::addFunctionDecl(const Ast::TypeSpec& parent, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();
    Ast::Scope& xref = addScope(name, Ast::ScopeType::XRef);
    Ast::FunctionDecl& functionDecl = addUnitNode(new Ast::FunctionDecl(parent, name, defType, functionSig, xref));
    Ast::Token token1(name.row(), name.col(), "_Out");
    Ast::FunctionRetn& functionRetn = addUnitNode(new Ast::FunctionRetn(functionDecl, token1, functionSig.outScope()));
    functionDecl.addChild(functionRetn);
    return functionDecl;
}

inline Ast::ValueInstanceExpr& Ast::NodeFactory::getValueInstanceExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec, const Ast::TemplateDefn& templateDefn, const Ast::Expr& expr) {
    unused(pos);
    Ast::ExprList& exprList = addExprList(pos);
    exprList.addExpr(expr);

    const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, qTypeSpec.isConst(), qTypeSpec.typeSpec(), true);
    Ast::ValueInstanceExpr& valueInstanceExpr = addUnitNode(new Ast::ValueInstanceExpr(pos, typeSpec, templateDefn, exprList));
    return valueInstanceExpr;
}

////////////////////////////////////////////////////////////
Ast::NodeFactory::NodeFactory(Context& ctx, Compiler& compiler, Ast::Module& module)
    : _ctx(ctx), _compiler(compiler), _module(module), _lastToken(0, 0, "") {
}

Ast::NodeFactory::~NodeFactory() {
}

////////////////////////////////////////////////////////////
void Ast::NodeFactory::aUnitStatementList(const Ast::EnterNamespaceStatement& nss) {
    Ast::LeaveNamespaceStatement& lns = addUnitNode(new Ast::LeaveNamespaceStatement(getToken(), nss));
    if(_ctx.level() == 0) {
        _module.addGlobalStatement(lns);
    }
    _ctx.leaveNamespace();
}

void Ast::NodeFactory::aImportStatement(const Ast::Token& pos, const Ast::AccessType::T& accessType, const Ast::HeaderType::T& headerType, const Ast::DefinitionType::T& defType, Ast::NamespaceList& list) {
    Ast::ImportStatement& statement = addUnitNode(new Ast::ImportStatement(pos, accessType, headerType, defType, list));
    _module.addGlobalStatement(statement);

    if(statement.defType() != Ast::DefinitionType::Native) {
        std::string filename;
        std::string sep = "";
        for(Ast::NamespaceList::List::const_iterator it = statement.list().begin(); it != statement.list().end(); ++it) {
            const Ast::Token& name = z::ref(*it).name();
            filename += sep;
            filename += name.text();
            sep = "/";
        }
        filename += ".ipp";
        _compiler.import(_module, filename, _ctx.level());
    }
}

Ast::NamespaceList* Ast::NodeFactory::aImportNamespaceList(Ast::NamespaceList& list, const Ast::Token &name) {
    Ast::Namespace& ns = addUnitNode(new Ast::Namespace(_ctx.currentTypeSpec(), name));
    list.addNamespace(ns);
    return z::ptr(list);
}

Ast::NamespaceList* Ast::NodeFactory::aImportNamespaceList(const Ast::Token& name) {
    Ast::NamespaceList& list = addUnitNode(new Ast::NamespaceList(name));
    return aImportNamespaceList(list, name);
}

Ast::EnterNamespaceStatement* Ast::NodeFactory::aNamespaceStatement(const Ast::Token& pos, Ast::NamespaceList& list) {
    Ast::EnterNamespaceStatement& statement = addUnitNode(new Ast::EnterNamespaceStatement(pos, list));
    if(_ctx.level() == 0) {
        _module.addGlobalStatement(statement);
    }
    return z::ptr(statement);
}

Ast::EnterNamespaceStatement* Ast::NodeFactory::aNamespaceStatement() {
    Ast::NamespaceList& list = addUnitNode(new Ast::NamespaceList(getToken()));
    return aNamespaceStatement(getToken(), list);
}

inline Ast::Namespace& Ast::NodeFactory::getUnitNamespace(const Ast::Token& name) {
    if(_ctx.level() == 0) {
        Ast::Namespace& ns = addUnitNode(new Ast::Namespace(_ctx.currentTypeSpec(), name));
        _ctx.currentTypeSpec().addChild(ns);
        return ns;
    }

    Ast::Namespace* cns = _module.unit().importNS().hasChild<Ast::Namespace>(name.string());
    if(cns) {
        return z::ref(cns);
    }

    Ast::Namespace& ns = addUnitNode(new Ast::Namespace(_ctx.currentTypeSpec(), name));
    _ctx.currentTypeSpec().addChild(ns);
    return ns;
}

Ast::NamespaceList* Ast::NodeFactory::aUnitNamespaceList(Ast::NamespaceList& list, const Ast::Token& name) {
    Ast::Namespace& ns = getUnitNamespace(name);
    _ctx.enterTypeSpec(ns);
    if(_ctx.level() == 0) {
        _module.unit().addNamespacePart(name);
    }
    _ctx.addNamespace(ns);
    list.addNamespace(ns);
    return z::ptr(list);
}

Ast::NamespaceList* Ast::NodeFactory::aUnitNamespaceList(const Ast::Token& name) {
    Ast::NamespaceList& list = addUnitNode(new Ast::NamespaceList(name));
    return aUnitNamespaceList(list, name);
}

Ast::Statement* Ast::NodeFactory::aGlobalStatement(Ast::Statement& statement) {
    if(_ctx.level() == 0) {
        _module.addGlobalStatement(statement);
    }
    printf("xx1\n");
    if(_ctx.hasStatementVisitor()) {
        printf("xx2\n");
        _ctx.statementVisitor().visitNode(statement);
    }
    return z::ptr(statement);
}

Ast::Statement* Ast::NodeFactory::aGlobalTypeSpecStatement(const Ast::AccessType::T& accessType, Ast::UserDefinedTypeSpec& typeSpec){
    typeSpec.accessType(accessType);
    Ast::UserDefinedTypeSpecStatement* statement = aUserDefinedTypeSpecStatement(typeSpec);
    return aGlobalStatement(z::ref(statement));
}

void Ast::NodeFactory::aGlobalCoerceStatement(Ast::CoerceList& list) {
    _module.unit().addCoercionList(list);
}

Ast::CoerceList* Ast::NodeFactory::aCoerceList(Ast::CoerceList& list, const Ast::TypeSpec& typeSpec) {
    list.addTypeSpec(typeSpec);
    return z::ptr(list);
}

Ast::CoerceList* Ast::NodeFactory::aCoerceList(const Ast::TypeSpec& typeSpec) {
    Ast::CoerceList& list = addUnitNode(new Ast::CoerceList(typeSpec.pos()));
    list.addTypeSpec(typeSpec);
    return z::ptr(list);
}

void Ast::NodeFactory::aGlobalDefaultStatement(const Ast::TypeSpec& typeSpec, const Ast::Expr& expr) {
    _module.unit().addDefaultValue(typeSpec, expr);
}

Ast::TypedefDecl* Ast::NodeFactory::aTypedefDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::TypedefDecl& typedefDefn = addUnitNode(new Ast::TypedefDecl(_ctx.currentTypeSpec(), name, defType));
    _ctx.currentTypeSpec().addChild(typedefDefn);
    return z::ptr(typedefDefn);
}

Ast::TypedefDefn* Ast::NodeFactory::aTypedefDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::QualifiedTypeSpec& qTypeSpec) {
    Ast::TypedefDefn& typedefDefn = addUnitNode(new Ast::TypedefDefn(_ctx.currentTypeSpec(), name, defType, qTypeSpec));
    _ctx.currentTypeSpec().addChild(typedefDefn);
    return z::ptr(typedefDefn);
}

Ast::TemplatePartList* Ast::NodeFactory::aTemplatePartList(Ast::TemplatePartList& list, const Ast::Token& name) {
    list.addPart(name);
    return z::ptr(list);
}

Ast::TemplatePartList* Ast::NodeFactory::aTemplatePartList(const Ast::Token& name) {
    Ast::TemplatePartList& list = addUnitNode(new Ast::TemplatePartList(name));
    list.addPart(name);
    return z::ptr(list);
}

Ast::TemplateDecl* Ast::NodeFactory::aTemplateDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::TemplatePartList& list) {
    Ast::TemplateDecl& templateDefn = addUnitNode(new Ast::TemplateDecl(_ctx.currentTypeSpec(), name, defType, list));
    _ctx.currentTypeSpec().addChild(templateDefn);
    return z::ptr(templateDefn);
}

Ast::EnumDefn* Ast::NodeFactory::aEnumDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::Scope& list) {
    Ast::EnumDefn& enumDefn = addUnitNode(new Ast::EnumDefn(_ctx.currentTypeSpec(), name, defType, list));
    _ctx.currentTypeSpec().addChild(enumDefn);
    return z::ptr(enumDefn);
}

Ast::EnumDefn* Ast::NodeFactory::aEnumDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& scope = addScope(name, Ast::ScopeType::Member);
    return aEnumDefn(name, defType, scope);
}

Ast::Scope* Ast::NodeFactory::aEnumMemberDefnList(Ast::Scope& list, const Ast::VariableDefn& variableDefn) {
    list.addVariableDef(variableDefn);
    return z::ptr(list);
}

Ast::Scope* Ast::NodeFactory::aEnumMemberDefnList(const Ast::VariableDefn& variableDefn) {
    Ast::Scope& scope = addScope(variableDefn.pos(), Ast::ScopeType::Member);
    return aEnumMemberDefnList(scope, variableDefn);
}

Ast::VariableDefn* Ast::NodeFactory::aEnumMemberDefn(const Ast::Token& name) {
    Ast::Token value(name.row(), name.col(), "#");
    const Ast::ConstantIntExpr& initExpr = aConstantIntExpr(value);
    Ast::VariableDefn& variableDefn = addUnitNode(new Ast::VariableDefn(initExpr.qTypeSpec(), name, initExpr));
    return z::ptr(variableDefn);
}

Ast::VariableDefn* Ast::NodeFactory::aEnumMemberDefn(const Ast::Token& name, const Ast::Expr& initExpr) {
    Ast::VariableDefn& variableDefn = addUnitNode(new Ast::VariableDefn(initExpr.qTypeSpec(), name, initExpr));
    return z::ptr(variableDefn);
}

Ast::StructDecl* Ast::NodeFactory::aStructDecl(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::StructDecl& structDecl = addUnitNode(new Ast::StructDecl(_ctx.currentTypeSpec(), name, defType));
    _ctx.currentTypeSpec().addChild(structDecl);
    return z::ptr(structDecl);
}

Ast::RootStructDefn* Ast::NodeFactory::aLeaveRootStructDefn(Ast::RootStructDefn& structDefn) {
    _ctx.leaveTypeSpec(structDefn);
    Ast::StructInitStatement& statement = addUnitNode(new Ast::StructInitStatement(structDefn.pos(), structDefn));
    structDefn.block().addStatement(statement);
    return z::ptr(structDefn);
}

Ast::ChildStructDefn* Ast::NodeFactory::aLeaveChildStructDefn(Ast::ChildStructDefn& structDefn) {
    _ctx.leaveTypeSpec(structDefn);
    Ast::StructInitStatement& statement = addUnitNode(new Ast::StructInitStatement(structDefn.pos(), structDefn));
    structDefn.block().addStatement(statement);
    return z::ptr(structDefn);
}

Ast::RootStructDefn* Ast::NodeFactory::aEnterRootStructDefn(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& list = addScope(name, Ast::ScopeType::Member);
    Ast::CompoundStatement& block = addUnitNode(new Ast::CompoundStatement(name));
    Ast::RootStructDefn& structDefn = addUnitNode(new Ast::RootStructDefn(_ctx.currentTypeSpec(), name, defType, list, block));
    _ctx.currentTypeSpec().addChild(structDefn);
    _ctx.enterTypeSpec(structDefn);
    return z::ptr(structDefn);
}

Ast::ChildStructDefn* Ast::NodeFactory::aEnterChildStructDefn(const Ast::Token& name, const Ast::StructDefn& base, const Ast::DefinitionType::T& defType) {
    if(base.defType() == Ast::DefinitionType::Final) {
        throw z::Exception("%s base struct is not abstract '%s'\n", err(_ctx.filename(), name).c_str(), base.name().text());
    }
    Ast::Scope& list = addScope(name, Ast::ScopeType::Member);
    Ast::CompoundStatement& block = addUnitNode(new Ast::CompoundStatement(name));
    Ast::ChildStructDefn& structDefn = addUnitNode(new Ast::ChildStructDefn(_ctx.currentTypeSpec(), base, name, defType, list, block));
    _ctx.currentTypeSpec().addChild(structDefn);
    _ctx.enterTypeSpec(structDefn);
    return z::ptr(structDefn);
}

void Ast::NodeFactory::aStructMemberVariableDefn(const Ast::VariableDefn& vdef) {
    Ast::StructDefn& sd = _ctx.getCurrentStructDefn(vdef.name());
    sd.addVariable(vdef);
    Ast::StructMemberVariableStatement& statement = addUnitNode(new Ast::StructMemberVariableStatement(vdef.pos(), sd, vdef));
    sd.block().addStatement(statement);
}

void Ast::NodeFactory::aStructMemberTypeDefn(Ast::UserDefinedTypeSpec& typeSpec) {
    Ast::StructDefn& sd = _ctx.getCurrentStructDefn(typeSpec.name());
    typeSpec.accessType(Ast::AccessType::Parent);
    Ast::Statement* statement = aUserDefinedTypeSpecStatement(typeSpec);
    sd.block().addStatement(z::ref(statement));
}

void Ast::NodeFactory::aStructMemberPropertyDefn(Ast::PropertyDecl& typeSpec) {
    aStructMemberTypeDefn(typeSpec);
    Ast::StructDefn& sd = _ctx.getCurrentStructDefn(typeSpec.name());
    sd.addProperty(typeSpec);
}

Ast::PropertyDeclRW* Ast::NodeFactory::aStructPropertyDeclRW(const Ast::Token& pos, const Ast::QualifiedTypeSpec& propertyType, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::PropertyDeclRW& structPropertyDecl = addUnitNode(new Ast::PropertyDeclRW(_ctx.currentTypeSpec(), name, defType, propertyType));
    _ctx.currentTypeSpec().addChild(structPropertyDecl);
    return z::ptr(structPropertyDecl);
}

Ast::PropertyDeclRO* Ast::NodeFactory::aStructPropertyDeclRO(const Ast::Token& pos, const Ast::QualifiedTypeSpec& propertyType, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::PropertyDeclRO& structPropertyDecl = addUnitNode(new Ast::PropertyDeclRO(_ctx.currentTypeSpec(), name, defType, propertyType));
    _ctx.currentTypeSpec().addChild(structPropertyDecl);
    return z::ptr(structPropertyDecl);
}

Ast::RoutineDecl* Ast::NodeFactory::aRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const Ast::DefinitionType::T& defType) {
    Ast::RoutineDecl& routineDecl = addUnitNode(new Ast::RoutineDecl(_ctx.currentTypeSpec(), outType, name, in, defType));
    _ctx.currentTypeSpec().addChild(routineDecl);
    return z::ptr(routineDecl);
}

Ast::RoutineDecl* Ast::NodeFactory::aVarArgRoutineDecl(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::Scope& in = addScope(name, Ast::ScopeType::VarArg);
    Ast::RoutineDecl& routineDecl = addUnitNode(new Ast::RoutineDecl(_ctx.currentTypeSpec(), outType, name, in, defType));
    _ctx.currentTypeSpec().addChild(routineDecl);
    return z::ptr(routineDecl);
}

Ast::RoutineDefn* Ast::NodeFactory::aRoutineDefn(Ast::RoutineDefn& routineDefn, const Ast::CompoundStatement& block) {
    routineDefn.setBlock(block);
    _ctx.leaveScope(routineDefn.inScope());
    _ctx.leaveTypeSpec(routineDefn);
    _module.unit().addBody(addUnitNode(new Ast::RoutineBody(block.pos(), routineDefn, block)));
    return z::ptr(routineDefn);
}

Ast::RoutineDefn* Ast::NodeFactory::aEnterRoutineDefn(const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const Ast::DefinitionType::T& defType) {
    Ast::RoutineDefn& routineDefn = addUnitNode(new Ast::RoutineDefn(_ctx.currentTypeSpec(), outType, name, in, defType));
    _ctx.currentTypeSpec().addChild(routineDefn);
    _ctx.enterScope(in);
    _ctx.enterTypeSpec(routineDefn);
    return z::ptr(routineDefn);
}

Ast::FunctionDecl* Ast::NodeFactory::aFunctionDecl(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    Ast::FunctionDecl& functionDecl = addFunctionDecl(_ctx.currentTypeSpec(), functionSig, defType);
    _ctx.currentTypeSpec().addChild(functionDecl);
    return z::ptr(functionDecl);
}

Ast::RootFunctionDefn* Ast::NodeFactory::aRootFunctionDefn(Ast::RootFunctionDefn& functionDefn, const Ast::CompoundStatement& block) {
    functionDefn.setBlock(block);
    _ctx.leaveScope(functionDefn.sig().inScope());
    _ctx.leaveScope(functionDefn.xrefScope());
    _ctx.leaveTypeSpec(functionDefn);
    _module.unit().addBody(addUnitNode(new Ast::FunctionBody(block.pos(), functionDefn, block)));
    return z::ptr(functionDefn);
}

Ast::RootFunctionDefn* Ast::NodeFactory::aEnterRootFunctionDefn(const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& defType) {
    const Ast::Token& name = functionSig.name();
    Ast::Scope& xref = addScope(name, Ast::ScopeType::XRef);
    Ast::RootFunctionDefn& functionDefn = addUnitNode(new Ast::RootFunctionDefn(_ctx.currentTypeSpec(), name, defType, functionSig, xref));
    _ctx.currentTypeSpec().addChild(functionDefn);
    _ctx.enterScope(functionDefn.xrefScope());
    _ctx.enterScope(functionSig.inScope());
    _ctx.enterTypeSpec(functionDefn);

    Ast::Token token1(name.row(), name.col(), "_Out");
    Ast::FunctionRetn& functionRetn = addUnitNode(new Ast::FunctionRetn(functionDefn, token1, functionSig.outScope()));
    functionDefn.addChild(functionRetn);

    return z::ptr(functionDefn);
}

Ast::ChildFunctionDefn* Ast::NodeFactory::aChildFunctionDefn(Ast::ChildFunctionDefn& functionDefn, const Ast::CompoundStatement& block) {
    functionDefn.setBlock(block);
    _ctx.leaveScope(functionDefn.sig().inScope());
    _ctx.leaveScope(functionDefn.xrefScope());
    _ctx.leaveTypeSpec(functionDefn);
    _module.unit().addBody(addUnitNode(new Ast::FunctionBody(block.pos(), functionDefn, block)));
    return z::ptr(functionDefn);
}

inline Ast::ChildFunctionDefn& Ast::NodeFactory::createChildFunctionDefn(Ast::TypeSpec& parent, const Ast::Function& base, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    if(base.defType() == Ast::DefinitionType::Final) {
        throw z::Exception("%s base function is not abstract '%s'\n", err(_ctx.filename(), name).c_str(), base.name().text());
    }
    Ast::Scope& xref = addScope(name, Ast::ScopeType::XRef);
    Ast::ChildFunctionDefn& functionDefn = addUnitNode(new Ast::ChildFunctionDefn(parent, name, defType, base.sig(), xref, base));
    parent.addChild(functionDefn);
    _ctx.enterScope(functionDefn.xrefScope());
    _ctx.enterScope(base.sig().inScope());
    _ctx.enterTypeSpec(functionDefn);
    return functionDefn;
}

Ast::ChildFunctionDefn* Ast::NodeFactory::aEnterChildFunctionDefn(const Ast::TypeSpec& base, const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(z::ptr(base));
    if(function == 0) {
        throw z::Exception("%s base type not a function '%s'\n", err(_ctx.filename(), name).c_str(), base.name().text());
    }
    Ast::ChildFunctionDefn& functionDefn = createChildFunctionDefn(_ctx.currentTypeSpec(), z::ref(function), name, defType);
    return z::ptr(functionDefn);
}

Ast::EventDecl* Ast::NodeFactory::aEventDecl(const Ast::Token& pos, const Ast::VariableDefn& in, const Ast::DefinitionType::T& eventDefType, const Ast::FunctionSig& functionSig, const Ast::DefinitionType::T& handlerDefType) {
    const Ast::Token& name = functionSig.name();

    Ast::Token eventName(pos.row(), pos.col(), name.string());
    Ast::EventDecl& eventDef = addUnitNode(new Ast::EventDecl(_ctx.currentTypeSpec(), eventName, in, eventDefType));
    _ctx.currentTypeSpec().addChild(eventDef);

    Ast::Token handlerName(pos.row(), pos.col(), "Handler");
    Ast::FunctionSig* handlerSig = aFunctionSig(functionSig.outScope(), handlerName, functionSig.inScope());
    Ast::FunctionDecl& funDecl = addFunctionDecl(eventDef, z::ref(handlerSig), handlerDefType);
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
    Ast::FunctionDecl& addDecl = addFunctionDecl(eventDef, z::ref(addSig), eventDefType);
    eventDef.setAddFunction(addDecl);

    inAdd.addVariableDef(in);
    inAdd.addVariableDef(vdef);

    return z::ptr(eventDef);
}

Ast::FunctionSig* Ast::NodeFactory::aFunctionSig(const Ast::Scope& out, const Ast::Token& name, Ast::Scope& in) {
    Ast::FunctionSig& functionSig = addUnitNode(new Ast::FunctionSig(out, name, in));
    return z::ptr(functionSig);
}

Ast::FunctionSig* Ast::NodeFactory::aFunctionSig(const Ast::QualifiedTypeSpec& typeSpec, const Ast::Token& name, Ast::Scope& in) {
    Ast::Scope& out = addScope(name, Ast::ScopeType::Param);
    out.isTuple(false);

    Ast::Token oname(name.row(), name.col(), "_out");
    Ast::VariableDefn& vdef = addVariableDefn(typeSpec, oname);
    out.addVariableDef(vdef);

    return aFunctionSig(out, name, in);
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

Ast::Scope* Ast::NodeFactory::aParam() {
    Ast::Scope& list = addScope(getToken(), Ast::ScopeType::Param);
    return z::ptr(list);
}

Ast::VariableDefn* Ast::NodeFactory::aVariableDefn(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name, const Ast::Expr& initExpr) {
    const Ast::Expr& expr = convertExprToExpectedTypeSpec(name, initExpr);
    Ast::VariableDefn& variableDef = addUnitNode(new Ast::VariableDefn(qualifiedTypeSpec, name, expr));
    _ctx.popExpectedTypeSpecOrAuto(name, Context::ExpectedTypeSpec::etAssignment);
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
    _ctx.pushExpectedTypeSpec(Context::ExpectedTypeSpec::etAssignment, qTypeSpec);
    return z::ptr(qTypeSpec);
}

void Ast::NodeFactory::aAutoQualifiedVariableDefn() {
    _ctx.pushExpectedTypeSpec(Context::ExpectedTypeSpec::etAuto);
}

Ast::QualifiedTypeSpec* Ast::NodeFactory::aQualifiedTypeSpec(const Ast::Token& pos, const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef) {
    Ast::QualifiedTypeSpec& qualifiedTypeSpec = addQualifiedTypeSpec(pos, isConst, typeSpec, isRef);
    return z::ptr(qualifiedTypeSpec);
}

Ast::QualifiedTypeSpec* Ast::NodeFactory::aQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef) {
    return aQualifiedTypeSpec(getToken(), isConst, typeSpec, isRef);
}

const Ast::TemplateDecl* Ast::NodeFactory::aTemplateTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return _ctx.setCurrentChildTypeRef<Ast::TemplateDecl>(parent, name, "template");
}

const Ast::TemplateDecl* Ast::NodeFactory::aTemplateTypeSpec(const Ast::Token& name) {
    return _ctx.setCurrentRootTypeRef<Ast::TemplateDecl>(name);
}

const Ast::TemplateDecl* Ast::NodeFactory::aTemplateTypeSpec(const Ast::TemplateDecl& templateDecl) {
    return _ctx.resetCurrentTypeRef<Ast::TemplateDecl>(templateDecl);
}

const Ast::StructDefn* Ast::NodeFactory::aStructTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return _ctx.setCurrentChildTypeRef<Ast::StructDefn>(parent, name, "struct");
}

const Ast::StructDefn* Ast::NodeFactory::aStructTypeSpec(const Ast::Token& name) {
    return _ctx.setCurrentRootTypeRef<Ast::StructDefn>(name);
}

const Ast::StructDefn* Ast::NodeFactory::aStructTypeSpec(const Ast::StructDefn& structDefn) {
    return _ctx.resetCurrentTypeRef<Ast::StructDefn>(structDefn);
}

const Ast::Routine* Ast::NodeFactory::aRoutineTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return _ctx.setCurrentChildTypeRef<Ast::Routine>(parent, name, "routine");
}

const Ast::Routine* Ast::NodeFactory::aRoutineTypeSpec(const Ast::Token& name) {
    return _ctx.setCurrentRootTypeRef<Ast::Routine>(name);
}

const Ast::Routine* Ast::NodeFactory::aRoutineTypeSpec(const Ast::Routine& routine) {
    return _ctx.resetCurrentTypeRef<Ast::Routine>(routine);
}

const Ast::Function* Ast::NodeFactory::aFunctionTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return _ctx.setCurrentChildTypeRef<Ast::Function>(parent, name, "function");
}

const Ast::Function* Ast::NodeFactory::aFunctionTypeSpec(const Ast::Token& name) {
    return _ctx.setCurrentRootTypeRef<Ast::Function>(name);
}

const Ast::Function* Ast::NodeFactory::aFunctionTypeSpec(const Ast::Function& function) {
    return _ctx.resetCurrentTypeRef<Ast::Function>(function);
}

const Ast::EventDecl* Ast::NodeFactory::aEventTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return _ctx.setCurrentChildTypeRef<Ast::EventDecl>(parent, name, "event");
}

const Ast::EventDecl* Ast::NodeFactory::aEventTypeSpec(const Ast::Token& name) {
    return _ctx.setCurrentRootTypeRef<Ast::EventDecl>(name);
}

const Ast::EventDecl* Ast::NodeFactory::aEventTypeSpec(const Ast::EventDecl& event) {
    return _ctx.resetCurrentTypeRef<Ast::EventDecl>(event);
}

const Ast::TypeSpec* Ast::NodeFactory::aOtherTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) {
    return _ctx.setCurrentChildTypeRef<Ast::TypeSpec>(parent, name, "parent");
}

const Ast::TypeSpec* Ast::NodeFactory::aOtherTypeSpec(const Ast::Token& name) {
    return _ctx.setCurrentRootTypeRef<Ast::TypeSpec>(name);
}

const Ast::TypeSpec* Ast::NodeFactory::aTypeSpec(const Ast::TypeSpec& TypeSpec) {
    return _ctx.resetCurrentTypeRef<Ast::TypeSpec>(TypeSpec);
}

const Ast::TemplateDefn* Ast::NodeFactory::aTemplateDefnTypeSpec(const Ast::TemplateDecl& typeSpec, const Ast::TemplateTypePartList& list) {
    Ast::TemplateDefn& templateDefn = addUnitNode(new Ast::TemplateDefn(_ctx.currentTypeSpec(), typeSpec.name(), Ast::DefinitionType::Final, typeSpec));
    for(Ast::TemplateTypePartList::List::const_iterator it = list.list().begin(); it != list.list().end(); ++it) {
        const Ast::QualifiedTypeSpec& part = z::ref(*it);
        templateDefn.addType(part);
    }
    return z::ptr(templateDefn);
}

Ast::TemplateTypePartList* Ast::NodeFactory::aTemplateTypePartList(Ast::TemplateTypePartList& list, const Ast::QualifiedTypeSpec& qTypeSpec) {
    list.addType(qTypeSpec);
    return z::ptr(list);
}

Ast::TemplateTypePartList* Ast::NodeFactory::aTemplateTypePartList(const Ast::QualifiedTypeSpec& qTypeSpec) {
    Ast::TemplateTypePartList& list = addUnitNode(new Ast::TemplateTypePartList(getToken()));
    return aTemplateTypePartList(list, qTypeSpec);
}

Ast::UserDefinedTypeSpecStatement* Ast::NodeFactory::aUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec) {
    Ast::UserDefinedTypeSpecStatement& userDefinedTypeSpecStatement = addUnitNode(new Ast::UserDefinedTypeSpecStatement(typeSpec.pos(), typeSpec));
    return z::ptr(userDefinedTypeSpecStatement);
}

Ast::AutoStatement* Ast::NodeFactory::aAutoStatement(const Ast::VariableDefn& defn) {
    Ast::AutoStatement& localStatement = addUnitNode(new Ast::AutoStatement(defn.pos(), defn));
    _ctx.currentScope().addVariableDef(defn);
    return z::ptr(localStatement);
}

Ast::ExprStatement* Ast::NodeFactory::aExprStatement(const Ast::Expr& expr) {
    Ast::ExprStatement& exprStatement = addUnitNode(new Ast::ExprStatement(expr.pos(), expr));
    return z::ptr(exprStatement);
}

Ast::PrintStatement* Ast::NodeFactory::aPrintStatement(const Ast::Token& pos, const Ast::Expr& expr) {
    Ast::PrintStatement& printStatement = addUnitNode(new Ast::PrintStatement(pos, expr));
    return z::ptr(printStatement);
}

Ast::IfStatement* Ast::NodeFactory::aIfStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& tblock) {
    Ast::IfStatement& ifStatement = addUnitNode(new Ast::IfStatement(pos, expr, tblock));
    return z::ptr(ifStatement);
}

Ast::IfElseStatement* Ast::NodeFactory::aIfElseStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& tblock, const Ast::CompoundStatement& fblock) {
    Ast::IfElseStatement& ifElseStatement = addUnitNode(new Ast::IfElseStatement(pos, expr, tblock, fblock));
    return z::ptr(ifElseStatement);
}

Ast::WhileStatement* Ast::NodeFactory::aWhileStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& block) {
    Ast::WhileStatement& whileStatement = addUnitNode(new Ast::WhileStatement(pos, expr, block));
    return z::ptr(whileStatement);
}

Ast::DoWhileStatement* Ast::NodeFactory::aDoWhileStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& block) {
    Ast::DoWhileStatement& doWhileStatement = addUnitNode(new Ast::DoWhileStatement(pos, expr, block));
    return z::ptr(doWhileStatement);
}

Ast::ForStatement* Ast::NodeFactory::aForStatement(const Ast::Token& pos, const Ast::Expr& init, const Ast::Expr& expr, const Ast::Expr& incr, const Ast::CompoundStatement& block) {
    Ast::ForExprStatement& forStatement = addUnitNode(new Ast::ForExprStatement(pos, init, expr, incr, block));
    return z::ptr(forStatement);
}

Ast::ForStatement* Ast::NodeFactory::aForStatement(const Ast::Token& pos, const Ast::VariableDefn& init, const Ast::Expr& expr, const Ast::Expr& incr, const Ast::CompoundStatement& block) {
    Ast::ForInitStatement& forStatement = addUnitNode(new Ast::ForInitStatement(pos, init, expr, incr, block));
    _ctx.leaveScope();
    return z::ptr(forStatement);
}

const Ast::VariableDefn* Ast::NodeFactory::aEnterForInit(const Ast::VariableDefn& init) {
    Ast::Scope& scope = addScope(init.pos(), Ast::ScopeType::Local);
    scope.addVariableDef(init);
    _ctx.enterScope(scope);
    return z::ptr(init);
}

Ast::ForeachStatement* Ast::NodeFactory::aForeachStatement(Ast::ForeachStatement& statement, const Ast::CompoundStatement& block) {
    statement.setBlock(block);
    _ctx.leaveScope();
    return z::ptr(statement);
}

Ast::ForeachStatement* Ast::NodeFactory::aEnterForeachInit(const Ast::Token& valName, const Ast::Expr& expr) {
    if(getTypeSpecName(expr.qTypeSpec().typeSpec(), GenMode::Import) == "string") {
        const Ast::QualifiedTypeSpec& valTypeSpec = getQualifiedTypeSpec(valName, "char");
        const Ast::VariableDefn& valDef = addVariableDefn(valTypeSpec, valName);
        Ast::Scope& scope = addScope(valName, Ast::ScopeType::Local);
        scope.addVariableDef(valDef);
        _ctx.enterScope(scope);
        Ast::ForeachStringStatement& foreachStatement = addUnitNode(new Ast::ForeachStringStatement(valName, valDef, expr));
        return z::ptr(foreachStatement);
    }

    const Ast::TemplateDefn& templateDefn = getTemplateDefn(valName, expr, "list", 1);
    const Ast::QualifiedTypeSpec& valTypeSpec = addQualifiedTypeSpec(valName, expr.qTypeSpec().isConst(), templateDefn.at(0).typeSpec(), true);
    const Ast::VariableDefn& valDef = addVariableDefn(valTypeSpec, valName);
    Ast::Scope& scope = addScope(valName, Ast::ScopeType::Local);
    scope.addVariableDef(valDef);
    _ctx.enterScope(scope);
    Ast::ForeachListStatement& foreachStatement = addUnitNode(new Ast::ForeachListStatement(valName, valDef, expr));
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
    _ctx.enterScope(scope);

    Ast::ForeachDictStatement& foreachStatement = addUnitNode(new Ast::ForeachDictStatement(keyName, keyDef, valDef, expr));
    return z::ptr(foreachStatement);
}

Ast::SwitchValueStatement* Ast::NodeFactory::aSwitchStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& list) {
    Ast::SwitchValueStatement& switchStatement = addUnitNode(new Ast::SwitchValueStatement(pos, expr, list));
    return z::ptr(switchStatement);
}

Ast::SwitchExprStatement* Ast::NodeFactory::aSwitchStatement(const Ast::Token& pos, const Ast::CompoundStatement& list) {
    Ast::SwitchExprStatement& switchStatement = addUnitNode(new Ast::SwitchExprStatement(pos, list));
    return z::ptr(switchStatement);
}

Ast::CompoundStatement* Ast::NodeFactory::aCaseList(Ast::CompoundStatement& list, const Ast::CaseStatement& stmt) {
    list.addStatement(stmt);
    return z::ptr(list);
}

Ast::CompoundStatement* Ast::NodeFactory::aCaseList(const Ast::CaseStatement& stmt) {
    Ast::CompoundStatement& list = addUnitNode(new Ast::CompoundStatement(stmt.pos()));
    list.addStatement(stmt);
    return z::ptr(list);
}

Ast::CaseStatement* Ast::NodeFactory::aCaseStatement(const Ast::Token& pos, const Ast::Expr& expr, const Ast::CompoundStatement& block) {
    Ast::CaseExprStatement& caseStatement = addUnitNode(new Ast::CaseExprStatement(pos, expr, block));
    return z::ptr(caseStatement);
}

Ast::CaseStatement* Ast::NodeFactory::aCaseStatement(const Ast::Token& pos, const Ast::CompoundStatement& block) {
    Ast::CaseDefaultStatement& caseStatement = addUnitNode(new Ast::CaseDefaultStatement(pos, block));
    return z::ptr(caseStatement);
}

Ast::BreakStatement* Ast::NodeFactory::aBreakStatement(const Ast::Token& pos) {
    Ast::BreakStatement& breakStatement = addUnitNode(new Ast::BreakStatement(pos));
    return z::ptr(breakStatement);
}

Ast::ContinueStatement* Ast::NodeFactory::aContinueStatement(const Ast::Token& pos) {
    Ast::ContinueStatement& continueStatement = addUnitNode(new Ast::ContinueStatement(pos));
    return z::ptr(continueStatement);
}

Ast::AddEventHandlerStatement* Ast::NodeFactory::aAddEventHandlerStatement(const Ast::Token& pos, const Ast::EventDecl& event, const Ast::Expr& source, Ast::FunctionTypeInstanceExpr& functor) {
    Ast::AddEventHandlerStatement& addEventHandlerStatement = addUnitNode(new Ast::AddEventHandlerStatement(pos, event, source, functor));
    _ctx.popExpectedTypeSpec(pos, Context::ExpectedTypeSpec::etEventHandler);
    return z::ptr(addEventHandlerStatement);
}

const Ast::EventDecl* Ast::NodeFactory::aEnterAddEventHandler(const Ast::EventDecl& eventDecl) {
    Ast::QualifiedTypeSpec& qts = addQualifiedTypeSpec(getToken(), false, eventDecl.handler(), false);
    _ctx.pushExpectedTypeSpec(Context::ExpectedTypeSpec::etEventHandler, qts);
    return z::ptr(eventDecl);
}

Ast::RoutineReturnStatement* Ast::NodeFactory::aRoutineReturnStatement(const Ast::Token& pos) {
    Ast::ExprList& exprList = addExprList(pos);
    Ast::RoutineReturnStatement& returnStatement = addUnitNode(new Ast::RoutineReturnStatement(pos, exprList));
    return z::ptr(returnStatement);
}

Ast::RoutineReturnStatement* Ast::NodeFactory::aRoutineReturnStatement(const Ast::Token& pos, const Ast::Expr& expr) {
    Ast::ExprList& exprList = addExprList(pos);
    exprList.addExpr(expr);
    Ast::RoutineReturnStatement& returnStatement = addUnitNode(new Ast::RoutineReturnStatement(pos, exprList));
    return z::ptr(returnStatement);
}

Ast::FunctionReturnStatement* Ast::NodeFactory::aFunctionReturnStatement(const Ast::Token& pos, const Ast::ExprList& exprList) {
    Ast::FunctionReturnStatement& returnStatement = addUnitNode(new Ast::FunctionReturnStatement(pos, exprList));
    return z::ptr(returnStatement);
}

Ast::CompoundStatement* Ast::NodeFactory::aStatementList() {
    Ast::CompoundStatement& statement = addUnitNode(new Ast::CompoundStatement(getToken()));
    return z::ptr(statement);
}

Ast::CompoundStatement* Ast::NodeFactory::aStatementList(Ast::CompoundStatement& list, const Ast::Statement& statement) {
    list.addStatement(statement);
    return z::ptr(list);
}

void Ast::NodeFactory::aEnterCompoundStatement(const Ast::Token& pos) {
    Ast::Scope& scope = addScope(pos, Ast::ScopeType::Local);
    _ctx.enterScope(scope);
}

void Ast::NodeFactory::aLeaveCompoundStatement() {
    _ctx.leaveScope();
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
    const Ast::QualifiedTypeSpec& qTypeSpec = _ctx.coerce(op2, rhs1.qTypeSpec(), rhs2.qTypeSpec());
    Ast::ConditionalExpr& expr = addUnitNode(new Ast::ConditionalExpr(qTypeSpec, op1, op2, lhs, rhs1, rhs2));
    return z::ptr(expr);
}

template <typename T>
inline Ast::Expr& Ast::NodeFactory::createBooleanExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(op, "bool");
    T& expr = addUnitNode(new T(qTypeSpec, op, lhs, rhs));
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
    const Ast::QualifiedTypeSpec& qTypeSpec = _ctx.coerce(op, lhs.qTypeSpec(), rhs.qTypeSpec());
    T& expr = addUnitNode(new T(qTypeSpec, op, lhs, rhs));
    return expr;
}

Ast::Expr& Ast::NodeFactory::aBinaryAssignEqualExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    const Ast::IndexExpr* indexExpr = dynamic_cast<const Ast::IndexExpr*>(z::ptr(lhs));
    if(indexExpr) {
        const Ast::QualifiedTypeSpec& qTypeSpec = _ctx.coerce(op, lhs.qTypeSpec(), rhs.qTypeSpec());
        Ast::SetIndexExpr& expr = addUnitNode(new Ast::SetIndexExpr(op, qTypeSpec, z::ref(indexExpr), rhs));
        return expr;
    }
    return createBinaryExpr<Ast::BinaryAssignEqualExpr>(op, lhs, rhs);
}

template <typename T>
inline Ast::Expr& Ast::NodeFactory::createBinaryOpExpr(const Ast::Token& op, const Ast::Expr& lhs, const Ast::Expr& rhs) {
    const Ast::IndexExpr* indexExpr = dynamic_cast<const Ast::IndexExpr*>(z::ptr(lhs));
    if(indexExpr) {
        throw z::Exception("%s Operator '%s' on index expression not implemented\n", err(_ctx.filename(), op).c_str(), op.text());
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
    T& expr = addUnitNode(new T(qTypeSpec, op, lhs));
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
    T& expr = addUnitNode(new T(qTypeSpec, op, rhs));
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
    _ctx.popExpectedTypeSpecOrAuto(pos, Context::ExpectedTypeSpec::etListVal);
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "list");
    templateDefn.addType(list.valueType());
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);

    Ast::ListExpr& expr = addUnitNode(new Ast::ListExpr(pos, qTypeSpec, list));
    return z::ptr(expr);
}

Ast::ListList* Ast::NodeFactory::aListList(const Ast::Token& pos, Ast::ListList& list, const Ast::ListItem& item) {
    list.addItem(item);
    const Ast::QualifiedTypeSpec& qValueTypeSpec = _ctx.coerce(pos, list.valueType(), item.valueExpr().qTypeSpec());
    list.valueType(qValueTypeSpec);
    return z::ptr(list);
}

Ast::ListList* Ast::NodeFactory::aListList(const Ast::Token& pos, const Ast::ListItem& item) {
    Ast::ListList& list = addUnitNode(new Ast::ListList(pos));
    list.addItem(item);
    const Ast::QualifiedTypeSpec& valType = _ctx.getExpectedTypeSpec(z::ptr(item.valueExpr().qTypeSpec()));
    list.valueType(valType);
    return z::ptr(list);
}

Ast::ListList* Ast::NodeFactory::aListList(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec) {
    Ast::ListList& list = addUnitNode(new Ast::ListList(pos));
    const Ast::QualifiedTypeSpec& valType = _ctx.getExpectedTypeSpec(z::ptr(qTypeSpec));
    list.valueType(valType);
    return z::ptr(list);
}

Ast::ListList* Ast::NodeFactory::aListList(const Ast::Token& pos) {
    Ast::ListList& list = addUnitNode(new Ast::ListList(pos));
    const Ast::QualifiedTypeSpec& valType = _ctx.getExpectedTypeSpec(0);
    list.valueType(valType);
    return z::ptr(list);
}

Ast::ListItem* Ast::NodeFactory::aListItem(const Ast::Expr& valueExpr) {
    const Ast::Expr& expr = convertExprToExpectedTypeSpec(valueExpr.pos(), valueExpr);
    Ast::ListItem& item = addUnitNode(new Ast::ListItem(valueExpr.pos(), expr));
//    popExpectedTypeSpec(valueExpr.pos(), ExpectedTypeSpec::etListVal);
    return z::ptr(item);
}

Ast::DictExpr* Ast::NodeFactory::aDictExpr(const Ast::Token& pos, const Ast::DictList& list) {
    _ctx.popExpectedTypeSpecOrAuto(pos, Context::ExpectedTypeSpec::etDictKey);
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "dict");
    templateDefn.addType(list.keyType());
    templateDefn.addType(list.valueType());
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);

    Ast::DictExpr& expr = addUnitNode(new Ast::DictExpr(pos, qTypeSpec, list));
    return z::ptr(expr);
}

Ast::DictList* Ast::NodeFactory::aDictList(const Ast::Token& pos, Ast::DictList& list, const Ast::DictItem& item) {
    list.addItem(item);
    const Ast::QualifiedTypeSpec& keyType = _ctx.coerce(pos, list.keyType(), item.keyExpr().qTypeSpec());
    const Ast::QualifiedTypeSpec& valType = _ctx.coerce(pos, list.valueType(), item.valueExpr().qTypeSpec());

    list.keyType(keyType);
    list.valueType(valType);
    return z::ptr(list);
}

Ast::DictList* Ast::NodeFactory::aDictList(const Ast::Token& pos, const Ast::DictItem& item) {
    Ast::DictList& list = addUnitNode(new Ast::DictList(pos));
    list.addItem(item);
    list.keyType(item.keyExpr().qTypeSpec());
    list.valueType(item.valueExpr().qTypeSpec());
    return z::ptr(list);
}

Ast::DictList* Ast::NodeFactory::aDictList(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qKeyTypeSpec, const Ast::QualifiedTypeSpec& qValueTypeSpec) {
    Ast::DictList& list = addUnitNode(new Ast::DictList(pos));
    list.keyType(qKeyTypeSpec);
    list.valueType(qValueTypeSpec);
    return z::ptr(list);
}

/// The sequence of calls in this function is important.
inline const Ast::Expr& Ast::NodeFactory::switchDictKeyValue(const Ast::Token& pos, const Ast::Context::ExpectedTypeSpec::Type& popType, const Ast::Context::ExpectedTypeSpec::Type& pushType, const size_t& idx, const Ast::Expr& initExpr) {
    const Ast::Expr& expr = convertExprToExpectedTypeSpec(pos, initExpr);
    bool isExpected = _ctx.popExpectedTypeSpecOrAuto(pos, popType);
    const Ast::TemplateDefn* td0 = _ctx.isEnteringList();

    if(isExpected) {
        if((td0) && (z::ref(td0).name().string() == "dict")) {
            const Ast::QualifiedTypeSpec& keyType = z::ref(td0).at(idx);
            _ctx.pushExpectedTypeSpec(pushType, keyType);
        } else {
            assert(false);
        }
    } else {
        _ctx.pushExpectedTypeSpec(Context::ExpectedTypeSpec::etAuto);
    }
    return expr;
}

Ast::DictItem* Ast::NodeFactory::aDictItem(const Ast::Token& pos, const Ast::Expr& keyExpr, const Ast::Expr& valueExpr) {
    const Ast::Expr& expr = switchDictKeyValue(pos, Context::ExpectedTypeSpec::etDictVal, Context::ExpectedTypeSpec::etDictKey, 0, valueExpr);
    Ast::DictItem& item = addUnitNode(new Ast::DictItem(pos, keyExpr, expr));
    return z::ptr(item);
}

const Ast::Expr* Ast::NodeFactory::aDictKey(const Ast::Expr& keyExpr) {
    const Ast::Expr& expr = switchDictKeyValue(keyExpr.pos(), Context::ExpectedTypeSpec::etDictKey, Context::ExpectedTypeSpec::etDictVal, 1, keyExpr);
    return z::ptr(expr);
}

const Ast::Token& Ast::NodeFactory::aEnterList(const Ast::Token& pos) {
    const Ast::TemplateDefn* td0 = _ctx.isEnteringList();
    if(td0) {
        if(z::ref(td0).name().string() == "list") {
            const Ast::QualifiedTypeSpec& valType = z::ref(td0).at(0);
            _ctx.pushExpectedTypeSpec(Context::ExpectedTypeSpec::etListVal, valType);
        } else if(z::ref(td0).name().string() == "dict") {
            const Ast::QualifiedTypeSpec& keyType = z::ref(td0).at(0);
            _ctx.pushExpectedTypeSpec(Context::ExpectedTypeSpec::etDictKey, keyType);
        } else {
            assert(false);
        }
    } else {
        _ctx.pushExpectedTypeSpec(Context::ExpectedTypeSpec::etAuto);
    }

    return pos;
}

Ast::FormatExpr* Ast::NodeFactory::aFormatExpr(const Ast::Token& pos, const Ast::Expr& stringExpr, const Ast::DictExpr& dictExpr) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "string");
    Ast::FormatExpr& formatExpr = addUnitNode(new Ast::FormatExpr(pos, qTypeSpec, stringExpr, dictExpr));
    return z::ptr(formatExpr);
}

Ast::RoutineCallExpr* Ast::NodeFactory::aRoutineCallExpr(const Ast::Token& pos, const Ast::Routine& routine, const Ast::ExprList& exprList) {
    _ctx.popCallArgList(pos, routine.inScope());
    const Ast::QualifiedTypeSpec& qTypeSpec = routine.outType();
    Ast::RoutineCallExpr& routineCallExpr = addUnitNode(new Ast::RoutineCallExpr(pos, qTypeSpec, routine, exprList));
    return z::ptr(routineCallExpr);
}

const Ast::Routine* Ast::NodeFactory::aEnterRoutineCall(const Ast::Routine& routine) {
    _ctx.pushCallArgList(routine.inScope());
    return z::ptr(routine);
}

Ast::FunctorCallExpr* Ast::NodeFactory::aFunctorCallExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::ExprList& exprList) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if(function == 0) {
        throw z::Exception("%s Unknown functor being called '%s'\n", err(_ctx.filename(), pos).c_str(), expr.qTypeSpec().typeSpec().name().text());
    }

    _ctx.popCallArgList(pos, z::ref(function).sig().inScope());

    const Ast::QualifiedTypeSpec& qTypeSpec = getFunctionReturnType(pos, z::ref(function));
    Ast::FunctorCallExpr& functorCallExpr = addUnitNode(new Ast::FunctorCallExpr(pos, qTypeSpec, expr, exprList));
    return z::ptr(functorCallExpr);
}

Ast::Expr* Ast::NodeFactory::aEnterFunctorCall(Ast::Expr& expr) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if(function == 0) {
        throw z::Exception("%s Unknown functor being called '%s'\n", err(_ctx.filename(), expr.pos()).c_str(), expr.qTypeSpec().typeSpec().name().text());
    }

    _ctx.pushCallArgList(z::ref(function).sig().inScope());
    return z::ptr(expr);
}

Ast::Expr* Ast::NodeFactory::aEnterFunctorCall(const Ast::Token& name) {
    Ast::VariableRefExpr* expr = aVariableRefExpr(name);
    return aEnterFunctorCall(z::ref(expr));
}

Ast::Expr* Ast::NodeFactory::aEnterFunctorCall(const Ast::Function& function) {
    Ast::QualifiedTypeSpec& qExprTypeSpec = addQualifiedTypeSpec(getToken(), false, function, false);
    Ast::ExprList& exprList = addExprList(getToken());
    Ast::FunctionInstanceExpr& expr = addUnitNode(new Ast::FunctionInstanceExpr(getToken(), qExprTypeSpec, function, exprList));
    return aEnterFunctorCall(expr);
}

Ast::ExprList* Ast::NodeFactory::aCallArgList(const Ast::Token& pos, Ast::ExprList& list, const Ast::Expr& expr) {
    const Ast::Expr& argExpr = convertExprToExpectedTypeSpec(pos, expr);
    list.addExpr(argExpr);
    _ctx.popCallArg(pos);
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

        Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "future");
        templateDefn.addType(qRetTypeSpec);
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);

        Ast::RunExpr& runExpr = addUnitNode(new Ast::RunExpr(pos, qTypeSpec, callExpr));
        return z::ptr(runExpr);
    }
    throw z::Exception("%s Unknown functor in run expression '%s'\n", err(_ctx.filename(), pos).c_str(), getQualifiedTypeSpecName(callExpr.expr().qTypeSpec(), GenMode::Import).c_str());
}

Ast::OrderedExpr* Ast::NodeFactory::aOrderedExpr(const Ast::Token& pos, const Ast::Expr& innerExpr) {
    Ast::OrderedExpr& expr = addUnitNode(new Ast::OrderedExpr(pos, innerExpr.qTypeSpec(), innerExpr));
    return z::ptr(expr);
}

Ast::IndexExpr* Ast::NodeFactory::aIndexExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::Expr& index) {
    const Ast::TypeSpec* listTypeSpec = resolveTypedef(expr.qTypeSpec().typeSpec());
    const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(listTypeSpec);
    if(td) {
        if(z::ref(td).name().string() == "list") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(td).at(0).isConst(), z::ref(td).at(0).typeSpec(), true);
            Ast::IndexExpr& indexExpr = addUnitNode(new Ast::IndexExpr(pos, qTypeSpec, expr, index));
            return z::ptr(indexExpr);
        }

        if(z::ref(td).name().string() == "dict") {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(td).at(1).isConst(), z::ref(td).at(1).typeSpec(), true);
            Ast::IndexExpr& indexExpr = addUnitNode(new Ast::IndexExpr(pos, qTypeSpec, expr, index));
            return z::ptr(indexExpr);
        }
    }

    const Ast::StructDefn* sd = dynamic_cast<const Ast::StructDefn*>(listTypeSpec);
    if(sd) {
        const Ast::Routine* routine = z::ref(sd).hasChild<const Ast::Routine>("at");
        if(routine) {
            const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(routine).outType().isConst(), z::ref(routine).outType().typeSpec(), true);
            Ast::IndexExpr& indexExpr = addUnitNode(new Ast::IndexExpr(pos, qTypeSpec, expr, index));
            return z::ptr(indexExpr);
        }
    }

    throw z::Exception("%s '%s' is not an indexable type\n", err(_ctx.filename(), pos).c_str(), getQualifiedTypeSpecName(expr.qTypeSpec(), GenMode::Import).c_str());
}

Ast::SpliceExpr* Ast::NodeFactory::aSpliceExpr(const Ast::Token& pos, const Ast::Expr& expr, const Ast::Expr& from, const Ast::Expr& to) {
    const Ast::TypeSpec* listTypeSpec = resolveTypedef(expr.qTypeSpec().typeSpec());
    const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(listTypeSpec);
    if((td) && (z::ref(td).name().string() == "list")) {
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, z::ref(td).at(0).isConst(), expr.qTypeSpec().typeSpec(), false);
        Ast::SpliceExpr& spliceExpr = addUnitNode(new Ast::SpliceExpr(pos, qTypeSpec, expr, from, to));
        return z::ptr(spliceExpr);
    }

    throw z::Exception("%s '%s' is not a spliceable type\n", err(_ctx.filename(), pos).c_str(), getQualifiedTypeSpecName(expr.qTypeSpec(), GenMode::Import).c_str());
}

Ast::TypeofTypeExpr* Ast::NodeFactory::aTypeofTypeExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& typeSpec) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "type");
    Ast::TypeofTypeExpr& typeofExpr = addUnitNode(new Ast::TypeofTypeExpr(pos, qTypeSpec, typeSpec));
    return z::ptr(typeofExpr);
}

Ast::TypeofExprExpr* Ast::NodeFactory::aTypeofExprExpr(const Ast::Token& pos, const Ast::Expr& expr) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(pos, "type");
    Ast::TypeofExprExpr& typeofExpr = addUnitNode(new Ast::TypeofExprExpr(pos, qTypeSpec, expr));
    return z::ptr(typeofExpr);
}

Ast::TypecastExpr* Ast::NodeFactory::aTypecastExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr) {
    unused(pos);
    /// \todo check if canCoerce
    const Ast::TemplateDefn* subType = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if((subType) && (z::ref(subType).name().string() == "pointer")) {
        const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, qTypeSpec.isConst(), qTypeSpec.typeSpec(), true);
        Ast::TypecastExpr& typecastExpr = addUnitNode(new Ast::DynamicTypecastExpr(pos, typeSpec, expr));
        return z::ptr(typecastExpr);
    }

    Ast::TypecastExpr& typecastExpr = addUnitNode(new Ast::StaticTypecastExpr(pos, qTypeSpec, expr));
    return z::ptr(typecastExpr);
}

Ast::PointerInstanceExpr* Ast::NodeFactory::aPointerInstanceExpr(const Ast::Token& pos, const Ast::Expr& expr) {
    Ast::TemplateDefn& templateDefn = createTemplateDefn(pos, "pointer");
    const Ast::QualifiedTypeSpec& typeSpec = addQualifiedTypeSpec(pos, expr.qTypeSpec().isConst(), expr.qTypeSpec().typeSpec(), true);
    templateDefn.addType(typeSpec);

    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn, false);
    Ast::ExprList& exprList = addExprList(pos);
    exprList.addExpr(expr);

    Ast::PointerInstanceExpr& pointerExpr = addUnitNode(new Ast::PointerInstanceExpr(pos, qTypeSpec, templateDefn, exprList));
    return z::ptr(pointerExpr);
}

Ast::ValueInstanceExpr* Ast::NodeFactory::aValueInstanceExpr(const Ast::Token& pos, const Ast::QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr) {
    const Ast::TemplateDefn* subType = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if((subType == 0) || (z::ref(subType).name().string() != "pointer")) {
        throw z::Exception("%s Expression is not a pointer to %s\n", err(_ctx.filename(), pos).c_str(), qTypeSpec.typeSpec().name().text());
    }

    Ast::ValueInstanceExpr& valueInstanceExpr = getValueInstanceExpr(pos, qTypeSpec, z::ref(subType), expr);
    return z::ptr(valueInstanceExpr);
}

Ast::ValueInstanceExpr* Ast::NodeFactory::aValueInstanceExpr(const Ast::Token& pos, const Ast::Expr& expr) {
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(expr.qTypeSpec().typeSpec()));
    if(templateDefn) {
        if(z::ref(templateDefn).name().string() == "pointer") {
            Ast::ValueInstanceExpr& valueInstanceExpr = getValueInstanceExpr(pos, z::ref(templateDefn).at(0), z::ref(templateDefn), expr);
            return z::ptr(valueInstanceExpr);
        }
    }

    throw z::Exception("%s Expression is not a pointer to %s\n", err(_ctx.filename(), pos).c_str(), expr.qTypeSpec().typeSpec().name().text());
}

Ast::TemplateDefnInstanceExpr* Ast::NodeFactory::aTemplateDefnInstanceExpr(const Ast::Token& pos, const Ast::TemplateDefn& templateDefn, const Ast::ExprList& exprList) {
    std::string name = templateDefn.name().string();
    if(name == "pointer") {
        Ast::TemplateDefn& newTemplateDefn = createTemplateDefn(pos, "pointer");
        const Ast::QualifiedTypeSpec& newTypeSpec = addQualifiedTypeSpec(pos, false, templateDefn.at(0).typeSpec(), true);
        newTemplateDefn.addType(newTypeSpec);

        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, newTemplateDefn, false);
        Ast::PointerInstanceExpr& expr = addUnitNode(new Ast::PointerInstanceExpr(pos, qTypeSpec, newTemplateDefn, exprList));
        return z::ptr(expr);
    }

    if(name == "value") {
        return aValueInstanceExpr(pos, templateDefn.at(0), exprList.at(0));
    }

    throw z::Exception("%s Invalid template instantiation %s\n", err(_ctx.filename(), pos).c_str(), templateDefn.name().text());
}

Ast::VariableRefExpr* Ast::NodeFactory::aVariableRefExpr(const Ast::Token& name) {
    Ast::RefType::T refType = Ast::RefType::Local;
    const Ast::VariableDefn* vref = _ctx.getVariableDef(_ctx.filename(), name, refType);
    if(vref == 0) {
        throw z::Exception("%s Variable not found: '%s'\n", err(_ctx.filename(), name).c_str(), name.text());
    }

    // create vref expression
    Ast::VariableRefExpr& vrefExpr = addUnitNode(new Ast::VariableRefExpr(name, z::ref(vref).qTypeSpec(), z::ref(vref), refType));
    return z::ptr(vrefExpr);
}

Ast::MemberExpr* Ast::NodeFactory::aMemberVariableExpr(const Ast::Expr& expr, const Ast::Token& name) {
    const Ast::TypeSpec& typeSpec = expr.qTypeSpec().typeSpec();

    const Ast::StructDefn* structDefn = dynamic_cast<const Ast::StructDefn*>(z::ptr(typeSpec));
    if(structDefn != 0) {
        for(StructBaseIterator sbi(structDefn); sbi.hasNext(); sbi.next()) {
            const Ast::VariableDefn* vref = _ctx.hasMember(sbi.get().scope(), name);
            if(vref) {
                const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, expr.qTypeSpec().isConst(), z::ref(vref).qTypeSpec().typeSpec(), true);
                Ast::MemberVariableExpr& vdefExpr = addUnitNode(new Ast::MemberVariableExpr(name, qTypeSpec, expr, z::ref(vref)));
                return z::ptr(vdefExpr);
            }

            for(Ast::StructDefn::PropertyList::const_iterator it = sbi.get().propertyList().begin(); it != sbi.get().propertyList().end(); ++it) {
                const Ast::PropertyDecl& pref = z::ref(*it);
                if(pref.name().string() == name.string()) {
                    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, expr.qTypeSpec().isConst(), pref.qTypeSpec().typeSpec(), true);
                    Ast::MemberPropertyExpr& vdefExpr = addUnitNode(new Ast::MemberPropertyExpr(name, qTypeSpec, expr, pref));
                    return z::ptr(vdefExpr);
                }
            }
        }

        throw z::Exception("%s '%s' is not a member of struct '%s'\n", err(_ctx.filename(), name).c_str(), name.text(), getTypeSpecName(typeSpec, GenMode::Import).c_str());
    }

    const Ast::FunctionRetn* functionRetn = dynamic_cast<const Ast::FunctionRetn*>(z::ptr(typeSpec));
    if(functionRetn != 0) {
        const Ast::VariableDefn* vref = _ctx.hasMember(z::ref(functionRetn).outScope(), name);
        if(vref) {
            Ast::MemberVariableExpr& vdefExpr = addUnitNode(new Ast::MemberVariableExpr(name, z::ref(vref).qTypeSpec(), expr, z::ref(vref)));
            return z::ptr(vdefExpr);
        }
        throw z::Exception("%s '%s' is not a member of function: '%s'\n", err(_ctx.filename(), name).c_str(), name.text(), getTypeSpecName(typeSpec, GenMode::Import).c_str());
    }

    throw z::Exception("%s Not an aggregate expression type '%s' (looking for member %s)\n", err(_ctx.filename(), name).c_str(), typeSpec.name().text(), name.text());
}

Ast::TypeSpecMemberExpr* Ast::NodeFactory::aTypeSpecMemberExpr(const Ast::TypeSpec& typeSpec, const Ast::Token& name) {
    const Ast::EnumDefn* enumDefn = dynamic_cast<const Ast::EnumDefn*>(z::ptr(typeSpec));
    if(enumDefn != 0) {
        const Ast::VariableDefn* vref = _ctx.hasMember(z::ref(enumDefn).scope(), name);
        if(vref == 0) {
            throw z::Exception("%s %s is not a member of type '%s'\n", err(_ctx.filename(), name).c_str(), name.text(), typeSpec.name().text());
        }
        const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(name, false, typeSpec, false);
        Ast::EnumMemberExpr& typeSpecMemberExpr = addUnitNode(new Ast::EnumMemberExpr(name, qTypeSpec, typeSpec, z::ref(vref)));
        return z::ptr(typeSpecMemberExpr);
    }

    const Ast::StructDefn* structDefn = dynamic_cast<const Ast::StructDefn*>(z::ptr(typeSpec));
    if(structDefn != 0) {
        const Ast::VariableDefn* vref = _ctx.hasMember(z::ref(structDefn).scope(), name);
        if(vref == 0) {
            throw z::Exception("%s %s is not a member of type '%s'\n", err(_ctx.filename(), name).c_str(), name.text(), typeSpec.name().text());
        }
        Ast::StructMemberExpr& typeSpecMemberExpr = addUnitNode(new Ast::StructMemberExpr(name, z::ref(vref).qTypeSpec(), typeSpec, z::ref(vref)));
        return z::ptr(typeSpecMemberExpr);
    }

    throw z::Exception("%s Not an aggregate type '%s' (looking for member %s)\n", err(_ctx.filename(), name).c_str(), typeSpec.name().text(), name.text());
}

Ast::StructInstanceExpr* Ast::NodeFactory::aStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn, const Ast::StructInitPartList& list) {
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, structDefn, false);
    Ast::StructInstanceExpr& structInstanceExpr = addUnitNode(new Ast::StructInstanceExpr(pos, qTypeSpec, structDefn, list));
    return z::ptr(structInstanceExpr);
}

Ast::StructInstanceExpr* Ast::NodeFactory::aStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn) {
    Ast::StructInitPartList& list = addUnitNode(new Ast::StructInitPartList(pos));
    return aStructInstanceExpr(pos, structDefn, list);
}

Ast::Expr* Ast::NodeFactory::aAutoStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn, const Ast::StructInitPartList& list) {
    const Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, structDefn, false);
    Ast::StructInstanceExpr& structInstanceExpr = addUnitNode(new Ast::StructInstanceExpr(pos, qTypeSpec, structDefn, list));
    return z::ptr(structInstanceExpr);
}

Ast::Expr* Ast::NodeFactory::aAutoStructInstanceExpr(const Ast::Token& pos, const Ast::StructDefn& structDefn) {
    Ast::StructInitPartList& list = addUnitNode(new Ast::StructInitPartList(pos));
    return aAutoStructInstanceExpr(pos, structDefn, list);
}

const Ast::StructDefn* Ast::NodeFactory::aEnterStructInstanceExpr(const Ast::StructDefn& structDefn) {
    _ctx.pushStructInit(structDefn);
    return z::ptr(structDefn);
}

const Ast::StructDefn* Ast::NodeFactory::aEnterAutoStructInstanceExpr(const Ast::Token& pos) {
    const Ast::StructDefn* sd = _ctx.isStructExpected();
    if(sd) {
        return aEnterStructInstanceExpr(z::ref(sd));
    }
    sd = _ctx.isPointerToStructExpected();
    if(sd) {
        return aEnterStructInstanceExpr(z::ref(sd));
    }
    throw z::Exception("%s No struct type expected\n", err(_ctx.filename(), pos).c_str());
}

void Ast::NodeFactory::aLeaveStructInstanceExpr() {
    _ctx.popStructInit();
}

const Ast::VariableDefn* Ast::NodeFactory::aEnterStructInitPart(const Ast::Token& name) {
    const Ast::StructDefn* structDefn = _ctx.structInit();
    if(structDefn == 0) {
        throw z::Exception("%s: Internal error initializing struct-member\n", err(_ctx.filename(), name).c_str());
    }

    for(StructBaseIterator sbi(structDefn); sbi.hasNext(); sbi.next()) {
        for(Ast::Scope::List::const_iterator it = sbi.get().list().begin(); it != sbi.get().list().end(); ++it) {
            const Ast::VariableDefn& vdef = z::ref(*it);
            if(vdef.name().string() == name.string()) {
                _ctx.pushExpectedTypeSpec(Context::ExpectedTypeSpec::etStructInit, vdef.qTypeSpec());
                return z::ptr(vdef);
            }
        }
    }

    throw z::Exception("%s: struct-member '%s' not found in '%s'\n", err(_ctx.filename(), name).c_str(), name.text(), getTypeSpecName(z::ref(structDefn), GenMode::Import).c_str());
}

void Ast::NodeFactory::aLeaveStructInitPart(const Ast::Token& pos) {
}

Ast::StructInitPartList* Ast::NodeFactory::aStructInitPartList(Ast::StructInitPartList& list, const Ast::StructInitPart& part) {
    list.addPart(part);
    return z::ptr(list);
}

Ast::StructInitPartList* Ast::NodeFactory::aStructInitPartList(const Ast::StructInitPart& part) {
    Ast::StructInitPartList& list = addUnitNode(new Ast::StructInitPartList(getToken()));
    list.addPart(part);
    return z::ptr(list);
}

Ast::StructInitPart* Ast::NodeFactory::aStructInitPart(const Ast::Token& pos, const Ast::VariableDefn& vdef, const Ast::Expr& initExpr) {
    const Ast::Expr& expr = convertExprToExpectedTypeSpec(pos, initExpr);
    _ctx.popExpectedTypeSpec(pos, Context::ExpectedTypeSpec::etStructInit);
    Ast::StructInitPart& part = addUnitNode(new Ast::StructInitPart(pos, vdef, expr));
    return z::ptr(part);
}

Ast::FunctionInstanceExpr* Ast::NodeFactory::aFunctionInstanceExpr(const Ast::Token& pos, const Ast::TypeSpec& typeSpec, const Ast::ExprList& exprList) {
    const Ast::Function* function = dynamic_cast<const Ast::Function*>(z::ptr(typeSpec));
    if(function != 0) {
        Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(pos, false, z::ref(function), false);
        Ast::FunctionInstanceExpr& functionInstanceExpr = addUnitNode(new Ast::FunctionInstanceExpr(pos, qTypeSpec, z::ref(function), exprList));
        return z::ptr(functionInstanceExpr);
    }

    throw z::Exception("%s: Not a function type '%s'\n", err(_ctx.filename(), pos).c_str(), typeSpec.name().text());
}

Ast::AnonymousFunctionExpr* Ast::NodeFactory::aAnonymousFunctionExpr(Ast::ChildFunctionDefn& functionDefn, const Ast::CompoundStatement& compoundStatement) {
    aChildFunctionDefn(functionDefn, compoundStatement);
    Ast::ExprList& exprList = addExprList(getToken());
    Ast::QualifiedTypeSpec& qTypeSpec = addQualifiedTypeSpec(getToken(), false, functionDefn, false);
    Ast::AnonymousFunctionExpr& functionInstanceExpr = addUnitNode(new Ast::AnonymousFunctionExpr(getToken(), qTypeSpec, functionDefn, exprList));
    return z::ptr(functionInstanceExpr);
}

Ast::ChildFunctionDefn* Ast::NodeFactory::aEnterAnonymousFunction(const Ast::Function& function) {
    char namestr[128];
    sprintf(namestr, "_anonymous_%lu", _module.unit().nodeList().size());
    Ast::Token name(getToken().row(), getToken().col(), namestr);

    Ast::TypeSpec* ts = 0;
    for(Context::TypeSpecStack::reverse_iterator it = _ctx.typeSpecStack().rbegin(); it != _ctx.typeSpecStack().rend(); ++it) {
        ts = *it;
        if(dynamic_cast<Ast::Namespace*>(ts) != 0)
            break;
        if(dynamic_cast<Ast::Root*>(ts) != 0)
            break;
    }

    if(ts == 0) {
        throw z::Exception("%s: Internal error: Unable to find parent for anonymous function %s\n", err(_ctx.filename(), name).c_str(), getTypeSpecName(function, GenMode::Import).c_str());
    }

    Ast::ChildFunctionDefn& functionDefn = createChildFunctionDefn(z::ref(ts), function, name, Ast::DefinitionType::Final);
    Ast::Statement* statement = aGlobalTypeSpecStatement(Ast::AccessType::Private, functionDefn);
    unused(statement);
    return z::ptr(functionDefn);
}

Ast::ChildFunctionDefn* Ast::NodeFactory::aEnterAutoAnonymousFunction(const Ast::Token& pos) {
    const Ast::Function* function = _ctx.isFunctionExpected();
    if(function == 0) {
        throw z::Exception("%s Internal error: no function type expected\n", err(_ctx.filename(), pos).c_str());
    }
    return aEnterAnonymousFunction(z::ref(function));
}

Ast::ConstantFloatExpr& Ast::NodeFactory::aConstantFloatExpr(const Ast::Token& token) {
    float value = 0;
    sscanf(token.text(), "%f", &value);

    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "float");
    Ast::ConstantFloatExpr& expr = addUnitNode(new Ast::ConstantFloatExpr(qTypeSpec, token, value));
    return expr;
}

Ast::ConstantDoubleExpr& Ast::NodeFactory::aConstantDoubleExpr(const Ast::Token& token) {
    double value = 0;
    sscanf(token.text(), "%lf", &value);

    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "double");
    Ast::ConstantDoubleExpr& expr = addUnitNode(new Ast::ConstantDoubleExpr(qTypeSpec, token, value));
    return expr;
}

Ast::ConstantBooleanExpr& Ast::NodeFactory::aConstantBooleanExpr(const Ast::Token& token) {
    const bool value = (token.string() == "true")?true:false;
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "bool");
    Ast::ConstantBooleanExpr& expr = addUnitNode(new Ast::ConstantBooleanExpr(qTypeSpec, token, value));
    return expr;
}

Ast::ConstantStringExpr& Ast::NodeFactory::aConstantStringExpr(const Ast::Token& token) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "string");
    Ast::ConstantStringExpr& expr = addUnitNode(new Ast::ConstantStringExpr(qTypeSpec, token));
    return expr;
}

Ast::ConstantCharExpr& Ast::NodeFactory::aConstantCharExpr(const Ast::Token& token) {
    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "char");
    Ast::ConstantCharExpr& expr = addUnitNode(new Ast::ConstantCharExpr(qTypeSpec, token));
    return expr;
}

Ast::ConstantLongExpr& Ast::NodeFactory::aConstantLongExpr(const Ast::Token& token) {
    long value = 0;
    sscanf(token.text(), "%ld", &value);

    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "long");
    Ast::ConstantLongExpr& expr = addUnitNode(new Ast::ConstantLongExpr(qTypeSpec, token, value));
    return expr;
}

Ast::ConstantIntExpr& Ast::NodeFactory::aConstantIntExpr(const Ast::Token& token) {
    int value = 0;
    sscanf(token.text(), "%d", &value);

    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "int");
    Ast::ConstantIntExpr& expr = addUnitNode(new Ast::ConstantIntExpr(qTypeSpec, token, value));
    return expr;
}

Ast::ConstantShortExpr& Ast::NodeFactory::aConstantShortExpr(const Ast::Token& token) {
    int value = 0;
    sscanf(token.text(), "%d", &value);

    const Ast::QualifiedTypeSpec& qTypeSpec = getQualifiedTypeSpec(token, "short");
    Ast::ConstantShortExpr& expr = addUnitNode(new Ast::ConstantShortExpr(qTypeSpec, token, value));
    return expr;
}