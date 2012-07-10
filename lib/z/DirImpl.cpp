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

z::string z::Dir::GetTemp() {
#if defined(WIN32)
    assert(false);
    return "c:\\temp"; /// \todo: replace with Sh-function call.
#else
    return "/tmp/";
#endif
}

z::string z::Dir::GetTmpDir(const z::string& path, const z::string& fmt) {
    z::string fpath = path;
    if(fpath.length() == 0) {
        fpath = GetTemp();
    }
    for(size_t i = 0; i < 10000; ++i) {
        z::string d = z::string(fmt).arg("i", i);
        z::string p = fpath + "/" + d + "/";
        std::cout << "p: " << p << std::endl;
        if(!dir::exists(p)) {
            std::cout << "creating p: " << p << std::endl;
            z::dir::mkpath(p); // make full path
            return d; // return just the dirname, not the full path
        }
    }
    throw z::Exception("Dir::GetTmpDir", z::string("Error creating temp directory"));
}
