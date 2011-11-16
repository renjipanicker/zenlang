#pragma once

#include "ast.hpp"
class Generator {
public:
    Generator(const Ast::Project& project, const Ast::Config& config, const Ast::Unit& unit);
    ~Generator();
    void run();
private:
    struct Impl;
    Impl* _impl;
};
