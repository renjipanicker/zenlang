#pragma once

class Exception {
public:
    Exception(const char* msg, ...);
private:
    std::string _msg;
};
