#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "core/DateTime.hpp"

inline std::tm getTm(const z::datetime& dt) {
    time_t t = dt.val();
#if defined(WIN32)
    std::tm tm;
    ::gmtime_s(&tm, &t);
    return tm;
#else
    std::tm* tm = ::gmtime(&t);
    return z::ref(tm);
#endif
}

z::string DateTime::toString(const z::datetime& dt) {
    std::tm tm = getTm(dt);
    char buf[50];
    ::strftime(buf, 50, "%Y-%m-%d %H:%M:%S %Z", &tm);
    return buf;
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

int DateTime::DaysDiff(const z::datetime& dt1, const z::datetime& dt2) {
    time_t t1 = dt1.val()/(60 * 60 * 24);
    time_t t2 = dt2.val()/(60 * 60 * 24);
    return t2 - t1;
}

int DateTime::Year(const z::datetime& dt) {
    std::tm tm = getTm(dt);
    return tm.tm_year;
}
