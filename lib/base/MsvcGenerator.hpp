#pragma once

#include "base/ast.hpp"
#include "base/generator.hpp"

namespace z {
class MsvcGenerator : public Generator {
public:
    MsvcGenerator(const z::Ast::Project& project);
    ~MsvcGenerator();
public:
    virtual void run();
private:
    class Impl;
    Impl* _impl;
};
}
