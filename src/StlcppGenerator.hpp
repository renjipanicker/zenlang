#pragma once

#include "generator.hpp"
#include "CompilerContext.hpp"

class StlcppGenerator : public Generator {
public:
    StlcppGenerator(const Ast::Project& project, const Ast::Config& config, const Ast::Module& module);
    ~StlcppGenerator();
    virtual void run();
private:
    struct Impl;
    Impl* _impl;
};
