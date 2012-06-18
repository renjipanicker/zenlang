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
                {
                    //void* p = (void*)::GetWindowLongPtr(hWnd, GWL_USERDATA);
                    //Window::HandleImpl* impl = reinterpret_cast<Window::HandleImpl*>(p);
                    //unused(impl); /// \todo Use this later when implementing read-only
                    if(wParam == VK_RETURN) {
                        TextEdit::OnEnter::Handler::_In in;
                        onTextEditEnterHandlerList.runHandler(hWnd, in);
                    }
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
    int flags = ES_AUTOVSCROLL | WS_VSCROLL;
    if(def.multiline) {
        flags |= ES_MULTILINE;
    }
    if(def.readonly) {
        flags |= ES_READONLY;
    }
    if(!def.wordwrap) {
        flags |= ES_AUTOHSCROLL;
    }

    // create the wnd handle.
    // Use richedit because plan edit has ugly gray background in read-only mode that cannot be easily changed.
    Window::HandleImpl& impl = Window::Native::createChildWindow(def, "RICHEDIT50W", flags, 0, parent);

    // set default font
    HFONT hFont=CreateFont(0,8,0,0,FW_NORMAL,0,0,0,0,0,0,0,0,TEXT("Courier"));
    SendMessage(impl._hWindow,WM_SETFONT,(WPARAM)hFont,0);

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
#elif defined(OSX)
    NSView* child = 0;
    if(def.multiline) {
        child = [NSTextView alloc];
    } else {
        child = [NSTextField alloc];
    }
    Window::HandleImpl& impl = Window::Native::createChildWindow(def, parent, child);
#elif defined(IOS)
    UNIMPL();
    Window::HandleImpl& impl = Window::Native::createChildWindow(def, parent);
#else
#error "Unimplemented GUI mode"
#endif
    Window::Handle win;
    win._wdata<Window::Handle>(z::ptr(impl));
    Window::Position p = Window::getChildPosition(win);
    return win;
}

#if defined(OSX)
inline bool isTextField(const Window::Handle& textedit) {
    // verify it is a NSTextField
    bool chk = [Window::impl(textedit)._hWindow isMemberOfClass:[NSTextField class]];
    if(chk) {
        return true;
    }

    // verify it is a NSTextView
    chk = [Window::impl(textedit)._hWindow isMemberOfClass:[NSTextView class]];
    if(chk) {
        return false;
    }

    z::string className = ""; /// \todo: get class name
    throw z::Exception("TextEdit", z::string("Instance is not a NSTextField or NSTextView: %{s}").arg("s", className));
}
#endif

void TextEdit::AppendText::run(const Window::Handle& wnd, const z::string& text) {
#if defined(WIN32)
    int len = Edit_GetTextLength(Window::impl(wnd)._hWindow);
    Edit_SetSel(Window::impl(wnd)._hWindow, len, len);
    Edit_ReplaceSel(Window::impl(wnd)._hWindow, z::s2e(text).c_str());
    ::SendMessage(Window::impl(wnd)._hWindow, EM_SCROLLCARET, 0, 0);
#elif defined(GTK)
    GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (Window::impl(wnd)._hWindow));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, z::s2e(text).c_str(), text.size());
#elif defined(OSX)
    if(isTextField(wnd)) {
        UNIMPL();
    } else {
        NSTextView* v = (NSTextView*)(Window::impl(wnd)._hWindow);
//        NSString* ntxt = [NSString stringWithUTF8String:z::s2e(text).c_str()];
        NSString* ntxt = [NSString stringWithUTF8String:""];
        [[[v textStorage] mutableString] appendString:ntxt];
    }
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void TextEdit::Clear::run(const Window::Handle& wnd) {
#if defined(WIN32)
    ::SetWindowText(Window::impl(wnd)._hWindow, "");
#elif defined(GTK)
    UNIMPL();
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

z::string TextEdit::GetText::run(const Window::Handle& wnd) {
#if defined(WIN32)
    int size = ::GetWindowTextLength(Window::impl(wnd)._hWindow);
    char* buf = new char[size+2];
    ::GetWindowText(Window::impl(wnd)._hWindow, buf, size+1);
    z::string val(buf);
    delete[] buf;
#elif defined(GTK)
    UNIMPL();
    z::string val;
#elif defined(OSX)
    UNIMPL();
    z::string val;
#elif defined(IOS)
    UNIMPL();
    z::string val;
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
#elif defined(OSX)
@interface OnEnterHandler : NSObject
@end
@implementation OnEnterHandler
-(IBAction)OnEnter:(id)sender {
    UNIMPL();
}
@end

#endif

TextEdit::OnEnter::Handler& TextEdit::OnEnter::addHandler(const Window::Handle& textedit, Handler* handler) {
//    TextEdit::OnEnter::add(handler);
#if defined(WIN32)
    TextEditImpl::onTextEditEnterHandlerList.addHandler(Window::impl(textedit)._hWindow, handler);
#elif defined(GTK)
    g_signal_connect (G_OBJECT (Window::impl(textedit)._hWindow), "activate", G_CALLBACK (onEnterPressed), handler);
#elif defined(OSX)
    OnEnterHandler* h = [OnEnterHandler alloc];
    if(isTextField(textedit)) {
        NSTextField* v = (NSTextField*)(Window::impl(textedit)._hWindow);
        // Set handler
        NSNotificationCenter* nc = [NSNotificationCenter defaultCenter];
        [nc addObserver:h selector:@selector(OnEnter:) name:NSTextDidEndEditingNotification object:v];
    } else {
        UNIMPL();
    }
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
    return z::ref(handler);
}
