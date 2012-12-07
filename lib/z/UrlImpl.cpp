#include "zenlang.hpp"

bool z::Url::Exists(const url& u) {
    struct stat b;
    return (0 == stat(z::s2e(u.path).c_str(), &b));
}

bool z::Url::FileExists(const z::string& path) {
    z::url u = Create(path);
    return Exists(u);
}

void z::Url::ParseQueryString(z::QueryList& ql, const z::string& queryStr) {
}

void z::Url::Parse(z::url& u, const z::string& urlstr) {
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
    u._port<url>(port);
    u._path<url>(path);
    ParseQueryString(u.queryList, query);
//    u._querystring<url>(query);
}

z::url z::Url::Create(const z::string& urlstr) {
    z::url u;
    Parse(u, urlstr);
    return u;
}

bool z::Url::OpenUrlString(const z::string& u) {
#if defined(WIN32)
    HINSTANCE rv = ::ShellExecute(NULL, "open", z::s2e(u).c_str(), NULL, NULL, SW_SHOWNORMAL);
    unused(rv);
#elif defined(__APPLE__)
    z::string nu = z::string("open %{u}").arg("u", u);
    system(z::s2e(nu).c_str());
#else
    z::string nu = z::string("xdg-open %{u}").arg("u", u);
    int r = system(z::s2e(nu).c_str());
    unused(r);
#endif
    return true;
}

bool z::Url::Open(const url& u) {
    z::string s = GetFullUrl(u);
    return OpenUrlString(s);
}

z::string z::Url::Encode(const z::string& u) {
    z::string qs;
    for(z::string::const_iterator it = u.begin(), ite = u.end(); it != ite; ++it) {
        const z::string::scharT& ch = *it;
        switch(ch) {
            // reserved characters
            case '!' : qs += "%21"; break;
            case '#' : qs += "%23"; break;
            case '$' : qs += "%24"; break;
            case '&' : qs += "%26"; break;
            case '\'': qs += "%27"; break;
            case '(' : qs += "%28"; break;
            case ')' : qs += "%29"; break;
            case '*' : qs += "%2A"; break;
            case '+' : qs += "%2B"; break;
            case ',' : qs += "%2C"; break;
            case '/' : qs += "%2F"; break;
            case ':' : qs += "%3A"; break;
            case ';' : qs += "%3B"; break;
            case '=' : qs += "%3D"; break;
            case '?' : qs += "%3F"; break;
            case '@' : qs += "%40"; break;
            case '[' : qs += "%5B"; break;
            case ']' : qs += "%5D"; break;

            // common characters
            case '\r': qs += "%0D"; break;
            case '\n': qs += "%0A"; break;
            case ' ' : qs += "%20"; break;
            case '"' : qs += "%22"; break;
            case '%' : qs += "%25"; break;
            case '-' : qs += "%2D"; break;
            case '.' : qs += "%2E"; break;
            case '<' : qs += "%3C"; break;
            case '>' : qs += "%3E"; break;
            case '\\': qs += "%5C"; break;
            case '^' : qs += "%5E"; break;
            case '_' : qs += "%5F"; break;
            case '`' : qs += "%60"; break;
            case '{' : qs += "%7B"; break;
            case '|' : qs += "%7C"; break;
            case '}' : qs += "%7D"; break;
            case '~' : qs += "%7E"; break;

            // default
            default  : qs += ch;    break;
        }
    }
    return qs;
}

z::string z::Url::Decode(const z::string& u) {
    z::string qs;
    for(z::string::const_iterator it = u.begin(), ite = u.end(); it != ite; ++it) {
        const z::string::scharT& ch = *it;
        if(ch == '+') {
            qs += ' ';
            continue;
        }

        if(ch != '%') {
            qs += ch;
            continue;
        }

        // check first char after %
        ++it;
        if(it == ite) {
            qs += ch;
            continue;
        }
        int v = 0;

        const z::string::scharT& ch1 = *it;
        if((ch1 >= '0') && (ch1 <= '9')) {
            v = (v * 16) + (ch1 - '0');
        } else if((ch1 >= 'A') && (ch1 <= 'F')) {
            v = (v * 16) + (ch1 - 'A' + 10);
        } else if((ch1 >= 'a') && (ch1 <= 'f')) {
            v = (v * 16) + (ch1 - 'a' + 10);
        } else {
            qs += ch;
            qs += ch1;
            continue;
        }

        // check next character after %
        ++it;
        if(it == ite) {
            qs += ch;
            qs += ch1;
            continue;
        }
        const z::string::scharT& ch2 = *it;
        if((ch2 >= '0') && (ch2 <= '9')) {
            v = (v * 16) + (ch2 - '0');
        } else if((ch2 >= 'A') && (ch2 <= 'F')) {
            v = (v * 16) + (ch2 - 'A' + 10);
        } else if((ch2 >= 'a') && (ch2 <= 'f')) {
            v = (v * 16) + (ch2 - 'a' + 10);
        } else {
            qs += ch;
            qs += ch1;
            qs += ch2;
            continue;
        }

        // v contains a valid decoded character
        qs += v;
    }
    return qs;
}
