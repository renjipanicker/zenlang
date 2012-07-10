#include "zenlang.hpp"

z::list<z::string> z::String::Split(const z::string& str, const z::string& sep) {
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

# include "utils/sqlite3/sqlite3_unicode.h"

z::char_t z::String::Fold(const z::char_t& ch) {
    return sqlite3_unicode_fold((z::char16_t)ch);
}

z::char_t z::String::CharToLower(const z::char_t& ch) {
    return sqlite3_unicode_lower((z::char16_t)ch);
}

z::string z::String::StringToLower(const z::string& str) {
    z::string rv;
    for(z::string::const_iterator it = str.begin();it != str.end(); ++it) {
        rv += CharToLower(*it);
    }
    return rv;
}

z::char_t z::String::CharToUpper(const z::char_t& ch) {
    return sqlite3_unicode_upper((z::char16_t)ch);
}

z::string z::String::StringToUpper(const z::string& str) {
    z::string rv;
    for(z::string::const_iterator it = str.begin();it != str.end(); ++it) {
        rv += CharToUpper(*it);
    }
    return rv;
}

int64_t z::String::StringToLong(const z::string& str) {
    return str.to<int64_t>();
}

bool z::String::IsSpaceChar(const z::char_t& ch) {
    if((ch >= 9) && (ch <=13))
        return true;

    if(ch == ' ')
        return true;
    if(ch == '\t')
        return true;

    return false;
}

z::string z::String::TrimStringCollect(const z::string& str, z::string& prev, z::string& post) {
    int start = 0;
    int end = str.length() - 1;
    while((start < end) && (String::IsSpaceChar(str[start])))
        ++start;

    while((end > start) && (String::IsSpaceChar(str[end])))
        --end;

    prev = str.substr(0, start);
    post = str.substr(end+1);

    z::string rv;
    if(end < start)
        return rv;
    rv = str.substr(start, end-start+1);
    return rv;
}

z::string z::String::TrimString(const z::string& str) {
    z::string prev;
    z::string post;
    return String::TrimStringCollect(str, prev, post);
}

z::string z::String::GetGuid() {
    return "";
}
