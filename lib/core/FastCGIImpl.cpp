#include "zenlang.hpp"
# include "utils/fcgi/fastcgi.hpp"

namespace zz {
    class OutputCallbackImpl : public ::FCGIProtocolDriver::OutputCallback {
    public:
        inline OutputCallbackImpl(SOCKET fd_) : fd(fd_), driver(z::ref(this)) {}
        virtual ~OutputCallbackImpl() {
#if defined(WIN32)
            closesocket(fd);
#else
            close(fd);
#endif
        }

        virtual void operator() (void const* buf, size_t count) {
            size_t rc;
#if defined(WIN32)
            rc = send(fd, (const char*)buf, count, 0);
#else
            rc = write(fd, buf, count);
#endif
            if (rc < 0)
                throw std::runtime_error("write() failed");
        }

        inline void onConnection() {
            // Read input from socket and put it into the protocol driver
            // for processing.

            FCGIRequest* req = 0;
            do {
                char buf[4*1024];
    #if defined(WIN32)
                size_t rc = recv(fd, buf, sizeof(buf), 0);
    #else
                size_t rc = read(fd, buf, sizeof(buf));
    #endif
                if (rc < 0)
                    throw std::runtime_error("read() failed.");
                std::cout << (int)buf[0] << ", " << (int)buf[1] << std::endl;
                driver.process_input(buf, rc);
                if (req == 0)
                    req = driver.get_request();
            }
            while (req == 0);

            // Make sure we don't get a keep-connection request.

            if (req->keep_connection) {
                std::cerr << "Test program can't handle KEEP_CONNECTION." << std::endl;
                req->end_request(1, FCGIRequest::CANT_MPX_CONN);
                return;
            }

            // Make sure we are a responder.
            if (req->role != FCGIRequest::RESPONDER) {
                std::cerr << "Test program can't handle any role but RESPONDER." << std::endl;
                req->end_request(1, FCGIRequest::UNKNOWN_ROLE);
                return;
            }

            // Print page with the environment details.

            std::cerr << "Starting to handle request #" << req->id << "." << std::endl;
            std::ostringstream os;
            os << "Content-type: text/html\r\n"
                << "\r\n"
                << "<title>FastCGI Test Program</title>" << std::endl
                << "<h1 align=center>FastCGI Test Program</h1>" << std::endl
                << "<h3>FastCGI Status</h3>" << std::endl
                << "Test Program Compile Time = " << __DATE__ " " __TIME__ << "<br>" << std::endl
//                << "Process id                = " << getpid() << "<br>" << std::endl
                << "Request fd            = " << fd << "<br>" << std::endl
                << "Request id                = " << req->id << "<br>" << std::endl
//                << "Request number            = " << req_counter << "<br>" << std::endl
                << "<h3>Request Environment</h3>" << std::endl;
            for (std::map<std::string,std::string>::const_iterator i = req->params.begin(); i != req->params.end(); ++i)
                os << i->first << "&nbsp;=&nbsp;" << i->second << "<br>" << std::endl;
            req->write(os.str().data(), os.str().size());

            // Make sure we read the entire standard input stream, then
            // echo it back.

            req->write("<h3>Input Stream</h3>\n" \
                "<pre>\n");
            while(req->stdin_eof == false) {
                char buf[4*1024];
    #if defined(WIN32)
                size_t rc = recv(fd, buf, sizeof(buf), 0);
    #else
                size_t rc = read(fd, buf, sizeof(buf));
    #endif
                if (rc < 0)
                    throw std::runtime_error("read() failed.");
                driver.process_input(buf, rc);
            }
            if (req->stdin_stream.empty() == false) {
                req->write(req->stdin_stream);
                req->stdin_stream.erase();
            }
            req->write("</pre>\n");

            // Terminate the request.

            std::cerr << "Request #" << req->id << " handled successfully." << std::endl;
            req->end_request(0, FCGIRequest::REQUEST_COMPLETE);
        }

    private:
        SOCKET fd;
        FCGIProtocolDriver driver;
    };
}

bool FastCGI::StartServer(const int& port) {
    sockaddr_in sa;
    SOCKET sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(-1 == sfd) {
        throw std::runtime_error("can not create socket.");
    }

    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons((short)port);
    sa.sin_addr.s_addr = INADDR_ANY;

    if(-1 == bind(sfd,(sockaddr *)&sa, sizeof(sa))) {
        throw std::runtime_error("error bind failed.");
    }

    if(-1 == listen(sfd, 10)) {
        throw std::runtime_error("error listen failed.");
    }

    std::cout << "sfd: " << sfd << std::endl;

    int cnt = 0;
    while(1) {
        std::cout << "\r" << ++cnt;
        fd_set aset;
        FD_ZERO(&aset);
        FD_SET(sfd, &aset);
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        if(::select(sfd, &aset, NULL, NULL, &timeout) == 1) {
            std::cout << ": connect: " << std::endl;
            int sa_len = sizeof(sa);
            SOCKET fd = accept(sfd, (sockaddr*)&sa, &sa_len);
            if (fd < 0)
                throw std::runtime_error("accept() failed.");

            // Setup the FCGI protocol driver.
            zz::OutputCallbackImpl op_cb(fd);
            op_cb.onConnection();
            return 0;
        }
    }

    // done
    std::cerr << "FastCGI test program terminating." << std::endl;
    return false;
}
