#ifndef PCH_HPP
#define PCH_HPP

// if compiling in GUI mode and WIN32 is not defined, assume Linux GTK or QT
/// \todo change this when adding support for other platforms.
#if defined(GUI)
#if !defined(WIN32)
#if !defined(GTK) && !defined(QT)
#if defined(QT_GUI_LIB)
#define QT 1
#else
#define GTK 1
#endif
#endif
#endif
#endif

// validation. one and exactly one of the 3 must be defined in GUI mode
#if defined(GUI)
#if defined(WIN32)
#if defined(GTK) || defined(QT)
#error GTK/QT defined in WIN32
#endif
#elif defined(GTK)
#if defined(WIN32) || defined(QT)
#error WIN32/QT defined in GTK
#endif
#elif defined(QT)
#if defined(WIN32) || defined(GTK)
#error WIN32/GTK defined in QT
#endif
#else
#error "Unknown platform. Please define WIN32 or GTK or QT"
#endif
#endif

// This is for system objects like HANDLE etc that is used
#if defined(WIN32)
    #undef UNICODE
    #undef _UNICODE
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <errno.h>
#include <sys/stat.h>
#include <assert.h>

#if defined(WIN32)
    #include <direct.h>
    #include <process.h>
    typedef HANDLE mutex_t;

//    typedef char      int8_t;
    typedef short     int16_t;
    typedef int       int32_t;
    typedef long long int64_t;

    typedef unsigned char      uint8_t;
    typedef unsigned short     uint16_t;
    typedef unsigned int       uint32_t;
    typedef unsigned long long uint64_t;
#else
    #include <libgen.h>
    #include <pthread.h>
    typedef pthread_mutex_t mutex_t;
    #include <stdint.h>
    #include <regex.h>
#endif

#if defined(WIN32)
    #define snprintf _snprintf_s
    #define sprintf sprintf_s
    #define sscanf sscanf_s
    #pragma warning (disable:4355) // this used in base ctor initialization.
#else
#endif

#if defined __cplusplus
    #include <string>
    #include <vector>
    #include <list>
    #include <map>
    #include <iterator>
    #include <algorithm>
    #include <iostream>
    #include <fstream>
    #include <sstream>
    #include <iomanip>
    #include <typeinfo>
    #if defined(WIN32)
    #else
    #include <cxxabi.h>
    #endif
    #include <ctime>
#endif // __cplusplus

// all GUI header files are included here.
#if defined(GUI)
    #if defined(WIN32)
        #undef UNICODE
        #undef _UNICODE
        #define WIN32_LEAN_AND_MEAN
        #include <Windows.h>
        #include <Windowsx.h>
        #include <Commctrl.h>
        #include <Shlwapi.h>
        #include <shellapi.h>
        #include <tchar.h>
    #endif
    #if defined(GTK)
        #include <gtk/gtk.h>
    #endif
    #if defined(QT)
        #include <QtGui/QApplication>
    #endif
#endif

#endif
