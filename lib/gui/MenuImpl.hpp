#pragma once
#include "gui/Menu.hpp"
#include "WidgetImpl.hpp"

namespace Menu {
    struct HandleImpl : public Widget::Handle::Impl {
#if defined(WIN32)
        inline HandleImpl() : _menu(0), _window(0) {}
        HMENU _menu;
        HWND _window;
#endif
#if defined(GTK)
        inline HandleImpl() : _menu(0) {}
        GtkWidget* _menu;
#endif
    private:
        inline HandleImpl(const HandleImpl& /*src*/) {}
    };

    inline HandleImpl& impl(const Widget::Handle& widget) {return Widget::impl<HandleImpl>(widget);}
}
