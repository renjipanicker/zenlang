#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "mainframe.hpp"
#include "windowimpl.hpp"

const MainFrame::Create::_Out& MainFrame::Create::run(const _In& _in) {
#if defined(WIN32)
    Window::Instance::Impl impl = Window::Native::createMainFrame(_in.def, WS_OVERLAPPEDWINDOW, WS_EX_WINDOWEDGE);
#endif
#if defined(GTK)
    Window::Instance::Impl impl = Window::Native::createWindow(_in.def, 0);
    if((_in.def.position.w != -1) && (_in.def.position.h != -1))
        gtk_widget_set_size_request (impl._hWindow, _in.def.position.w, _in.def.position.h);

    impl._hFixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(impl._hWindow), impl._hFixed);
    gtk_widget_show(impl._hFixed);
    if(_in.def.visible) {
        gtk_widget_show(impl._hWindow);
    }
#endif
   return out(new _Out(impl));
}
