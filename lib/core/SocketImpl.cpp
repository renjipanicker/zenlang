#include "zenlang.hpp"

z::socket Socket::InitServer(const int& port) {
    sockaddr_in sa;
    SOCKET sfd = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(-1 == sfd) {
        throw z::Exception("Socket::StartServer", z::string("Error creating socket"));
    }

    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons((short)port);
    sa.sin_addr.s_addr = INADDR_ANY;

    if(-1 == ::bind(sfd,(sockaddr *)&sa, sizeof(sa))) {
        throw z::Exception("Socket::StartServer", z::string("Error bind() failed"));
    }
    return z::socket(sfd);
}

void Socket::StartServer(const z::socket& s) {
    if(-1 == ::listen(s.val(), 10)) {
        throw z::Exception("Socket::StartServer", z::string("Error listen() failed"));
    }
}

void Socket::Close(const z::socket& s) {
#if defined(WIN32)
    if(-1 == ::closesocket(s.val())) {
#else
    if(-1 == ::close(s.val())) {
#endif
        throw z::Exception("Socket::Close", z::string("Error close() failed"));
    }
}

void Socket::OnConnect::addHandler(const z::socket& s, const z::pointer<Handler>& h) {
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

bool Socket::OnConnectDevice::run(const int& timeout) {
    static int cnt = 0;
    if(!zz::isDataAvailable(s, timeout)) {
        return false;
    }

    SOCKET fd = ::accept(s.val(), 0, 0);
    if (fd < 0) {
        throw z::Exception("Socket::OnConnectDevice", z::string("Error accept() failed"));
    }
    if (fd == 0) {
        return true;
    }
    h.get().run(fd);
    return false;
}

void Socket::OnRecv::addHandler(const z::socket& s, const z::pointer<Handler>& h) {
    Socket::OnRecvDevice* d = new Socket::OnRecvDevice(s, h);
    z::ctx().startPoll(d);
}

bool Socket::OnRecvDevice::run(const int& timeout) {
    static int cnt = 0;
    if(!zz::isDataAvailable(s, timeout)) {
        return false;
    }

    char buf[4*1024];
#if defined(WIN32)
    ssize_t rc = ::recv(s.val(), buf, sizeof(buf), 0);
#else
    ssize_t rc = ::recv(s.val(), buf, sizeof(buf), 0);
//    int rc = ::read(s.val(), buf, sizeof(buf));
#endif
    if (rc <= 0) {
        if (rc < 0) {
            /// \todo invoke error-handler
            //DWORD e = ::WSAGetLastError();
            z::mlog("Socket::OnrecvDevice", z::string("recv() error: %{e}").arg("e", rc));
        }
        return true;
    }

    z::data d((const uint8_t*)buf, rc);
    h.get().run(d);
    return false;
}

namespace zz {
    inline int SendSocket(const z::socket& s, const char* bfr, const size_t& len) {
#if defined(WIN32)
        ssize_t rc = ::send(s.val(), bfr, len, 0);
#else
        ssize_t rc = ::send(s.val(), bfr, len, 0);
//    int rc = ::write(s.val(), (const char*)d.c_str(), d.length());
#endif
        if (rc <= 0) {
            if (rc < 0) {
                z::mlog("zz::SendSocket", z::string("send() error: %{e}").arg("e", rc));
                //throw z::Exception("Socket::SendString", z::string("Error send() failed: socket: %{s}").arg("s", s.val()));
            }
            return rc;
        }
        return rc;
    }
}

int Socket::SendData(const z::socket& s, const z::data& d) {
    return zz::SendSocket(s, (const char*)d.c_str(), d.length());
}

int Socket::SendString(const z::socket& s, const z::string& d) {
    z::estring es = z::s2e(d);
    return zz::SendSocket(s, (const char*)es.c_str(), es.length());
}
