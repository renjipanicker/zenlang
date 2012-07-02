#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/Window.hpp"
#include "gui/Button.hpp"
#include "WindowImpl.hpp"

#if defined(WIN32)
uint32_t Window::Native::getNextWmID() {
    static uint32_t lastWM = WM_APP;
    return lastWM++;
}

uint32_t Window::Native::getNextResID() {
    static uint32_t lastRes = 1000;
    return lastRes++;
}

namespace zz {
    static z::string getNextClassID() {
        static int lastclassId = 1;
        return z::string("zenclass_%{d}").arg("d", lastclassId++);
    }

    ULONGLONG GetDllVersion(LPCTSTR lpszDllName) {
        ULONGLONG ullVersion = 0;
        HINSTANCE hinstDll;
        hinstDll = LoadLibrary(lpszDllName);
        if(hinstDll) {
            DLLGETVERSIONPROC pDllGetVersion;
            pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");
            if(pDllGetVersion)
            {
                DLLVERSIONINFO dvi;
                HRESULT hr;
                ZeroMemory(&dvi, sizeof(dvi));
                dvi.cbSize = sizeof(dvi);
                hr = (*pDllGetVersion)(&dvi);
                if(SUCCEEDED(hr))
                    ullVersion = MAKEDLLVERULL(dvi.dwMajorVersion, dvi.dwMinorVersion,0,0);
            }
            FreeLibrary(hinstDll);
        }
        return ullVersion;
    }

    static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        if(message == WM_NCCREATE) {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
            void* p = z::ref(pcs).lpCreateParams;
            z::widget::impl* impl = reinterpret_cast<z::widget::impl*>(p);
            Window::Native::setImpl(hWnd, impl);
        }
        switch (message) {
            case WM_SIZE: {
                Window::OnResize::Handler::_In in;
                Window::OnResize::list().runHandler(Window::Native::impl(hWnd), in);
                break;
            }

            case WM_CLOSE: {
                Window::OnClose::Handler::_In in;
                Window::OnClose::list().runHandler(Window::Native::impl(hWnd), in);
                break;
            }
        }

        return ::DefWindowProc(hWnd, message, wParam, lParam);
    }

    z::string registerClass(HBRUSH bg) {
        z::string className = getNextClassID();
        z::estring eclassName = z::s2e(className);
        WNDCLASSEX wcx = {0};

        // Fill in the wnd class structure with parameters
        // that describe the main wnd.

        wcx.cbSize = sizeof(wcx);          // size of structure
        wcx.style = CS_HREDRAW | CS_VREDRAW;                    // redraw if size changes
        wcx.lpfnWndProc = zz::WinProc;     // points to wnd procedure
        wcx.cbClsExtra = 0;                // no extra class memory
        wcx.cbWndExtra = sizeof(z::widget*);        // store wnd data
        wcx.hInstance = z::app().instance();             // handle to Handle
        wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);     // predefined app. icon
        wcx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);   // predefined app. icon
        wcx.hCursor = LoadCursor(NULL, IDC_ARROW);       // predefined arrow
        wcx.hbrBackground = bg;
        wcx.lpszMenuName =  _T("MainMenu");    // name of menu resource
        wcx.lpszClassName = eclassName.c_str();  // name of wnd class

        // Register the wnd class.
        if(!::RegisterClassEx(&wcx)) {
            throw z::Exception("Window", z::string("Unable to register class %{s}: %{e}").arg("s", className).arg("e", ::GetLastError()));
        }

        return className;
    }
} // namespace zz
#endif

#if defined(WIN32)
z::widget::impl& Window::Native::createWindow(const Window::Definition& def, const z::string& className, int style, int xstyle, HWND parent) {
    Position pos = Position()
            ._x<Position>(CW_USEDEFAULT)
            ._y<Position>(CW_USEDEFAULT)
            ._w<Position>(CW_USEDEFAULT)
            ._h<Position>(CW_USEDEFAULT);

    if(def.position.x != -1)
        pos.x = def.position.x;
    if(def.position.y != -1)
        pos.y = def.position.y;
    if(def.position.w != -1)
        pos.w = def.position.w;
    if(def.position.h != -1)
        pos.h = def.position.h;

    z::widget::impl* impl = new z::widget::impl();
    z::ref(impl)._val = ::CreateWindowEx(xstyle,
                                     z::s2e(className).c_str(),
                                     z::s2e(def.title).c_str(),
                                     style,
                                     pos.x, pos.y, pos.w, pos.h,
                                     parent, (HMENU)NULL,
                                     z::app().instance(), (LPVOID)impl);
    if(z::ref(impl)._val == NULL) {
        throw z::Exception("Window", z::string("Unable to create wnd of class %{s}: %{e}").arg("s", className).arg("e", ::GetLastError()));
    }

    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
    HFONT hFont = ::CreateFontIndirect(&ncm.lfMessageFont);
    ::SendMessage(z::ref(impl)._val, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0)); 
    return z::ref(impl);
}

z::widget::impl& Window::Native::createMainFrame(const Window::Definition& def, int style, int xstyle) {
    HBRUSH brush = (def.style == Window::Style::Dialog)?(HBRUSH)GetSysColorBrush(COLOR_3DFACE):(HBRUSH)GetStockObject(WHITE_BRUSH);
    z::string className = zz::registerClass(brush);
    return createWindow(def, className, style, xstyle, (HWND)NULL);
}

z::widget::impl& Window::Native::createChildFrame(const Window::Definition& def, int style, int xstyle, const z::widget &parent) {
    HBRUSH brush = (def.style == Window::Style::Dialog)?(HBRUSH)GetSysColorBrush(COLOR_3DFACE):(HBRUSH)GetStockObject(WHITE_BRUSH);
    z::string className = zz::registerClass(brush);
    return createWindow(def, className, style, xstyle, parent.val()._val);
}

z::widget::impl& Window::Native::createChildWindow(const Window::Definition& def, const z::string& className, int style, int xstyle, const z::widget& parent) {
    style |= WS_CHILD;
    if(def.border == 1) {
        style |= WS_BORDER;
        xstyle |= WS_EX_CLIENTEDGE;
    }
    if(def.visible) {
        style |= WS_VISIBLE;
    }

    return createWindow(def, className, style, xstyle, parent.val()._val);
}
#elif defined(GTK)
z::widget::impl& Window::Native::initWindowImpl(GtkWidget* hwnd) {
    z::widget::impl* impl = new z::widget::impl();
    z::ref(impl)._val = hwnd;
    z::ref(impl)._hFixed = 0;
    g_object_set_data(G_OBJECT(z::ref(impl)._val), "impl", impl);
    return z::ref(impl);
}

z::widget::impl& Window::Native::createWindow(const Window::Definition& def, GtkWidget *parent) {
    unused(def);
    unused(parent);
    GtkWidget* hwnd = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    z::widget::impl& impl = initWindowImpl(hwnd);
    return impl;
}

z::widget::impl& Window::Native::createChildWindow(GtkWidget* hwnd, const Window::Definition& def, const z::widget& parent) {
    gtk_fixed_put (GTK_FIXED (parent.val()._hFixed), hwnd, def.position.x, def.position.y);
    z::widget::impl& impl = initWindowImpl(hwnd);
    gtk_widget_show(impl._val);
    return impl;
}
#elif defined(OSX)
z::widget::impl& Window::Native::createMainFrame(const Window::Definition& def) {
    Position pos = Position();
    if(def.position.x != -1)
        pos.x = def.position.x;
    if(def.position.y != -1)
        pos.y = def.position.y;
    if(def.position.w != -1)
        pos.w = def.position.w;
    if(def.position.h != -1)
        pos.h = def.position.h;

    z::widget::impl* impl = new z::widget::impl();
    NSRect windowRect = NSMakeRect(pos.x, pos.y, pos.w, pos.h);

    NSWindow* w = [[NSWindow alloc] initWithContentRect:NSMakeRect(pos.x, pos.y, pos.w, pos.h)
                    styleMask:(NSResizableWindowMask|NSClosableWindowMask|NSTitledWindowMask)
                    backing:NSBackingStoreBuffered
                    defer:false];

    NSView<NSWindowDelegate>* v = [[NSView<NSWindowDelegate> alloc] initWithFrame:windowRect];
    [w setContentView:v];
    [w setDelegate:v];
    [w makeKeyAndOrderFront:nil];
    [w setLevel:3];

    z::ref(impl)._frame = w;
    z::ref(impl)._val = v;

    if(z::ref(impl)._val == 0) {
        throw z::Exception("Window", z::string("Unable to create main wnd"));
    }
    return z::ref(impl);
}

z::widget::impl& Window::Native::createChildWindow(const Window::Definition& def, const z::widget& parent, NSView* child) {
    if(child == 0) {
        throw z::Exception("Window", z::string("Unable to create child wnd"));
    }
    z::widget::impl* impl = new z::widget::impl();
    z::ref(impl)._val = child;
//    [parent.val()._val addSubView:child];
    return z::ref(impl);
}
#elif defined(IOS)
z::widget::impl& Window::Native::createMainFrame(const Window::Definition& def) {
    throw z::Exception("Window", z::string("NotImplemented: createMainFrame()"));
}

z::widget::impl& Window::Native::createChildWindow(const Window::Definition& def, const z::widget& parent) {
    throw z::Exception("Window", z::string("NotImplemented: createChildFrame()"));
}
#else
#error "Unimplemented GUI mode"
#endif

////////////////////////////////////////////////////////////////////////////////
Window::Position Window::getWindowPosition(const z::widget& wnd) {
#if defined(WIN32)
    RECT rc;
    ::GetWindowRect(wnd.val()._val, &rc);
    const Window::Position pos = Window::Position()
            ._x<Window::Position>(rc.left)
            ._y<Window::Position>(rc.top)
            ._w<Window::Position>(rc.right - rc.left)
            ._h<Window::Position>(rc.bottom - rc.top);
#elif defined(GTK)
    GtkRequisition req;
    gtk_widget_size_request(wnd.val()._val, &req);
    const Window::Position pos = Window::Position()
            ._x<Window::Position>(0)
            ._y<Window::Position>(0)
            ._w<Window::Position>(req.width)
            ._h<Window::Position>(req.height);
#elif defined(OSX)
    UNIMPL();
    const Window::Position pos;
#elif defined(IOS)
    UNIMPL();
    const Window::Position pos;
#else
#error "Unimplemented GUI mode"
#endif
    return pos;
}

Window::Position Window::getChildPosition(const z::widget& wnd) {
#if defined(WIN32)
    RECT rc;
    ::GetWindowRect(wnd.val()._val, &rc);
    ::MapWindowPoints(HWND_DESKTOP, ::GetParent(wnd.val()._val), (LPPOINT) &rc, 2);
    return Window::Position()
            ._x<Window::Position>(rc.left)
            ._y<Window::Position>(rc.top)
            ._w<Window::Position>(rc.right - rc.left)
            ._h<Window::Position>(rc.bottom - rc.top);
#elif defined(GTK)
    GtkRequisition req;
    gtk_widget_size_request(wnd.val()._val, &req);
    return Window::Position()
            ._x<Window::Position>(0)
            ._y<Window::Position>(0)
            ._w<Window::Position>(req.width)
            ._h<Window::Position>(req.height);
#elif defined(OSX)
    NSSize sz = wnd.val()._val.frame.size;
    return Window::Position()
            ._x<Window::Position>(0)
            ._y<Window::Position>(0)
            ._w<Window::Position>(sz.width)
            ._h<Window::Position>(sz.height);
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void Window::Delete::run(const z::widget& wnd) {
    wnd.clear();
}

void Window::SetTitle::run(const z::widget& wnd, const z::string& title) {
#if defined(WIN32)
    ::SetWindowText(wnd.val()._val, z::s2e(title).c_str());
#elif defined(GTK)
    gtk_window_set_title (GTK_WINDOW (wnd.val()._val), z::s2e(title).c_str());
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void Window::SetFocus::run(const z::widget& frame, const z::widget& wnd) {
#if defined(WIN32)
    unused(frame);
    ::SetFocus(wnd.val()._val);
#elif defined(GTK)
    unused(frame);
    unused(wnd);
    UNIMPL();
#elif defined(OSX)
    [frame.val()._frame makeFirstResponder:wnd.val()._val];
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void Window::Show::run(const z::widget& wnd) {
#if defined(WIN32)
    ::ShowWindow(wnd.val()._val, SW_SHOW);
#elif defined(GTK)
    gtk_widget_show(GTK_WIDGET(wnd.val()._val));
    gtk_window_deiconify(GTK_WINDOW(wnd.val()._val));
#elif defined(OSX)
    UNIMPL();
//    [wnd.val()._val makeKeyAndOrderFront:wnd.val()._val];
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void Window::Hide::run(const z::widget& wnd) {
#if defined(WIN32)
    ::ShowWindow(wnd.val()._val, SW_HIDE);
#elif defined(GTK)
    gtk_widget_hide(GTK_WIDGET(wnd.val()._val));
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void Window::Move::run(const z::widget& wnd, const Window::Position& position) {
#if defined(WIN32)
    ::MoveWindow(wnd.val()._val, position.x, position.y, position.w, position.h, TRUE);
#elif defined(GTK)
    unused(wnd); unused(position);
    //gtk_widget_set_uposition(wnd.val()._val, position.x, position.y);
    //gtk_window_set_default_size (wnd.val()._val, position.w, position.h);
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void Window::Size::run(const z::widget& wnd, const int& w, const int& h) {
#if defined(WIN32)
    RECT rc;
    ::GetWindowRect(wnd.val()._val, &rc);
    int tw = (w == -1)?(rc.right - rc.left): w;
    int th = (h == -1)?(rc.bottom - rc.top): h;
    ::MoveWindow(wnd.val()._val, rc.left, rc.top, tw, th, TRUE);
#elif defined(GTK)
    gtk_widget_set_size_request(wnd.val()._val, w, h);
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

#if defined(GTK)
namespace zz {
    static gboolean onConfigureEvent(GtkWindow* wnd, GdkEvent* event, gpointer phandler) {
        unused(wnd);
        unused(event);
        Window::OnResize::Handler* handler = static_cast<Window::OnResize::Handler*>(phandler);
        Window::OnResize::Handler::_In in;
        z::ref(handler)._run(in);
        return FALSE;
    }
}
#endif

#if defined(OSX)
@interface CResizer : NSObject {
    Window::OnResize::Handler* handler;
    NSView* view;
}
@end

@implementation CResizer
-(void) initResizer: (Window::OnResize::Handler*) h withView:(NSView*) v {
    handler = h;
    view = v;
}

-(void) targetMethod:(NSTimer*)theTimer {
    NSLog(@"onresize");
}
@end

@interface CCloser : NSObject {
    Window::OnClose::Handler* handler;
    NSView* view;
}
@end

@implementation CCloser
-(void) initCloser: (Window::OnClose::Handler*) h withView:(NSView*) v {
    handler = h;
    view = v;
}

-(void) targetMethod:(NSTimer*)theTimer {
    NSLog(@"onclose");
}
@end
#endif

void Window::OnResize::addHandler(const z::widget& wnd, const z::pointer<Handler>& handler) {
#if defined(WIN32)
#elif defined(GTK)
    g_signal_connect (G_OBJECT (wnd.val()._val), "configure-event", G_CALLBACK (zz::onConfigureEvent), handler);
#elif defined(OSX)
    CResizer* resizer = [CResizer alloc];
    [resizer initResizer:z::ptr(handler.get()) withView:wnd.val()._val];
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

#if defined(GTK)
namespace zz {
    static gboolean onWindowCloseEvent(GtkWindow* wnd, gpointer phandler) {
        unused(wnd);
        Window::OnClose::Handler* handler = static_cast<Window::OnClose::Handler*>(phandler);
        Window::OnClose::Handler::_In in;
        z::ref(handler)._run(in);
        return FALSE;
    }
}
#endif

void Window::OnClose::addHandler(const z::widget& wnd, const z::pointer<Handler>& handler) {
#if defined(WIN32)
#elif defined(GTK)
    g_signal_connect (G_OBJECT (wnd.val()._val), "closed", G_CALLBACK (zz::onWindowCloseEvent), z::ref(handler));
#elif defined(OSX)
    CCloser* closer = [CCloser alloc];
    [closer initCloser:z::ptr(handler.get()) withView:wnd.val()._val];
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}
