#pragma once
#include "Systray.hpp"

struct Systray::Handle::Impl {
#if defined(WIN32)
    NOTIFYICONDATA _ni;
#endif
#if defined(GTK)
    inline Impl() : _icon(0) {}
    GtkStatusIcon* _icon;
#endif
private:
    inline Impl(const Impl& src) {}
};
