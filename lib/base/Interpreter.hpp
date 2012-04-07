#pragma once
#include "base/generator.hpp"

class Interpreter : public Generator {
public:
    Interpreter(const Ast::Project& project, const Ast::Config& config);
    ~Interpreter();
    virtual void run();
private:
    struct Impl;
    Impl* _impl;
};
