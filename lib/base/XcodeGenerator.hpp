#pragma once

#include "base/ast.hpp"
#include "base/generator.hpp"

namespace z {
class XcodeGenerator : public Generator {
public:
    XcodeGenerator(const z::Ast::Project& project);
    ~XcodeGenerator();
public:
    virtual void run();
private:
    class Impl;
    Impl* _impl;
};
}
