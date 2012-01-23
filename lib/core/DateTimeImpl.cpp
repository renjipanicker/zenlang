#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "DateTime.hpp"

z::datetime DateTime::Now() {
    z::datetime dt = std::time(0);
    return dt;
}

int DateTime::Year(const z::datetime& dt) {
    time_t t = dt.val();
    return z::ref(std::gmtime(&t)).tm_year;
}
