#include "pch.hpp"
#include "common.hpp"
#include "exception.hpp"
#include "context.hpp"

Context::Context(Compiler& compiler, Ast::Unit& unit, const int& level) : _compiler(compiler), _unit(unit), _level(level) {
    Ast::TypeSpec* currentTypeSpec = ptr((_level == 0)?_unit.rootNS():_unit.importNS());
    _typeSpecStack.push_back(currentTypeSpec);
}

Context::~Context() {
    //assert(_typeSpecStack.size() == 1);
    _typeSpecStack.pop_back();
}

Ast::VariableDefList& Context::addVariableDefList() {
    Ast::VariableDefList& paramDefList = _unit.addNode(new Ast::VariableDefList());
    return paramDefList;
}

Ast::VariableDef& Context::addVariableDef(const Ast::QualifiedTypeSpec &qualifiedTypeSpec, const Ast::Token &name) {
    Ast::VariableDef& paramDef = _unit.addNode(new Ast::VariableDef(qualifiedTypeSpec, name));
    return paramDef;
}

Ast::QualifiedTypeSpec & Context::addQualifiedTypeSpec(const bool &isConst, const Ast::TypeSpec &typeSpec, const bool &isRef) {
    Ast::QualifiedTypeSpec& qualifiedTypeSpec = _unit.addNode(new Ast::QualifiedTypeSpec(isConst, typeSpec, isRef));
    return qualifiedTypeSpec;
}

Ast::TypeDef& Context::addTypeDefSpec(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    Ast::TypeDef& typeDef = _unit.addNode(new Ast::TypeDef(currentTypeSpec(), name, defType));
    currentTypeSpec().addChild(typeDef);
    return typeDef;
}

Ast::StructDef& Context::addStructDefSpec(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::VariableDefList& list) {
    Ast::StructDef& structDef = _unit.addNode(new Ast::StructDef(currentTypeSpec(), name, defType, list));
    currentTypeSpec().addChild(structDef);
    return structDef;
}

Ast::StructDef& Context::addStructDefSpecEmpty(const Ast::Token& name, const Ast::DefinitionType::T& defType) {
    const Ast::VariableDefList list;
    return addStructDefSpec(name, defType, list);
}

Ast::FunctionDef& Context::addFunctionDefSpec(const Ast::VariableDefList& out, const Ast::Token& name, const Ast::VariableDefList& in, const Ast::DefinitionType::T& defType) {
    Ast::FunctionDef& functionDef = _unit.addNode(new Ast::FunctionDef(currentTypeSpec(), out, name, in, defType));
    currentTypeSpec().addChild(functionDef);
    return functionDef;
}

Ast::Namespace& Context::addNamespace(const Ast::Token& name) {
    _unit.addNamespace(name);
    Ast::Namespace& ns = _unit.addNode(new Ast::Namespace(currentTypeSpec(), name));
    currentTypeSpec().addChild(ns);
    _typeSpecStack.push_back(ptr(ns));
    return ns;
}

Ast::UserDefinedTypeSpecStatement& Context::addUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec) {
    Ast::UserDefinedTypeSpecStatement& userDefinedTypeSpecStatement = _unit.addNode(new Ast::UserDefinedTypeSpecStatement(typeSpec));
    return userDefinedTypeSpecStatement;
}

Ast::ImportStatement& Context::addImportStatement() {
    Ast::ImportStatement& importStatement = _unit.addNode(new Ast::ImportStatement());
    _unit.addImportStatement(importStatement);
    return importStatement;
}

void Context::addGlobalStatement(const Ast::Statement &statement) {
    if(_level > 0)
        return;
    _unit.addGlobalStatement(statement);
}

void Context::importHeader(const Ast::ImportStatement &statement) {
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

const Ast::TypeSpec* Context::findTypeSpec(const Ast::TypeSpec &parent, const Ast::Token &name) const {
    const Ast::TypeSpec* child = parent.hasChild(name.text());
    if(child)
        return child;
    const Ast::ChildTypeSpec* parentx = dynamic_cast<const Ast::ChildTypeSpec*>(ptr(parent));
    if(!parentx)
        return 0;
    return findTypeSpec(ref(parentx).parent(), name);
}

const Ast::TypeSpec& Context::getRootTypeSpec(const Ast::Token &name) const {
    const Ast::TypeSpec* typeSpec = findTypeSpec(currentTypeSpec(), name);
    if(!typeSpec) {
        if(_level == 0) {
            typeSpec = _unit.importNS().hasChild(name.text());
            if(!typeSpec) {
                throw Exception("Unknown root type '%s'\n", name.text());
            }
        }
    }
    return ref(typeSpec);
}

const Ast::TypeSpec & Context::getChildTypeSpec(const Ast::TypeSpec &parent, const Ast::Token &name) const {
    const Ast::TypeSpec* typeSpec = parent.hasChild(name.text());
    if(!typeSpec) {
        throw Exception("Unknown child type '%s'\n", name.text());
    }
    return ref(typeSpec);
}
