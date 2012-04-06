#pragma once

#if defined(UN_AMALGAMATED)
#include "base/generator.hpp"
#include "base/unit.hpp"
#endif

class StlcppGenerator : public Generator {
public:
    StlcppGenerator(const Ast::Project& project, const Ast::Config& config, const Ast::Module& module);
    ~StlcppGenerator();
    virtual void run();
private:
    struct Impl;
    Impl* _impl;
};
