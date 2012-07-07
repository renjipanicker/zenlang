#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/MenuItem.hpp"
#include "WindowImpl.hpp"
#include "MenuItemImpl.hpp"
#include "MenuImpl.hpp"

#if defined(WIN32)
namespace zz {
namespace MenuItemImpl {
    typedef WindowImpl::WidgetMap<UINT> ItemMap;
    static ItemMap itemMap;

    static WNDPROC OrigWndProc = 0;
    static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_COMMAND:
                z::MenuItem::OnSelect::Handler::_In in;
                z::MenuItem::OnSelect::list().runHandler(itemMap.impl(LOWORD(lParam)), in);
                break;
        }
        assert(OrigWndProc);
        return CallWindowProc(OrigWndProc, hWnd, message, wParam, lParam);
    }
}
}
#endif

z::widget z::MenuItem::Create::run(const z::widget& pmenu, const z::MenuItem::Definition& def) {
#if defined(WIN32)
    z::widget::impl* impl = new z::widget::impl();
    uint32_t wm = WindowImpl::getNextWmID();
    ::InsertMenu(pmenu.val()._menu, (UINT)-1, MF_BYPOSITION, wm, z::s2e(def.label).c_str());
    z::ref(impl)._id = wm;
    // set subclass function
    zz::MenuItemImpl::OrigWndProc = (WNDPROC)SetWindowLong(pmenu.val()._val, GWL_WNDPROC, (LONG)zz::MenuItemImpl::WinProc);
#elif defined(GTK)
    z::widget::impl* impl = new z::widget::impl();
    z::ref(impl)._menuItem = gtk_menu_item_new_with_label(z::s2e(def.label).c_str());
    gtk_menu_shell_append (GTK_MENU_SHELL (pmenu.val()._menu), z::ref(impl)._menuItem);
#elif defined(QT)
    UNIMPL();
    z::widget::impl* impl = new z::widget::impl();
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
    z::MenuItem::OnSelect::Handler* handler = static_cast<z::MenuItem::OnSelect::Handler*>(phandler);
    z::MenuItem::OnSelect::Handler::_In in;
    z::ref(handler)._run(in);
}
#endif

void z::MenuItem::OnSelect::addHandler(const z::widget& menuitem, const z::pointer<Handler>& handler) {
#if defined(WIN32)
#elif defined(GTK)
    g_signal_connect (G_OBJECT (menuitem.val()._menuItem), "activate", G_CALLBACK (onMenuItemSelectClick), z::ptr(handler.get()) );
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
