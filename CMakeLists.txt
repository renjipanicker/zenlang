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
 IF(CMAKE_COMPILER_IS_GNUCXX)
     ADD_DEFINITIONS( "-Wall" )
 ENDIF(CMAKE_COMPILER_IS_GNUCXX)

# add debug definitions
IF( CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS( "-DDEBUG" )
ENDIF()

SET(zen_SOURCES ${zen_SOURCES} src/ast.hpp)
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
SET(zen_SOURCES ${zen_SOURCES} src/Context.hpp)
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
    COMMAND "${lemon_cc}" o=".hpp" -q "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.y"
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.hpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.y"
)
SET(zen_SOURCES ${zen_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.hpp")

ADD_CUSTOM_COMMAND(
    COMMAND "${re2c_cc}" -c -i -u -o "${parser_odir}/lexerGen.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/lexerGen.re"
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/lexerGen.hpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/lexerGen.re"
)
SET(zen_SOURCES ${zen_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/lexerGen.hpp")

#add_custom_target(cc_parser.y COMMAND "${lemon_cc}" -q "${CMAKE_CURRENT_SOURCE_DIR}/src/parserGen.y")
#SET(zen_DEPENDS ${zen_DEPENDS} cc_parser.y)

#add_custom_target(cc_lexer.re COMMAND "${re2c_cc}" -c -i -u -o "${parser_odir}/lexerGen.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/lexerGen.re")
#SET(zen_DEPENDS ${zen_DEPENDS} cc_lexer.re)

ADD_EXECUTABLE(zen ${zen_SOURCES})
#ADD_DEPENDENCIES(zen ${zen_DEPENDS})


#-DCMAKE_BUILD_TYPE=Debug
