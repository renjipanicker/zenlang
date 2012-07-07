#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/Systray.hpp"
#include "WindowImpl.hpp"
#include "SystrayImpl.hpp"

#if defined(WIN32)
namespace zz {
namespace SystrayImpl {
    typedef WindowImpl::WidgetMap<UINT> IconMap;
    static IconMap iconMap;

    static WNDPROC OrigWndProc = 0;
    static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        if(lParam == WM_LBUTTONDOWN) {
            z::Systray::OnActivation::Handler::_In in;
            z::Systray::OnActivation::list().runHandler(iconMap.impl(message), in);
        }

        if((lParam == WM_RBUTTONDOWN) || (lParam == WM_CONTEXTMENU)) {
            z::Systray::OnContextMenu::Handler::_In in;
            z::Systray::OnContextMenu::list().runHandler(iconMap.impl(message), in);
        }

        assert(OrigWndProc);
        return CallWindowProc(OrigWndProc, hWnd, message, wParam, lParam);
    }
}
}
#endif

void z::Systray::SetTooltip::run(const z::widget& handle, const z::string& text) {
#if defined(WIN32)
    NOTIFYICONDATA& ni = handle.ni();
    lstrcpyn(ni.szTip, z::s2e(text).c_str(), z::s2e(text).length());
    ni.uFlags |= NIF_TIP;
    //SysTray::Native::setIconFile(This._sysTray._impl->_ni, This._text);
    ::Shell_NotifyIcon(NIM_MODIFY, z::ptr(ni));
#elif defined(GTK)
    gtk_status_icon_set_tooltip_text(handle.val()._icon, z::s2e(text).c_str());
#elif defined(QT)
    UNIMPL();
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void z::Systray::SetIconfile::run(const z::widget& handle, const z::string& filename) {
#if defined(WIN32)
    NOTIFYICONDATA& ni = handle.ni();
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
    gtk_status_icon_set_from_icon_name(handle.val()._icon, GTK_STOCK_MEDIA_STOP);
#elif defined(QT)
    UNIMPL();
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void z::Systray::Show::run(const z::widget& handle) {
#if defined(WIN32)
    ::Shell_NotifyIcon(NIM_ADD, z::ptr(handle.ni()));
#elif defined(GTK)
    gtk_status_icon_set_visible(handle.val()._icon, TRUE);
#elif defined(QT)
    UNIMPL();
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void z::Systray::Hide::run(const z::widget& handle) {
#if defined(WIN32)
    ::Shell_NotifyIcon(NIM_DELETE, z::ptr(handle.ni()));
#elif defined(GTK)
    gtk_status_icon_set_visible(handle.val()._icon, FALSE);
#elif defined(QT)
    UNIMPL();
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

z::widget z::Systray::Create::run(const z::widget& parent, const z::Systray::Definition& def) {
    z::unused_t(parent);
    z::widget::impl* impl = new z::widget::impl();
    z::widget handle(impl);
#if defined(WIN32)
    z::ref(impl)._id = WindowImpl::getNextWmID();
    zz::SystrayImpl::iconMap.add(z::ref(impl)._id, impl);

    int res = WindowImpl::getNextResID();

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
//    ULONGLONG ullVersion = WindowImpl::GetDllVersion(_T("Shell32.dll"));
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
    z::ref(impl)._ni.hWnd = parent.val()._val;
    z::ref(impl)._ni.uCallbackMessage = z::ref(impl)._id;
    ::Shell_NotifyIcon(NIM_ADD, z::ptr(z::ref(impl)._ni));

    // set subclass function
    zz::SystrayImpl::OrigWndProc = (WNDPROC)SetWindowLong(parent.val()._val, GWL_WNDPROC, (LONG)zz::SystrayImpl::WinProc);
#elif defined(GTK)
    z::ref(impl)._icon = gtk_status_icon_new_from_stock(GTK_STOCK_GO_UP);
    g_object_set_data(G_OBJECT(z::ref(impl)._icon), "impl", impl);
#elif defined(QT)
    UNIMPL();
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
    z::Systray::OnActivation::Handler* handler = static_cast<z::Systray::OnActivation::Handler*>(phandler);
    z::Systray::OnActivation::Handler::_In in;
    z::ref(handler)._run(in);
    return FALSE;
}
#endif

void z::Systray::OnActivation::addHandler(const z::widget& systray, const z::pointer<Handler>& handler) {
#if defined(WIN32)
#elif defined(GTK)
    g_signal_connect(G_OBJECT (systray.val()._icon), "activate", G_CALLBACK (onSystrayActivateEvent), z::ptr(handler.get()) );
#elif defined(QT)
    UNIMPL();
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

    z::Systray::OnContextMenu::Handler* handler = static_cast<z::Systray::OnContextMenu::Handler*>(phandler);
    z::Systray::OnContextMenu::Handler::_In in;
    z::ref(handler)._run(in);
    return FALSE;
}
#endif

void z::Systray::OnContextMenu::addHandler(const z::widget& systray, const z::pointer<Handler>& handler) {
#if defined(WIN32)
#elif defined(GTK)
    g_signal_connect(G_OBJECT (systray.val()._icon), "popup-menu", G_CALLBACK (onSystrayContextMenuEvent), z::ptr(handler.get()));
#elif defined(QT)
    UNIMPL();
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}
