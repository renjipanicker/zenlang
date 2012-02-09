INCLUDE(../tools/SetupZL.cmake)

#####################################################################
# build zenlang library
#####################################################################
GET_TARGET_PROPERTY(ZEN_EXE zen LOCATION)
IF( CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET( ZEN_DBG_FLAG "--debug" )
ENDIF()


#########################
# build all base/ files (actually just copy them to output dir)
SET(libbase_ZPP_SOURCES ${libbase_ZPP_SOURCES} "${ZEN_ROOT}/lib/base/pch.hpp")
SET(libbase_ZPP_SOURCES ${libbase_ZPP_SOURCES} "${ZEN_ROOT}/lib/base/zenlang.hpp")
SET(libbase_ZPP_SOURCES ${libbase_ZPP_SOURCES} "${ZEN_ROOT}/lib/base/zenlang.cpp")
SET(libbase_ZPP_SOURCES ${libbase_ZPP_SOURCES} "${ZEN_ROOT}/lib/base/args.hpp")

FOREACH(_FILE ${libbase_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME)
    CONFIGURE_FILE("${ZEN_ROOT}/lib/base/${_BASE}" "${ZEN_OUTPUT_PATH}/include/base/${_BASE}" COPYONLY @ONLY)
ENDFOREACH()

#########################
# build libcore
CONFIGURE_FILE("${ZEN_ROOT}/lib/core/core.ipp" "${ZEN_OUTPUT_PATH}/include/core/core.ipp" COPYONLY @ONLY)

SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${ZEN_ROOT}/lib/core/String.zpp")
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${ZEN_ROOT}/lib/core/DateTime.zpp")
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${ZEN_ROOT}/lib/core/File.zpp")
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${ZEN_ROOT}/lib/core/Dir.zpp")
SET(libcore_ZPP_SOURCES ${libcore_ZPP_SOURCES} "${ZEN_ROOT}/lib/core/Url.zpp")

# Although these are files from the base/ and utils/ dir, we add it to libcore because there is no libbase binary created.
SET(libcore_SOURCES ${libcore_SOURCES} "${ZEN_ROOT}/lib/utils/sqlite3/sqlite3.c")
SET(libcore_SOURCES ${libcore_SOURCES} "${ZEN_ROOT}/lib/utils/sqlite3/sqlite3.h")
SET(libcore_SOURCES ${libcore_SOURCES} "${ZEN_ROOT}/lib/utils/sqlite3/sqlite3ext.h")
SET(libcore_SOURCES ${libcore_SOURCES} "${ZEN_ROOT}/lib/utils/sqlite3/vfs_compress.c")
SET(libcore_SOURCES ${libcore_SOURCES} "${ZEN_ROOT}/lib/utils/sqlite3/vfs_compress.c")

FOREACH(_FILE ${libcore_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME_WE)
    ADD_CUSTOM_COMMAND(
        COMMAND ${ZEN_EXE} ${ZEN_DBG_FLAG} -c --verbose --api "${ZEN_OUTPUT_PATH}/include/core" --src "./core" ${_FILE}
        OUTPUT "core/${_BASE}.cpp"
        DEPENDS ${ZEN_EXE} ${_FILE}
    )
    SET(libcore_SOURCES ${libcore_SOURCES} "core/${_BASE}.cpp")
    SET(libcore_SOURCES ${libcore_SOURCES} "${ZEN_ROOT}/lib/core/${_BASE}Impl.cpp")
ENDFOREACH()

ADD_LIBRARY(core STATIC ${libcore_SOURCES})

#########################
# build all utils/ files (actually just copy them to output dir)
SET(libutil_ZPP_SOURCES ${libutil_ZPP_SOURCES} "${ZEN_ROOT}/lib/utils/sqlite3/sqlite3ext.h")
SET(libutil_ZPP_SOURCES ${libutil_ZPP_SOURCES} "${ZEN_ROOT}/lib/utils/sqlite3/sqlite3.h")

# NOTE: copying from utils/ to util/ (no 's' is dst path)
FOREACH(_FILE ${libutil_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME)
    CONFIGURE_FILE("${ZEN_ROOT}/lib/utils/sqlite3/${_BASE}" "${ZEN_OUTPUT_PATH}/include/util/sqlite3/${_BASE}" COPYONLY @ONLY)
ENDFOREACH()

#########################
# build all tools/ files (actually just copy them to output dir)
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${ZEN_ROOT}/tools/FindGTK3.cmake")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${ZEN_ROOT}/tools/SetupZL.cmake")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${ZEN_ROOT}/tools/lemon")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${ZEN_ROOT}/tools/lemon.exe")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${ZEN_ROOT}/tools/lempar.c")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${ZEN_ROOT}/tools/re2c")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${ZEN_ROOT}/tools/re2c.exe")
SET(libtool_ZPP_SOURCES ${libtool_ZPP_SOURCES} "${ZEN_ROOT}/tools/zpp.xml")

FOREACH(_FILE ${libtool_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME)
    CONFIGURE_FILE("${ZEN_ROOT}/tools/${_BASE}" "${ZEN_OUTPUT_PATH}/tools/${_BASE}" COPYONLY @ONLY)
ENDFOREACH()

#########################
# build libgui. Its in a separate project because it requires the GUI includes and libraries.
ADD_SUBDIRECTORY(gui)
