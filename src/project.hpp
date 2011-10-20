#pragma once

class Project {
public:
    typedef std::list<std::string> PathList;
public:
    inline Project() {}
public:
    inline Project& addInclude(const std::string& dir) { _includeList.push_back(dir); return ref(this);}
    inline const PathList& includeList() const {return _includeList;}
public:
    inline Project& addIncludeFile(const std::string& file) { _includeFileList.push_back(file); return ref(this);}
    inline const PathList& includeFileList() const {return _includeFileList;}
public:
    inline Project& addSource(const std::string& file) { _sourceList.push_back(file); return ref(this);}
    inline const PathList& sourceList() const {return _sourceList;}
private:
    PathList _includeList;
    PathList _includeFileList;
    PathList _sourceList;
};
