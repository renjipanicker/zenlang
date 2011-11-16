#pragma once

struct Window::Instance::Impl {
#if defined(WIN32)
    HWND _hWindow;
#endif
#if defined(GTK)
    GtkWidget* _hWindow;
    GtkWidget* _hFixed;
#endif
};

namespace Window {
namespace Native {

#if defined(WIN32)
Window::Instance::Impl createWindow(const Window::Definition& def, const std::string& className, const int& style, const int& xstyle, HWND parent);
Window::Instance::Impl createMainFrame(const Window::Definition& def, const int& style, const int& xstyle);
Window::Instance::Impl createChildFrame(const Window::Definition& def, const int &style, const int &xstyle, const Window::Instance &parent);
Window::Instance::Impl createChildWindow(const Window::Definition& def, const std::string& className, const int& style, const int& xstyle, const Window::Instance& parent);
#endif

#if defined(GTK)
Window::Instance::Impl createWindow(const Window::Definition& def, GtkWidget *parent);
void createChildWindow(Window::Instance::Impl& impl, const Window::Definition& def, const Window::Instance& parent);
#endif

}
}
