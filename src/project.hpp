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
    inline Project& addSource(const std::string& dir) { _sourceList.push_back(dir); return ref(this);}
    inline const PathList& sourceList() const {return _sourceList;}
private:
    PathList _includeList;
    PathList _sourceList;
};
