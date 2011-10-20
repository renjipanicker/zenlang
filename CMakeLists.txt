CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(zen)

SET(zen_SOURCES ${zen_SOURCES} src/main.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/lexer.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/parser.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/compiler.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/context.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/generator.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/project.cpp)

set(parser_odir "${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${parser_odir})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} "lib/")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} "src/")

# add debug definitions
IF( CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS( "-DDEBUG" )
ENDIF()

IF(WIN32)
    SET(lemon_cc "${CMAKE_CURRENT_SOURCE_DIR}/tools/lemon.exe")
    SET(re2c_cc "${CMAKE_CURRENT_SOURCE_DIR}/tools/re2c.exe")
ELSE(WIN32)
    SET(lemon_cc "lemon")
    SET(re2c_cc "re2c")
ENDIF(WIN32)

add_custom_target(lemon_parser
        COMMAND "${lemon_cc}" -q "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.y"
)

add_custom_target(re2c_scanner
        COMMAND "${re2c_cc}" -c -i -u -o "${parser_odir}/lexerGen.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/lexerGen.re"
)

ADD_EXECUTABLE(zen ${zen_SOURCES})
add_dependencies(zen lemon_parser re2c_scanner)
add_dependencies(lemon_parser lemon)

#-DCMAKE_BUILD_TYPE=Debug
