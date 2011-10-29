#pragma once

struct OutputFile {
    inline OutputFile(FILE*& fp, const std::string& filename) : _fp(fp) {
        _fp = fopen(filename.c_str(), "w");
        if(_fp == 0) {
            throw Exception("Unable to open output file %s\n", filename.c_str());
        }
    }

    inline ~OutputFile() {
        fclose(_fp);
    }
    FILE*& _fp;
};

inline std::string getBaseName(const std::string& filename) {
    std::string basename = filename;
    size_t idx = -1;

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

inline std::string getExtention(const std::string& filename) {
    size_t idx = -1;

    // find last extension, if any
    idx = filename.rfind('.');
    if(idx >= 0)
        return filename.substr(idx + 1);
    return "";
}

