#include "zenlang.hpp"

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

inline std::tm getLocalTm(const z::datetime& dt) {
    time_t t = dt.val();
#if defined(WIN32)
    std::tm tm;
    errno_t eno = ::localtime_s(&tm, &t);
    unused(eno);
    return tm;
#else
    std::tm* tm = ::localtime(&t);
    return z::ref(tm);
#endif
}


z::string DateTime::ToFormatString(const z::datetime& dt, const z::string& format) {
    std::tm tm = getTm(dt);
    char buf[50];
    ::strftime(buf, 50, z::s2e(format).c_str(), &tm);
    return buf;
}

z::string DateTime::ToFormatStringLocal(const z::datetime& dt, const z::string& format) {
    std::tm tm = getLocalTm(dt);
    char buf[50];
    ::strftime(buf, 50, z::s2e(format).c_str(), &tm);
    return buf;
}

z::string DateTime::ToIsoString(const z::datetime& dt) {
    return ToFormatString(dt, "%Y-%m-%d %H:%M:%S %Z");
}

z::string DateTime::ToIsoStringLocal(const z::datetime& dt) {
    return ToFormatStringLocal(dt, "%Y-%m-%d %H:%M:%S %Z");
}

z::string DateTime::ToValueString(const z::datetime& dt) {
    z::string dstr = z::string("%{s}").arg("s", dt.val());
    return dstr;
}

z::datetime DateTime::FromValueString(const z::string& str) {
    int64_t t = str.to<int64_t>();
    z::datetime dt = t;
    return dt;
}

z::datetime DateTime::FromIsoString(const z::string& str) {
    unused(str);
    assert(false); /// \todo parse ISO string to date
    z::datetime dt = std::time(0);
    return dt;
}

z::datetime DateTime::Now() {
    z::datetime dt = std::time(0);
    return dt;
}

int DateTime::Year(const z::datetime& dt) {
    std::tm tm = getTm(dt);
    return tm.tm_year;
}

int DateTime::Month(const z::datetime& dt) {
    std::tm tm = getTm(dt);
    return tm.tm_mon;
}

int DateTime::Date(const z::datetime& dt) {
    std::tm tm = getTm(dt);
    return tm.tm_mday;
}

z::datetime DateTime::AddYears(const z::datetime& dt, const int& years) {
    std::tm tm = getTm(dt);
    tm.tm_year += years;
    time_t nt = ::mktime(&tm);
    assert(nt > 0);
    return z::datetime(nt);
}

z::datetime DateTime::AddMonths(const z::datetime& dt, const int& months) {
    std::tm tm = getTm(dt);
    tm.tm_mon += months;
    time_t nt = ::mktime(&tm);
    assert(nt > 0);
    return z::datetime(nt);
}

z::datetime DateTime::AddDays(const z::datetime& dt, const int& days) {
    std::tm tm = getTm(dt);
    tm.tm_mday += days;
    time_t nt = ::mktime(&tm);
    assert(nt > 0);
    return z::datetime(nt);
}

z::datetime DateTime::SetYear(const z::datetime& dt, const int& year) {
    std::tm tm = getTm(dt);
    tm.tm_year = year;
    time_t nt = ::mktime(&tm);
    assert(nt > 0);
    return z::datetime(nt);
}

z::datetime DateTime::SetMonth(const z::datetime& dt, const int& month) {
    std::tm tm = getTm(dt);
    tm.tm_mon = month - 1;
    time_t nt = ::mktime(&tm);
    assert(nt > 0);
    return z::datetime(nt);
}

z::datetime DateTime::SetDay(const z::datetime& dt, const int& day) {
    std::tm tm = getTm(dt);
    tm.tm_mday = day - 1;
    time_t nt = ::mktime(&tm);
    assert(nt > 0);
    return z::datetime(nt);
}

int DateTime::DaysDiff(const z::datetime& dt1, const z::datetime& dt2) {
    time_t t1 = dt1.val()/(60 * 60 * 24);
    time_t t2 = dt2.val()/(60 * 60 * 24);
    return (int)(t2 - t1);
}
