#pragma once

#include "ast.hpp"

class Compiler {
public:
    inline Compiler(const Ast::Project& project, const Ast::Config& config) : _project(project), _config(config) {}
    void initContext(Ast::Unit& unit);
    void compile();
    void import(Ast::Unit& unit, const std::string& filename, const int& level);
    bool parseString(Ast::Unit& unit, const std::string& data, const int& level);
private:
    inline bool parseFile(Ast::Unit& unit, const std::string& filename, const int& level);
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
};
