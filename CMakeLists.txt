CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(zen)
SET(ZEN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE(tools/SetupZL.cmake)

SET(parser_odir "${CMAKE_CURRENT_SOURCE_DIR}/src")
INCLUDE_DIRECTORIES(${parser_odir})
INCLUDE_DIRECTORIES("lib/")
INCLUDE_DIRECTORIES("src/")

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
SET(libbase_ZPP_SOURCES ${libbase_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/base/zenlang.cpp")
SET(libbase_ZPP_SOURCES ${libbase_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/base/args.hpp")

FOREACH(_FILE ${libbase_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME)
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/lib/base/${_BASE}" "${ZEN_OUTPUT_PATH}/include/base/${_BASE}" COPYONLY @ONLY)
ENDFOREACH()

#########################
# build all libraries
GET_TARGET_PROPERTY(ZEN_EXE zen LOCATION)

IF( CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET( ZEN_DBG_FLAG "--debug" )
ENDIF()

#########################
# build libcore
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/lib/core/core.ipp" "${ZEN_OUTPUT_PATH}/include/core/core.ipp" COPYONLY @ONLY)

SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/core/String.zpp")
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/core/DateTime.zpp")
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/core/File.zpp")
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/core/Dir.zpp")
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/core/Url.zpp")

# Although these are files from the base/ and utils/ dir, we add it to libcore because there is no libbase binary created.
SET(libcore_SOURCES ${libcore_SOURCES} "lib/utils/sqlite3/sqlite3.c")
SET(libcore_SOURCES ${libcore_SOURCES} "lib/utils/sqlite3/sqlite3.h")
SET(libcore_SOURCES ${libcore_SOURCES} "lib/utils/sqlite3/sqlite3ext.h")
SET(libcore_SOURCES ${libcore_SOURCES} "lib/utils/sqlite3/vfs_compress.c")

FOREACH(_FILE ${libcore_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME_WE)
    ADD_CUSTOM_COMMAND(
        COMMAND ${ZEN_EXE} ${ZEN_DBG_FLAG} -c --verbose --api "${ZEN_OUTPUT_PATH}/include/core" --src "./core" ${_FILE}
        OUTPUT "core/${_BASE}.cpp"
        DEPENDS ${ZEN_EXE} ${_FILE}
    )
    SET(libcore_SOURCES ${libcore_SOURCES} "core/${_BASE}.cpp")
    SET(libcore_SOURCES ${libcore_SOURCES} "lib/core/${_BASE}Impl.cpp")
ENDFOREACH()

ADD_LIBRARY(core STATIC ${libcore_SOURCES})

#########################
# build all utils/ files (actually just copy them to output dir)
SET(libutil_ZPP_SOURCES ${libutil_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/utils/sqlite3/sqlite3ext.h")
SET(libutil_ZPP_SOURCES ${libutil_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/lib/utils/sqlite3/sqlite3.h")

# NOTE: copying from utils/ to util/ (no 's' is dst path)
FOREACH(_FILE ${libutil_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME)
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/lib/utils/sqlite3/${_BASE}" "${ZEN_OUTPUT_PATH}/include/util/sqlite3/${_BASE}" COPYONLY @ONLY)
ENDFOREACH()

#########################
# build all tools/ files (actually just copy them to output dir)
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/tools/FindGTK3.cmake")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/tools/SetupZL.cmake")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/tools/lemon")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/tools/lemon.exe")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/tools/lempar.c")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/tools/re2c")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/tools/re2c.exe")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/tools/zpp.xml")

FOREACH(_FILE ${libtool_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME)
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/tools/${_BASE}" "${ZEN_OUTPUT_PATH}/tools/${_BASE}" COPYONLY @ONLY)
ENDFOREACH()

#########################
# build libgui. Its in a separate project because it requires the GUI includes and libraries.
ADD_SUBDIRECTORY(lib/gui)

#-DCMAKE_BUILD_TYPE=Debug
