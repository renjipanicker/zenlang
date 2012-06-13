#ifndef PCH_HPP
#define PCH_HPP

////////////////////////////////////////////////////
// Define this to tell zenlang.hpp that this is a exe
//#define Z_EXE 1

// Define this to tell zenlang.hpp that this is a GUI exe
//#define GUI 1

// Define this to tell zenlang.hpp that this is a debug build
//#define DEBUG 1

// Define this to enable unit tests in the build
//#define UNIT_TEST 1

////////////////////////////////////////////////////
// Select char width
#if !defined(CHAR_WIDTH_08) && !defined(CHAR_WIDTH_16) && !defined(CHAR_WIDTH_32)
// choose any one of these 3...
//#define CHAR_WIDTH_08
//#define CHAR_WIDTH_16
#define CHAR_WIDTH_32
#endif

////////////////////////////////////////////////////
// Define one of these to specify the GUI type
//#define WIN32 1  // To tell zenlang.hpp that this is a Win32 GUI exe
//#define QT 1     // To tell zenlang.hpp that this is a Qt GUI exe
//#define GTK 1    // To tell zenlang.hpp that this is a Gtk GUI exe
//#define OSX 1    // To tell zenlang.hpp that this is a OSX app
//#define IOS 1    // To tell zenlang.hpp that this is a IOS app

// if compiling in GUI mode and WIN32 is not defined, assume Linux GTK or QT
/// \todo change this when adding support for other platforms.
#if defined(GUI)
    #if !defined(WIN32) && !defined(GTK) && !defined(QT) && !defined(OSX) && !defined(IOS)
        #if defined(QT_GUI_LIB)
            #define QT 1
        #elif defined(__APPLE__)
            #if !defined(IOS)
                #define OSX 1
            #endif
        #else
            #define GTK 1
        #endif
    #endif
#endif

// validation. one and exactly one of the 3 must be defined in GUI mode
#if defined(GUI)
#if defined(WIN32)
    #if defined(GTK) || defined(QT) || defined(OSX) || defined(IOS)
    #error GTK/QT/OSX/IOS defined in WIN32
    #endif
#elif defined(GTK)
    #if defined(WIN32) || defined(QT) || defined(OSX) || defined(IOS)
    #error WIN32/QT/OSX/IOS defined in GTK
    #endif
#elif defined(QT)
    #if defined(WIN32) || defined(GTK) || defined(OSX) || defined(IOS)
    #error WIN32/GTK/OSX/IOS defined in QT
    #endif
#elif defined(OSX)
    #if defined(WIN32) || defined(GTK) || defined(QT) || defined(IOS)
    #error WIN32/GTK/QT/IOS defined in OSX
    #endif
#elif defined(IOS)
    #if defined(WIN32) || defined(GTK) || defined(QT) || defined(OSX)
    #error WIN32/GTK/QT/OSX defined in IOS
    #endif
#else
    #error "Unknown platform. Please define WIN32 or GTK or QT or OSX or IOS"
#endif
#endif

// This is for system objects like HANDLE etc that is used
#if defined(WIN32)
    #undef UNICODE
    #undef _UNICODE
    #define WIN32_LEAN_AND_MEAN
    # include <windows.h>
#endif

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <stdarg.h>
# include <memory.h>
# include <time.h>
# include <errno.h>
# include <sys/stat.h>
# include <assert.h>

#if defined(WIN32)
    # include <direct.h>
    # include <process.h>
    typedef HANDLE mutex_t;

    # include <WinSock2.h>

//    typedef char      int8_t;
    typedef short     int16_t;
    typedef int       int32_t;
    typedef long long int64_t;

    typedef unsigned char      uint8_t;
    typedef unsigned short     uint16_t;
    typedef unsigned int       uint32_t;
    typedef unsigned long long uint64_t;
#else
    # include <libgen.h>
    # include <pthread.h>
    typedef pthread_mutex_t mutex_t;
    # include <stdint.h>
    # include <regex.h>

    # include <sys/stat.h>
    # include <unistd.h>
    # include <sys/socket.h>
    # include <sys/ioctl.h>
    # include <netinet/in.h>
    # include <netdb.h>
    typedef int SOCKET;
#endif

#if defined(WIN32)
    #define snprintf _snprintf_s
    #define sprintf sprintf_s
    #define sscanf sscanf_s

    // "this used in base ctor initialization."
    // safe to disable this because ctors are used only for
    // initializing by reference we never implement any functionality in ctors.
    #pragma warning (disable:4355)

    // assignment operator could not be generated
    // caused due to const members in struct's and classes. 
    // can disable this because an error will be raised if an actual assignment
    // operation is attempted later on.
    #pragma warning (disable:4512)
#else
#endif

#if defined __cplusplus
    # include <string>
    # include <vector>
    # include <list>
    # include <set>
    # include <map>
    # include <iterator>
    # include <algorithm>
    # include <iostream>
    # include <fstream>
    # include <sstream>
    # include <iomanip>
    # include <typeinfo>
    # include <ctime>
    #if defined(WIN32)
    #else
    # include <cxxabi.h>
    #endif
#endif // __cplusplus

// all GUI header files are included here.
#if defined(GUI)
    #if defined(WIN32)
        #undef UNICODE
        #undef _UNICODE
        #define WIN32_LEAN_AND_MEAN
        # include <Windows.h>
        # include <Windowsx.h>
        # include <Commctrl.h>
        # include <Shlwapi.h>
        # include <tchar.h>
        # include <richedit.h>
    #elif defined(GTK)
        # include <gtk/gtk.h>
    #elif defined(QT)
    #elif defined(OSX)
    #else
    #endif
#else
#endif

// include these in gui and non-gui mode, since it can be used in non-gui apps
// to, for example, launch browser windows.
#if defined(WIN32)
    # include <Shellapi.h>
    # include <Shlobj.h>
#endif

#endif
