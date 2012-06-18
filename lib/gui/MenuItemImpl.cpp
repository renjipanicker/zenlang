#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/MenuItem.hpp"
#include "WindowImpl.hpp"
#include "MenuItemImpl.hpp"
#include "MenuImpl.hpp"

#if defined(WIN32)
namespace zz {
namespace MenuItemImpl {
    typedef Window::Native::WidgetMap<UINT> ItemMap;
    static ItemMap itemMap;

    static WNDPROC OrigWndProc = 0;
    static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_COMMAND:
                MenuItem::OnSelect::Handler::_In in;
                MenuItem::OnSelect::list().run(itemMap.impl(LOWORD(lParam)), in);
                break;
        }
        assert(OrigWndProc);
        return CallWindowProc(OrigWndProc, hWnd, message, wParam, lParam);
    }
}
}
#endif

z::widget MenuItem::Create::run(const z::widget& pmenu, const MenuItem::Definition& def) {
#if defined(WIN32)
    z::widget::impl* impl = new z::widget::impl();
    uint32_t wm = Window::Native::getNextWmID();
    ::InsertMenu(pmenu.val()._menu, (UINT)-1, MF_BYPOSITION, wm, z::s2e(def.label).c_str());
    z::ref(impl)._id = wm;
    // set subclass function
    zz::MenuItemImpl::OrigWndProc = (WNDPROC)SetWindowLong(pmenu.val()._val, GWL_WNDPROC, (LONG)zz::MenuItemImpl::WinProc);
#elif defined(GTK)
    z::widget::impl* impl = new z::widget::impl();
    z::ref(impl)._menuItem = gtk_menu_item_new_with_label(z::s2e(def.label).c_str());
    gtk_menu_shell_append (GTK_MENU_SHELL (Menu::impl(pmenu)._menu), z::ref(impl)._menuItem);
#elif defined(OSX)
    UNIMPL();
    z::widget::impl* impl = new z::widget::impl();
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
static void onMenuItemSelectClick(GtkMenuItem* item, gpointer phandler) {
    unused(item);
    MenuItem::OnSelect::Handler* handler = static_cast<MenuItem::OnSelect::Handler*>(phandler);
    MenuItem::OnSelect::Handler::_In in;
    z::ref(handler)._run(in);
}
#endif

MenuItem::OnSelect::Handler& MenuItem::OnSelect::addHandler(const z::widget& menuitem, Handler* handler) {
//@    MenuItem::OnSelect::add(handler);
//@    MenuItem::OnSelect::list().add(menuitem, handler);
#if defined(WIN32)
//@    MenuItemImpl::onMenuItemSelectHandlerList.addHandler(MenuItem::impl(menuitem)._id, handler);
#elif defined(GTK)
    g_signal_connect (G_OBJECT (MenuItem::impl(menuitem)._menuItem), "activate", G_CALLBACK (onMenuItemSelectClick), handler);
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
    return z::ref(handler);
}
