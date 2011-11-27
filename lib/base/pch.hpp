#ifndef PCH_HPP
#define PCH_HPP

// All #defines are validated here
#if !defined(Z_EXE) && !defined(Z_DLL) && !defined(Z_LIB)
#define Z_EXE 1
#endif

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
#include <typeinfo>

#if defined(WIN32)
    #define snprintf _snprintf_s
    #define sprintf sprintf_s
    #pragma warning (disable:4355) // this used in base ctor initialization.
#else
    #include <libgen.h>
    #include <cxxabi.h>
#endif

#if defined __cplusplus
    #include <string>
    #include <vector>
    #include <list>
    #include <map>
    #include <iostream>
    #include <sstream>
#endif // __cplusplus

#endif
