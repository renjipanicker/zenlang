#pragma once

#include "ast.hpp"
#include "compiler.hpp"

class Context {
public:
    inline Context(Compiler& compiler, Ast::Unit& unit, const int& level) : _compiler(compiler), _unit(unit), _level(level), _currentTypeSpec(0) {
        _currentTypeSpec = ptr((_level == 0)?_unit.rootNS():_unit.importNS());
    }

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
private:
    Compiler& _compiler;
    Ast::Unit& _unit;
    const int _level;
private:
    Ast::ChildTypeSpec* _currentTypeSpec;
};
