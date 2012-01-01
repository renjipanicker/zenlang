#pragma once

#include "ast.hpp"
class Generator {
public:
    virtual void run() = 0;
};

class StlCppGenerator : public Generator {
public:
    StlCppGenerator(const Ast::Project& project, const Ast::Config& config, const Ast::Unit& unit);
    ~StlCppGenerator();
    virtual void run();
private:
    struct Impl;
    Impl* _impl;
};
