#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "WindowImpl.hpp"
#include "MenuItemImpl.hpp"
#include "MenuItem.hpp"
#include "MenuImpl.hpp"

#if defined(WIN32)
static HandlerList<HWND, Button::OnClick::Handler> onMenuItemSelectHandlerList;
struct WinProc : public Window::Native::WndProc {
    virtual LRESULT handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_COMMAND:
                Button::OnClick::Handler::_In in;
                if(onMenuItemSelectHandlerList.runHandler(LOWORD(Param), in))
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
    MenuItem::Handle::Impl* impl = new MenuItem::Handle::Impl();
    int wm = Window::Native::getNextWmID();
    ::InsertMenu(ref(pmenu.wdata)._menu, -1, MF_BYPOSITION, wm, def.label.c_str());
    ref(impl)._id = wm;
#endif
#if defined(GTK)
    MenuItem::Handle::Impl* impl = new MenuItem::Handle::Impl();
    ref(impl)._menuItem = gtk_menu_item_new_with_label(def.label.c_str());
    gtk_menu_shell_append (GTK_MENU_SHELL (ref(pmenu.wdata)._menu), ref(impl)._menuItem);
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
    ref(handler)._run(in);
}
#endif

void MenuItem::OnSelect::addHandler(const MenuItem::Handle& menuitem, Handler* handler) {
    MenuItem::OnSelect::add(handler);
#if defined(WIN32)
    onMenuItemSelectHandlerList.addHandler(menuitem.wdata->_id, handler);
#endif
#if defined(GTK)
    g_signal_connect (G_OBJECT (menuitem.wdata->_menuItem), "clicked", G_CALLBACK (onMenuItemSelectClick), handler);
#endif
}
