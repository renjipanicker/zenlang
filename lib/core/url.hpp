#pragma once

namespace z {
    struct url {
    public:
        void parse(const z::string& urlstr);

        inline void dump() {
            std::cout << "url::dump(): fullPath=" << _fullUrl << ", protocol=" << _protocol << ", host=" << _host << ", path=" << _path << ", query=" << _query << std::endl;
        }

        inline url() {}
        inline url(const z::string& urlstr) {
            parse(urlstr);
        }

        inline const z::string& fullUrl() const {return _fullUrl;}
        inline const z::string& protocol() const {return _protocol;}
        inline const z::string& host() const {return _host;}
        inline const z::string& path() const {return _path;}
        inline const z::string& query() const {return _query;}

    private:
        z::string _fullUrl;
        z::string _protocol;
        z::string _host;
        z::string _path;
        z::string _query;
    };

}
