#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "WindowImpl.hpp"
#include "MenuImpl.hpp"
#include "Menu.hpp"

Menu::Handle Menu::Create::run(const Window::Handle& window, const Menu::Definition& def) {
    unused(def);
#if defined(WIN32)
    Menu::Handle::Impl* impl = new Menu::Handle::Impl();
    ref(impl)._menu = ::CreatePopupMenu();
    ref(impl)._window = window._hWindow;
#endif
#if defined(GTK)
    Menu::Handle::Impl* impl = new Menu::Handle::Impl();
    ref(impl)._menu = gtk_menu_new();
    unused(window);
#endif
    Menu::Handle handle;
    handle._wdata<Menu::Handle>(impl);
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
        *x = ref(p).x;
        *y = ref(p).y;
    } else {
        GdkDisplay* display = gdk_display_get_default();
        gdk_display_get_pointer(display, NULL, x, y, NULL);
    }
    *push_in = TRUE;
}
#endif

void Menu::ShowAt::run(const Menu::Handle& handle, const int& x, const int& y) {
#if defined(WIN32)
    ::SetForegroundWindow(ref(handle.wdata)._window);
    ::TrackPopupMenu(ref(handle.wdata)._menu, TPM_BOTTOMALIGN, x, y, 0, ref(handle.wdata)._window, NULL );
#endif
#if defined(GTK)
    gtk_widget_show_all (ref(handle.wdata)._menu);
    pos p(x, y);
    gtk_menu_popup(GTK_MENU(ref(handle.wdata)._menu), NULL, NULL, getMenuPosition, &p, 0, gtk_get_current_event_time());
#endif
}

void Menu::Show::run(const Menu::Handle& handle) {
#if defined(WIN32)
    POINT pt;
    ::GetCursorPos(&pt);
    ::SetForegroundWindow(ref(handle.wdata)._window);
    ::TrackPopupMenu(ref(handle.wdata)._menu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, ref(handle.wdata)._window, NULL );
#endif
#if defined(GTK)
    gtk_widget_show_all (ref(handle.wdata)._menu);
    gtk_menu_popup(GTK_MENU(ref(handle.wdata)._menu), NULL, NULL, getMenuPosition, 0, 0, gtk_get_current_event_time());
#endif
}
