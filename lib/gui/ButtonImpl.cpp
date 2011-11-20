#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "Button.hpp"
#include "WindowImpl.hpp"

#if defined(WIN32)
static HandlerList<HWND, Button::OnClickHandler> onButtonClickHandlerList;
struct WinProc : public Window::Native::WndProc {
    virtual LRESULT WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_COMMAND:
                if(LOWORD(wParam) == BN_CLICKED) {
                    if(onButtonClickHandlerList.runHandler((HWND)lParam))
                        return 1;
                }
                break;
        }
        return 0;
    }
};
static WinProc s_winProc;

#endif

const Button::Create::_Out& Button::Create::run(const _In& _in) {
#if defined(WIN32)
    Window::Instance::Impl impl = Window::Native::createChildWindow(_in.def, "BUTTON", BS_PUSHBUTTON|WS_CHILD|WS_VISIBLE, 0, _in.parent);
#endif
#if defined(GTK)
    Window::Instance::Impl impl;
    impl._hWindow = gtk_button_new_with_label(_in.def.title.c_str());
    Window::Native::createChildWindow(impl, _in.def, _in.parent);
#endif
   return out(new _Out(impl));
}

#if defined(GTK)
static void onButtonClick(GtkMenuItem* item, gpointer phandler) {
    unused(item);
    Button::OnClick::Handler* handler = static_cast<Button::OnClick::Handler*>(phandler);
    Button::OnClick::Handler::_In in;
    ref(handler).run(in);
}
#endif

void Button::OnClick::addHandler(const Window::Instance& button, Handler* handler) {
    Button::OnClick::add(handler);
#if defined(WIN32)
    Window::Native::addOnButtonClickHandler(button._impl->_hWindow, handler);
#endif
#if defined(GTK)
    g_signal_connect (G_OBJECT (button._impl->_hWindow), "clicked", G_CALLBACK (onButtonClick), handler);
#endif
}
