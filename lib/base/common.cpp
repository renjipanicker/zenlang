#include "pch.hpp"
#include "common.hpp"

void _trace(const std::string& txt) {
#if defined(WIN32)
    OutputDebugStringA(txt.c_str());
#else
    std::cout << txt.c_str() << std::flush;
#endif
}
