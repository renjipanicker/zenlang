#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "WindowImpl.hpp"
#include "SystrayImpl.hpp"
#include "Systray.hpp"

Systray::Handle Systray::Create::run(const Window::Handle& parent, const Systray::Definition& def) {
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
    Systray::Handle handle;
    handle._wdata<Systray::Handle>(impl);
    return handle;
}

void Systray::SetTooltip::run(const Systray::Handle& handle, const std::string& text) {
#if defined(WIN32)
    lstrcpyn(ni.szTip, text.c_str(), text.length());
    ni.uFlags |= NIF_TIP;
    //SysTray::Native::setIconFile(This._sysTray._impl->_ni, This._text);
    ::Shell_NotifyIcon(NIM_MODIFY, ptr(ref(handle.wdata)._icon._ni));
#endif
#if defined(GTK)
    gtk_status_icon_set_tooltip_text(ref(handle.wdata)._icon, text.c_str());
    //SysTray::Native::setIconFile(This._sysTray, This._text);
#endif
}

void Systray::SetIconfile::run(const Systray::Handle& handle, const std::string& filename) {
#if defined(WIN32)
    ni.hIcon = (HICON)LoadImageA(NULL, filename.c_str(), IMAGE_ICON,
                                 GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                                 LR_LOADFROMFILE);
    if(0 == ni.hIcon) {
        throw Exception("Unable to load icon");
    }

    ni.uFlags |= NIF_ICON;
    ::Shell_NotifyIcon(NIM_MODIFY, ptr(ref(handle.wdata)._icon._ni));
#endif
#if defined(GTK)
    gtk_status_icon_set_from_icon_name(ref(handle.wdata)._icon, GTK_STOCK_MEDIA_STOP);
#endif
}

void Systray::Show::run(const Systray::Handle& handle) {
#if defined(WIN32)
    ::Shell_NotifyIcon(NIM_ADD, ptr(ref(handle.wdata)._icon._ni));
#endif
#if defined(GTK)
    gtk_status_icon_set_visible(ref(handle.wdata)._icon, TRUE);
#endif
}

void Systray::Hide::run(const Systray::Handle& handle) {
#if defined(WIN32)
    ::Shell_NotifyIcon(NIM_DELETE, ptr(ref(handle.wdata)._icon._ni));
#endif
#if defined(GTK)
    gtk_status_icon_set_visible(ref(handle.wdata)._icon, FALSE);
#endif
}

#if defined(GTK)
static gboolean onSystrayActivateEvent(GtkStatusIcon* status_icon, gpointer phandler) {
    unused(status_icon);
    Systray::OnActivation::Handler* handler = static_cast<Systray::OnActivation::Handler*>(phandler);
    Systray::OnActivation::Handler::_In in;
    ref(handler)._run(in);
    return FALSE;
}
#endif

void Systray::OnActivation::addHandler(const Systray::Handle& systray, Handler* handler) {
    Systray::OnActivation::add(handler);
#if defined(WIN32)
    onCloseHandlerList.addHandler(ref(systray.wdata)._icon, handler);
#endif
#if defined(GTK)
    g_signal_connect(G_OBJECT (ref(systray.wdata)._icon), "activate", G_CALLBACK (onSystrayActivateEvent), handler);
#endif
}

#if defined(GTK)
static gboolean onSystrayContextMenuEvent(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer phandler) {
    unused(status_icon);
    unused(button);
    unused(activate_time);

    Systray::OnContextMenu::Handler* handler = static_cast<Systray::OnContextMenu::Handler*>(phandler);
    Systray::OnContextMenu::Handler::_In in;
    ref(handler)._run(in);
    return FALSE;
}
#endif

void Systray::OnContextMenu::addHandler(const Systray::Handle& systray, Handler* handler) {
    Systray::OnContextMenu::add(handler);
#if defined(WIN32)
    onCloseHandlerList.addHandler(ref(systray.wdata)._icon, handler);
#endif
#if defined(GTK)
    g_signal_connect(G_OBJECT (ref(systray.wdata)._icon), "popup-menu", G_CALLBACK (onSystrayContextMenuEvent), handler);
#endif
}
