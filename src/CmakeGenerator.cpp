#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "CmakeGenerator.hpp"
#include "compiler.hpp"

class CmakeGenerator::Impl {
public:
    inline Impl(const Ast::Project& project) : _project(project) {}
public:
    void run();
private:
    inline void generateConfig(const Ast::Config& config);
    inline void generateProject(const Ast::Config& config);
private:
    const Ast::Project& _project;
//@    FILE* _fpPro;
};

inline void CmakeGenerator::Impl::generateProject(const Ast::Config& config) {
//@    OutputFile ofPro(_fpPro, config.srcdir(), "CMakeLists.txt");unused(ofPro);
    z::file ofPro(config.srcdir(), "CMakeLists.txt", "w", z::file::makePath);
    fprintf(ofPro.val(), "CMAKE_MINIMUM_REQUIRED(VERSION 2.6)\n");
    fprintf(ofPro.val(), "PROJECT(%s)\n", _project.name().c_str());
    fprintf(ofPro.val(), "SET(ZEN_ROOT \"%s\")\n", config.zlibPath().c_str());
    if(config.gui()) {
        fprintf(ofPro.val(), "SET(ZEN_GUI 1)\n");
    }
    fprintf(ofPro.val(), "INCLUDE(${ZEN_ROOT}/tools/SetupZL.cmake)\n");
    fprintf(ofPro.val(), "\n");

    fprintf(ofPro.val(), "INCLUDE_DIRECTORIES(\"${ZEN_ROOT}/include/\")\n");
    fprintf(ofPro.val(), "LINK_DIRECTORIES(\"${ZEN_ROOT}/lib/\")\n");

    fprintf(ofPro.val(), "IF(CMAKE_COMPILER_IS_GNUCXX)\n");
    fprintf(ofPro.val(), "    ADD_DEFINITIONS( \"-Wall\" )\n");
    fprintf(ofPro.val(), "ENDIF(CMAKE_COMPILER_IS_GNUCXX)\n");
    fprintf(ofPro.val(), "\n");

    if(config.debug()) {
        fprintf(ofPro.val(), "ADD_DEFINITIONS( \"-DDEBUG\" )\n");
    }

    if(config.test()) {
        fprintf(ofPro.val(), "ADD_DEFINITIONS( \"-DUNIT_TEST\" )\n");
    }

    fprintf(ofPro.val(), "INCLUDE_DIRECTORIES(\"${CMAKE_CURRENT_SOURCE_DIR}\")\n");
    fprintf(ofPro.val(), "INCLUDE_DIRECTORIES(\"%s\")\n", config.apidir().c_str());
    fprintf(ofPro.val(), "INCLUDE_DIRECTORIES(\"%s\")\n", config.srcdir().c_str());
    for(Ast::Config::PathList::const_iterator it = config.includePathList().begin(); it != config.includePathList().end(); ++it) {
        const z::string& dir = *it;
        fprintf(ofPro.val(), "INCLUDE_DIRECTORIES(\"%s\")\n", dir.c_str());
    }
    fprintf(ofPro.val(), "\n");

    fprintf(ofPro.val(), "SET(project_SOURCES ${project_SOURCES} ${ZEN_ROOT}/include/base/zenlang.cpp)\n");

    z::string zexePath = config.zexePath();
    zexePath.replace("\\", "/");
    for(Ast::Config::PathList::const_iterator it = config.sourceFileList().begin(); it != config.sourceFileList().end(); ++it) {
        const z::string& filename = *it;
        z::string basename = getBaseName(filename);
        z::string ext = getExtention(filename);

        if((_project.hppExt().find(ext) != z::string::npos) || (_project.cppExt().find(ext) != z::string::npos)) {
            fprintf(ofPro.val(), "SET(project_SOURCES ${project_SOURCES} %s)\n", filename.c_str());
        } else if(_project.zppExt().find(ext) != z::string::npos) {
            z::string debugFlag = config.debug()?" --debug":"";
            z::string testFlag = config.test()?" ":" --test";
            fprintf(ofPro.val(), "ADD_CUSTOM_COMMAND(\n");
            fprintf(ofPro.val(), "    COMMAND \"%s\"%s%s -c \"%s\"\n", zexePath.c_str(), debugFlag.c_str(), testFlag.c_str(), filename.c_str());
            fprintf(ofPro.val(), "    OUTPUT \"%s.cpp\"\n", basename.c_str());
            fprintf(ofPro.val(), "    DEPENDS \"%s\"\n", filename.c_str());
            fprintf(ofPro.val(), ")\n");
            fprintf(ofPro.val(), "SET(project_SOURCES ${project_SOURCES} %s.cpp)\n", basename.c_str());
        } else {
            throw z::Exception("CmakeGenerator", z::fmt("Unknown file type for: %{s}").add("s", filename));
        }
    }
    fprintf(ofPro.val(), "\n");

    switch(config.buildMode()) {
        case Ast::Config::BuildMode::Executable:
            fprintf(ofPro.val(), "ADD_DEFINITIONS( \"-DZ_EXE\" )\n");
            if(config.gui()) {
                fprintf(ofPro.val(), "IF(WIN32)\n");
                fprintf(ofPro.val(), "    ADD_EXECUTABLE(%s WIN32 ${project_SOURCES})\n", _project.name().c_str());
                fprintf(ofPro.val(), "ELSE(WIN32)\n");
                fprintf(ofPro.val(), "    ADD_EXECUTABLE(%s ${project_SOURCES})\n", _project.name().c_str());
                fprintf(ofPro.val(), "ENDIF(WIN32)\n");
            } else {
                fprintf(ofPro.val(), "ADD_EXECUTABLE(%s ${project_SOURCES})\n", _project.name().c_str());
            }
            break;
        case Ast::Config::BuildMode::Shared:
            fprintf(ofPro.val(), "ADD_DEFINITIONS( \"-DZ_DLL\" )\n");
            fprintf(ofPro.val(), "ADD_LIBRARY(%s SHARED ${project_SOURCES})\n", _project.name().c_str());
            break;
        case Ast::Config::BuildMode::Static:
            fprintf(ofPro.val(), "ADD_DEFINITIONS( \"-DZ_LIB\" )\n");
            fprintf(ofPro.val(), "ADD_LIBRARY(%s STATIC ${project_SOURCES})\n", _project.name().c_str());
            break;
        case Ast::Config::BuildMode::Compile:
            throw z::Exception("CmakeGenerator", z::fmt("Compile mode not allowed during project generaion"));
    }

    for(Ast::Config::PathList::const_iterator it = config.linkFileList().begin(); it != config.linkFileList().end(); ++it) {
        const z::string& filename = *it;
        fprintf(ofPro.val(), "TARGET_LINK_LIBRARIES(%s %s)\n", _project.name().c_str(), filename.c_str());
    }

    if(config.gui()) {
        fprintf(ofPro.val(), "TARGET_LINK_LIBRARIES(%s ${ZENLANG_LIBRARIES})\n", _project.name().c_str());
        fprintf(ofPro.val(), "\n");
    }
}
inline void CmakeGenerator::Impl::generateConfig(const Ast::Config& config) {
    Compiler compiler(_project, config);
    compiler.compile();
    if(config.buildMode() != Ast::Config::BuildMode::Compile) {
        generateProject(config);
    }
}

void CmakeGenerator::Impl::run() {
    for(Ast::Project::ConfigList::const_iterator it = _project.configList().begin(); it != _project.configList().end(); ++it) {
        const Ast::Config& config = z::ref(it->second);
        generateConfig(config);
    }
}

CmakeGenerator::CmakeGenerator(const Ast::Project& project) : _impl(0) {_impl = new Impl(project);}
CmakeGenerator::~CmakeGenerator() {delete _impl;}
void CmakeGenerator::run() {return z::ref(_impl).run();}
