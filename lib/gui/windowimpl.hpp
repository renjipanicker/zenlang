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
void createWindow(Window::Create& action, const std::string& className, const int& style, const int& xstyle, HWND parent);
#endif

#if defined(GTK)
Window::Instance::Impl createWindow(const Window::Definition& def, GtkWidget *parent);
void createChildWindow(Window::Instance::Impl& impl, const Window::Definition& def, const Window::Instance& parent);
#endif

}
}
