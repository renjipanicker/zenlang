ADD_DEFINITIONS( "-DGUI" )
IF(WIN32)
ELSE(WIN32)
    SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../tools/")
    FIND_PACKAGE(GTK3)
    IF(GTK3_FOUND)
        INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
    ENDIF(GTK3_FOUND)
ENDIF(WIN32)

SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/Widget.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/Window.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/Mainframe.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/Button.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/TextEdit.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/Systray.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/Menu.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/MenuItem.zpp")
SET(libgui_ZPP_SOURCES ${libgui_ZPP_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/WindowCreator.zpp")

SET(libgui_Window_DEP "gui/Widget.cpp")
SET(libgui_Mainframe_DEP "gui/Window.cpp")
SET(libgui_WindowCreator_DEP "gui/Mainframe.cpp" "gui/Button.cpp" "gui/TextEdit.cpp" "gui/Systray.cpp" "gui/Menu.cpp" "gui/MenuItem.cpp")
SET(libgui_MenuItem_DEP "gui/Menu.cpp")
SET(libgui_Systray_DEP "gui/Window.cpp")
SET(libgui_Menu_DEP "gui/Window.cpp")

FOREACH(_FILE ${libgui_ZPP_SOURCES})
    GET_FILENAME_COMPONENT(_BASE ${_FILE} NAME_WE)
    ADD_CUSTOM_COMMAND(
        COMMAND ${ZEN_EXE} ${ZEN_DBG_FLAG} -c --gui --api "${ZEN_OUTPUT_PATH}/include/gui" --src "./gui" ${_FILE}
        OUTPUT "gui/${_BASE}.cpp"
        DEPENDS ${ZEN_EXE} ${_FILE} ${libgui_${_BASE}_DEP}
    )
    SET(libgui_SOURCES ${libgui_SOURCES} "gui/${_BASE}.cpp")
    SET(libgui_SOURCES ${libgui_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/${_BASE}Impl.cpp")
ENDFOREACH()

ADD_LIBRARY(gui STATIC ${libgui_SOURCES})
