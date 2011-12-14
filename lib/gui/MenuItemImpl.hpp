#pragma once
#include "MenuItem.hpp"

struct MenuItemHandleImpl {
#if defined(WIN32)
    inline MenuItemHandleImpl() : _id(0) {}
    int _id;
#endif
#if defined(GTK)
    inline MenuItemHandleImpl() : _menuItem(0) {}
    GtkWidget* _menuItem;
#endif
private:
    inline MenuItemHandleImpl(const MenuItemHandleImpl& src) {}
};
