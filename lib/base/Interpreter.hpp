#pragma once
#include "base/generator.hpp"

namespace z {
class Interpreter : public Generator {
public:
    Interpreter(const z::Ast::Project& project, const z::Ast::Config& config);
    ~Interpreter();
    virtual void run();
private:
    struct Impl;
    Impl* _impl;
};
}
