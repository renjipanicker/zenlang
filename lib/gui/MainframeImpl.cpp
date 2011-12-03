#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "WindowImpl.hpp"
#include "Mainframe.hpp"

const MainFrame::Create::_Out& MainFrame::Create::run(const MainFrame::Definition& def) {
#if defined(WIN32)
    Window::Instance::Impl& impl = Window::Native::createMainFrame(def, WS_OVERLAPPEDWINDOW, WS_EX_WINDOWEDGE);
    ::PostMessage(impl._hWindow, WM_SIZE, 0, 0);
#endif
#if defined(GTK)
    Window::Instance::Impl& impl = Window::Native::createWindow(def, 0);
    if((def.position.w != -1) && (def.position.h != -1))
        gtk_widget_set_size_request (impl._hWindow, def.position.w, def.position.h);

    printf("mainfrmae: w %d h %d\n", def.position.w, def.position.h);
    impl._hFixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(impl._hWindow), impl._hFixed);
    gtk_widget_show(impl._hFixed);
#endif
    Window::Instance win;
    win._wdata<Window::Instance>(ptr(impl));
    if(def.visible) {
        Window::Show().run(win);
    }

    return out(_Out(win));
}
