#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/MenuItem.hpp"
#include "WindowImpl.hpp"
#include "MenuItemImpl.hpp"
#include "MenuImpl.hpp"

#if defined(WIN32)
static z::HandlerList<int, MenuItem::OnSelect::Handler> onMenuItemSelectHandlerList;
struct WinProc : public Window::Native::WndProc {
    virtual LRESULT handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_COMMAND:
                MenuItem::OnSelect::Handler::_In in;
                if(onMenuItemSelectHandlerList.runHandler(LOWORD(lParam), in))
                    return 1;
                break;
        }
        return 0;
    }
};
static WinProc s_winProc;
#endif

MenuItem::Handle MenuItem::Create::run(const Menu::Handle& pmenu, const MenuItem::Definition& def) {
#if defined(WIN32)
    MenuItem::HandleImpl* impl = new MenuItem::HandleImpl();
    int wm = Window::Native::getNextWmID();
    ::InsertMenu(Menu::impl(pmenu)._menu, -1, MF_BYPOSITION, wm, def.label.c_str());
    z::ref(impl)._id = wm;
#endif
#if defined(GTK)
    MenuItem::HandleImpl* impl = new MenuItem::HandleImpl();
    z::ref(impl)._menuItem = gtk_menu_item_new_with_label(def.label.c_str());
    gtk_menu_shell_append (GTK_MENU_SHELL (Menu::impl(pmenu)._menu), z::ref(impl)._menuItem);
#endif

    MenuItem::Handle handle;
    handle._wdata<MenuItem::Handle>(impl);
    return handle;
}

#if defined(GTK)
static void onMenuItemSelectClick(GtkMenuItem* item, gpointer phandler) {
    unused(item);
    MenuItem::OnSelect::Handler* handler = static_cast<MenuItem::OnSelect::Handler*>(phandler);
    MenuItem::OnSelect::Handler::_In in;
    z::ref(handler)._run(in);
}
#endif

void MenuItem::OnSelect::addHandler(const MenuItem::Handle& menuitem, Handler* handler) {
    MenuItem::OnSelect::add(handler);
#if defined(WIN32)
    onMenuItemSelectHandlerList.addHandler(MenuItem::impl(menuitem)._id, handler);
#endif
#if defined(GTK)
    g_signal_connect (G_OBJECT (MenuItem::impl(menuitem)._menuItem), "activate", G_CALLBACK (onMenuItemSelectClick), handler);
#endif
}
