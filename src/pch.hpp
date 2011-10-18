#ifndef PCH_HPP
#define PCH_HPP

#if defined(GUI)
#if defined(WIN32)
#undef UNICODE
#undef _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Commctrl.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <tchar.h>
#elif defined(GTK)
#include <gtk/gtk.h>
#else
#error "Unknown platform. Please define WIN32 or GTK"
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
#endif

#if defined __cplusplus

#include <string>
#include <list>
#include <map>
#include <iostream>
#include <sstream>

#endif // __cplusplus

#endif
