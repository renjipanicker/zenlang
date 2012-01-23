#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "Dir.hpp"

#define PATH_MAX (255)

z::string Dir::CurrentDir() {
    char buff[PATH_MAX];
    getcwd( buff, PATH_MAX );
    z::string cwd(buff);
    return cwd;
}

z::string Dir::cleanPath(const z::string& path) {
    /// \todo implement cleanPath
    return path;
}

bool Dir::removePath(const z::string& path) {
    ::remove(path.c_str());
    return false;
}
