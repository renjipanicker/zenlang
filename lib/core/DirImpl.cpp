#include "zenlang.hpp"

#ifndef PATH_MAX
#define PATH_MAX (255)
#endif

z::string Dir::CurrentDir() {
    return z::file::cwd();
}

z::string Dir::CleanPath(const z::string& path) {
    /// \todo implement cleanPath
    return path;
}

bool Dir::RemovePath(const z::string& path) {
    return (0 == ::remove(z::s2e(path).c_str()));
}

z::string Dir::ResolveParent(const z::string& parentpath, const z::string& path) {
    z::string p = path;
    if(parentpath.at(0) != '/') {
        p = parentpath + '/' + p;
    }
    return p;
}

z::string Dir::GetPath(const z::string& path) {
    if(path.at(0) == '/') {
        return path;
    }
    return z::file::getPath(path);
}
