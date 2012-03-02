#pragma once

#include "base/ast.hpp"
#include "generator.hpp"

class CmakeGenerator : public Generator {
public:
    CmakeGenerator(const Ast::Project& project);
    ~CmakeGenerator();
public:
    virtual void run();
private:
    class Impl;
    Impl* _impl;
};
