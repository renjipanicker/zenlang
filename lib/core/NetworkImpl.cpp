#include "zenlang.hpp"

#if defined(WIN32)
#include <WinSock2.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

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

static void error(const char *msg)
{
    perror(msg);
    exit(0);
}

static bool queryHttpText(const Url::url& u, OnDataReceivedHandler& drh) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    const char* hname = z::s2e(u.host).c_str();
    const char* port = z::s2e(u.port).c_str();
    std::cout << "(z): Connecting to " << hname << ", port " << port << std::endl;
    const size_t BLEN = 256;
    char buffer[BLEN];
    portno = atoi(port);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return false;
    }
    server = gethostbyname(hname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        return false;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
        return false;
    }

    bzero(buffer,BLEN);
    z::string s = "";
    if(u.querystring.length() > 0)
        s = "?";
    sprintf(buffer, "GET %s%s%s\n\n", z::s2e(u.path).c_str(), z::s2e(s).c_str(), z::s2e(u.querystring).c_str());
    std::cout << "sending " << buffer << std::endl;
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) {
        error("ERROR writing to socket");
        return false;
    }
    do {
        bzero(buffer,BLEN);
        n = read(sockfd,buffer,BLEN-1);
        if (n < 0) {
            error("ERROR reading from socket");
            break;
        }
    } while(false == drh.run(buffer));
    close(sockfd);
    return true;
}

bool Network::getUrl(const Url::url& u, Response::response& r) {
    OnDataReceivedHandler drh(r);
    queryHttpText(u, drh);
    return false;
}
