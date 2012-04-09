#pragma once

#include "base/ast.hpp"
#include "base/generator.hpp"

namespace z {
class CmakeGenerator : public Generator {
public:
    CmakeGenerator(const z::Ast::Project& project);
    ~CmakeGenerator();
public:
    virtual void run();
private:
    class Impl;
    Impl* _impl;
};
}
