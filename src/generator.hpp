#pragma once

#include "ast.hpp"
#include "project.hpp"
class Generator {
public:
    Generator(const Project& project, const Ast::Unit& unit);
    ~Generator();
    void run();
private:
    struct Impl;
    Impl* _impl;
};
