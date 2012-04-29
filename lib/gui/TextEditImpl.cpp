#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/TextEdit.hpp"
#include "WindowImpl.hpp"

#if defined(WIN32)
namespace TextEditImpl {
    static z::HandlerList<HWND, TextEdit::OnEnter::Handler> onTextEditEnterHandlerList;
    static WNDPROC OrigWndProc = 0;
    static LRESULT CALLBACK TextWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_KEYDOWN:
                void* p = (void*)::GetWindowLongPtr(hWnd, GWL_USERDATA);
                Window::HandleImpl* impl = reinterpret_cast<Window::HandleImpl*>(p);
                unused(impl); /// \todo Use this later when implementing read-only
                if(wParam == VK_RETURN) {
                    TextEdit::OnEnter::Handler::_In in;
                    onTextEditEnterHandlerList.runHandler(hWnd, in);
                }
                break;
        }
        assert(OrigWndProc);
        return CallWindowProc(OrigWndProc, hWnd, message, wParam, lParam);
    }
}
#endif

Window::Handle TextEdit::Create::run(const Window::Handle& parent, const TextEdit::Definition& def) {
#if defined(WIN32)
    int flags = ES_AUTOVSCROLL;
    if(def.multiline) {
        flags |= ES_MULTILINE;
    }
    if(def.readonly) {
        flags |= ES_READONLY;
    }
    if(!def.wordwrap) {
        flags |= ES_AUTOHSCROLL;
    }

    // create the window handle
    Window::HandleImpl& impl = Window::Native::createChildWindow(def, "EDIT", flags, WS_EX_CLIENTEDGE, parent);

    // set default font
    HFONT hFont=CreateFont(0,8,0,0,FW_NORMAL,0,0,0,0,0,0,0,0,TEXT("Courier"));
    if(hFont == NULL) {
        hFont = CreateFont(0,8,0,0,FW_NORMAL,0,0,0,0,0,0,0,0,TEXT("Verdana"));
    }
    SendMessage(impl._hWindow,WM_SETFONT,(WPARAM)hFont,0);
    HDC hdc=GetDC(impl._hWindow);
    SelectObject(hdc, hFont);
    SIZE tmpSize;
    GetTextExtentPoint32(hdc,"00000000",8,&tmpSize);
    ReleaseDC(impl._hWindow, hdc);

    // set subclass function
    TextEditImpl::OrigWndProc = (WNDPROC)SetWindowLong(impl._hWindow, GWL_WNDPROC, (LONG)TextEditImpl::TextWinProc);
#elif defined(GTK)
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
            gtk_text_buffer_set_text (buffer, z::s2e(def.title).c_str(), -1);
        } else {
        }
    }
#else
#error "Unimplemented GUI mode"
#endif
    Window::Handle win;
    win._wdata<Window::Handle>(z::ptr(impl));
    Window::Position p = Window::getChildPosition(win);
    return win;
}

void TextEdit::AppendText::run(const Window::Handle& window, const z::string& text) {
#if defined(WIN32)
    int len = Edit_GetTextLength(Window::impl(window)._hWindow);
    Edit_SetSel(Window::impl(window)._hWindow, len, len);
    Edit_ReplaceSel(Window::impl(window)._hWindow, z::s2e(text).c_str());
#elif defined(GTK)
    GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (Window::impl(window)._hWindow));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, z::s2e(text).c_str(), text.size());
#else
#error "Unimplemented GUI mode"
#endif
}

void TextEdit::Clear::run(const Window::Handle& window) {
#if defined(WIN32)
    ::SetWindowText(Window::impl(window)._hWindow, "");
#elif defined(GTK)
    assert(false);
#else
#error "Unimplemented GUI mode"
#endif
}

z::string TextEdit::GetText::run(const Window::Handle& window) {
#if defined(WIN32)
    int size = ::GetWindowTextLength(Window::impl(window)._hWindow);
    char* buf = new char[size+2];
    ::GetWindowText(Window::impl(window)._hWindow, buf, size+1);
    z::string val(buf);
    delete[] buf;
#elif defined(GTK)
    assert(false);
#else
#error "Unimplemented GUI mode"
#endif
    return val;
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
    TextEditImpl::onTextEditEnterHandlerList.addHandler(Window::impl(textedit)._hWindow, handler);
#elif defined(GTK)
    g_signal_connect (G_OBJECT (Window::impl(textedit)._hWindow), "activate", G_CALLBACK (onEnterPressed), handler);
#else
#error "Unimplemented GUI mode"
#endif
}
