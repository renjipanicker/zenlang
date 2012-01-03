#pragma once

#include "ast.hpp"

namespace Ast {
    class Context;
}

class Compiler {
public:
    inline Compiler(const Ast::Project& project, const Ast::Config& config) : _project(project), _config(config) {}
    void initContext(Ast::Context& ctx, Ast::Unit& unit);
    void compile();
    void import(Ast::Context& ctx, Ast::Unit& unit, const std::string& filename, const int& level);
    bool parseString(Ast::Context& ctx, Ast::Unit& unit, const std::string& data, const int& level);
private:
    inline bool parseFile(Ast::Context& ctx, Ast::Unit& unit, const std::string& filename, const int& level);
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
};
