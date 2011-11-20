#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "TextEdit.hpp"
#include "WindowImpl.hpp"

const TextEdit::Create::_Out& TextEdit::Create::run(const _In& _in) {
#if defined(WIN32)
    Window::Instance::Impl impl = Window::Native::createChildWindow(_in.def, "EDIT", WS_CHILD|WS_VISIBLE, WS_EX_CLIENTEDGE, _in.parent);
#endif
#if defined(GTK)
    Window::Instance::Impl impl;
    impl._hWindow = gtk_text_view_new();
    Window::Native::createChildWindow(impl, _in.def, _in.parent);
    if(_in.def.title.size() > 0) {
        GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (impl._hWindow));
        gtk_text_buffer_set_text (buffer, _in.def.title.c_str(), -1);
    }
#endif
   return out(new _Out(impl));
}

const TextEdit::AppendText::_Out& TextEdit::AppendText::run(const _In& _in) {
#if defined(WIN32)
#endif
#if defined(GTK)
    GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (_in.window._impl->_hWindow));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, _in.text.c_str(), _in.text.size());
#endif
    return out(new _Out());
}
