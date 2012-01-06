#pragma once

#include "ast.hpp"
#include "lexer.hpp"

class Compiler {
public:
    inline Compiler(const Ast::Project& project, const Ast::Config& config) : _project(project), _config(config) {}
    void initContext(Ast::Unit& unit);
    void compile();
    void import(Ast::Module& module, const std::string& filename, const int& level);
    void parseString(Ast::Context& ctx, Lexer& lexer, Ast::Module& module, const std::string& data, const int& level, const bool& isEof);
private:
    inline std::string findImport(const std::string& filename);
    inline bool parseFile(Ast::Module& module, const std::string& filename, const int& level);
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
};
