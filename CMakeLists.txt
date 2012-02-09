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

SET(ZEN_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../out)
SET(EXECUTABLE_OUTPUT_PATH ${ZEN_OUTPUT_PATH}/bin)
SET(LIBRARY_OUTPUT_PATH ${ZEN_OUTPUT_PATH}/lib)
INCLUDE_DIRECTORIES("${ZEN_OUTPUT_PATH}/include/")

ADD_SUBDIRECTORY(lib)

#-DCMAKE_BUILD_TYPE=Debug
