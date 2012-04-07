#pragma once
#include "base/unit.hpp"
#include "base/lexer.hpp"

class Compiler {
public:
    inline Compiler(const Ast::Project& project, const Ast::Config& config) : _project(project), _config(config) {}
    void initContext(Ast::Unit& unit);
    void compile();
    void import(Ast::Module& module);
    void compileString(Ast::Module& module, Lexer& lexer, const z::string& data, const bool& isEof);
    bool compileFile(Ast::Module& module, const z::string& filename, const z::string& msg);
private:
    inline z::string findImport(const z::string& filename);
    inline bool parseFile(Ast::Module& module, const z::string& msg);
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
};
