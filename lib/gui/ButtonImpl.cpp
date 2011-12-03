#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "WindowImpl.hpp"
#include "Button.hpp"

#if defined(WIN32)
static HandlerList<HWND, Button::OnClick::Handler> onButtonClickHandlerList;
struct WinProc : public Window::Native::WndProc {
    virtual LRESULT handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_COMMAND:
                if(LOWORD(wParam) == BN_CLICKED) {
                    Button::OnClick::Handler::_In in;
                    if(onButtonClickHandlerList.runHandler((HWND)lParam, in))
                        return 1;
                }
                break;
        }
        return 0;
    }
};
static WinProc s_winProc;

#endif

const Button::Create::_Out& Button::Create::run(const Window::Instance& parent, const Button::Definition& def) {
#if defined(WIN32)
    Window::Instance::Impl& impl = Window::Native::createChildWindow(def, "BUTTON", BS_DEFPUSHBUTTON|WS_CHILD|WS_VISIBLE, 0, parent);
#endif
#if defined(GTK)
    GtkWidget* hWnd = gtk_button_new_with_label(def.title.c_str());
    Window::Instance::Impl& impl = Window::Native::createChildWindow(hWnd, def, parent);
#endif
    Window::Instance win;
    win._wdata<Window::Instance>(ptr(impl));
    return out(_Out(win));
}

#if defined(GTK)
static void onButtonClick(GtkMenuItem* item, gpointer phandler) {
    unused(item);
    Button::OnClick::Handler* handler = static_cast<Button::OnClick::Handler*>(phandler);
    Button::OnClick::Handler::_In in;
    ref(handler)._run(in);
}
#endif

void Button::OnClick::addHandler(const Window::Instance& button, Handler* handler) {
    Button::OnClick::add(handler);
#if defined(WIN32)
    onButtonClickHandlerList.addHandler(button.wdata->_hWindow, handler);
#endif
#if defined(GTK)
    g_signal_connect (G_OBJECT (button.wdata->_hWindow), "clicked", G_CALLBACK (onButtonClick), handler);
#endif
}
