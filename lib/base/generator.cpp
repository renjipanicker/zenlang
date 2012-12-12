#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/generator.hpp"

char z::Indent::ind[Size] = {32};
int z::Indent::_indent = -1;

void z::Generator::Impl::init() {
    for(z::Ast::Project::ConfigList::const_iterator it = _project.configList().begin(); it != _project.configList().end(); ++it) {
        const z::Ast::Config& config = z::ref(it->second);
        for(z::Ast::Config::PathList::const_iterator it = config.sourceFileList().begin(); it != config.sourceFileList().end(); ++it) {
            const z::string& p = *it;
            const z::string ext = z::dir::getExtention(p);
            if(ext == "zpp") {
                _zppFileList.insert(p);
            } else if ((ext == "h") || (ext == "hpp") || (ext == "inl")) {
                _hppFileList.insert(p);
            } else if ((ext == "c") || (ext == "cpp") || (ext == "rc")) {
                _cppFileList.insert(p);
            } else if ((ext == "y") || (ext == "re")) {
                _otherFileList.insert(p);
            } else {
                std::cout << "Unknown file type: " << p << std::endl;
            }
        }
        for(z::Ast::Config::PathList::const_iterator it = config.guiFileList().begin(), ite = config.guiFileList().end(); it != ite; ++it) {
            const z::string& p = *it;
            _guiFileList.insert(p);
        }

        if((_pch.length() > 0) && (_pch != config.pch()) && (config.pch() != "zenlang.hpp")) {
            throw z::Exception("MsvcGenerator", z::string("PCH header filename must be same in all configurations"));
        }

        if((_pchfile.length() > 0) && (_pchfile != config.pchfile()) && (config.pchfile() != "zenlang.cpp")) {
            throw z::Exception("MsvcGenerator", z::string("PCH source filename must be same in all configurations"));
        }

        if(config.pch() != "zenlang.hpp") {
            _pch = config.pch();
        }

        if(config.pchfile() != "zenlang.cpp") {
            _pchfile = config.pchfile();
        }
    }

    if(_pch.length() == 0) {
        _pch = "zenlang.hpp";
    }

    if(_pchfile.length() == 0) {
        _pchfile = "zenlang.cpp";
    }
}
