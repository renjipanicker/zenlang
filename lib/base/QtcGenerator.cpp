#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/QtcGenerator.hpp"
#include "base/compiler.hpp"

class z::QtcGenerator::Impl : public z::Generator::Impl {
public:
    inline Impl(const z::Ast::Project& project) : Generator::Impl(project) {}
public:
    void run();
private:
    inline void generateConfig(const z::Ast::Config& config, ofile& os);
    inline void generateProject(const z::Ast::Config& config, z::ofile& os);
};

inline void z::QtcGenerator::Impl::generateProject(const z::Ast::Config& config, z::ofile& os) {
    if(config.abstract()) {
        return;
    }

    std::string s = z::s2e(config.name()).c_str();
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    assert((s == "debug") || (s == "release"));
    os() << "CONFIG(" << s << ", debug|release)" << " {" << std::endl;
    z::string ind = "    ";
    if(config.gui()) {
        os() << ind << "QT += gui webkit" << std::endl;
    } else {
        os() << ind << "QT -= core gui" << std::endl;
    }
    switch(config.buildMode()) {
        case z::Ast::Config::BuildMode::Executable:
            os() << ind << "TEMPLATE = app" << std::endl;
            os() << ind << "DEFINES += Z_EXE" << std::endl;
            break;
        case z::Ast::Config::BuildMode::Shared:
            os() << ind << "TEMPLATE = lib" << std::endl;
            os() << ind << "CONFIG += shared" << std::endl;
            break;
        case z::Ast::Config::BuildMode::Static:
            os() << ind << "TEMPLATE = lib" << std::endl;
            break;
    }
    if(config.gui()) {
        os() << ind << "DEFINES += GUI" << std::endl;
    } else {
        os() << ind << "CONFIG += console" << std::endl;
    }
    os() << ind << "CONFIG -= app_bundle" << std::endl;

    if(config.debug()) {
        os() << ind << "DEFINES += DEBUG" << std::endl;
    } else {
        os() << ind << "DEFINES += REL" << std::endl;
    }

    for(z::Ast::Config::PathList::const_iterator it = config.includePathList().begin(), ite = config.includePathList().end(); it != ite; ++it) {
        const z::string& dir = *it;
        os() << ind << "INCLUDEPATH += " << dir << std::endl;
    }
    os() << std::endl;
    os() << "}" << std::endl;
}

inline void z::QtcGenerator::Impl::generateConfig(const z::Ast::Config& config, z::ofile& os) {
    z::Compiler compiler(_project, config);
    compiler.compile();
    generateProject(config, os);
}

void z::QtcGenerator::Impl::run() {
    z::dir::mkpath(_gendir + "/");
    z::ofile os(_gendir + "/" + _project.name() + ".pro");

    os() << "TARGET = " << _project.name() << std::endl;

    os() << "ZENLANG_SRC_DIR=" << _project.zlibPath() << std::endl;
    os() << "ZEN_CC=$$ZENLANG_SRC_DIR/zenlang" << std::endl;
    os() << "RE2C_CC=$$ZENLANG_SRC_DIR/re2c.linux" << std::endl;
    os() << "LEMON_CC=$$ZENLANG_SRC_DIR/lemon.linux" << std::endl;
    os() << "INCLUDEPATH += $$ZENLANG_SRC_DIR" << std::endl;
    os() << "HEADERS += $$ZENLANG_SRC_DIR/zenlang.hpp" << std::endl;
    os() << "SOURCES += $$ZENLANG_SRC_DIR/zenlang.cpp" << std::endl;
    os() << std::endl;

    os() << "zenlang.name = zenlang" << std::endl;
    os() << "zenlang.input = ZENLANG_SOURCES" << std::endl;
    os() << "zenlang.commands = $$ZEN_CC -v -c ${QMAKE_FILE_IN}" << std::endl;
    os() << "zenlang.output = ${QMAKE_FILE_BASE}.cpp" << std::endl;
    os() << "zenlang.variable_out = SOURCES" << std::endl;
    os() << "zenlang.CONFIG += target_predeps" << std::endl;
    os() << "QMAKE_EXTRA_COMPILERS += zenlang" << std::endl;
    os() << std::endl;

    os() << "re2c.name = re2c" << std::endl;
    os() << "re2c.input = RE2C_SOURCES" << std::endl;
    os() << "re2c.commands = $$RE2C_CC -f -u -c -i -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}" << std::endl;
    os() << "re2c.output = ${QMAKE_FILE_BASE}.cpp" << std::endl;
    os() << "re2c.variable_out = SOURCES" << std::endl;
    os() << "re2c.CONFIG += target_predeps" << std::endl;
    os() << "QMAKE_EXTRA_COMPILERS += re2c" << std::endl;
    os() << std::endl;

    os() << "lemon.name = lemon" << std::endl;
    os() << "lemon.input = LEMON_SOURCES" << std::endl;
    os() << "lemon.commands = $$LEMON_CC -o.cpp -q ${QMAKE_FILE_IN}" << std::endl;
    os() << "lemon.output = ${QMAKE_FILE_BASE}.cpp" << std::endl;
    os() << "lemon.variable_out = SOURCES" << std::endl;
    os() << "lemon.CONFIG += target_predeps" << std::endl;
    os() << "QMAKE_EXTRA_COMPILERS += lemon" << std::endl;
    os() << std::endl;

    for(z::Ast::Project::ConfigList::const_iterator it = _project.configList().begin(); it != _project.configList().end(); ++it) {
        const z::Ast::Config& config = z::ref(it->second);
        generateConfig(config, os);
    }

    for(FileList::const_iterator it = _hppFileList.begin(), ite = _hppFileList.end(); it != ite; ++it) {
        const z::string& f = *it;
        os() << "HEADERS += " << f << std::endl;
    }

    for(FileList::const_iterator it = _cppFileList.begin(), ite = _cppFileList.end(); it != ite; ++it) {
        const z::string& f = *it;
        os() << "SOURCES += " << f << std::endl;
    }

    for(FileList::const_iterator it = _zppFileList.begin(), ite = _zppFileList.end(); it != ite; ++it) {
        const z::string& f = *it;
        os() << "ZENLANG_SOURCES += " << f << std::endl;
        os() << "OTHER_FILES += " << f << std::endl;
    }

    for(FileList::const_iterator it = _otherFileList.begin(), ite = _otherFileList.end(); it != ite; ++it) {
        const z::string& f = *it;
        if(z::dir::getExtention(f) == "re") {
            os() << "RE2C_SOURCES += " << f << std::endl;
            os() << "OTHER_FILES += " << f << std::endl;
        } else if(z::dir::getExtention(f) == "y") {
            os() << "LEMON_SOURCES += " << f << std::endl;
            os() << "OTHER_FILES += " << f << std::endl;
        } else {
            assert(false);
        }
    }
    if(_guiFileList.size() > 0) {
        os() << "RESOURCES += res.qrc" << std::endl;
        z::ofile gos(_gendir + "/res.qrc");
        gos() << "<RCC>" << std::endl;
        gos() << "    <qresource prefix=\"/res\">" << std::endl;
        for(FileList::const_iterator it = _guiFileList.begin(), ite = _guiFileList.end(); it != ite; ++it) {
            const z::string& f = *it;
            const z::string filename = z::dir::getFilename(f);
            gos() << "        <file alias=\"" << filename << "\">" << f << "</file>" << std::endl;
        }
        gos() << "    </qresource>" << std::endl;
        gos() << "</RCC>" << std::endl;
    }

    os() << std::endl;
    os() << "unix:LIBS += -lsqlite3" << std::endl;
}

z::QtcGenerator::QtcGenerator(const z::Ast::Project& project) : _impl(0) {_impl = new Impl(project);}
z::QtcGenerator::~QtcGenerator() {delete _impl;}
void z::QtcGenerator::run() {return z::ref(_impl).run();}
