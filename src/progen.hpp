#pragma once

class ProGen {
public:
    ProGen();
    ~ProGen();
public:
    void run();
private:
    class Impl;
    Impl* _impl;
};
