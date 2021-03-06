#include "zenlang.hpp"

namespace zz {
/*
 * Copyright (c) 1999 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of KTH nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY KTH AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KTH OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

//#ifdef WIN32

static const char *abb_weekdays[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    NULL
};

static const char *full_weekdays[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    NULL
};

static const char *abb_month[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
    NULL
};

static const char *full_month[] = {
    "January",
    "February",
    "Mars",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
    NULL,
};

static const char *ampm[] = {
    "am",
    "pm",
    NULL
};

/*
 * Try to match `*buf' to one of the strings in `strs'.  Return the
 * index of the matching string (or -1 if none).  Also advance buf.
 */

static int
match_string (const char **buf, const char **strs)
{
    size_t i = 0;

    for (i = 0; strs[i] != NULL; ++i) {
    size_t len = strlen (strs[i]);
#ifdef WIN32
    if (_strnicmp (*buf, strs[i], len) == 0) {
#else
    if (strncasecmp (*buf, strs[i], len) == 0) {
#endif
        *buf += len;
        return (int)i;
    }
    }
    return -1;
}

/*
 * tm_year is relative this year */

const int tm_year_base = 1900;

/*
 * Return TRUE iff `year' was a leap year.
 */

static int
is_leap_year (int year)
{
    return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
}

/*
 * Return the weekday [0,6] (0 = Sunday) of the first day of `year'
 */

static int
first_day (int year)
{
    int ret = 4;

    for (; year > 1970; --year)
    ret = (ret + 365 + is_leap_year (year) ? 1 : 0) % 7;
    return ret;
}

/*
 * Set `timeptr' given `wnum' (week number [0, 53])
 */

static void
set_week_number_sun (struct tm *timeptr, int wnum)
{
    int fday = first_day (timeptr->tm_year + tm_year_base);

    timeptr->tm_yday = wnum * 7 + timeptr->tm_wday - fday;
    if (timeptr->tm_yday < 0) {
    timeptr->tm_wday = fday;
    timeptr->tm_yday = 0;
    }
}

/*
 * Set `timeptr' given `wnum' (week number [0, 53])
 */

static void
set_week_number_mon (struct tm *timeptr, int wnum)
{
    int fday = (first_day (timeptr->tm_year + tm_year_base) + 6) % 7;

    timeptr->tm_yday = wnum * 7 + (timeptr->tm_wday + 6) % 7 - fday;
    if (timeptr->tm_yday < 0) {
    timeptr->tm_wday = (fday + 1) % 7;
    timeptr->tm_yday = 0;
    }
}

/*
 * Set `timeptr' given `wnum' (week number [0, 53])
 */

static void
set_week_number_mon4 (struct tm *timeptr, int wnum)
{
    int fday = (first_day (timeptr->tm_year + tm_year_base) + 6) % 7;
    int offset = 0;

    if (fday < 4)
    offset += 7;

    timeptr->tm_yday = offset + (wnum - 1) * 7 + timeptr->tm_wday - fday;
    if (timeptr->tm_yday < 0) {
    timeptr->tm_wday = fday;
    timeptr->tm_yday = 0;
    }
}

/*
 *
 */

char *
parsetime (const char *buf, const char *format, struct tm *timeptr)
{
    char c;

    for (; (c = *format) != '\0'; ++format) {
    char *s;
    int ret;

    if (isspace (c)) {
        while (isspace (*buf))
        ++buf;
    } else if (c == '%' && format[1] != '\0') {
        c = *++format;
        if (c == 'E' || c == 'O')
        c = *++format;
        switch (c) {
        case 'A' :
        ret = match_string (&buf, full_weekdays);
        if (ret < 0)
            return NULL;
        timeptr->tm_wday = ret;
        break;
        case 'a' :
        ret = match_string (&buf, abb_weekdays);
        if (ret < 0)
            return NULL;
        timeptr->tm_wday = ret;
        break;
        case 'B' :
        ret = match_string (&buf, full_month);
        if (ret < 0)
            return NULL;
        timeptr->tm_mon = ret;
        break;
        case 'b' :
        case 'h' :
        ret = match_string (&buf, abb_month);
        if (ret < 0)
            return NULL;
        timeptr->tm_mon = ret;
        break;
        case 'C' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        timeptr->tm_year = (ret * 100) - tm_year_base;
        buf = s;
        break;
        case 'c' :
        abort ();
        case 'D' :		/* %m/%d/%y */
        s = parsetime (buf, "%m/%d/%y", timeptr);
        if (s == NULL)
            return NULL;
        buf = s;
        break;
        case 'd' :
        case 'e' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        timeptr->tm_mday = ret;
        buf = s;
        break;
        case 'H' :
        case 'k' :
                ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        timeptr->tm_hour = ret;
        buf = s;
        break;
        case 'I' :
        case 'l' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        if (ret == 12)
            timeptr->tm_hour = 0;
        else
            timeptr->tm_hour = ret;
        buf = s;
        break;
        case 'j' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        timeptr->tm_yday = ret - 1;
        buf = s;
        break;
        case 'm' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        timeptr->tm_mon = ret - 1;
        buf = s;
        break;
        case 'M' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        timeptr->tm_min = ret;
        buf = s;
        break;
        case 'n' :
        if (*buf == '\n')
            ++buf;
        else
            return NULL;
        break;
        case 'p' :
        ret = match_string (&buf, ampm);
        if (ret < 0)
            return NULL;
        if (timeptr->tm_hour == 0) {
            if (ret == 1)
            timeptr->tm_hour = 12;
        } else
            timeptr->tm_hour += 12;
        break;
        case 'r' :		/* %I:%M:%S %p */
        s = parsetime (buf, "%I:%M:%S %p", timeptr);
        if (s == NULL)
            return NULL;
        buf = s;
        break;
        case 'R' :		/* %H:%M */
        s = parsetime (buf, "%H:%M", timeptr);
        if (s == NULL)
            return NULL;
        buf = s;
        break;
        case 'S' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        timeptr->tm_sec = ret;
        buf = s;
        break;
        case 't' :
        if (*buf == '\t')
            ++buf;
        else
            return NULL;
        break;
        case 'T' :		/* %H:%M:%S */
        case 'X' :
        s = parsetime (buf, "%H:%M:%S", timeptr);
        if (s == NULL)
            return NULL;
        buf = s;
        break;
        case 'u' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        timeptr->tm_wday = ret - 1;
        buf = s;
        break;
        case 'w' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        timeptr->tm_wday = ret;
        buf = s;
        break;
        case 'U' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        set_week_number_sun (timeptr, ret);
        buf = s;
        break;
        case 'V' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        set_week_number_mon4 (timeptr, ret);
        buf = s;
        break;
        case 'W' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        set_week_number_mon (timeptr, ret);
        buf = s;
        break;
        case 'x' :
        s = parsetime (buf, "%Y:%m:%d", timeptr);
        if (s == NULL)
            return NULL;
        buf = s;
        break;
        case 'y' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        if (ret < 70)
            timeptr->tm_year = 100 + ret;
        else
            timeptr->tm_year = ret;
        buf = s;
        break;
        case 'Y' :
        ret = (int)strtol (buf, &s, 10);
        if (s == buf)
            return NULL;
        timeptr->tm_year = ret - tm_year_base;
        buf = s;
        break;
        case 'Z' :
        abort ();
        case '\0' :
        --format;
        /* FALLTHROUGH */
        case '%' :
        if (*buf == '%')
            ++buf;
        else
            return NULL;
        break;
        default :
        if (*buf == '%' || *++buf == c)
            ++buf;
        else
            return NULL;
        break;
        }
    } else {
        if (*buf == c)
        ++buf;
        else
        return NULL;
    }
    }
    return (char *)buf;
}

//#endif /* WIN32 */

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

    const char* isoFormat = "%Y-%m-%d %H:%M:%S %Z";
} // namespace zz



z::string z::DateTime::ToFormatString(const z::datetime& dt, const z::string& format) {
    std::tm tm = zz::getTm(dt);
    char buf[50];
    ::strftime(buf, 50, z::s2e(format).c_str(), &tm);
    return buf;
}

z::string z::DateTime::ToFormatStringLocal(const z::datetime& dt, const z::string& format) {
    std::tm tm = zz::getLocalTm(dt);
    char buf[50];
    ::strftime(buf, 50, z::s2e(format).c_str(), &tm);
    return buf;
}

z::string z::DateTime::ToIsoString(const z::datetime& dt) {
    return ToFormatString(dt, zz::isoFormat);
}

z::string z::DateTime::ToIsoStringLocal(const z::datetime& dt) {
    return ToFormatStringLocal(dt, zz::isoFormat);
}

z::string z::DateTime::ToValueString(const z::datetime& dt) {
    z::string dstr = z::string("%{s}").arg("s", dt.val());
    return dstr;
}

z::datetime z::DateTime::FromValueString(const z::string& str) {
    int64_t t = str.to<int64_t>();
    z::datetime dt = t;
    return dt;
}

z::datetime z::DateTime::FromIsoString(const z::string& str) {
    std::tm tm;
    ::memset(&tm, 0, sizeof(tm));
    z::estring estr = z::s2e(str);
    zz::parsetime(estr.c_str(), zz::isoFormat, &tm);
    time_t nt = ::mktime(&tm);
    assert(0 < nt);
    return z::datetime(nt);
}

z::datetime z::DateTime::FromIsoStringLocal(const z::string& str) {
    std::tm tm;
    ::memset(&tm, 0, sizeof(tm));
    tm.tm_hour = 12;
    z::estring estr = z::s2e(str);
    zz::parsetime(estr.c_str(), zz::isoFormat, &tm);
    time_t nt = ::mktime(&tm);
    assert(0 < nt);
    return z::datetime(nt);
}

z::datetime z::DateTime::Now() {
    z::datetime dt = std::time(0);
    return dt;
}

int z::DateTime::Year(const z::datetime& dt) {
    std::tm tm = zz::getTm(dt);
    return tm.tm_year;
}

int z::DateTime::Month(const z::datetime& dt) {
    std::tm tm = zz::getTm(dt);
    return tm.tm_mon;
}

int z::DateTime::Date(const z::datetime& dt) {
    std::tm tm = zz::getTm(dt);
    return tm.tm_mday;
}

z::datetime z::DateTime::AddYears(const z::datetime& dt, const int& years) {
    std::tm tm = zz::getTm(dt);
    tm.tm_year += years;
    time_t nt = ::mktime(&tm);
    assert(nt > 0);
    return z::datetime(nt);
}

z::datetime z::DateTime::AddMonths(const z::datetime& dt, const int& months) {
    std::tm tm = zz::getTm(dt);
    tm.tm_mon += months;
    time_t nt = ::mktime(&tm);
    assert(nt > 0);
    return z::datetime(nt);
}

z::datetime z::DateTime::AddDays(const z::datetime& dt, const int& days) {
    time_t nt = dt.val() + (days * 60 * 60 * 24);
    assert(nt > 0);
    return z::datetime(nt);
}

z::datetime z::DateTime::SetYear(const z::datetime& dt, const int& year) {
    std::tm tm = zz::getTm(dt);
    tm.tm_year = year;
    time_t nt = ::mktime(&tm);
    assert(nt > 0);
    return z::datetime(nt);
}

z::datetime z::DateTime::SetMonth(const z::datetime& dt, const int& month) {
    std::tm tm = zz::getTm(dt);
    tm.tm_mon = month - 1;
    time_t nt = ::mktime(&tm);
    assert(nt > 0);
    return z::datetime(nt);
}

z::datetime z::DateTime::SetDay(const z::datetime& dt, const int& day) {
    std::tm tm = zz::getTm(dt);
    tm.tm_mday = day - 1;
    time_t nt = ::mktime(&tm);
    assert(nt > 0);
    return z::datetime(nt);
}

int z::DateTime::DaysDiff(const z::datetime& dt1, const z::datetime& dt2) {
    std::tm tm1 = zz::getTm(dt1);
    std::tm tm2 = zz::getTm(dt2);
    time_t tt1 = ::mktime(&tm1);
    time_t tt2 = ::mktime(&tm2);
    double d = (::difftime(tt2, tt1) + (60 * 60 * 24)/2)/(60 * 60 * 24); // add half a day to round the diff to nearest day
    return (int)d;
}
