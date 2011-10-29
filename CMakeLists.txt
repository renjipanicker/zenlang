CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(zen)

IF(CMAKE_CONFIGURATION_TYPES)
    SET(CMAKE_CONFIGURATION_TYPES "Debug;Release;FooBar" CACHE STRING "" FORCE)
ENDIF(CMAKE_CONFIGURATION_TYPES)

set(parser_odir "${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${parser_odir})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} "lib/")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} "src/")

# add debug definitions
IF( CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS( "-DDEBUG" )
ENDIF()

SET(zen_SOURCES ${zen_SOURCES} src/main.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/lexer.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/parser.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/compiler.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/context.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/generator.cpp)
SET(zen_SOURCES ${zen_SOURCES} src/progen.cpp)

IF(WIN32)
    SET(lemon_cc "${CMAKE_CURRENT_SOURCE_DIR}/tools/lemon.exe")
    SET(re2c_cc "${CMAKE_CURRENT_SOURCE_DIR}/tools/re2c.exe")
ELSE(WIN32)
    SET(lemon_cc "${CMAKE_CURRENT_SOURCE_DIR}/tools/lemon")
    SET(re2c_cc "${CMAKE_CURRENT_SOURCE_DIR}/tools/re2c")
ENDIF(WIN32)

add_custom_target(cc_parser.y COMMAND "${lemon_cc}" -q "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.y")
SET(zen_DEPENDS ${zen_DEPENDS} cc_parser.y)

add_custom_target(cc_lexer.re COMMAND "${re2c_cc}" -c -i -u -o "${parser_odir}/lexerGen.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/lexerGen.re")
SET(zen_DEPENDS ${zen_DEPENDS} cc_lexer.re)

ADD_EXECUTABLE(zen ${zen_SOURCES})
ADD_DEPENDENCIES(zen ${zen_DEPENDS})

#-DCMAKE_BUILD_TYPE=Debug

#ADD_CUSTOM_COMMAND(
#    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.c"
#    COMMAND "${lemon_cc}" -q "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.y"
#    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.y"
#)

#SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.c" PROPERTIES LANGUAGE CXX)
#SET(zen_SOURCES ${zen_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.c")
