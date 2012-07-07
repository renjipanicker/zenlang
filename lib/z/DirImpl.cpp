#include "zenlang.hpp"

#ifndef PATH_MAX
#define PATH_MAX (255)
#endif

z::string z::Dir::CurrentDir() {
    return z::dir::cwd();
}

z::string z::Dir::CleanPath(const z::string& path) {
    /// \todo implement cleanPath
    return path;
}

bool z::Dir::RemovePath(const z::string& path) {
    return (0 == ::remove(z::s2e(path).c_str()));
}

z::string z::Dir::ResolveParent(const z::string& parentpath, const z::string& path) {
    z::string p = path;
    p = parentpath + '/' + p;
    return p;
}

z::string z::Dir::GetPath(const z::string& path) {
    return z::dir::getPath(path);
}
