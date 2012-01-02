#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "WindowImpl.hpp"
#include "TextEdit.hpp"

Window::Handle TextEdit::Create::run(const Window::Handle& parent, const TextEdit::Definition& def) {
#if defined(WIN32)
    int flags = WS_VISIBLE|WS_CHILD|WS_BORDER|WS_VSCROLL|WS_HSCROLL|ES_WANTRETURN|ES_AUTOHSCROLL|ES_AUTOVSCROLL;
    if(def.multiline) {
        flags |= ES_MULTILINE;
    }
    Window::HandleImpl& impl = Window::Native::createChildWindow(def, "EDIT", flags, WS_EX_CLIENTEDGE, parent);
#endif
#if defined(GTK)
    GtkWidget* hWnd = 0;
    if(def.multiline) {
        hWnd = gtk_text_view_new();
    } else {
        assert(false);
    }

    Window::HandleImpl& impl = Window::Native::createChildWindow(hWnd, def, parent);
    if(def.title.size() > 0) {
        GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (impl._hWindow));
        gtk_text_buffer_set_text (buffer, def.title.c_str(), -1);
    }
#endif
    Window::Handle win;
    win._wdata<Window::Handle>(z::ptr(impl));
    return win;
}

void TextEdit::AppendText::run(const Window::Handle& window, const std::string& text) {
#if defined(WIN32)
    int len = Edit_GetTextLength(Window::impl(window)._hWindow);
    Edit_SetSel(Window::impl(window)._hWindow, len, len);
    Edit_ReplaceSel(Window::impl(window)._hWindow, text.c_str());
#endif
#if defined(GTK)
    GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (Window::impl(window)._hWindow));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, text.c_str(), text.size());
#endif
}
