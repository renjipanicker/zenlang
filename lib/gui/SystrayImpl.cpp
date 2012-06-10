#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/Systray.hpp"
#include "WindowImpl.hpp"
#include "SystrayImpl.hpp"

#if defined(WIN32)
namespace SystrayImpl {
    static z::HandlerList<int, Systray::OnActivation::Handler> onSystrayActivationHandlerList;
    static z::HandlerList<int, Systray::OnContextMenu::Handler> onSystrayContextMenuHandlerList;
    struct WinProc : public Window::Native::WndProc {
        virtual LRESULT handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
            if(lParam == WM_LBUTTONDOWN) {
                Systray::OnActivation::Handler::_In in;
                onSystrayActivationHandlerList.runHandler(message, in);
            }

            if((lParam == WM_RBUTTONDOWN) || (lParam == WM_CONTEXTMENU)) {
                Systray::OnContextMenu::Handler::_In in;
                onSystrayContextMenuHandlerList.runHandler(message, in);
            }

            return 0;
        }
    };
    static WinProc s_winProc;
}
#endif

void Systray::SetTooltip::run(const Systray::Handle& handle, const z::string& text) {
#if defined(WIN32)
    NOTIFYICONDATA& ni = Systray::impl(handle)._ni;
    lstrcpyn(ni.szTip, z::s2e(text).c_str(), z::s2e(text).length());
    ni.uFlags |= NIF_TIP;
    //SysTray::Native::setIconFile(This._sysTray._impl->_ni, This._text);
    ::Shell_NotifyIcon(NIM_MODIFY, z::ptr(ni));
#elif defined(GTK)
    gtk_status_icon_set_tooltip_text(Systray::impl(handle)._icon, z::s2e(text).c_str());
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void Systray::SetIconfile::run(const Systray::Handle& handle, const z::string& filename) {
#if defined(WIN32)
    NOTIFYICONDATA& ni = Systray::impl(handle)._ni;
    ni.hIcon = (HICON)LoadImageA(NULL, z::s2e(filename).c_str(), IMAGE_ICON,
                                 GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                                 LR_LOADFROMFILE);
    if(0 == ni.hIcon) {
        throw z::Exception("Systray", z::string("Unable to load icon %{s}").arg("s", filename));
    }

    ni.uFlags |= NIF_ICON;
    ::Shell_NotifyIcon(NIM_MODIFY, z::ptr(ni));
#elif defined(GTK)
    unused(filename);
    gtk_status_icon_set_from_icon_name(Systray::impl(handle)._icon, GTK_STOCK_MEDIA_STOP);
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void Systray::Show::run(const Systray::Handle& handle) {
#if defined(WIN32)
    ::Shell_NotifyIcon(NIM_ADD, z::ptr(Systray::impl(handle)._ni));
#elif defined(GTK)
    gtk_status_icon_set_visible(Systray::impl(handle)._icon, TRUE);
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void Systray::Hide::run(const Systray::Handle& handle) {
#if defined(WIN32)
    ::Shell_NotifyIcon(NIM_DELETE, z::ptr(Systray::impl(handle)._ni));
#elif defined(GTK)
    gtk_status_icon_set_visible(Systray::impl(handle)._icon, FALSE);
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

Systray::Handle Systray::Create::run(const Window::Handle& parent, const Systray::Definition& def) {
    unused(parent);
    Systray::HandleImpl* impl = new Systray::HandleImpl();
    Systray::Handle handle;
    handle._wdata<Systray::Handle>(impl);
#if defined(WIN32)
    z::ref(impl)._wm = Window::Native::getNextWmID();
    int res = Window::Native::getNextResID();

    // Fill the NOTIFYICONDATA structure and call Shell_NotifyIcon
    // zero the structure - note:   Some Windows funtions require this but
    //                              I can't be bothered which ones do and
    //                              which ones don't.
    ZeroMemory(&(z::ref(impl)._ni), sizeof(NOTIFYICONDATA));

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

    z::ref(impl)._ni.cbSize = sizeof(NOTIFYICONDATA);

    // the ID number can be anything you choose
    z::ref(impl)._ni.uID = res;

    // state which structure members are valid
    //impl->_ni.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    z::ref(impl)._ni.uFlags = NIF_MESSAGE;

    // the window to send messages to and the message to send
    //      note:   the message value should be in the
    //              range of WM_APP through 0xBFFF
    z::ref(impl)._ni.hWnd = Window::impl(parent)._hWindow;
    z::ref(impl)._ni.uCallbackMessage = z::ref(impl)._wm;
    ::Shell_NotifyIcon(NIM_ADD, z::ptr(z::ref(impl)._ni));
#elif defined(GTK)
    z::ref(impl)._icon = gtk_status_icon_new_from_stock(GTK_STOCK_GO_UP);
    g_object_set_data(G_OBJECT(z::ref(impl)._icon), "impl", impl);
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif

    if(def.tooltip.length() > 0) {
        SetTooltip().run(handle, def.tooltip);
    }

    if(def.iconFile.length() > 0) {
        SetIconfile().run(handle, def.iconFile);
    }

    if(def.visible) {
        Show().run(handle);
    }

    return handle;
}

#if defined(GTK)
static gboolean onSystrayActivateEvent(GtkStatusIcon* status_icon, gpointer phandler) {
    unused(status_icon);
    Systray::OnActivation::Handler* handler = static_cast<Systray::OnActivation::Handler*>(phandler);
    Systray::OnActivation::Handler::_In in;
    z::ref(handler)._run(in);
    return FALSE;
}
#endif

void Systray::OnActivation::addHandler(const Systray::Handle& systray, Handler* handler) {
    Systray::OnActivation::add(handler);
#if defined(WIN32)
    SystrayImpl::onSystrayActivationHandlerList.addHandler(Systray::impl(systray)._wm, handler);
#elif defined(GTK)
    g_signal_connect(G_OBJECT (Systray::impl(systray)._icon), "activate", G_CALLBACK (onSystrayActivateEvent), handler);
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

#if defined(GTK)
static gboolean onSystrayContextMenuEvent(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer phandler) {
    unused(status_icon);
    unused(button);
    unused(activate_time);

    Systray::OnContextMenu::Handler* handler = static_cast<Systray::OnContextMenu::Handler*>(phandler);
    Systray::OnContextMenu::Handler::_In in;
    z::ref(handler)._run(in);
    return FALSE;
}
#endif

void Systray::OnContextMenu::addHandler(const Systray::Handle& systray, Handler* handler) {
    Systray::OnContextMenu::add(handler);
#if defined(WIN32)
    SystrayImpl::onSystrayContextMenuHandlerList.addHandler(Systray::impl(systray)._wm, handler);
#elif defined(GTK)
    g_signal_connect(G_OBJECT (Systray::impl(systray)._icon), "popup-menu", G_CALLBACK (onSystrayContextMenuEvent), handler);
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}
