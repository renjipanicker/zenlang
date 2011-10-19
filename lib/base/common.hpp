#pragma once

#define unused(x) ((void)x)

void _trace(const std::string& txt);

inline std::string ssprintfv(const char* txt, va_list vlist) {
    const int len = 1024;
    char buf[len];
#if defined(WIN32)
    vsnprintf_s(buf, len, _TRUNCATE, txt, vlist);
#else
    vsnprintf(buf, len, txt, vlist);
#endif
    va_end(vlist);
    return buf;
}

inline std::string ssprintf(const char* txt, ...) {
    va_list vlist;
    va_start(vlist, txt);
    return ssprintfv(txt, vlist);
}

inline void trace(const char* txt, ...) {
#if defined(DEBUG)
    va_list vlist;
    va_start(vlist, txt);
    std::string buf = ssprintfv(txt, vlist);
    _trace(buf);
#else
    unused(txt);
#endif
}

template <typename T>
inline T& ref(T* t) {
    assert(t);
    return *t;
}

template <typename T>
inline T* ptr(T& t) {
    assert(&t);
    return &t;
}

template <typename FromT, typename ToT>
inline ToT& refX(FromT* from) {
    assert(from);
    ToT* to = static_cast<ToT*>(from);
    assert(to);
    return *to;
}

typedef std::string string;

/// \brief Namespace to hold all types intrinsic within zen
namespace z {
}
