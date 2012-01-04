#pragma once

#include "generator.hpp"

class Interpreter : public Generator {
public:
    Interpreter(const Ast::Project& project, const Ast::Config& config, const Ast::Unit& unit);
    ~Interpreter();
    virtual void run();
private:
    struct Impl;
    Impl* _impl;
};
