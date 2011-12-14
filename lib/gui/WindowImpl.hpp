#pragma once
#include "Window.hpp"
#include "WidgetImpl.hpp"

namespace Window {
struct HandleImpl : public Widget::Handle::Impl {
#if defined(WIN32)
    inline HandleImpl() : _hWindow(0) {}
    HWND _hWindow;
#endif
#if defined(GTK)
    inline HandleImpl() : _hWindow(0), _hFixed(0) {}
    GtkWidget* _hWindow;
    GtkWidget* _hFixed;
#endif
private:
    inline HandleImpl(const HandleImpl& src) {}
};

inline HandleImpl& impl(const Widget::Handle& widget) {return Widget::impl<HandleImpl>(widget);}

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

Window::HandleImpl& createWindow(const Window::Definition& def, const std::string& className, const int& style, const int& xstyle, HWND parent);
Window::HandleImpl& createMainFrame(const Window::Definition& def, const int& style, const int& xstyle);
Window::HandleImpl& createChildFrame(const Window::Definition& def, const int &style, const int &xstyle, const Window::Handle &parent);
Window::HandleImpl& createChildWindow(const Window::Definition& def, const std::string& className, const int& style, const int& xstyle, const Window::Handle& parent);
#endif

#if defined(GTK)
Window::HandleImpl& initWindowImpl(GtkWidget* hwnd);
Window::HandleImpl& createWindow(const Window::Definition& def, GtkWidget *parent);
Window::HandleImpl& createChildWindow(GtkWidget* hwnd, const Window::Definition& def, const Window::Handle& parent);
#endif

}
}
