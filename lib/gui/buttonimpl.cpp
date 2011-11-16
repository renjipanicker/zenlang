#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "button.hpp"
#include "windowimpl.hpp"

const Button::Create::_Out& Button::Create::run(const _In& _in) {
#if defined(WIN32)
    Window::Native::createChildWindow(This, "BUTTON", BS_PUSHBUTTON|WS_CHILD|WS_VISIBLE, 0, This._parent);
#endif
#if defined(GTK)
    Window::Instance::Impl impl;
    impl._hWindow = gtk_button_new_with_label(_in.def.title.c_str());
    Window::Native::createChildWindow(impl, _in.def, _in.parent);
#endif
   return out(new _Out(impl));
}
