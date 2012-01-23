#pragma once

namespace z {
    struct url {
    public:
        inline void parse(const z::string& urlstr) {
            _fullUrl = urlstr;

            const std::string url_s = urlstr.val();
            std::string protocol;
            std::string host;
            std::string path;
            std::string query;

            const std::string prot_end("://");
            std::string::const_iterator prot_i = std::search(url_s.begin(), url_s.end(), prot_end.begin(), prot_end.end());
            protocol.reserve(std::distance(url_s.begin(), prot_i));
            std::transform(url_s.begin(), prot_i, std::back_inserter(protocol), std::ptr_fun<int,int>(tolower)); // protocol is icase
            if( prot_i == url_s.end() )
                return;

            std::advance(prot_i, prot_end.length());
            z::string::const_iterator path_i = std::find(prot_i, url_s.end(), '/');
            host.reserve(distance(prot_i, path_i));
            std::transform(prot_i, path_i, std::back_inserter(host), std::ptr_fun<int,int>(tolower)); // host is icase
            z::string::const_iterator query_i = std::find(path_i, url_s.end(), '?');
            path.assign(path_i, query_i);
            if( query_i != url_s.end() )
                ++query_i;

            query.assign(query_i, url_s.end());

            _protocol = protocol;
            _host = host;
            _path = path;
            _query = query;
        }

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
