#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "WindowImpl.hpp"
#include "SystrayImpl.hpp"
#include "Systray.hpp"

const Systray::Create::_Out& Systray::Create::run(const Window::Handle& parent, const Systray::Definition& def) {
#if defined(WIN32)
    Systray::Handle::Impl* impl = new Systray::Handle::Impl();

    int wm = Window::Native::getNextWmID();
    int res = Window::Native::getNextResID();

    // Fill the NOTIFYICONDATA structure and call Shell_NotifyIcon
    // zero the structure - note:   Some Windows funtions require this but
    //                              I can't be bothered which ones do and
    //                              which ones don't.
    ZeroMemory(&(ref(impl)._ni), sizeof(NOTIFYICONDATA));

    // get Shell32 version number and set the size of the structure
    //      note:   the MSDN documentation about this is a little
    //              dubious and I'm not at all sure if the method
    //              bellow is correct
    ULONGLONG ullVersion = Window::Native::GetDllVersion(_T("Shell32.dll"));
    if(ullVersion >= MAKEDLLVERULL(5, 0,0,0))
        ref(impl)._ni.cbSize = sizeof(NOTIFYICONDATA);
    else
        ref(impl)._ni.cbSize = NOTIFYICONDATA_V2_SIZE;

    // the ID number can be anything you choose
    ref(impl)._ni.uID = res;

    // state which structure members are valid
    //impl->_ni.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    ref(impl)._ni.uFlags = NIF_MESSAGE;

    if(def.tooltip.length() > 0) {
        SysTray::Native::setToolTip(ref(impl)._ni, def.tooltip);
    }

    if(def.iconFile.length() > 0) {
        SysTray::Native::setIconFile(ref(impl)._ni, def.iconFile);
    }

    // the window to send messages to and the message to send
    //      note:   the message value should be in the
    //              range of WM_APP through 0xBFFF
    ref(impl)._ni.hWnd = ref(parent._impl)._hWindow;
    ref(impl)._ni.uCallbackMessage = wm;
    ::Shell_NotifyIcon(NIM_ADD, ptr(ref(impl)._ni));
#endif
#if defined(GTK)
    Systray::Handle::Impl* impl = new Systray::Handle::Impl();
    ref(impl)._icon = gtk_status_icon_new_from_stock(GTK_STOCK_GO_UP);
    g_object_set_data(G_OBJECT(ref(impl)._icon), "impl", impl);
    if(def.iconFile.length() > 0) {
    //    gtk_status_icon_set_from_icon_name(ref(impl)._icon, GTK_STOCK_MEDIA_STOP); /// \todo use specified file instead of stock icon
    }
    if(def.tooltip.length() > 0) {
        gtk_status_icon_set_tooltip_text(ref(impl)._icon, def.tooltip.c_str());
    }
    if(def.visible) {
        gtk_status_icon_set_visible(ref(impl)._icon, TRUE);
    }
#endif
    Systray::Handle st;
    st._wdata<Systray::Handle>(impl);
    return out(_Out(st));
}
