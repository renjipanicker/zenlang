#pragma once

#include "base/ast.hpp"
#include "base/generator.hpp"

namespace z {
    class QtcGenerator : public Generator {
    public:
        QtcGenerator(const z::Ast::Project& project);
        ~QtcGenerator();
    public:
        virtual void run();
    private:
        class Impl;
        Impl* _impl;
    };
}
