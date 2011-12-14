#pragma once
#include "Window.hpp"

struct WindowHandleImpl : public Widget::Handle::Impl {
#if defined(WIN32)
    inline WindowHandleImpl() : _hWindow(0) {}
    HWND _hWindow;
#endif
#if defined(GTK)
    inline WindowHandleImpl() : _hWindow(0), _hFixed(0) {}
    GtkWidget* _hWindow;
    GtkWidget* _hFixed;
#endif
private:
    inline WindowHandleImpl(const WindowHandleImpl& src) {}
};

inline WindowHandleImpl& wih(const Widget::Handle& widget) {
    Widget::Handle::Impl* wdata = widget.wdata;
    WindowHandleImpl* wh = dynamic_cast<WindowHandleImpl*>(wdata);
    return ref(wh);
}

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

WindowHandleImpl& createWindow(const Window::Definition& def, const std::string& className, const int& style, const int& xstyle, HWND parent);
WindowHandleImpl& createMainFrame(const Window::Definition& def, const int& style, const int& xstyle);
WindowHandleImpl& createChildFrame(const Window::Definition& def, const int &style, const int &xstyle, const Window::Handle &parent);
WindowHandleImpl& createChildWindow(const Window::Definition& def, const std::string& className, const int& style, const int& xstyle, const Window::Handle& parent);
#endif

#if defined(GTK)
WindowHandleImpl& initWindowImpl(GtkWidget* hwnd);
WindowHandleImpl& createWindow(const Window::Definition& def, GtkWidget *parent);
WindowHandleImpl& createChildWindow(GtkWidget* hwnd, const Window::Definition& def, const Window::Handle& parent);
#endif

}
}
