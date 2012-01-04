#pragma once

#include "ast.hpp"

namespace Ast {
    class Context {
    public:
        typedef std::list<Ast::Scope*> ScopeStack;
    protected:
        inline Context() {}
    public:
        Ast::Scope& enterScope(Ast::Scope& scope);
        Ast::Scope& leaveScope();
        Ast::Scope& leaveScope(Ast::Scope& scope);
        Ast::Scope& currentScope();
        inline const Ast::VariableDefn* hasMember(const Ast::Scope& scope, const Ast::Token& name) const;
    public:
        const Ast::VariableDefn* getVariableDef(const std::string& filename, const Ast::Token& name, Ast::RefType::T& refType) const;
    private:
        ScopeStack _scopeStack;
    };
}
