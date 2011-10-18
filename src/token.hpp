#pragma once
#include <string.h>
#include <string>
#include <assert.h>
#include <stdio.h>

struct TokenData {
private:
    static const int Size = 32;
    int _id;
    int _row;
    int _col;
    char _text[Size];
    char* _lvalue;
public:
    inline void init() {
        _id = 0;
        _row = 0;
        _col = 0;
        _text[0] = 0;
        _lvalue = 0;
    }

public:
    inline const int& id() const {return _id;}
    inline const int& row() const {return _row;}
    inline const int& col() const {return _col;}
    inline const char* text() const {return (_lvalue?_lvalue:_text);}

public:
    static TokenData createT(const int& id, const int& row, const int& col, const char* start, const char* end) {
        TokenData td;
        td.init();
        td._id = id;
        td._row = row;
        td._col = col;

        assert(start > 0);
        assert(end > 0);
        assert(start <= end);
        size_t len = end - start;
        char* buf = 0;
        if(len < Size) {
            buf = td._text;
        } else {
            td._lvalue = new char[len + 1];
            buf = td._lvalue;
        }

        char* s = buf;
        for(const char* t = start; t < end; ++t, ++s) {
            *s = *t;
        }
        *s = 0;
        return td;
    }

    static inline void deleteT(TokenData& self) {
        if(self._lvalue != 0) {
            delete[] self._lvalue;
            self._lvalue = 0;
        }
    }
};
