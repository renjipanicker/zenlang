#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "WindowImpl.hpp"
#include "MenuItemImpl.hpp"
#include "MenuItem.hpp"
#include "MenuImpl.hpp"

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
