#pragma once

#include "ast.hpp"
#include "lexer.hpp"

class Compiler {
public:
    inline Compiler(const Ast::Project& project, const Ast::Config& config) : _project(project), _config(config) {}
    void initContext(Ast::Unit& unit);
    void compile();
    void import(Ast::Module& module);
    void compileString(Ast::Module& module, Lexer& lexer, const std::string& data, const bool& isEof);
    bool compileFile(Ast::Module& module, const std::string& filename, const std::string& msg);
private:
    inline std::string findImport(const std::string& filename);
    inline bool parseFile(Ast::Module& module, const std::string& msg);
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
};
