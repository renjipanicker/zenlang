#pragma once
#include "Menu.hpp"

struct MenuHandleImpl {
#if defined(WIN32)
    inline MenuHandleImpl() : _menu(0), _window(0) {}
    HMENU _menu;
    HWND _window;
#endif
#if defined(GTK)
    inline MenuHandleImpl() : _menu(0) {}
    GtkWidget* _menu;
#endif
private:
    inline MenuHandleImpl(const MenuHandleImpl& src) {}
};
