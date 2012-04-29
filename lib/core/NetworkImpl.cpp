#include "zenlang.hpp"

namespace z {
struct OnDataReceivedHandler {
    inline OnDataReceivedHandler(Response::response& r) : _ps(psProtocol), _inHeader(true), _r(r) {}
private:
    enum ParseState {
        psProtocol,
        psStatus,
        psMessage,
        psKey,
        psVal,
        psBody,
        psDone
    };

    z::string _token0;
    z::string _token1;
    ParseState _ps;
    bool _inHeader;
    Response::response& _r;
    inline size_t contentLength() const {
        z::dict<z::string,z::string>::const_iterator it = _r.header.find("Content-Length");
        if(it == _r.header.end()) {
            return 0;
        }
        const z::string& v = it->second;
        return v.to<size_t>();
    }
public:
    bool run(const std::string& buffer) {
//        if((!_inHeader) && (_response.isCached())) {
//            _r.writeBody(buffer.c_str(), buffer.length());
//            if(_r.getCachedSize() >= _response.getContentLength())
//                _ps = psDone;
//        }

        if(_ps != psDone) {
            for(size_t i = 0; i < buffer.length(); ++i) {
                const char& ch = buffer.at(i);
                if((_inHeader) && (ch == '\r'))
                    continue;

                switch(_ps) {
                case psProtocol:
                    if(ch == ' ') {
                        _r.protocol = _token0;
                        _token0 = "";
                        _ps = psStatus;
                    } else {
                        _token0 += ch;
                    }
                    break;
                case psStatus:
                    if(ch == ' ') {
                        _r.status = _token0;
                        _token0 = "";
                        _ps = psMessage;
                    } else {
                        _token0 += ch;
                    }
                    break;
                case psMessage:
                    if(ch == '\n') {
                        _r.message = _token0;
                        _token0 = "";
                        _ps = psKey;
                    } else {
                        _token0 += ch;
                    }
                    break;
                case psKey:
                    if(ch == '\n') {
                        _inHeader = false;
                        if(contentLength() > 0) {
                            _ps = psBody;
//                            _r.openCacheFile();
//                            if(_r.isCached()) {
//                                _r.writeBody(buffer.c_str()+i+1, buffer.length()-i-1);
//                                i = buffer.length() - 1; // exit the for loop
//                            }
                        } else {
                            _ps = psDone;
                        }
                    } else if(ch == ':') {
                        _ps = psVal;
                    } else {
                        _token0 += ch;
                    }
                    break;
                case psVal:
                    if(ch == '\n') {
                        _r.header[_token0] = _token1.substr(1);
                        _token0 = "";
                        _token1 = "";
                        _ps = psKey;
                    } else {
                        _token1 += ch;
                    }
                    break;
                case psBody:
                    _r.body += ch;
                    if(_r.body.length() >= contentLength()) {
                        _ps = psDone;
                    }
                    break;
                case psDone:
                    break;
                }
            }
        }
        return (_ps == psDone);
    }
};

static bool queryHttpText(const Url::url& u, OnDataReceivedHandler& drh) {
#if defined(WIN32)
    SOCKET sockfd;
#else
    int sockfd;
#endif
    int portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    z::estring ehost = z::s2e(u.host);
    z::estring eport = z::s2e(u.port);
    const char* hname = ehost.c_str();
    const char* port = eport.c_str();
    const size_t BLEN = 256;
    char buffer[BLEN];
    portno = atoi(port);

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#if defined(WIN32)
    if (sockfd == INVALID_SOCKET) {
#else
    if (sockfd < 0) {
#endif
        return false;
    }
    server = gethostbyname(hname);
    if (server == NULL) {
        std::cout << "ERROR, no such host" << std::endl;
        return false;
    }
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr,
         (char *)server->h_addr,
          server->h_length);
    serv_addr.sin_port = htons((u_short)portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        std::cout << "ERROR connecting" << std::endl;
        return false;
    }

    memset(buffer, 0, BLEN);
    z::string s = "";
    if(u.querystring.length() > 0)
        s = "?";
    sprintf(buffer, "GET %s%s%s\n\n", z::s2e(u.path).c_str(), z::s2e(s).c_str(), z::s2e(u.querystring).c_str());
//    std::cout << "sending " << buffer << std::endl;
    n = send(sockfd,buffer,strlen(buffer), 0);
    if (n < 0) {
        std::cout << "ERROR writing to socket" << std::endl;
        return false;
    }
    do {
        memset(buffer, 0, BLEN);
        n = recv(sockfd, buffer, BLEN-1, 0);
        if (n < 0) {
            std::cout << "ERROR reading from socket" << std::endl;
            break;
        }
    } while(false == drh.run(buffer));
#if defined(WIN32)
    closesocket(sockfd);
#else
    close(sockfd);
#endif
    return true;
}
} // namespace z

bool Network::GetUrl(const Url::url& u, Response::response& r) {
    z::OnDataReceivedHandler drh(r);
    z::queryHttpText(u, drh);
    return false;
}
