#pragma once
#include "Systray.hpp"

struct SystrayHandleImpl {
#if defined(WIN32)
    inline SystrayHandleImpl() : _wm(0) {}
    int _wm;
    NOTIFYICONDATA _ni;
#endif
#if defined(GTK)
    inline SystrayHandleImpl() : _icon(0) {}
    GtkStatusIcon* _icon;
#endif
private:
    inline SystrayHandleImpl(const SystrayHandleImpl& src) {}
};
