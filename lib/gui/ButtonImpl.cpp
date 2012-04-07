#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/Button.hpp"
#include "WindowImpl.hpp"

#if defined(WIN32)
namespace ButtonImpl {
    static z::HandlerList<HWND, Button::OnClick::Handler> onButtonClickHandlerList;
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
}
#endif

Window::Handle Button::Create::run(const Window::Handle& parent, const Button::Definition& def) {
#if defined(WIN32)
    Window::HandleImpl& impl = Window::Native::createChildWindow(def, "BUTTON", BS_DEFPUSHBUTTON|WS_CHILD|WS_VISIBLE, 0, parent);
#endif
#if defined(GTK)
    GtkWidget* hWnd = gtk_button_new_with_label(z::s2e(def.title).c_str());
    Window::HandleImpl& impl = Window::Native::createChildWindow(hWnd, def, parent);
#endif
    Window::Handle win;
    win._wdata<Window::Handle>(z::ptr(impl));
    return win;
}

#if defined(GTK)
static void onButtonClick(GtkMenuItem* item, gpointer phandler) {
    unused(item);
    Button::OnClick::Handler* handler = static_cast<Button::OnClick::Handler*>(phandler);
    Button::OnClick::Handler::_In in;
    z::ref(handler)._run(in);
}
#endif

void Button::OnClick::addHandler(const Window::Handle& button, Handler* handler) {
    Button::OnClick::add(handler);
#if defined(WIN32)
    ButtonImpl::onButtonClickHandlerList.addHandler(Window::impl(button)._hWindow, handler);
#endif
#if defined(GTK)
    g_signal_connect (G_OBJECT (Window::impl(button)._hWindow), "clicked", G_CALLBACK (onButtonClick), handler);
#endif
}
