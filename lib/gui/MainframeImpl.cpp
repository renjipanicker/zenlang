#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/Mainframe.hpp"
#include "WindowImpl.hpp"

MainFrame::Create::_Out MainFrame::Create::run(const MainFrame::Definition& def) {
#if defined(WIN32)
    z::widget::impl& impl = Window::Native::createMainFrame(def, WS_OVERLAPPEDWINDOW, WS_EX_WINDOWEDGE);
    ::PostMessage(impl._val, WM_SIZE, 0, 0);
    if(def.iconres.length() > 0) {
        z::string resname = "IDI_" + def.iconres;
        HICON hIcon = ::LoadIcon(::GetModuleHandle(NULL), z::s2e(resname).c_str());
        if(hIcon == 0) {
            z::elog("MainFrame", "Error loading icon: " + resname);
        }
        ::SendMessage(impl._val, WM_SETICON, ICON_BIG, (LPARAM)(hIcon));
        ::SendMessage(impl._val, WM_SETICON, ICON_SMALL, (LPARAM)(hIcon));
    }
#elif defined(GTK)
    z::widget::impl& impl = Window::Native::createWindow(def, 0);
    if((def.position.w != -1) && (def.position.h != -1))
        gtk_widget_set_size_request (impl._val, def.position.w, def.position.h);

    impl._hFixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(impl._val), impl._hFixed);
    gtk_widget_show(impl._hFixed);
#elif defined(OSX)
    z::widget::impl& impl = Window::Native::createMainFrame(def);
#elif defined(IOS)
    UNIMPL();
    z::widget::impl& impl = Window::Native::createMainFrame(def);
#else
#error "Unimplemented GUI mode"
#endif
    z::widget win(impl);
#if !defined(OSX)
    if(def.visible) {
        Window::Show().run(win);
    }
#endif
    return _Out(win);
}
