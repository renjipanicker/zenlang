#pragma once

#include "generator.hpp"

class ZenlangGenerator : public Generator {
public:
    ZenlangGenerator(const Ast::Project& project, const Ast::Config& config, const Ast::Unit& unit);
    ~ZenlangGenerator();
    virtual void run();
private:
    struct Impl;
    Impl* _impl;
};
