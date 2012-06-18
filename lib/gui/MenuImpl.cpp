#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/Menu.hpp"
#include "WindowImpl.hpp"
#include "MenuImpl.hpp"

z::widget Menu::Create::run(const z::widget& wnd, const Menu::Definition& def) {
    unused(def);
#if defined(WIN32)
    z::widget::impl* impl = new z::widget::impl();
    z::ref(impl)._menu = ::CreatePopupMenu();
    z::ref(impl)._val = wnd.val()._val;
#elif defined(GTK)
    z::widget::impl* impl = new z::widget::impl();
    z::ref(impl)._menu = gtk_menu_new();
    unused(wnd);
#elif defined(OSX)
    z::widget::impl* impl = new z::widget::impl();
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
    z::widget::impl* impl = new z::widget::impl();
#else
#error "Unimplemented GUI mode"
#endif
    z::widget handle(impl);
    return handle;
}

#if defined(GTK)
struct pos {
    inline pos(const int& px, const int& py) : x(px), y(py) {}
    int x;
    int y;
};

static void getMenuPosition(GtkMenu* menu, gint* x, gint* y, gboolean* push_in, gpointer user_data) {
    unused(menu);
    if(user_data) {
        pos* p = static_cast<pos*>(user_data);
        *x = z::ref(p).x;
        *y = z::ref(p).y;
    } else {
        GdkDisplay* display = gdk_display_get_default();
        gdk_display_get_pointer(display, NULL, x, y, NULL);
    }
    *push_in = TRUE;
}
#endif

void Menu::ShowAt::run(const z::widget& handle, const int& x, const int& y) {
#if defined(WIN32)
    ::SetForegroundWindow(handle.val()._val);
    ::TrackPopupMenu(handle.val()._menu, TPM_BOTTOMALIGN, x, y, 0, handle.val()._val, NULL );
#elif defined(GTK)
    gtk_widget_show_all (handle.val()._menu);
    pos p(x, y);
    gtk_menu_popup(GTK_MENU(handle.val()._menu), NULL, NULL, getMenuPosition, &p, 0, gtk_get_current_event_time());
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void Menu::Show::run(const z::widget& handle) {
#if defined(WIN32)
    POINT pt;
    ::GetCursorPos(&pt);
    ::SetForegroundWindow(handle.val()._val);
    ::TrackPopupMenu(handle.val()._menu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, handle.val()._val, NULL );
#elif defined(GTK)
    gtk_widget_show_all (handle.val()._menu);
    gtk_menu_popup(GTK_MENU(handle.val()._menu), NULL, NULL, getMenuPosition, 0, 0, gtk_get_current_event_time());
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}
