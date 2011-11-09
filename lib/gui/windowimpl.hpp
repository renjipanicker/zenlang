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
