#pragma once
#include "Menu.hpp"

struct Menu::Handle::Impl {
#if defined(WIN32)
    inline Impl() : _menu(0), _window(0) {}
    HMENU _menu;
    HWND _window;
#endif
#if defined(GTK)
    inline Impl() : _menu(0) {}
    GtkWidget* _menu;
#endif
private:
    inline Impl(const Impl& src) {}
};
