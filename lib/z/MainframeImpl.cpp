#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/Mainframe.hpp"
#include "WindowImpl.hpp"

z::MainFrame::Create::_Out z::MainFrame::Create::run(const z::MainFrame::Definition& def) {
#if defined(WIN32)
    z::widget::impl& impl = WindowImpl::createMainFrame(def, WS_OVERLAPPEDWINDOW, WS_EX_WINDOWEDGE);
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
    z::widget::impl& impl = WindowImpl::createWindow(def, 0);
    if((def.position.w != -1) && (def.position.h != -1))
        gtk_widget_set_size_request (impl._val, def.position.w, def.position.h);

    impl._fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(impl._val), impl._fixed);
    gtk_widget_show(impl._fixed);
#elif defined(OSX)
    z::widget::impl& impl = WindowImpl::createMainFrame(def);
#elif defined(QT)
    UNIMPL();
    z::widget::impl& impl = *((z::widget::impl*)(0));
#elif defined(IOS)
    UNIMPL();
    z::widget::impl& impl = WindowImpl::createMainFrame(def);
#else
#error "Unimplemented GUI mode"
#endif
    z::widget win(impl);
#if !defined(OSX)
    if(def.visible) {
        z::Window::Show().run(win);
    }
#endif
    return _Out(win);
}
