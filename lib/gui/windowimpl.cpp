#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "window.hpp"
#include "windowimpl.hpp"

Window::Position Window::getWindowPosition(const Instance& window) {
#if defined(WIN32)
    RECT rc;
    ::GetWindowRect(window._impl->_hWindow, &rc);
    return Window::Position()._x(rc.left)._y(rc.top)._w(rc.right - rc.left)._h(rc.bottom - rc.top);
#endif
#if defined(GTK)
    GtkRequisition req;
    gtk_widget_size_request(window._impl->_hWindow, &req);
    return Window::Position()._x(0)._y(0)._w(req.width)._h(req.height);
#endif
}

Window::Position Window::getChildPosition(const Instance& window) {
#if defined(WIN32)
    RECT rc;
    ::GetClientRect(window._impl->_hWindow, &rc);
    ::MapWindowPoints(window._impl->_hWindow, ::GetParent(window._impl->_hWindow), (LPPOINT) &rc, 2);
    return Window::Position()._x(rc.left)._y(rc.top)._w(rc.right - rc.left)._h(rc.bottom - rc.top);
#endif
#if defined(GTK)
    GtkRequisition req;
    gtk_widget_size_request(window._impl->_hWindow, &req);
    return Window::Position()._x(0)._y(0)._w(req.width)._h(req.height);
#endif
}

const Window::Delete::_Out& Window::Delete::run(const _In& _in) {
    delete _in.window._impl;
   //_in.window._impl = 0;
   return out(new _Out());
}

const Window::SetTitle::_Out& Window::SetTitle::run(const _In& _in) {
#if defined(WIN32)
    ::SetWindowText(_in.window._impl->_hWindow, _in.title.c_str());
#endif
#if defined(GTK)
    gtk_window_set_title (GTK_WINDOW (_in.window._impl->_hWindow), _in.title.c_str());
#endif
   return out(new _Out());
}

const Window::Show::_Out& Window::Show::run(const _In& _in) {
#if defined(WIN32)
    ::ShowWindow(_in.window._impl->_hWindow, SW_SHOW);
#endif
#if defined(GTK)
    gtk_widget_show(GTK_WIDGET(_in.window._impl->_hWindow));
    gtk_window_deiconify(GTK_WINDOW(_in.window._impl->_hWindow));
#endif
   return out(new _Out());
}

const Window::Hide::_Out& Window::Hide::run(const _In& _in) {
#if defined(WIN32)
    ::ShowWindow(_in.window._impl->_hWindow, SW_HIDE);
#endif
#if defined(GTK)
    gtk_widget_hide(GTK_WIDGET(_in.window._impl->_hWindow));
#endif
   return out(new _Out());
}

const Window::Move::_Out& Window::Move::run(const _In& _in) {
#if defined(WIN32)
    ::MoveWindow(_in.window._impl->_hWindow, _in.position.x, _in.position.y, _in.position.w, _in.position.h, TRUE);
#endif
#if defined(GTK)
    unused(_in);
    //gtk_widget_set_uposition(_window._impl->_hWindow, _position.x, _position.y);
    //gtk_window_set_default_size (_window._impl->_hWindow, _position.w, _position.h);
#endif
   return out(new _Out());
}

const Window::Size::_Out& Window::Size::run(const _In& _in) {
#if defined(WIN32)
    RECT rc;
    ::GetWindowRect(_in.window._impl->_hWindow, &rc);
    int w = (_in.w == -1)?(rc.right - rc.left): _in.w;
    int h = (_in.h == -1)?(rc.bottom - rc.top): _in.h;
    ::MoveWindow(_in.window._impl->_hWindow, rc.left, rc.top, w, h, TRUE);
#endif
#if defined(GTK)
    gtk_widget_set_size_request(_in.window._impl->_hWindow, _in.w, _in.h);
#endif
   return out(new _Out());
}

#if defined(GTK)
static gboolean onConfigureEvent(GtkWindow* window, GdkEvent* event, gpointer phandler) {
    unused(window);
    unused(event);
    Window::OnResize::Handler* handler = static_cast<Window::OnResize::Handler*>(phandler);
    Window::OnResize::Handler::_In in;
    ref(handler).run(in);
    return FALSE;
}
#endif

#if defined(WIN32)
static HandlerList<HWND, Invocation> onResizeHandlerList;
#endif

const Window::OnResize::Add::_Out& Window::OnResize::Add::run(const _In& _in) {
    Window::OnResize::Handler& h = Window::OnResize::add(_in.handler);
#if defined(WIN32)
    onResizeHandlerList.addHandler(_in.window._impl->_hWindow, ptr(h));
#endif
#if defined(GTK)
    trace("Window::OnResize::Handler::run(): handler = %lu, window = %d\n", ptr(h), _in.window._impl->_hWindow);
    g_signal_connect (G_OBJECT (_in.window._impl->_hWindow), "configure-event", G_CALLBACK (onConfigureEvent), ptr(h));
#endif
   return out(new _Out());
}

#if defined(GTK)
static gboolean onWindowCloseEvent(GtkWindow* window, gpointer phandler) {
    unused(window);
    trace("onWindowCloseEvent\n");
    Window::OnClose::Handler* handler = static_cast<Window::OnClose::Handler*>(phandler);
    Window::OnClose::Handler::_In in;
    ref(handler).run(in);
    return FALSE;
}
#endif

#if defined(WIN32)
static HandlerList<HWND, Invocation> onCloseHandlerList;
#endif

const Window::OnClose::Add::_Out& Window::OnClose::Add::run(const _In& _in) {
    Window::OnClose::Handler& h = Window::OnClose::add(_in.handler);
#if defined(WIN32)
    onCloseHandlerList.addHandler(_in.window._impl->_hWindow, ptr(h));
#endif
#if defined(GTK)
    g_signal_connect (G_OBJECT (_in.window._impl->_hWindow), "closed", G_CALLBACK (onWindowCloseEvent), ptr(h));
#endif
   return out(new _Out());
}
