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

uint16_t sqlite3_unicode_fold(uint16_t c);

z::char_t String::fold(const z::char_t& ch) {
    return sqlite3_unicode_fold(ch);
}
