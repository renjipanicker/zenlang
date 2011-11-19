#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "textedit.hpp"
#include "windowimpl.hpp"

//#if defined(WIN32)
//static HandlerList<HWND, TextEdit::OnClickHandler> onTextEditClickHandlerList;
//struct WinProc : public Window::Native::WndProc {
//    virtual LRESULT WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
//        switch (message) {
//            case WM_COMMAND:
//                if(LOWORD(wParam) == BN_CLICKED) {
//                    if(onTextEditClickHandlerList.runHandler((HWND)lParam))
//                        return 1;
//                }
//                break;
//        }
//        return 0;
//    }
//};
//static WinProc s_winProc;
//#endif

const TextEdit::Create::_Out& TextEdit::Create::run(const _In& _in) {
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

//#if defined(GTK)
//static void onTextEditClick(GtkMenuItem* item, gpointer phandler) {
//    unused(item);
//    TextEdit::OnClick::Handler* handler = static_cast<TextEdit::OnClick::Handler*>(phandler);
//    TextEdit::OnClick::Handler::_In in;
//    ref(handler).run(in);
//}
//#endif

//void TextEdit::OnClick::addHandler(const Window::Instance& button, Handler* handler) {
//    TextEdit::OnClick::add(handler);
//#if defined(WIN32)
//    Window::Native::addOnTextEditClickHandler(button._impl->_hWindow, handler);
//#endif
//#if defined(GTK)
//    g_signal_connect (G_OBJECT (button._impl->_hWindow), "clicked", G_CALLBACK (onTextEditClick), handler);
//#endif
//}
