#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "generator.hpp"

char Indent::ind[Size] = {32};
int Indent::_indent = -1;

OutputFile::OutputFile(FILE*& fp, const std::string& filename) : _fp(fp) {
    _fp = fopen(filename.c_str(), "w");
    if(_fp == 0) {
        throw z::Exception("Unable to open output file %s\n", filename.c_str());
    }
}

OutputFile::~OutputFile() {
    fclose(_fp);
}
