CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(zen)

SET(zen_SOURCES ${zen_SOURCES} src/main.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/lexer.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/parser.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/compiler.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/context.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/generator.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/project.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/common.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/exception.cpp)

set(parser_odir "${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${parser_odir})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} "src/")

add_custom_target(lemon_parser
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/lemon.exe" -q "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.y"
)

add_custom_target(re2c_scanner
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/re2c.exe" -c -i -u -o "${parser_odir}/lexerGen.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/lexerGen.re"
)

ADD_EXECUTABLE(zen ${zen_SOURCES})
add_dependencies(zen lemon_parser re2c_scanner)
add_dependencies(lemon_parser lemon)

#-DCMAKE_BUILD_TYPE=Debug
