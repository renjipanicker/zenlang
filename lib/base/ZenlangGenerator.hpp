#pragma once

#include "base/generator.hpp"
#include "base/unit.hpp"

class ZenlangGenerator : public Generator {
public:
    ZenlangGenerator(const Ast::Project& project, const Ast::Config& config, const Ast::Module& module);
    ~ZenlangGenerator();
    virtual void run();
    static z::string convertExprToString(const Ast::Expr& expr);
private:
    struct Impl;
    Impl* _impl;
};
