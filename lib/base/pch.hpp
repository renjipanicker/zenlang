#ifndef PCH_HPP
#define PCH_HPP

// All #defines are validated here
#if defined(GUI)
    #if !defined(WIN32)
        #define GTK 1
    #endif
#endif

#if defined(GUI)
    #if defined(WIN32)
    #elif defined(GTK)
    #else
        #error "Unknown platform. Please define WIN32 or GTK"
    #endif
#endif

// all header files are included here.
#if defined(WIN32)
    #undef UNICODE
    #undef _UNICODE
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

#if defined(GUI)
    #if defined(WIN32)
        #undef UNICODE
        #undef _UNICODE
        #define WIN32_LEAN_AND_MEAN
        #include <Windowsx.h>
        #include <Commctrl.h>
        #include <Shlwapi.h>
        #include <shellapi.h>
        #include <tchar.h>
    #endif
    #if defined(GTK)
        #include <gtk/gtk.h>
    #endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <assert.h>

#if defined(WIN32)
    #define snprintf _snprintf_s
    #define sprintf sprintf_s
    #pragma warning (disable:4355) // this used in base ctor initialization.
#else
    #include <libgen.h>
#endif

#ifdef _MSC_VER
typedef char      int8_t;
typedef short     int16_t;
typedef int       int32_t;
typedef long long int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
#else
#include <stdint.h>
#endif

typedef char char_t;

#if defined __cplusplus
    #include <string>
    #include <vector>
    #include <list>
    #include <map>
    #include <algorithm>
    #include <iostream>
    #include <fstream>
    #include <sstream>
    #include <iomanip>
    #include <typeinfo>
    #ifdef _MSC_VER
    #else
    #include <cxxabi.h>
    #include <ctime>
    #endif
#endif // __cplusplus

#endif
