#pragma once
#include "base/generator.hpp"
#include "base/unit.hpp"

class StlcppGenerator : public Generator {
public:
    StlcppGenerator(const Ast::Project& project, const Ast::Config& config, const Ast::Module& module);
    ~StlcppGenerator();
    virtual void run();
private:
    struct Impl;
    Impl* _impl;
};
