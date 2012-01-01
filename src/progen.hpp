#pragma once

#include "ast.hpp"

class ProGen {
public:
    virtual void run() = 0;
};

class CmakeProGen : public ProGen {
public:
    CmakeProGen(const Ast::Project& project);
    ~CmakeProGen();
public:
    virtual void run();
private:
    class Impl;
    Impl* _impl;
};
