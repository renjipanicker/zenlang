#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/TextEdit.hpp"
#include "WindowImpl.hpp"

#if defined(WIN32)
static z::HandlerList<HWND, TextEdit::OnEnter::Handler> onTextEditEnterHandlerList;
struct WinProc : public Window::Native::WndProc {
    virtual LRESULT handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_COMMAND:
                if(LOWORD(wParam) == BN_CLICKED) {
                    TextEdit::OnEnter::Handler::_In in;
                    if(onTextEditEnterHandlerList.runHandler((HWND)lParam, in))
                        return 1;
                }
                break;
        }
        return 0;
    }
};
static WinProc s_winProc;
#endif

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
        hWnd = gtk_entry_new();
        gtk_entry_set_has_frame(GTK_ENTRY(hWnd), 1);
    }

    Window::HandleImpl& impl = Window::Native::createChildWindow(hWnd, def, parent);
    if(def.title.size() > 0) {
        if(def.multiline) {
            GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (impl._hWindow));
            gtk_text_buffer_set_text (buffer, def.title.c_str(), -1);
        } else {
        }
    }
#endif
    Window::Handle win;
    win._wdata<Window::Handle>(z::ptr(impl));
    return win;
}

void TextEdit::AppendText::run(const Window::Handle& window, const z::string& text) {
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

#if defined(GTK)
static void onEnterPressed(GtkMenuItem* item, gpointer phandler) {
    unused(item);
    TextEdit::OnEnter::Handler* handler = static_cast<TextEdit::OnEnter::Handler*>(phandler);
    TextEdit::OnEnter::Handler::_In in;
    z::ref(handler)._run(in);
}
#endif

void TextEdit::OnEnter::addHandler(const Window::Handle& textedit, Handler* handler) {
    TextEdit::OnEnter::add(handler);
#if defined(WIN32)
    onTextEditEnterHandlerList.addHandler(Window::impl(textedit)._hWindow, handler);
#endif
#if defined(GTK)
    g_signal_connect (G_OBJECT (Window::impl(textedit)._hWindow), "activate", G_CALLBACK (onEnterPressed), handler);
#endif
}
