#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "TextEdit.hpp"
#include "WindowImpl.hpp"

const TextEdit::Create::_Out& TextEdit::Create::run(const _In& _in) {
#if defined(WIN32)
    int flags = WS_VISIBLE|WS_CHILD|WS_BORDER|WS_VSCROLL|WS_HSCROLL|ES_WANTRETURN|ES_AUTOHSCROLL|ES_AUTOVSCROLL;
    if(_in.def.multiline) {
        flags |= ES_MULTILINE;
    }
    Window::Instance::Impl impl = Window::Native::createChildWindow(_in.def, "EDIT", flags, WS_EX_CLIENTEDGE, _in.parent);
#endif
#if defined(GTK)
    Window::Instance::Impl impl;
    if(_in.def.multiline) {
        impl._hWindow = gtk_text_view_new();
    } else {
        assert(false);
    }
    Window::Native::createChildWindow(impl, _in.def, _in.parent);
    if(_in.def.title.size() > 0) {
        GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (impl._hWindow));
        gtk_text_buffer_set_text (buffer, _in.def.title.c_str(), -1);
    }
#endif
   return out(_Out(impl));
}

const TextEdit::AppendText::_Out& TextEdit::AppendText::run(const _In& _in) {
#if defined(WIN32)
    int len = Edit_GetTextLength(_in.window._impl->_hWindow);
    Edit_SetSel(_in.window._impl->_hWindow, len, len);
    Edit_ReplaceSel(_in.window._impl->_hWindow, _in.text.c_str());
#endif
#if defined(GTK)
    GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (_in.window._impl->_hWindow));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, _in.text.c_str(), _in.text.size());
#endif
    return out(_Out());
}
