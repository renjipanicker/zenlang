#pragma once

#include "ast.hpp"

class ProGen {
public:
    ProGen(const Ast::Project& project);
    ~ProGen();
public:
    void run();
private:
    class Impl;
    Impl* _impl;
};
