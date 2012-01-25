#pragma once

#include "ast.hpp"
class Generator {
public:
    virtual void run() = 0;
};

struct Indent {
    inline Indent() {
        ind[_indent] = 32;
        _indent += 4;
        ind[_indent] = 0;
    }
    inline ~Indent() {
        ind[_indent] = 32;
        _indent -= 4;
        ind[_indent] = 0;
    }
    inline static const char* get() {return ind;}
    inline static void init() {
        if(_indent < 0) {
            memset(ind, 32, Size);
            _indent = 0;
            ind[_indent] = 0;
        }
    }
private:
    static const int Size = 1024;
    static char ind[Size];
    static int _indent;
};
#define INDENT Indent _ind_

struct OutputFile {
    OutputFile(FILE*& fp, const z::string& dir, const z::string& filename);
    ~OutputFile();
    FILE*& _fp;
    inline const z::string& name() const {return _name;}
private:
    z::string _name;
};

inline z::string getBaseName(const z::string& filename) {
    z::string basename = filename;
    z::string::size_type idx = -1;

    // strip last extension, if any
    idx = basename.rfind('.');
    if(idx >= 0)
        basename = basename.substr(0, idx);

    // strip path, if any
    idx = basename.rfind('/');
    if(idx >= 0)
        basename = basename.substr(idx + 1);

    return basename;
}

inline z::string getExtention(const z::string& filename) {
    z::string::size_type idx = -1;

    // find last extension, if any
    idx = filename.rfind('.');
    if(idx >= 0)
        return filename.substr(idx + 1);
    return "";
}

