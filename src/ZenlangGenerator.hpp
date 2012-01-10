#pragma once

#include "generator.hpp"

class ZenlangGenerator : public Generator {
public:
    ZenlangGenerator(const Ast::Project& project, const Ast::Config& config, const Ast::Unit& unit);
    ~ZenlangGenerator();
    virtual void run();
    static std::string convertExprToString(const Ast::Expr& expr);
private:
    struct Impl;
    Impl* _impl;
};
