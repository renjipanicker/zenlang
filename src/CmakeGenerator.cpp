#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "CmakeGenerator.hpp"
#include "compiler.hpp"

class CmakeGenerator::Impl {
public:
    inline Impl(const Ast::Project& project) : _project(project), _fpPro(0) {}
public:
    void run();
private:
    inline void generateConfig(const Ast::Config& config);
    inline void generateProject(const Ast::Config& config);
private:
    const Ast::Project& _project;
    FILE* _fpPro;
};

inline void CmakeGenerator::Impl::generateProject(const Ast::Config& config) {
    OutputFile ofPro(_fpPro, "CMakeLists.txt");unused(ofPro);
    fprintf(_fpPro, "CMAKE_MINIMUM_REQUIRED(VERSION 2.6)\n");
    fprintf(_fpPro, "PROJECT(%s)\n", _project.name().c_str());
    fprintf(_fpPro, "\n");

    fprintf(_fpPro, "IF(CMAKE_COMPILER_IS_GNUCXX)\n");
    fprintf(_fpPro, "    ADD_DEFINITIONS( \"-Wall\" )\n");
    fprintf(_fpPro, "ENDIF(CMAKE_COMPILER_IS_GNUCXX)\n");
    fprintf(_fpPro, "\n");

    if(config.debug()) {
        fprintf(_fpPro, "ADD_DEFINITIONS( \"-DDEBUG\" )\n");
    }

    if(config.test()) {
        fprintf(_fpPro, "ADD_DEFINITIONS( \"-DUNIT_TEST\" )\n");
    }

    if(config.gui()) {
        fprintf(_fpPro, "ADD_DEFINITIONS( \"-DGUI\" )\n");

        fprintf(_fpPro, "IF(WIN32)\n");
        fprintf(_fpPro, "ELSE(WIN32)\n");
        fprintf(_fpPro, "    SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} \"%s/../tools/\")\n", config.zlibPath().c_str());
        fprintf(_fpPro, "    FIND_PACKAGE(GTK3)\n");
        fprintf(_fpPro, "    IF(GTK3_FOUND)\n");
        fprintf(_fpPro, "        INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})\n");
        fprintf(_fpPro, "    ENDIF(GTK3_FOUND)\n");
        fprintf(_fpPro, "ENDIF(WIN32)\n");
    }

    fprintf(_fpPro, "include_directories(${CMAKE_CURRENT_SOURCE_DIR} \".\")\n");
    fprintf(_fpPro, "include_directories(${CMAKE_CURRENT_SOURCE_DIR} \"%s\")\n", config.zlibPath().c_str());
    for(Ast::Config::PathList::const_iterator it = config.includePathList().begin(); it != config.includePathList().end(); ++it) {
        const std::string& dir = *it;
        fprintf(_fpPro, "include_directories(${CMAKE_CURRENT_SOURCE_DIR} \"%s\")\n", dir.c_str());
    }
    fprintf(_fpPro, "\n");

    fprintf(_fpPro, "SET(project_SOURCES ${project_SOURCES} %s/base/zenlang.cpp)\n", config.zlibPath().c_str());

    std::string zexePath = config.zexePath();
    String::replace(zexePath, "\\", "/");
    for(Ast::Config::PathList::const_iterator it = config.sourceFileList().begin(); it != config.sourceFileList().end(); ++it) {
        const std::string& filename = *it;
        std::string basename = getBaseName(filename);
        std::string ext = getExtention(filename);

        if((_project.hppExt().find(ext) != std::string::npos) || (_project.cppExt().find(ext) != std::string::npos)) {
            fprintf(_fpPro, "SET(project_SOURCES ${project_SOURCES} %s)\n", filename.c_str());
        } else if(_project.zppExt().find(ext) != std::string::npos) {
            std::string guiFlag = config.gui()?" --gui":"";
            std::string debugFlag = config.debug()?" --debug":"";
            std::string testFlag = config.test()?" ":" --test";
            fprintf(_fpPro, "ADD_CUSTOM_COMMAND(\n");
            fprintf(_fpPro, "    COMMAND \"%s\"%s%s%s -c \"%s\"\n", zexePath.c_str(), guiFlag.c_str(), debugFlag.c_str(), testFlag.c_str(), filename.c_str());
            fprintf(_fpPro, "    OUTPUT \"%s.cpp\"\n", basename.c_str());
            fprintf(_fpPro, "    DEPENDS \"%s\"\n", filename.c_str());
            fprintf(_fpPro, ")\n");
            fprintf(_fpPro, "SET(project_SOURCES ${project_SOURCES} %s.cpp)\n", basename.c_str());
        } else {
            throw z::Exception("Unknown file type for: %s", filename.c_str());
        }
    }
    fprintf(_fpPro, "\n");

    switch(config.mode()) {
        case Ast::Config::Mode::Executable:
            fprintf(_fpPro, "ADD_DEFINITIONS( \"-DZ_EXE\" )\n");
            if(config.gui()) {
                fprintf(_fpPro, "IF(WIN32)\n");
                fprintf(_fpPro, "    ADD_EXECUTABLE(%s WIN32 ${project_SOURCES})\n", _project.name().c_str());
                fprintf(_fpPro, "ELSE(WIN32)\n");
                fprintf(_fpPro, "    ADD_EXECUTABLE(%s ${project_SOURCES})\n", _project.name().c_str());
                fprintf(_fpPro, "ENDIF(WIN32)\n");
            } else {
                fprintf(_fpPro, "ADD_EXECUTABLE(%s ${project_SOURCES})\n", _project.name().c_str());
            }
            break;
        case Ast::Config::Mode::Shared:
            fprintf(_fpPro, "ADD_DEFINITIONS( \"-DZ_DLL\" )\n");
            fprintf(_fpPro, "ADD_LIBRARY(%s SHARED ${project_SOURCES})\n", _project.name().c_str());
            break;
        case Ast::Config::Mode::Static:
            fprintf(_fpPro, "ADD_DEFINITIONS( \"-DZ_LIB\" )\n");
            fprintf(_fpPro, "ADD_LIBRARY(%s STATIC ${project_SOURCES})\n", _project.name().c_str());
            break;
        case Ast::Config::Mode::Compile:
            throw z::Exception("Compile mode not allowed during project generaion");
    }

    if(config.gui()) {
        fprintf(_fpPro, "IF(WIN32)\n");
        fprintf(_fpPro, "    TARGET_LINK_LIBRARIES(%s comctl32)\n", _project.name().c_str());
        fprintf(_fpPro, "ELSE(WIN32)\n");
        fprintf(_fpPro, "IF(GTK3_FOUND)\n");
        fprintf(_fpPro, "    TARGET_LINK_LIBRARIES(%s ${GTK3_LIBRARIES})\n", _project.name().c_str());
        fprintf(_fpPro, "ENDIF(GTK3_FOUND)\n");
        fprintf(_fpPro, "ENDIF(WIN32)\n");
        fprintf(_fpPro, "\n");
    }
}
inline void CmakeGenerator::Impl::generateConfig(const Ast::Config& config) {
    Compiler compiler(_project, config);
    compiler.compile();
    if(config.mode() != Ast::Config::Mode::Compile) {
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
