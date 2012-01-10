#pragma once

#include "ast.hpp"
#include "lexer.hpp"

class Compiler {
public:
    inline Compiler(const Ast::Project& project, const Ast::Config& config) : _project(project), _config(config) {}
    void initContext(Ast::Unit& unit);
    void compile();
    void import(Ast::Unit& unit, const std::string& filename, const int& level);
    void parseString(Ast::Context& ctx, Lexer& lexer, Ast::Unit& unit, const std::string& data, const int& level, const bool& isEof);
    bool parseFile(Ast::Context& ctx, Ast::Unit& unit, Lexer& lexer, const std::string& filename, const int& level, const std::string& msg);
private:
    inline std::string findImport(const std::string& filename);
    inline bool parseFile(Ast::Unit& unit, const std::string& filename, const int& level, const std::string& msg);
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
};
