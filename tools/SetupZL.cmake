IF(CMAKE_CONFIGURATION_TYPES)
    SET(CMAKE_CONFIGURATION_TYPES "Debug;Release;" CACHE STRING "" FORCE)
ENDIF(CMAKE_CONFIGURATION_TYPES)

SET(ZEN_GUI_LIBRARIES)

# add debug definitions
 IF(CMAKE_COMPILER_IS_GNUCXX)
     ADD_DEFINITIONS( "-Wall" )
 ENDIF(CMAKE_COMPILER_IS_GNUCXX)

# add debug definitions
IF( CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS( "-DDEBUG" )
ENDIF()

IF(WIN32)
    SET(lemon_cc "${ZEN_ROOT}/tools/lemon.exe")
    SET(re2c_cc "${ZEN_ROOT}/tools/re2c.exe")
ELSE(WIN32)
    SET(lemon_cc "${ZEN_ROOT}/tools/lemon")
    SET(re2c_cc "${ZEN_ROOT}/tools/re2c")
ENDIF(WIN32)

IF(ZEN_GUI)
    ADD_DEFINITIONS( "-DGUI" )
    IF(WIN32)
        SET(ZEN_GUI_LIBRARIES ${ZEN_GUI_LIBRARIES} comctl32)
    ELSE(WIN32)
        SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${ZEN_ROOT}/tools")
        FIND_PACKAGE(GTK3 REQUIRED)
        INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
        SET(ZEN_GUI_LIBRARIES ${ZEN_GUI_LIBRARIES} ${GTK3_LIBRARIES})
    ENDIF(WIN32)
ENDIF()
