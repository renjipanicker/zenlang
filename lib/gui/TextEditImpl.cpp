#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "WindowImpl.hpp"
#include "TextEdit.hpp"

const TextEdit::Create::_Out& TextEdit::Create::run(const Window::Instance& parent, const TextEdit::Definition& def) {
#if defined(WIN32)
    int flags = WS_VISIBLE|WS_CHILD|WS_BORDER|WS_VSCROLL|WS_HSCROLL|ES_WANTRETURN|ES_AUTOHSCROLL|ES_AUTOVSCROLL;
    if(def.multiline) {
        flags |= ES_MULTILINE;
    }
    Window::Instance::Impl& impl = Window::Native::createChildWindow(def, "EDIT", flags, WS_EX_CLIENTEDGE, parent);
#endif
#if defined(GTK)
    GtkWidget* hWnd = 0;
    if(def.multiline) {
        hWnd = gtk_text_view_new();
    } else {
        assert(false);
    }

    Window::Instance::Impl& impl = Window::Native::createChildWindow(hWnd, def, parent);
    if(def.title.size() > 0) {
        GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (impl._hWindow));
        gtk_text_buffer_set_text (buffer, def.title.c_str(), -1);
    }
#endif
    Window::Instance win;
    win._wdata<Window::Instance>(ptr(impl));
    return out(_Out(win));
}

const TextEdit::AppendText::_Out& TextEdit::AppendText::run(const Window::Instance& window, const std::string& text) {
#if defined(WIN32)
    int len = Edit_GetTextLength(window._impl->_hWindow);
    Edit_SetSel(window._impl->_hWindow, len, len);
    Edit_ReplaceSel(window._impl->_hWindow, text.c_str());
#endif
#if defined(GTK)
    GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (window.wdata->_hWindow));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, text.c_str(), text.size());
#endif
    return out(_Out());
}
