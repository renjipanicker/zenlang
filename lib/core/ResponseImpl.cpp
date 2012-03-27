#include "pch.hpp"
#include "zenlang.hpp"
//#include "response.hpp"
//#include "system.hpp"
//
//void Response::addHeader(const std::string& key, const std::string& val) {
//    if(key == "Content-Length") {
//        _contentLength = atol(val.c_str());
//    } else if(key == "Content-Type") {
//        _contentType = val;
//    }
//    _headerList[key] = val;
//}
//
//inline bool Response::isCachable() {
//    if(_contentType == "application/zip") {
//        return true;
//    }
//    return false;
//}
//
//void Response::openCacheFile() {
//    if(!isCachable()) {
//        return;
//    }
//
//    std::string filename = _cacheDir + "/" + _filename;
//    //System::makeFile(filename);
//    _isCached = true;
//    _fp = fopen(filename.c_str(), "wb");
//}
//
//void Response::closeCacheFile() {
//    if(!isCachable()) {
//        return;
//    }
//    fclose(_fp);
//}
//
//void Response::writeBody(const char *data, const int &len) {
//    _cachedSize += len;
//    fwrite(data, len, 1, _fp);
//}
