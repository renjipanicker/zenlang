#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "progen.hpp"

class ProGen::Impl {
public:
    void run();
};

void ProGen::Impl::run() {
}

ProGen::ProGen() : _impl(0) {_impl = new Impl();}
ProGen::~ProGen() {delete _impl;}
void ProGen::run() {return ref(_impl).run();}
