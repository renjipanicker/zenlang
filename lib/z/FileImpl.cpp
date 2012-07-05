#include "zenlang.hpp"

z::string File::CompleteBasePath(const z::string& path) {
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

z::file File::Open(const z::string& filename, const z::string& mode) {
#if defined(WIN32)
    FILE* fs = 0;
    errno_t e = fopen_s(&fs, z::s2e(filename).c_str(), z::s2e(mode).c_str());
    if(e != 0) {
        throw z::Exception("File::Open", z::string("Error opening file: %{f}").arg("f", filename));
    }
#else
    FILE* fs = fopen(z::s2e(filename).c_str(), z::s2e(mode).c_str());
    if(0 == fs) {
        throw z::Exception("File::Open", z::string("Error opening file: %{f}").arg("f", filename));
    }
#endif
    return z::file(filename, fs);
}

void File::ReadEachLine::addHandler(const z::string& s, const z::pointer<Handler>& h) {
    z::file f = Open(s, "r");
    File::ReadLineDevice* d = new File::ReadLineDevice(f, h);
    z::ctx().startPoll(d);
}

bool File::ReadLineDevice::run(const int& timeout) {
    char b[1024];
    char* rv = fgets(b, 1024, f.val());
    if(rv == 0) {
        return true;
    }

    z::string line(b);
    h.get().run(line);

    //static int cnt = 0;
    //std::cout << "\ronc::poll: " << ++cnt;
    //if(zz::isDataAvailable(s, timeout) == 1) {
    //    SOCKET fd = ::accept(s.val(), 0, 0);
    //    if (fd < 0) {
    //        throw z::Exception("Socket::OnConnectDevice", z::string("Error accept() failed"));
    //    }
    //    h.get().run(fd);
    //    std::cout << "leave" << std::endl;
    //    return;
    //}
    return false;
}
