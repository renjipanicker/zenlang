#pragma once
#include "MenuItem.hpp"

namespace MenuItem {
    struct HandleImpl : public Widget::Handle::Impl {
    #if defined(WIN32)
        inline HandleImpl() : _id(0) {}
        int _id;
    #endif
    #if defined(GTK)
        inline HandleImpl() : _menuItem(0) {}
        GtkWidget* _menuItem;
    #endif
    private:
        inline HandleImpl(const HandleImpl& src) {}
    };

    inline HandleImpl& impl(const Widget::Handle& widget) {return Widget::impl<HandleImpl>(widget);}
}
