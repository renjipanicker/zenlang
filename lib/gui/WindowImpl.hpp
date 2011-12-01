#pragma once
#include "Window.hpp"

struct Window::Instance::Impl {
#if defined(WIN32)
    inline Impl() : _hWindow(0) {}
    HWND _hWindow;
#endif
#if defined(GTK)
    inline Impl() : _hWindow(0), _hFixed(0) {}
    GtkWidget* _hWindow;
    GtkWidget* _hFixed;
#endif
    Window::ChildList _childList;
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
Window::Instance::Impl& createWindow(const Window::Definition& def, const std::string& className, const int& style, const int& xstyle, HWND parent);
Window::Instance::Impl& createMainFrame(const Window::Definition& def, const int& style, const int& xstyle);
Window::Instance::Impl& createChildFrame(const Window::Definition& def, const int &style, const int &xstyle, const Window::Instance &parent);
Window::Instance::Impl& createChildWindow(const Window::Definition& def, const std::string& className, const int& style, const int& xstyle, const Window::Instance& parent);
#endif

#if defined(GTK)
Window::Instance::Impl& initWindowImpl(GtkWidget* hwnd);
Window::Instance::Impl& createWindow(const Window::Definition& def, GtkWidget *parent);
Window::Instance::Impl& createChildWindow(GtkWidget* hwnd, const Window::Definition& def, const Window::Instance& parent);
#endif

}
}
