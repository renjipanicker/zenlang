CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(main)

IF(CMAKE_COMPILER_IS_GNUCXX)
    ADD_DEFINITIONS( "-Wall" )
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

ADD_DEFINITIONS( "-DDEBUG" )
ADD_DEFINITIONS( "-DUNIT_TEST" )
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ".")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} "../../zenlang/lib")

SET(project_SOURCES ${project_SOURCES} ../../zenlang/lib/base/zenlang.cpp)
