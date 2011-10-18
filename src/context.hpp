#pragma once

#include "ast.hpp"
#include "compiler.hpp"

class Context {
public:
    Context(Compiler& compiler, Ast::Unit& unit, const int& level);
    ~Context();

public:
    const Ast::TypeSpec& getRootTypeSpec(const Ast::Token& name) const;
    const Ast::TypeSpec& getChildTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const;

public:
    Ast::QualifiedTypeSpec& addQualifiedTypeSpec(const bool& isConst, const Ast::TypeSpec& typeSpec, const bool& isRef);
    Ast::VariableDef& addVariableDef(const Ast::QualifiedTypeSpec& qualifiedTypeSpec, const Ast::Token& name);
    Ast::VariableDefList& addVariableDefList();

public:
    Ast::TypeDef& addTypeDefSpec(const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::StructDef& addStructDefSpec(const Ast::Token& name, const Ast::DefinitionType::T& defType, const Ast::VariableDefList& list);
    Ast::StructDef& addStructDefSpecEmpty(const Ast::Token& name, const Ast::DefinitionType::T& defType);
    Ast::FunctionDef& addFunctionDefSpec(const Ast::VariableDefList& out, const Ast::Token& name, const Ast::VariableDefList& in, const Ast::DefinitionType::T& defType);

public:
    Ast::UserDefinedTypeSpecStatement& addUserDefinedTypeSpecStatement(const Ast::UserDefinedTypeSpec& typeSpec);
    Ast::Namespace& addNamespace(const Ast::Token& name);
    Ast::ImportStatement& addImportStatement();

public:
    void importHeader(const Ast::ImportStatement& statement);
    void addGlobalStatement(const Ast::Statement& statement);

private:
    const Ast::TypeSpec* findTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const;
    inline Ast::TypeSpec& currentTypeSpec() const {assert(_typeSpecStack.size() > 0); return ref(_typeSpecStack.back());}

private:
    Compiler& _compiler;
    Ast::Unit& _unit;
    const int _level;
private:
    std::list<Ast::TypeSpec*> _typeSpecStack;
};
