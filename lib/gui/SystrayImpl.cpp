#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "WindowImpl.hpp"
#include "SystrayImpl.hpp"
#include "Systray.hpp"

#if defined(WIN32)
static HandlerList<int, Systray::OnActivation::Handler> onSystrayActivationHandlerList;
static HandlerList<int, Systray::OnContextMenu::Handler> onSystrayContextMenuHandlerList;
struct WinProc : public Window::Native::WndProc {
    virtual LRESULT handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        if(lParam == WM_LBUTTONDOWN) {
            Systray::OnActivation::Handler::_In in;
            if(onSystrayActivationHandlerList.runHandler(message, in))
                return 1;
        }

        if((lParam == WM_RBUTTONDOWN) || (lParam == WM_CONTEXTMENU)) {
            Systray::OnContextMenu::Handler::_In in;
            if(onSystrayContextMenuHandlerList.runHandler(message, in))
                return 1;
        }

        return 0;
    }
};
static WinProc s_winProc;
#endif

void Systray::setTooltip(const Systray::Handle& handle, const std::string& text) {
#if defined(WIN32)
    NOTIFYICONDATA& ni = ref(handle.wdata)._ni;
    lstrcpyn(ni.szTip, text.c_str(), text.length());
    ni.uFlags |= NIF_TIP;
    //SysTray::Native::setIconFile(This._sysTray._impl->_ni, This._text);
    ::Shell_NotifyIcon(NIM_MODIFY, ptr(ni));
#endif
#if defined(GTK)
    gtk_status_icon_set_tooltip_text(ref(handle.wdata)._icon, text.c_str());
#endif
}

void Systray::setIconfile(const Systray::Handle& handle, const std::string& filename) {
#if defined(WIN32)
    NOTIFYICONDATA& ni = ref(handle.wdata)._ni;
    ni.hIcon = (HICON)LoadImageA(NULL, filename.c_str(), IMAGE_ICON,
                                 GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                                 LR_LOADFROMFILE);
    if(0 == ni.hIcon) {
        throw Exception("Unable to load icon");
    }

    ni.uFlags |= NIF_ICON;
    ::Shell_NotifyIcon(NIM_MODIFY, ptr(ni));
#endif
#if defined(GTK)
    gtk_status_icon_set_from_icon_name(ref(handle.wdata)._icon, GTK_STOCK_MEDIA_STOP);
#endif
}

void Systray::show(const Systray::Handle& handle) {
#if defined(WIN32)
    ::Shell_NotifyIcon(NIM_ADD, ptr(ref(handle.wdata)._ni));
#endif
#if defined(GTK)
    gtk_status_icon_set_visible(ref(handle.wdata)._icon, TRUE);
#endif
}

void Systray::hide(const Systray::Handle& handle) {
#if defined(WIN32)
    ::Shell_NotifyIcon(NIM_DELETE, ptr(ref(handle.wdata)._ni));
#endif
#if defined(GTK)
    gtk_status_icon_set_visible(ref(handle.wdata)._icon, FALSE);
#endif
}

Systray::Handle Systray::Create::run(const Window::Handle& parent, const Systray::Definition& def) {
    Systray::Handle::Impl* impl = new Systray::Handle::Impl();
    Systray::Handle handle;
    handle._wdata<Systray::Handle>(impl);
#if defined(WIN32)
    ref(impl)._wm = Window::Native::getNextWmID();
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
    /// \todo Fix linker error when calling GetDllVersion()
//    ULONGLONG ullVersion = Window::Native::GetDllVersion(_T("Shell32.dll"));
//    if(ullVersion >= MAKEDLLVERULL(5, 0,0,0))
//        ref(impl)._ni.cbSize = sizeof(NOTIFYICONDATA);
//    else
//        ref(impl)._ni.cbSize = NOTIFYICONDATA_V2_SIZE;

    ref(impl)._ni.cbSize = sizeof(NOTIFYICONDATA);

    // the ID number can be anything you choose
    ref(impl)._ni.uID = res;

    // state which structure members are valid
    //impl->_ni.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    ref(impl)._ni.uFlags = NIF_MESSAGE;

    // the window to send messages to and the message to send
    //      note:   the message value should be in the
    //              range of WM_APP through 0xBFFF
    ref(impl)._ni.hWnd = ref(parent.wdata)._hWindow;
    ref(impl)._ni.uCallbackMessage = ref(impl)._wm;
    ::Shell_NotifyIcon(NIM_ADD, ptr(ref(impl)._ni));
#endif
#if defined(GTK)
    ref(impl)._icon = gtk_status_icon_new_from_stock(GTK_STOCK_GO_UP);
    g_object_set_data(G_OBJECT(ref(impl)._icon), "impl", impl);
#endif

    if(def.tooltip.length() > 0) {
        setTooltip(handle, def.tooltip);
    }

    if(def.iconFile.length() > 0) {
        setIconfile(handle, def.iconFile);
    }

    if(def.visible) {
        show(handle);
    }

    return handle;
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
    onSystrayActivationHandlerList.addHandler(ref(systray.wdata)._wm, handler);
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
    onSystrayContextMenuHandlerList.addHandler(ref(systray.wdata)._wm, handler);
#endif
#if defined(GTK)
    g_signal_connect(G_OBJECT (ref(systray.wdata)._icon), "popup-menu", G_CALLBACK (onSystrayContextMenuEvent), handler);
#endif
}
