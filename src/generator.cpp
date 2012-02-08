#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "generator.hpp"

char Indent::ind[Size] = {32};
int Indent::_indent = -1;

//@
//inline bool exists(const z::string& path) {
//    struct stat b;
//    return (0 == stat(path.c_str(), &b));
//}

//inline void mkpath(const z::string& path, const z::string& dir) {
//    if(dir.size() == 0)
//        return;

//#ifdef WIN32
//        z::string sep = "\\";
//#else
//    z::string sep = "/";
//#endif

//    z::string p;
//    if(path.size() > 0) {
//        p += path;
//        p += sep;
//    }
//    p += dir;
//    if(!exists(p)) {
//#if defined(WIN32)
//        _mkdir(p.c_str());
//#else
//        mkdir(p.c_str(), S_IRWXU | S_IRGRP | S_IROTH);
//#endif
//    }
//}

//@
//OutputFile::OutputFile(FILE*& fp, const z::string& dir, const z::string& filename) : _fp(fp) {
//#ifdef WIN32
//    z::string sep = "\\";
//#else
//    z::string sep = "/";
//#endif
//    z::string base = "";
//    z::string::size_type prev = 0;
//    for(z::string::size_type next = dir.find(sep); next != z::string::npos;next = dir.find(sep, next+1)) {
//        z::string sdir = dir.substr(prev, next - prev);
//        mkpath(base, sdir);
//        base += sdir;
//        base += sep;
//        prev = next + 1;
//    }
//    z::string sdir = dir.substr(prev);
//    mkpath(base, sdir);

//    z::string fpath = dir + sep + filename;
//#ifdef WIN32
//    _fp = fopen(fpath.c_str(), "w");
//#else
//    _fp = fopen(fpath.c_str(), "w");
//#endif
//    if(_fp == 0) {
//        throw z::Exception("OutputFile", z::fmt("Unable to open output file %{s}").add("s", fpath));
//    }
//    _name = fpath;
//}

//OutputFile::~OutputFile() {
//    fclose(_fp);
//}
