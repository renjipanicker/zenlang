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
