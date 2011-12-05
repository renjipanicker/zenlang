#pragma once
#include "MenuItem.hpp"

struct MenuItem::Handle::Impl {
#if defined(WIN32)
    inline Impl() : _id(0) {}
    int _id;
#endif
#if defined(GTK)
    inline Impl() : _menuItem(0) {}
    GtkWidget* _menuItem;
#endif
private:
    inline Impl(const Impl& src) {}
};
