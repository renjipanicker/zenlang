#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/QtcGenerator.hpp"
#include "base/compiler.hpp"

class z::QtcGenerator::Impl {
public:
    inline Impl(const z::Ast::Project& project) : _project(project) {}
public:
    void run();
private:
    inline void generateConfig(const z::Ast::Config& config);
    inline void generateProject(const z::Ast::Config& config, z::ofile& os);
private:
    const z::Ast::Project& _project;
};

inline void z::QtcGenerator::Impl::generateProject(const z::Ast::Config& config, z::ofile& os) {
    if(config.gui()) {
        os() << "QT += webkit" << std::endl;
    } else {
        os() << "QT -= core gui" << std::endl;
    }
    switch(config.buildMode()) {
        case z::Ast::Config::BuildMode::Executable:
            os() << "TEMPLATE = app" << std::endl;
            break;
        case z::Ast::Config::BuildMode::Shared:
            os() << "TEMPLATE = lib" << std::endl;
            os() << "CONFIG += shared" << std::endl;
            break;
        case z::Ast::Config::BuildMode::Static:
            os() << "TEMPLATE = lib" << std::endl;
            break;
    }
    os() << "TARGET = " << _project.name() << std::endl;
    if(config.gui()) {
//        os() << "CONFIG += link_pkgconfig" << std::endl;
//        os() << "PKGCONFIG = webkit-1.0" << std::endl;
        os() << "DEFINES += GUI" << std::endl;
        os() << "DEFINES += QT" << std::endl;
    } else {
        os() << "CONFIG += console" << std::endl;
    }
    os() << "CONFIG -= app_bundle" << std::endl;

    os() << "CONFIG(debug, debug|release) {" << std::endl;
    os() << "    DEFINES += DEBUG" << std::endl;
    os() << "}" << std::endl;

    os() << "DEFINES += Z_EXE" << std::endl;

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

    for(z::Ast::Config::PathList::const_iterator it = config.includePathList().begin(); it != config.includePathList().end(); ++it) {
        const z::string& dir = *it;
        os() << "INCLUDEPATH += " << dir << std::endl;
    }
    os() << std::endl;

    for(z::Ast::Config::PathList::const_iterator it = config.sourceFileList().begin(); it != config.sourceFileList().end(); ++it) {
        const z::string& filename = *it;
        z::string ext = z::dir::getExtention(filename);

        if(_project.hppExt().find(ext) != z::string::npos) {
            os() << "HEADERS += " << filename << std::endl;
        } else if(_project.cppExt().find(ext) != z::string::npos) {
            os() << "SOURCES += " << filename << std::endl;
        } else if(_project.zppExt().find(ext) != z::string::npos) {
            os() << "ZENLANG_SOURCES += " << filename << std::endl;
            os() << "OTHER_FILES += " << filename << std::endl;
        } else {
            throw z::Exception("QtcGenerator", zfmt(z::Ast::Token(filename, 0, 0, ""), "Unknown file type for: %{s}").arg("s", filename));
        }
    }
    os() << "unix:LIBS += -lsqlite3" << std::endl;
}

inline void z::QtcGenerator::Impl::generateConfig(const z::Ast::Config& config) {
    z::Compiler compiler(_project, config);
    compiler.compile();
    z::dir::mkpath(config.srcdir() + "/");
    z::ofile osPro(config.srcdir() + "/" + _project.name() + ".pro");
    generateProject(config, osPro);
}

void z::QtcGenerator::Impl::run() {
    for(z::Ast::Project::ConfigList::const_iterator it = _project.configList().begin(); it != _project.configList().end(); ++it) {
        const z::Ast::Config& config = z::ref(it->second);
        generateConfig(config);
    }
}

z::QtcGenerator::QtcGenerator(const z::Ast::Project& project) : _impl(0) {_impl = new Impl(project);}
z::QtcGenerator::~QtcGenerator() {delete _impl;}
void z::QtcGenerator::run() {return z::ref(_impl).run();}
