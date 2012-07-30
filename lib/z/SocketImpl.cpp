#include "zenlang.hpp"

namespace zz {
    inline z::string geterror() {
#if defined(WIN32)
        char buf[1024];
        ::strerror_s(buf, 1024, errno);
        return buf;
#else
        return ::strerror(errno);
#endif
    }
}

z::socket z::Socket::OpenServer(const int& port) {
    SOCKET sfd = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sfd < 0) {
        throw z::Exception("Socket::InitServer", z::string("Error creating socket"));
    }

    int on = 1;
    if(::setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) < 0) {
        throw z::Exception("Socket::InitServer", z::string("Error setsocketopt() failed"));
    }

    sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons((short)port);
    sa.sin_addr.s_addr = INADDR_ANY;

    if(::bind(sfd,(sockaddr *)&sa, sizeof(sa)) < 0) {
        throw z::Exception("Socket::InitServer", z::string("Error bind() failed"));
    }

    return z::socket(sfd);
}

z::socket z::Socket::OpenClient(const z::string& host, const z::string& portstr) {
    SOCKET sfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sfd < 0) {
        throw z::Exception("Socket::InitClient", z::string("Error creating socket"));
    }

    ::hostent* server = ::gethostbyname(z::s2e(host).c_str());
    if (server == NULL) {
        throw z::Exception("Socket::InitClient", z::string("Error: No such host: %{s}").arg("s", host));
    }

    int port = atoi(z::s2e(portstr).c_str());

    ::sockaddr_in sa;
    memset((char *) &sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    memcpy((char *)&sa.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    sa.sin_port = htons((u_short)port);
    if (::connect(sfd, (::sockaddr*) &sa,sizeof(sa)) < 0) {
        throw z::Exception("Socket::InitClient", z::string("Error: Error connecting to host: %{s}").arg("s", host));
    }

    return z::socket(sfd);
}

void z::Socket::StartServer(const z::socket& s) {
    std::cout << "server starting" << std::endl;
    if(-1 == ::listen(s.val(), 10)) {
        throw z::Exception("Socket::StartServer", z::string("Error listen() failed"));
    }
}

void z::Socket::Close(const z::socket& s) {
#if defined(WIN32)
    if(-1 == ::closesocket(s.val())) {
#else
    if(-1 == ::close(s.val())) {
#endif
        throw z::Exception("Socket::Close", z::string("Error close() failed"));
    }
}

void z::Socket::OnConnect::addHandler(const z::socket& s, const z::pointer<Handler>& h) {
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
        return (::select(s.val()+1, &aset, NULL, NULL, &to) != 0);
    }
}

bool z::Socket::OnConnectDevice::run(const int& timeout) {
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

void z::Socket::OnRecv::addHandler(const z::socket& s, const z::pointer<Handler>& h) {
    Socket::OnRecvDevice* d = new Socket::OnRecvDevice(s, h);
    z::ctx().startPoll(d);
}

bool z::Socket::OnRecvDevice::run(const int& timeout) {
    if(!zz::isDataAvailable(s, timeout)) {
        return false;
    }

    char buf[4*1024];
    long rc = ::recv(s.val(), buf, sizeof(buf), 0);
    if (rc <= 0) {
        if (rc < 0) {
            /// \todo invoke error-handler
            //DWORD e = ::WSAGetLastError();
            //z::mlog("Socket::OnrecvDevice", z::string("recv() error: %{e}").arg("e", rc));
        }
        return true;
    }

    z::data d((const uint8_t*)buf, rc);
    h.get().run(d);
    return false;
}

namespace zz {
    inline size_t SendSocket(const z::socket& s, const char* bfr, const size_t& len) {
        long rc = ::send(s.val(), bfr, len, 0);
        if (rc <= 0) {
            if (rc < 0) {
                throw z::Exception("Socket::SendString", z::string("Error send() failed: socket: %{s}").arg("s", geterror()));
            }
            assert(rc == 0);
            return rc;
        }
        return rc;
    }
}

size_t z::Socket::SendData(const z::socket& s, const z::data& d) {
    return zz::SendSocket(s, (const char*)d.c_str(), d.length());
}

size_t z::Socket::SendString(const z::socket& s, const z::string& d) {
    z::estring es = z::s2e(d);
    return zz::SendSocket(s, (const char*)es.c_str(), es.length());
}
