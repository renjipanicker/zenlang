#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/TextEdit.hpp"
#include "WindowImpl.hpp"

#if defined(WIN32)
namespace zz {
namespace TextEditImpl {
    static WNDPROC OrigWndProc = 0;
    static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_KEYDOWN: {
                if(wParam == VK_RETURN) {
                    TextEdit::OnEnter::Handler::_In in;
                    TextEdit::OnEnter::list().run(Window::Native::impl(hWnd), in);
                }
                break;
            }
        }
        assert(OrigWndProc);
        return CallWindowProc(OrigWndProc, hWnd, message, wParam, lParam);
    }
}
}
#endif

z::widget TextEdit::Create::run(const z::widget& parent, const TextEdit::Definition& def) {
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
    z::widget::impl& impl = Window::Native::createChildWindow(def, "RICHEDIT50W", flags, 0, parent);

    // set subclass function
    Window::Native::setImpl(impl._val, z::ptr(impl));
    zz::TextEditImpl::OrigWndProc = (WNDPROC)SetWindowLong(impl._val, GWL_WNDPROC, (LONG)zz::TextEditImpl::WinProc);

    // set default font
    HFONT hFont=CreateFont(0,8,0,0,FW_NORMAL,0,0,0,0,0,0,0,0,TEXT("Courier"));
    SendMessage(impl._val,WM_SETFONT,(WPARAM)hFont,0);

#elif defined(GTK)
    GtkWidget* hWnd = 0;
    if(def.multiline) {
        hWnd = gtk_text_view_new();
    } else {
        hWnd = gtk_entry_new();
        gtk_entry_set_has_frame(GTK_ENTRY(hWnd), 1);
    }

    z::widget::impl& impl = Window::Native::createChildWindow(hWnd, def, parent);
    if(def.title.size() > 0) {
        if(def.multiline) {
            GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (impl._val));
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
    z::widget::impl& impl = Window::Native::createChildWindow(def, parent, child);
#elif defined(IOS)
    UNIMPL();
    z::widget::impl& impl = Window::Native::createChildWindow(def, parent);
#else
#error "Unimplemented GUI mode"
#endif
    z::widget win(impl);
    Window::Position p = Window::getChildPosition(win);
    return win;
}

#if defined(OSX)
inline bool isTextField(const z::widget& textedit) {
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

void TextEdit::AppendText::run(const z::widget& wnd, const z::string& text) {
#if defined(WIN32)
    int len = Edit_GetTextLength(wnd.val()._val);
    Edit_SetSel(wnd.val()._val, len, len);
    Edit_ReplaceSel(wnd.val()._val, z::s2e(text).c_str());
    ::SendMessage(wnd.val()._val, EM_SCROLLCARET, 0, 0);
#elif defined(GTK)
    GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (wnd.val()._val));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, z::s2e(text).c_str(), text.size());
#elif defined(OSX)
    if(isTextField(wnd)) {
        UNIMPL();
    } else {
        NSTextView* v = (NSTextView*)(wnd.val()._val);
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

void TextEdit::Clear::run(const z::widget& wnd) {
#if defined(WIN32)
    ::SetWindowText(wnd.val()._val, "");
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

z::string TextEdit::GetText::run(const z::widget& wnd) {
#if defined(WIN32)
    int size = ::GetWindowTextLength(wnd.val()._val);
    char* buf = new char[size+2];
    ::GetWindowText(wnd.val()._val, buf, size+1);
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

void TextEdit::OnEnter::addHandler(const z::widget& textedit, const z::pointer<Handler>& handler) {
#if defined(WIN32)
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
}
