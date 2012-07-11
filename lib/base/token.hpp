#pragma once

namespace z {
struct TokenData {
private:
    static const size_t Size = 32;
    const z::char_t* _filename;
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
    inline const z::char_t* filename() const {return _filename;}
    inline const int& id() const {return _id;}
    inline const int& row() const {return _row;}
    inline const int& col() const {return _col;}
    inline const char* text() const {return (_lvalue?_lvalue:_text);}

public:
    static z::string getText(const char* start, const char* end) {
        z::string txt;

        assert(start > 0);
        assert(end > 0);
        assert(start <= end);

        for(const char* t = start; t < end; ++t) {
            txt += *t;
        }
//        printf("getText: text: %s\n", txt.c_str());
        return txt;
    }

    static TokenData createT(const z::string& filename, const int& id, const int& row, const int& col, const z::string& txt) {
//        printf("createT(%d, %d): start: %lu, end %lu, end-start: %ld\n", row, col, (unsigned long)start, (unsigned long)end, end-start);
        TokenData td;
        td.init();
        td._filename = filename.c_str();
        td._id = id;
        td._row = row;
        td._col = col;

        char* buf = 0;
        if(txt.size() >= Size) {
            td._lvalue = new char[txt.size() + 1];
            buf = td._lvalue;
        } else {
            td._lvalue = 0;
            buf = td._text;
        }

        char* s = buf;
        for(z::string::const_iterator it = txt.begin(); it != txt.end(); ++it) {
            *s = (char)*it;
            ++s;
        }
        *s = 0;
//        printf("createT(%d, %d): id: %d: %lu, text: %s\n", td.row(), td.col(), td.id(), (unsigned long)td._lvalue, td.text());
        return td;
    }

    static inline void deleteT(TokenData& td) {
//        printf("deleteT(%d, %d): id: %d: %lu, text: %s\n", td.row(), td.col(), td.id(), (unsigned long)td._lvalue, td.text());
        if(td._lvalue != 0) {
            delete[] td._lvalue;
            td._lvalue = 0;
        }
    }
};
}
