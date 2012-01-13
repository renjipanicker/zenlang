#pragma once

#include "ast.hpp"
#include "lexer.hpp"

class Compiler {
public:
    inline Compiler(const Ast::Project& project, const Ast::Config& config) : _project(project), _config(config) {}
    void initContext(Ast::Module& module);
    void compile();
    void import(Ast::Module& module, const std::string& filename, const int& level);
    void compileString(Ast::Module& module, Lexer& lexer, const std::string& data, const int& level, const bool& isEof);
    bool compileFile(Ast::Module& module, Lexer& lexer, const std::string& filename, const int& level, const std::string& msg);
private:
    inline std::string findImport(const std::string& filename);
    inline bool parseFile(Ast::Module& module, const std::string& filename, const int& level, const std::string& msg);
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
};
