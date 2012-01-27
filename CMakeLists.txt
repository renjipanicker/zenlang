#-DCMAKE_BUILD_TYPE=Debug

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(zen)

IF(CMAKE_CONFIGURATION_TYPES)
    SET(CMAKE_CONFIGURATION_TYPES "Debug;Release;FooBar" CACHE STRING "" FORCE)
ENDIF(CMAKE_CONFIGURATION_TYPES)

SET(parser_odir "${CMAKE_CURRENT_SOURCE_DIR}/src")
INCLUDE_DIRECTORIES(${parser_odir})
INCLUDE_DIRECTORIES("lib/")
INCLUDE_DIRECTORIES("src/")

# add debug definitions
 IF(CMAKE_COMPILER_IS_GNUCXX)
     ADD_DEFINITIONS( "-Wall" )
 ENDIF(CMAKE_COMPILER_IS_GNUCXX)

# add debug definitions
IF( CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS( "-DDEBUG" )
ENDIF()

# strictly speaking, these are not required in the compiler project
# but including them to ease find-replace and source-navigation
SET(zen_SOURCES ${zen_SOURCES} lib/base/pch.hpp)
SET(zen_SOURCES ${zen_SOURCES} lib/base/zenlang.hpp)
SET(zen_SOURCES ${zen_SOURCES} lib/base/zenlang.cpp)

SET(zen_SOURCES ${zen_SOURCES} src/ast.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/ast.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/error.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/typename.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/typename.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/token.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/lexer.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/lexer.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/parser.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/parser.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/compiler.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/compiler.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/Unit.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/Unit.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/NodeFactory.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/NodeFactory.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/generator.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/generator.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/ZenlangGenerator.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/ZenlangGenerator.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/StlcppGenerator.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/StlcppGenerator.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/Interpreter.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/Interpreter.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/CmakeGenerator.hpp)
SET(zen_SOURCES ${zen_SOURCES} src/CmakeGenerator.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/main.cpp)

IF(WIN32)
    SET(lemon_cc "${CMAKE_CURRENT_SOURCE_DIR}/tools/lemon.exe")
    SET(re2c_cc "${CMAKE_CURRENT_SOURCE_DIR}/tools/re2c.exe")
ELSE(WIN32)
    SET(lemon_cc "${CMAKE_CURRENT_SOURCE_DIR}/tools/lemon")
    SET(re2c_cc "${CMAKE_CURRENT_SOURCE_DIR}/tools/re2c")
ENDIF(WIN32)

ADD_CUSTOM_COMMAND(
    # -l no line numbers
    # -q don't print report
    COMMAND "${lemon_cc}" o=".hpp" -q "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.y"
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.hpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.y"
)
SET(zen_SOURCES ${zen_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.hpp")

ADD_CUSTOM_COMMAND(
    # -c conditions-mode
    # -f use storable-state
    # -u unicode char-width
    # -i no line-numbers
    COMMAND "${re2c_cc}" -c -f -u -o "${parser_odir}/lexerGen.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/lexerGen.re"
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/lexerGen.hpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/lexerGen.re"
)
SET(zen_SOURCES ${zen_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/lexerGen.hpp")

ADD_EXECUTABLE(zen ${zen_SOURCES})

#####################################################################
# build zenlang library
#####################################################################
SET(ZEN_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../out)
SET(EXECUTABLE_OUTPUT_PATH ${ZEN_OUTPUT_PATH}/bin)
SET(LIBRARY_OUTPUT_PATH ${ZEN_OUTPUT_PATH}/lib)
INCLUDE_DIRECTORIES("${ZEN_OUTPUT_PATH}/include/")

#########################
# build all base/ files (actually just copy them to output dir)
SET(libbase_ZPP_SOURCES ${libbase_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/base/pch.hpp")
SET(libbase_ZPP_SOURCES ${libbase_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/base/zenlang.hpp")
SET(libbase_ZPP_SOURCES ${libbase_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/base/args.hpp")

FOREACH(_FILE ${libbase_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME)
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/lib/base/${_BASE}" "${ZEN_OUTPUT_PATH}/include/base/${_BASE}" [COPYONLY])
ENDFOREACH()

#########################
# build all libraries
GET_TARGET_PROPERTY(ZEN_EXE zen LOCATION)

IF( CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET( ZEN_DBG_FLAG "--debug" )
ENDIF()

#########################
# build libcore
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/core/String.zpp")
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/core/DateTime.zpp")
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/core/File.zpp")
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/core/Dir.zpp")
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/core/Url.zpp")

# Although these are files from the base/ and utils/ dir, we add it to libcore because there is no libbase binary created.
SET(libcore_SOURCES ${libcore_SOURCES} "lib/base/zenlang.cpp")
SET(libcore_SOURCES ${libcore_SOURCES} "lib/utils/sqlite3/sqlite3.c")
SET(libcore_SOURCES ${libcore_SOURCES} "lib/utils/sqlite3/sqlite3.h")
SET(libcore_SOURCES ${libcore_SOURCES} "lib/utils/sqlite3/sqlite3ext.h")

FOREACH(_FILE ${libcore_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME_WE)
    ADD_CUSTOM_COMMAND(
        COMMAND ${ZEN_EXE} ${ZEN_DBG_FLAG} -c --api "${ZEN_OUTPUT_PATH}/include/core" --src "./core" ${_FILE}
        OUTPUT "core/${_BASE}.cpp"
        DEPENDS ${ZEN_EXE} ${_FILE}
    )
    SET(libcore_SOURCES ${libcore_SOURCES} "core/${_BASE}.cpp")
    SET(libcore_SOURCES ${libcore_SOURCES} "lib/core/${_BASE}Impl.cpp")
ENDFOREACH()

ADD_LIBRARY(core STATIC ${libcore_SOURCES})

#########################
# build libgui
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/gui/Widget.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/gui/Window.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/gui/Mainframe.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/gui/Button.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/gui/TextEdit.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/gui/Systray.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/gui/Menu.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/gui/MenuItem.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/gui/WindowCreator.zpp")

FOREACH(_FILE ${libgui_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME_WE)
    ADD_CUSTOM_COMMAND(
        COMMAND ${ZEN_EXE} ${ZEN_DBG_FLAG} -c --api "${ZEN_OUTPUT_PATH}/include/gui" --src "./gui" ${_FILE}
        OUTPUT "gui/${_BASE}.cpp"
        DEPENDS ${ZEN_EXE} ${_FILE}
    )
    SET(libgui_SOURCES ${libgui_SOURCES} "gui/${_BASE}.cpp")
    SET(libgui_SOURCES ${libgui_SOURCES} "lib/gui/${_BASE}Impl.cpp")
ENDFOREACH()

ADD_LIBRARY(gui STATIC ${libgui_SOURCES})

#########################
# build all utils/ files (actually just copy them to output dir)
SET(libutil_ZPP_SOURCES ${libutil_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/utils/sqlite3/sqlite3ext.h")
SET(libutil_ZPP_SOURCES ${libutil_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/utils/sqlite3/sqlite3.h")

# NOTE: copying from utils/ to util/ (no 's' is dst path)
FOREACH(_FILE ${libutil_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME)
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/lib/utils/sqlite3/${_BASE}" "${ZEN_OUTPUT_PATH}/include/util/sqlite3/${_BASE}" [COPYONLY])
ENDFOREACH()
