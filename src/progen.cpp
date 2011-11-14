#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "progen.hpp"
#include "outfile.hpp"

class ProGen::Impl {
public:
    inline Impl(const Ast::Project& project) : _project(project), _fpPro(0) {}
public:
    void run();
private:
    const Ast::Project& _project;
    FILE* _fpPro;
};

void ProGen::Impl::run() {
    OutputFile ofPro(_fpPro, "CMakeLists.txt");unused(ofPro);
    fprintf(_fpPro, "CMAKE_MINIMUM_REQUIRED(VERSION 2.6)\n");
    fprintf(_fpPro, "PROJECT(%s)\n", _project.name().c_str());
    fprintf(_fpPro, "\n");

    std::string configNameList;
    for(Ast::Project::ConfigList::const_iterator it = _project.configList().begin(); it != _project.configList().end(); ++it) {
        const Ast::Config& config = ref(*it);
        configNameList += config.name();
        configNameList += ";";
    }

    fprintf(_fpPro, "IF(CMAKE_CONFIGURATION_TYPES)\n");
    fprintf(_fpPro, "    SET(CMAKE_CONFIGURATION_TYPES \"%s\" CACHE STRING \"\" FORCE)\n", configNameList.c_str());
    fprintf(_fpPro, "ENDIF(CMAKE_CONFIGURATION_TYPES)\n");
    fprintf(_fpPro, "\n");

    fprintf(_fpPro, "IF(CMAKE_COMPILER_IS_GNUCXX)\n");
    fprintf(_fpPro, "    ADD_DEFINITIONS( \"-Wall\" )\n");
    fprintf(_fpPro, "ENDIF(CMAKE_COMPILER_IS_GNUCXX)\n");
    fprintf(_fpPro, "\n");

    fprintf(_fpPro, "IF( CMAKE_BUILD_TYPE STREQUAL \"Debug\")\n");
    fprintf(_fpPro, "    ADD_DEFINITIONS( \"-DDEBUG\" )\n");
    fprintf(_fpPro, "ENDIF()\n");
    fprintf(_fpPro, "\n");

    if(_project.global().gui()) {
        fprintf(_fpPro, "ADD_DEFINITIONS( \"-DGUI\" )\n");

        fprintf(_fpPro, "IF(WIN32)\n");
        fprintf(_fpPro, "ELSE(WIN32)\n");
        fprintf(_fpPro, "    SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} \"%s/../tools/\")\n", _project.zlibPath().c_str());
        fprintf(_fpPro, "    FIND_PACKAGE(GTK3)\n");
        fprintf(_fpPro, "    IF(GTK3_FOUND)\n");
        fprintf(_fpPro, "        INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})\n");
        fprintf(_fpPro, "    ENDIF(GTK3_FOUND)\n");
        fprintf(_fpPro, "ENDIF(WIN32)\n");
    }

    fprintf(_fpPro, "include_directories(${CMAKE_CURRENT_SOURCE_DIR} \".\")\n");
    fprintf(_fpPro, "include_directories(${CMAKE_CURRENT_SOURCE_DIR} \"%s\")\n", _project.zlibPath().c_str());
    for(Ast::Config::PathList::const_iterator it = _project.global().includePathList().begin(); it != _project.global().includePathList().end(); ++it) {
        const std::string& dir = *it;
        fprintf(_fpPro, "include_directories(${CMAKE_CURRENT_SOURCE_DIR} \"%s\")\n", dir.c_str());
    }
    fprintf(_fpPro, "\n");

    fprintf(_fpPro, "SET(project_SOURCES ${project_SOURCES} %s/base/zenlang.cpp)\n", _project.zlibPath().c_str());

    std::string zexePath = _project.zexePath();
    String::replace(zexePath, "\\", "/");
    for(Ast::Config::PathList::const_iterator it = _project.global().sourceFileList().begin(); it != _project.global().sourceFileList().end(); ++it) {
        const std::string& filename = *it;
        std::string basename = getBaseName(filename);
        std::string ext = getExtention(filename);

        if((_project.hppExt().find(ext) != std::string::npos) || (_project.cppExt().find(ext) != std::string::npos)) {
            fprintf(_fpPro, "SET(project_SOURCES ${project_SOURCES} %s)\n", filename.c_str());
        } else if(_project.zppExt().find(ext) != std::string::npos) {
            fprintf(_fpPro, "ADD_CUSTOM_COMMAND(\n");
            fprintf(_fpPro, "    COMMAND \"%s\" -c \"%s\"\n", zexePath.c_str(), filename.c_str());
            fprintf(_fpPro, "    OUTPUT \"%s.cpp\"\n", basename.c_str());
            fprintf(_fpPro, "    DEPENDS \"%s\"\n", filename.c_str());
            fprintf(_fpPro, ")\n");
            fprintf(_fpPro, "SET(project_SOURCES ${project_SOURCES} %s.cpp)\n", basename.c_str());
        } else {
            throw Exception("Unknown file type for: %s", filename.c_str());
        }
    }
    fprintf(_fpPro, "\n");

    for(Ast::Project::ConfigList::const_iterator it = _project.configList().begin(); it != _project.configList().end(); ++it) {
        const Ast::Config& config = ref(*it);
        fprintf(_fpPro, "IF( CMAKE_BUILD_TYPE STREQUAL \"%s\")\n", config.name().c_str());
        /// \todo config values go here
        fprintf(_fpPro, "ENDIF( CMAKE_BUILD_TYPE STREQUAL \"%s\")\n", config.name().c_str());
    }
    fprintf(_fpPro, "\n");

    switch(_project.mode()) {
        case Ast::Project::Mode::Executable:
            fprintf(_fpPro, "ADD_DEFINITIONS( \"-DZ_EXE\" )\n");
            fprintf(_fpPro, "ADD_EXECUTABLE(%s ${project_SOURCES})\n", _project.name().c_str());
            break;
        case Ast::Project::Mode::Shared:
            fprintf(_fpPro, "ADD_DEFINITIONS( \"-DZ_DLL\" )\n");
            fprintf(_fpPro, "ADD_LIBRARY(%s SHARED ${project_SOURCES})\n", _project.name().c_str());
            break;
        case Ast::Project::Mode::Static:
            fprintf(_fpPro, "ADD_DEFINITIONS( \"-DZ_LIB\" )\n");
            fprintf(_fpPro, "ADD_LIBRARY(%s STATIC ${project_SOURCES})\n", _project.name().c_str());
            break;
        case Ast::Project::Mode::Compile:
            throw Exception("Compile mode not allowed during project generaion");
    }

    fprintf(_fpPro, "IF(GTK3_FOUND)\n");
    fprintf(_fpPro, "    TARGET_LINK_LIBRARIES(%s ${GTK3_LIBRARIES})\n", _project.name().c_str());
    fprintf(_fpPro, "ENDIF(GTK3_FOUND)\n");
    fprintf(_fpPro, "\n");
}

ProGen::ProGen(const Ast::Project& project) : _impl(0) {_impl = new Impl(project);}
ProGen::~ProGen() {delete _impl;}
void ProGen::run() {return ref(_impl).run();}
