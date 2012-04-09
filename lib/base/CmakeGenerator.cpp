#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/CmakeGenerator.hpp"
#include "base/compiler.hpp"

class z::CmakeGenerator::Impl {
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

inline void z::CmakeGenerator::Impl::generateProject(const z::Ast::Config& config, z::ofile& os) {
    os() << "CMAKE_MINIMUM_REQUIRED(VERSION 2.6)" << std::endl;
    os() << "PROJECT(" << _project.name() << ")\n";
    os() << "SET(ZEN_ROOT \"" << config.zlibPath() << "\")" << std::endl;
    if(config.gui()) {
        os() << "SET(ZEN_GUI 1)" << std::endl;
    }
    os() << "INCLUDE(${ZEN_ROOT}/tools/SetupZL.cmake)" << std::endl;
    os() << std::endl;

    os() << "INCLUDE_DIRECTORIES(\"${ZEN_ROOT}/include/\")" << std::endl;
    os() << "LINK_DIRECTORIES(\"${ZEN_ROOT}/lib/\")" << std::endl;

    os() << "IF(CMAKE_COMPILER_IS_GNUCXX)" << std::endl;
    os() << "    ADD_DEFINITIONS( \"-Wall\" )" << std::endl;
    os() << "ENDIF(CMAKE_COMPILER_IS_GNUCXX)" << std::endl;
    os() << std::endl;

    if(config.debug()) {
        os() << "ADD_DEFINITIONS( \"-DDEBUG\" )" << std::endl;
    }

    if(config.test()) {
        os() << "ADD_DEFINITIONS( \"-DUNIT_TEST\" )" << std::endl;
    }

    os() << "INCLUDE_DIRECTORIES(\"${CMAKE_CURRENT_SOURCE_DIR}\")" << std::endl;
    os() << "INCLUDE_DIRECTORIES(\"" << config.apidir() << "\")" << std::endl;
    os() << "INCLUDE_DIRECTORIES(\"" << config.srcdir() << "\")" << std::endl;
    for(z::Ast::Config::PathList::const_iterator it = config.includePathList().begin(); it != config.includePathList().end(); ++it) {
        const z::string& dir = *it;
        os() << "INCLUDE_DIRECTORIES(\"" << dir << "\")" << std::endl;
    }
    os() << std::endl;

    os() << "SET(project_SOURCES ${project_SOURCES} ${ZEN_ROOT}/include/base/zenlang.cpp)" << std::endl;

    z::string zexePath = config.zexePath();
    zexePath.replace("\\", "/");
    for(z::Ast::Config::PathList::const_iterator it = config.sourceFileList().begin(); it != config.sourceFileList().end(); ++it) {
        const z::string& filename = *it;
        z::string basename = getBaseName(filename);
        z::string ext = getExtention(filename);

        if((_project.hppExt().find(ext) != z::string::npos) || (_project.cppExt().find(ext) != z::string::npos)) {
            os() << "SET(project_SOURCES ${project_SOURCES} " << filename << ")" << std::endl;
        } else if(_project.zppExt().find(ext) != z::string::npos) {
            z::string debugFlag = config.debug()?" --debug":"";
            z::string testFlag = config.test()?" ":" --test";
            os() << "ADD_CUSTOM_COMMAND(" << std::endl;
            os() << "    COMMAND \"" << zexePath << "\"" << debugFlag << testFlag << " -c \"" << filename << "\"" << std::endl;
            os() << "    OUTPUT \"" << basename << ".cpp\"" << std::endl;
            os() << "    DEPENDS \"" << filename << "\"" << std::endl;
            os() << ")" << std::endl;
            os() << "SET(project_SOURCES ${project_SOURCES} " << basename << ".cpp)" << std::endl;
        } else {
            throw z::Exception("CmakeGenerator", zfmt(z::Ast::Token(filename, 0, 0, ""), "Unknown file type for: %{s}").arg("s", filename));
        }
    }
    os() << std::endl;

    switch(config.buildMode()) {
        case z::Ast::Config::BuildMode::Executable:
            os() << "ADD_DEFINITIONS( \"-DZ_EXE\" )" << std::endl;
            if(config.gui()) {
                os() << "IF(WIN32)" << std::endl;
                os() << "    ADD_EXECUTABLE(" << _project.name() << " WIN32 ${project_SOURCES})" << std::endl;
                os() << "ELSE(WIN32)" << std::endl;
                os() << "    ADD_EXECUTABLE(" << _project.name() << " ${project_SOURCES})" << std::endl;
                os() << "ENDIF(WIN32)" << std::endl;
            } else {
                os() << "ADD_EXECUTABLE(" << _project.name() << " ${project_SOURCES})" << std::endl;
            }
            break;
        case z::Ast::Config::BuildMode::Shared:
            os() << "ADD_DEFINITIONS( \"-DZ_DLL\" )" << std::endl;
            os() << "ADD_LIBRARY(" << _project.name() << " SHARED ${project_SOURCES})" << std::endl;
            break;
        case z::Ast::Config::BuildMode::Static:
            os() << "ADD_DEFINITIONS( \"-DZ_LIB\" )" << std::endl;
            os() << "ADD_LIBRARY(" << _project.name() << " STATIC ${project_SOURCES})" << std::endl;
            break;
        case z::Ast::Config::BuildMode::Compile:
            throw z::Exception("CmakeGenerator", zfmt(z::Ast::Token("", 0, 0, ""), z::string("Compile mode not allowed during project generation")));
    }

    for(z::Ast::Config::PathList::const_iterator it = config.linkFileList().begin(); it != config.linkFileList().end(); ++it) {
        const z::string& filename = *it;
        os() << "TARGET_LINK_LIBRARIES(" << _project.name() << " " << filename << ")" << std::endl;
    }

    if(config.gui()) {
        os() << "TARGET_LINK_LIBRARIES(" << _project.name() << " ${ZENLANG_LIBRARIES})" << std::endl;
        os() << std::endl;
    }
}

inline void z::CmakeGenerator::Impl::generateConfig(const z::Ast::Config& config) {
    z::Compiler compiler(_project, config);
    compiler.compile();
    if(config.buildMode() != z::Ast::Config::BuildMode::Compile) {
        z::file::mkpath(config.srcdir() + "/");
        z::ofile osPro(config.srcdir() + "/" + "CMakeLists.txt");
        generateProject(config, osPro);
    }
}

void z::CmakeGenerator::Impl::run() {
    for(z::Ast::Project::ConfigList::const_iterator it = _project.configList().begin(); it != _project.configList().end(); ++it) {
        const z::Ast::Config& config = z::ref(it->second);
        generateConfig(config);
    }
}

z::CmakeGenerator::CmakeGenerator(const z::Ast::Project& project) : _impl(0) {_impl = new Impl(project);}
z::CmakeGenerator::~CmakeGenerator() {delete _impl;}
void z::CmakeGenerator::run() {return z::ref(_impl).run();}
