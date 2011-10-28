#pragma once

#include "ast.hpp"

class Compiler {
public:
    inline Compiler(Ast::Project& project) : _project(project) {}
    void compile();
    void import(Ast::Unit& unit, const std::string& filename, const int& level);
private:
    inline bool parseFile(Ast::Unit& unit, const std::string& filename, const int& level);
private:
    Ast::Project& _project;
};
