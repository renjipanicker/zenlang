#pragma once

#include "generator.hpp"

class StlcppGenerator : public Generator {
public:
    StlcppGenerator(const Ast::Project& project, const Ast::Config& config, const Ast::Unit& unit);
    ~StlcppGenerator();
    virtual void run();
private:
    struct Impl;
    Impl* _impl;
};
