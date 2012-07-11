#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/Button.hpp"
#include "WindowImpl.hpp"

#if defined(WIN32)
namespace zz {
namespace ButtonImpl {
    static WNDPROC OrigWndProc = 0;
    static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_COMMAND: {
                if(LOWORD(wParam) == BN_CLICKED) {
                    z::Button::OnClick::Handler::_In in;
                    z::Button::OnClick::list().runHandler(WindowImpl::impl(hWnd), in);
                }
                break;
            }
        }
        assert(OrigWndProc != 0);
        return CallWindowProc(OrigWndProc, hWnd, message, wParam, lParam);
    }
}
}
#endif

z::widget z::Button::Create::run(const z::widget& parent, const z::Button::Definition& def) {
#if defined(WIN32)
    z::widget::impl& impl = WindowImpl::createChildWindow(def, "BUTTON", BS_DEFPUSHBUTTON, 0, parent);
    // set subclass function
    WindowImpl::setImpl(impl._val, z::ptr(impl));
    zz::ButtonImpl::OrigWndProc = (WNDPROC)SetWindowLong(impl._val, GWL_WNDPROC, (LONG)zz::ButtonImpl::WinProc);
#elif defined(GTK)
    GtkWidget* hWnd = gtk_button_new_with_label(z::s2e(def.title).c_str());
    z::widget::impl& impl = WindowImpl::createChildWindow(hWnd, def, parent);
#elif defined(QT)
    UNIMPL();
    z::widget::impl* impl = new z::widget::impl();
#elif defined(OSX)
    NSView* child = 0;
    z::widget::impl& impl = WindowImpl::createChildWindow(def, parent, child);
    UNIMPL();
#elif defined(IOS)
    z::widget::impl& impl = WindowImpl::createChildWindow(def, parent);
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
    z::widget win(impl);
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

void z::Button::OnClick::addHandler(const z::widget& button, const z::pointer<Handler>& handler) {
#if defined(WIN32)
#elif defined(GTK)
    g_signal_connect (G_OBJECT (button.val()._val), "clicked", G_CALLBACK (onButtonClick), z::ptr(handler.get()) );
#elif defined(QT)
    UNIMPL();
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}
