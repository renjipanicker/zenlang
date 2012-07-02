/*
 * Copyright (c) 2001-2010 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "zenlang.hpp"
#include "fastcgi.hpp"

#if defined(WIN32)
int getpid() {
    return 0;
}

#endif

class OutputCallback : public FCGIProtocolDriver::OutputCallback
{
public:
  OutputCallback(int fd_) : fd(fd_)
  {
  }
  virtual ~OutputCallback()
  {
#if defined(WIN32)
    closesocket(fd);
#else
    close(fd);
#endif
  }
  virtual void operator() (void const* buf, size_t count)
  {
      const char* bb = (const char*)buf;
      for(size_t i = 0; i < count; ++i) {
          std::cout << i << ":" << (int)(bb[i]) << std::endl;
      }
    size_t rc;
#if defined(WIN32)
    rc = send(fd, (const char*)buf, count, 0);
#else
    rc = write(fd, buf, count);
#endif
    if (rc < 0)
      throw std::runtime_error("write() failed");
  }

private:
   int fd;
};

int main(int, char** argv)
try
{
#if defined(WIN32)
    // Initialize Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    sockaddr_in sa;
    int sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(-1 == sfd)
    {
      throw std::runtime_error("can not create socket.");
    }

    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(9001);
    sa.sin_addr.s_addr = INADDR_ANY;

    if(-1 == bind(sfd,(sockaddr *)&sa, sizeof(sa)))
    {
      throw std::runtime_error("error bind failed.");
    }

    if(-1 == listen(sfd, 10))
    {
      throw std::runtime_error("error listen failed.");
    }

   std::cout << "sfd: " << sfd << std::endl;
   for(size_t req_counter = 0;;)
  {
    // Accept a connection.
    int       socket;
    int sa_len = sizeof(sa);

    socket = accept(sfd, (sockaddr*)&sa, &sa_len);
    if (socket < 0)
      throw std::runtime_error("accept() failed.");

    // Setup the FCGI protocol driver.

    OutputCallback op_cb(socket);
    FCGIProtocolDriver driver(op_cb);

    // Read input from socket and put it into the protocol driver
    // for processing.

    FCGIRequest* req = 0;
    do
    {
      char buf[4*1024];
#if defined(WIN32)
     size_t rc = recv(socket, buf, sizeof(buf), 0);
#else
      size_t rc = read(socket, buf, sizeof(buf));
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

    if (req->keep_connection)
    {
      std::cerr << "Test program can't handle KEEP_CONNECTION." << std::endl;
      req->end_request(1, FCGIRequest::CANT_MPX_CONN);
      continue;
    }

    // Make sure we are a responder.

    if (req->role != FCGIRequest::RESPONDER)
    {
      std::cerr << "Test program can't handle any role but RESPONDER." << std::endl;
      req->end_request(1, FCGIRequest::UNKNOWN_ROLE);
      continue;
    }

    // Print page with the environment details.

    std::cerr << argv[0] << "[" << getpid() << "]: Starting to handle request #" << req->id << "." << std::endl;
    ++req_counter;
    std::ostringstream os;
    os << "Content-type: text/html\r\n\r\n<html><body>Hello</body></html>\n";
    //os << "Content-type: text/html\r\n"
    //   << "\r\n"
    //   << "<title>FastCGI Test Program</title>" << std::endl
    //   << "<h1 align=center>FastCGI Test Program</h1>" << std::endl
    //   << "<h3>FastCGI Status</h3>" << std::endl
    //   << "Test Program Compile Time = " << __DATE__ " " __TIME__ << "<br>" << std::endl
    //   << "Process id                = " << getpid() << "<br>" << std::endl
    //   << "Request socket            = " << socket << "<br>" << std::endl
    //   << "Request id                = " << req->id << "<br>" << std::endl
    //   << "Request number            = " << req_counter << "<br>" << std::endl
    //   << "<h3>Request Environment</h3>" << std::endl;
    //for (std::map<std::string,std::string>::const_iterator i = req->params.begin(); i != req->params.end(); ++i)
    //  os << i->first << "&nbsp;=&nbsp;" << i->second << "<br>" << std::endl;
    req->write(os.str().data(), os.str().size());

    // Make sure we read the entire standard input stream, then
    // echo it back.

    req->write("<h3>Input Stream</h3>\n" \
              "<pre>\n");
//    while(req->stdin_eof == false)
//    {
//      char buf[4*1024];
//#if defined(WIN32)
//      size_t rc = recv(socket, buf, sizeof(buf), 0);
//#else
//      size_t rc = read(socket, buf, sizeof(buf));
//#endif
//      if (rc < 0)
//        throw std::runtime_error("read() failed.");
//      driver.process_input(buf, rc);
//    }
//    if (req->stdin_stream.empty() == false)
//    {
//      req->write(req->stdin_stream);
//      req->stdin_stream.erase();
//    }
    req->write("</pre>\n");

    // Terminate the request.

    std::cerr << argv[0] << "[" << getpid() << "]: Request #" << req->id << " handled successfully." << std::endl;
    req->end_request(0, FCGIRequest::REQUEST_COMPLETE);
  }

  // done

  std::cerr << "FastCGI test program terminating." << std::endl;
  return 0;
}
catch(std::exception const & e)
{
  std::cerr << "Caught exception: " << e.what() << std::endl;
  return 1;
}
catch(...)
{
  std::cerr << "Caught unknown exception." << std::endl;
  return 1;
}
