#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "mainframe.hpp"
#include "windowimpl.hpp"

const MainFrame::Create::_Out& MainFrame::Create::run(const _In& _in) {
#if defined(WIN32)
    Window::Native::createMainFrame(This, WS_OVERLAPPEDWINDOW, WS_EX_WINDOWEDGE);
#endif
#if defined(GTK)
    Window::Instance::Impl impl = Window::Native::createWindow(_in.def, 0);
    if((This._def._position.w != -1) && (This._def._position.h != -1))
        gtk_widget_set_size_request (This._window._impl->_hWindow, This._def._position.w, This._def._position.h);

    This._window._impl->_hFixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(This._window._impl->_hWindow), This._window._impl->_hFixed);
    gtk_widget_show(This._window._impl->_hFixed);
#endif
   return out(new _Out());
}
