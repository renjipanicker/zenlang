#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "core/DateTime.hpp"

z::string DateTime::toString(const z::datetime& dt) {
    unused(dt);
    assert(false);
    return "";
}

z::datetime DateTime::fromString(const z::string& str) {
    unused(str);
    assert(false);
    z::datetime dt = std::time(0);
    return dt;
}

z::datetime DateTime::Now() {
    z::datetime dt = std::time(0);
    return dt;
}

z::datetime DateTime::AddDays(const z::datetime& dt, const int& days) {
    z::datetime ndt = dt.val() + (days * 24 * 60 * 60);
    return ndt;

}

int DateTime::Year(const z::datetime& dt) {
    time_t t = dt.val();
#if defined(WIN32)
    std::tm tm;
    ::gmtime_s(&tm, &t);
    return tm.tm_year;
#else
    std::tm* tm = ::gmtime(&t);
    return z::ref(tm).tm_year;
#endif
}
