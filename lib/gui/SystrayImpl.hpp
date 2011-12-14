#pragma once
#include "Systray.hpp"
#include "WidgetImpl.hpp"

namespace Systray {
    struct HandleImpl : public Widget::Handle::Impl {
#if defined(WIN32)
        inline HandleImpl() : _wm(0) {}
        int _wm;
        NOTIFYICONDATA _ni;
#endif
#if defined(GTK)
        inline HandleImpl() : _icon(0) {}
        GtkStatusIcon* _icon;
#endif
    private:
        inline HandleImpl(const HandleImpl& src) {}
    };

    inline HandleImpl& impl(const Widget::Handle& widget) {return Widget::impl<HandleImpl>(widget);}
}
