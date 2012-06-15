#include "zenlang.hpp"

z::socket Socket::InitServer(const int& port) {
    sockaddr_in sa;
    SOCKET sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(-1 == sfd) {
        throw z::Exception("Socket::StartServer", z::string("Error creating socket"));
    }

    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons((short)port);
    sa.sin_addr.s_addr = INADDR_ANY;

    if(-1 == bind(sfd,(sockaddr *)&sa, sizeof(sa))) {
        throw z::Exception("Socket::StartServer", z::string("Error bind() failed"));
    }
    return z::socket(sfd);
}

void Socket::StartServer(const z::socket& s) {
    if(-1 == listen(s.val(), 10)) {
        throw z::Exception("Socket::StartServer", z::string("Error listen() failed"));
    }
}

void Socket::OnConnect::addHandler(const z::socket& s, Socket::OnConnect::Handler* h) {
    Socket::OnConnectDevice* d = new Socket::OnConnectDevice(s, h);
    z::ctx().startPoll(d);
}

namespace zz {
    inline bool isDataAvailable(const z::socket& s, const int& timeout) {
        fd_set aset;
        FD_ZERO(&aset);
#if defined(WIN32)
    #pragma warning (disable:4127)
#endif
        FD_SET(s.val(), &aset);
#if defined(WIN32)
    #pragma warning (default:4127)
#endif

        timeval to;
        to.tv_sec = 0;
        to.tv_usec = timeout;
        return (::select(s.val(), &aset, NULL, NULL, &to) == 1);
    }
}

void Socket::OnConnectDevice::run(const int& timeout) {
    static int cnt = 0;
    std::cout << "\ronc::poll: " << ++cnt;
    if(zz::isDataAvailable(s, timeout) == 1) {
        SOCKET fd = ::accept(s.val(), 0, 0);
        if (fd < 0) {
            throw z::Exception("Socket::OnConnectDevice", z::string("Error accept() failed"));
        }
        z::ref(h).run(fd);
        std::cout << "leave" << std::endl;
        return;
    }
}

void Socket::OnRecv::addHandler(const z::socket& s, Socket::OnRecv::Handler* h) {
    Socket::OnRecvDevice* d = new Socket::OnRecvDevice(s, h);
    z::ctx().startPoll(d);
}

void Socket::OnRecvDevice::run(const int& timeout) {
    static int cnt = 0;
    std::cout << "\ronr::poll: " << ++cnt;
    if(zz::isDataAvailable(s, timeout) == 1) {
        char buf[4*1024];
#if defined(WIN32)
        size_t rc = ::recv(s.val(), buf, sizeof(buf), 0);
#else
        size_t rc = ::read(s.val(), buf, sizeof(buf));
#endif
        if (rc < 0) {
            throw z::Exception("Socket::OnRecvDevice", z::string("Error read() failed"));
        }

        if (rc == 0) {
            return;
        }

        z::data d(buf, rc);
        z::ref(h).run(d);
        return;
    }
}
