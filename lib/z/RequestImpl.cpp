#include "zenlang.hpp"
//#include "request.hpp"
//
//#if defined(WIN32)
//#define snprintf _snprintf_s
//#endif
//
//std::string Request::getPostDataString() const {
//    std::stringstream ss;
//    std::string sep;
//    for(PostData::const_iterator it = _postData.begin(); it != _postData.end(); ++it ) {
//        const PostValue& postValue = *it;
//        ss << sep << postValue.key << "=" << postValue.val;
//        sep = "&";
//    }
//    return ss.str();
//}
//
//std::string Request::getString() const {
//    std::string pd = getPostDataString();
//
//    const int BufferSize = 1024;
//    char buffer[BufferSize];
//    memset(buffer, 0, BufferSize);
//
//    // create request string
//    snprintf(buffer, BufferSize, "%s %s HTTP/1.1\r\n"
//            "Content-Length: %d\r\n"
//            "Content-Type: application/x-www-form-urlencoded\r\n"
//            "\r\n"
//            "%s", _method.c_str(), _url.path().c_str(), (int)pd.length(), pd.c_str());
//
//    return std::string(buffer);
//}
