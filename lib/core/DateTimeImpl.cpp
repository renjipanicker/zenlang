#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "core/DateTime.hpp"

z::string DateTime::toString(const z::datetime& dt) {
    assert(false);
    return "";
}

z::datetime DateTime::fromString(const z::string& str) {
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
    return z::ref(std::gmtime(&t)).tm_year;
}
