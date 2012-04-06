#include "zenlang.hpp"

z::string File::completeBasePath(const z::string& path) {
    z::string cbase = path;
    z::string::size_type spos = cbase.rfind('/');
    if(spos != z::string::npos) {
        cbase = cbase.substr(spos+1);
    }
    z::string::size_type dpos = cbase.rfind('.');
    if(dpos != z::string::npos) {
        cbase = cbase.substr(0, dpos);
    }
    return cbase;
}
