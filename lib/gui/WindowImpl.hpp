#pragma once
#include "Window.hpp"

struct Window::Handle::Impl {
#if defined(WIN32)
    inline Impl() : _hWindow(0) {}
    HWND _hWindow;
#endif
#if defined(GTK)
    inline Impl() : _hWindow(0), _hFixed(0) {}
    GtkWidget* _hWindow;
    GtkWidget* _hFixed;
#endif
private:
    inline Impl(const Impl& src) {}
};

namespace Window {
namespace Native {

#if defined(WIN32)
struct WndProc {
    WndProc();
    virtual LRESULT handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
    WndProc* _next;
};
#endif

#if defined(WIN32)
int getNextWmID();
int getNextResID();
ULONGLONG GetDllVersion(LPCTSTR lpszDllName);

Window::Handle::Impl& createWindow(const Window::Definition& def, const std::string& className, const int& style, const int& xstyle, HWND parent);
Window::Handle::Impl& createMainFrame(const Window::Definition& def, const int& style, const int& xstyle);
Window::Handle::Impl& createChildFrame(const Window::Definition& def, const int &style, const int &xstyle, const Window::Handle &parent);
Window::Handle::Impl& createChildWindow(const Window::Definition& def, const std::string& className, const int& style, const int& xstyle, const Window::Handle& parent);
#endif

#if defined(GTK)
Window::Handle::Impl& initWindowImpl(GtkWidget* hwnd);
Window::Handle::Impl& createWindow(const Window::Definition& def, GtkWidget *parent);
Window::Handle::Impl& createChildWindow(GtkWidget* hwnd, const Window::Definition& def, const Window::Handle& parent);
#endif

}
}
