#include "zenlang.hpp"

bool Url::Exists(const url& u) {
    struct stat b;
    return (0 == stat(z::s2e(u.path).c_str(), &b));
}

bool Url::FileExists(const z::string& path) {
    Url::url u = Create(path);
    return Exists(u);
}

void Url::Parse(Url::url& u, const z::string& urlstr) {
    const z::string::sstringT url_s = urlstr.val();
    z::string::sstringT protocol;
    z::string::sstringT host;
    z::string::sstringT port;
    z::string::sstringT path;
    z::string::sstringT query;

    const z::string zprot_end("://");
    const z::string::sstringT prot_end = zprot_end.val();
    z::string::sstringT::const_iterator prot_i = std::search(url_s.begin(), url_s.end(), prot_end.begin(), prot_end.end());
    if( prot_i != url_s.end() ) {
        // protocol detected, parse as url
        protocol.reserve(std::distance(url_s.begin(), prot_i));
        std::transform(url_s.begin(), prot_i, std::back_inserter(protocol), std::ptr_fun<int,int>(tolower)); // protocol is icase
        std::advance(prot_i, prot_end.length());

        // search for path
        z::string::const_iterator path_i = std::find(prot_i, url_s.end(), '/');
        host.reserve(distance(prot_i, path_i));
        std::transform(prot_i, path_i, std::back_inserter(host), std::ptr_fun<int,int>(tolower)); // host is icase

        // search for query
        z::string::const_iterator query_i = std::find(path_i, url_s.end(), '?');
        path.assign(path_i, query_i);
        if( query_i != url_s.end() )
            ++query_i;

        query.assign(query_i, url_s.end());
    } else {
        // protocol not detected, assume regular file path
        protocol = z::string("file").val();
        host = z::string("").val();
        path = url_s;
        query = z::string("").val();;
    }

    if(port.length() == 0) {
        if(z::string(protocol) == "http") {
            z::string t("80");
            port.assign(t.c_str());
        }
    }
    u._fullUrl<url>(urlstr);
    u._protocol<url>(protocol);
    u._host<url>(host);
    u._path<url>(path);
    u._querystring<url>(query);
}

Url::url Url::Create(const z::string& urlstr) {
    Url::url u;
    Parse(u, urlstr);
    return u;
}
