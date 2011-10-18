#pragma once

#include "ast.hpp"
#include "project.hpp"

class Compiler {
public:
    inline Compiler(Project& project) : _project(project) {}
    void compile();
    void import(Ast::Unit& unit, const std::string& filename, const int& level);
private:
    inline bool parseFile(Ast::Unit& unit, const std::string& filename, const int& level);
private:
    Project& _project;
};
