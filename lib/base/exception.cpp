#include "base/pch.hpp"
#include "base/common.hpp"
#include "base/exception.hpp"

Exception::Exception(const char* txt, ...) {
    va_list vlist;
    va_start(vlist, txt);
    _msg = ssprintfv(txt, vlist);
    printf("Error: %s\n", _msg.c_str());
}
