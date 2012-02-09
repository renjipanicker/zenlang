#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "core/String.hpp"

z::list<z::string> String::split(const z::string& str, const z::string& sep) {
    z::list<z::string> sl;
    enum State {
        sInit,
        sSep,
        sWord
    };
    State state = sInit;

    z::string v;
    for(z::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        const z::char_t& ch = *it;
        switch(state) {
            case sInit:
                if(sep.find(ch) != z::string::npos) {
                    state = sSep;
                } else {
                    v += ch;
                    state = sWord;
                }
                break;
            case sSep:
                if(sep.find(ch) != z::string::npos) {
                    state = sSep;
                } else {
                    v += ch;
                    state = sWord;
                }
                break;
            case sWord:
                if(sep.find(ch) != z::string::npos) {
                    sl.add(v);
                    v.clear();
                    state = sSep;
                } else {
                    v += ch;
                    state = sWord;
                }
                break;
        }
    }
    if(state == sWord)
        sl.add(v);
    return sl;
}

#include "utils/sqlite3/sqlite3_unicode.h"

z::char_t String::fold(const z::char_t& ch) {
    return sqlite3_unicode_fold(ch);
}

z::char_t String::CharToLower(const z::char_t& ch) {
    return sqlite3_unicode_lower(ch);
}

z::string String::StringToLower(const z::string& str) {
    z::string rv;
    for(z::string::const_iterator it = str.begin();it != str.end(); ++it) {
        rv += CharToLower(*it);
    }
    return rv;
}

bool String::isSpaceChar(const z::char_t& ch) {
    if((ch >= 9) && (ch <=13))
        return true;

    if(ch == ' ')
        return true;
    if(ch == '\t')
        return true;

    return false;
}

