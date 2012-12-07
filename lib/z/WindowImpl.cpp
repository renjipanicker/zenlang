#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "gui/Window.hpp"
#include "gui/Button.hpp"
#include "WindowImpl.hpp"

#if defined(WIN32)
uint32_t WindowImpl::getNextWmID() {
    static uint32_t lastWM = WM_APP;
    return lastWM++;
}

uint32_t WindowImpl::getNextResID() {
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
            WindowImpl::setImpl(hWnd, impl);
        }
        switch (message) {
            case WM_SIZE: {
                z::Window::OnResize::Handler::_In in;
                z::Window::OnResize::list().runHandler(WindowImpl::impl(hWnd), in);
                break;
            }

            case WM_CLOSE: {
                z::Window::OnClose::Handler::_In in;
                z::Window::OnClose::list().runHandler(WindowImpl::impl(hWnd), in);
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
z::widget::impl& WindowImpl::createWindow(const z::Window::Definition& def, const z::string& className, int style, int xstyle, HWND parent) {
    z::Window::Position pos = z::Window::Position()
            ._x<z::Window::Position>(CW_USEDEFAULT)
            ._y<z::Window::Position>(CW_USEDEFAULT)
            ._w<z::Window::Position>(CW_USEDEFAULT)
            ._h<z::Window::Position>(CW_USEDEFAULT);

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

z::widget::impl& WindowImpl::createMainFrame(const z::Window::Definition& def, int style, int xstyle) {
    HBRUSH brush = (def.style == z::Window::Style::Dialog)?(HBRUSH)GetSysColorBrush(COLOR_3DFACE):(HBRUSH)GetStockObject(WHITE_BRUSH);
    z::string className = zz::registerClass(brush);
    return createWindow(def, className, style, xstyle, (HWND)NULL);
}

z::widget::impl& WindowImpl::createChildFrame(const z::Window::Definition& def, int style, int xstyle, const z::widget &parent) {
    HBRUSH brush = (def.style == z::Window::Style::Dialog)?(HBRUSH)GetSysColorBrush(COLOR_3DFACE):(HBRUSH)GetStockObject(WHITE_BRUSH);
    z::string className = zz::registerClass(brush);
    return createWindow(def, className, style, xstyle, parent.val()._val);
}

z::widget::impl& WindowImpl::createChildWindow(const z::Window::Definition& def, const z::string& className, int style, int xstyle, const z::widget& parent) {
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
z::widget::impl& WindowImpl::initWindowImpl(GtkWidget* hwnd) {
    z::widget::impl* impl = new z::widget::impl();
    z::ref(impl)._val = hwnd;
    z::ref(impl)._fixed = 0;
    g_object_set_data(G_OBJECT(z::ref(impl)._val), "impl", impl);
    return z::ref(impl);
}

z::widget::impl& WindowImpl::createWindow(const z::Window::Definition& def, GtkWidget *parent) {
    unused(def);
    unused(parent);
    GtkWidget* hwnd = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    z::widget::impl& impl = initWindowImpl(hwnd);
    return impl;
}

z::widget::impl& WindowImpl::createChildWindow(GtkWidget* hwnd, const z::Window::Definition& def, const z::widget& parent) {
    gtk_fixed_put (GTK_FIXED (parent.val()._fixed), hwnd, def.position.x, def.position.y);
    z::widget::impl& impl = initWindowImpl(hwnd);
    gtk_widget_show(impl._val);
    return impl;
}
#elif defined(QT)
// no functions defined here.
#elif defined(OSX)
z::widget::impl& WindowImpl::createMainFrame(const z::Window::Definition& def) {
    z::Window::Position pos = z::Window::Position();
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

z::widget::impl& WindowImpl::createChildWindow(const z::Window::Definition& def, const z::widget& parent, NSView* child) {
    if(child == 0) {
        throw z::Exception("Window", z::string("Unable to create child wnd"));
    }
    z::widget::impl* impl = new z::widget::impl();
    z::ref(impl)._val = child;
//    [parent.val()._val addSubView:child];
    return z::ref(impl);
}
#elif defined(IOS)
z::widget::impl& WindowImpl::createMainFrame(const z::Window::Definition& def) {
    throw z::Exception("Window", z::string("NotImplemented: createMainFrame()"));
}

z::widget::impl& WindowImpl::createChildWindow(const z::Window::Definition& def, const z::widget& parent) {
    throw z::Exception("Window", z::string("NotImplemented: createChildFrame()"));
}
#else
#error "Unimplemented GUI mode"
#endif

////////////////////////////////////////////////////////////////////////////////
z::Window::Position z::Window::getWindowPosition(const z::widget& wnd) {
#if defined(WIN32)
    RECT rc;
    ::GetWindowRect(wnd.val()._val, &rc);
    const z::Window::Position pos = z::Window::Position()
            ._x<z::Window::Position>(rc.left)
            ._y<z::Window::Position>(rc.top)
            ._w<z::Window::Position>(rc.right - rc.left)
            ._h<z::Window::Position>(rc.bottom - rc.top);
#elif defined(GTK)
    GtkRequisition req;
    gtk_widget_size_request(wnd.val()._val, &req);
    const z::Window::Position pos = z::Window::Position()
            ._x<z::Window::Position>(0)
            ._y<z::Window::Position>(0)
            ._w<z::Window::Position>(req.width)
            ._h<z::Window::Position>(req.height);
#elif defined(QT)
    UNIMPL();
    const z::Window::Position pos;
#elif defined(OSX)
    UNIMPL();
    const z::Window::Position pos;
#elif defined(IOS)
    UNIMPL();
    const z::Window::Position pos;
#else
#error "Unimplemented GUI mode"
#endif
    return pos;
}

z::Window::Position z::Window::getChildPosition(const z::widget& wnd) {
#if defined(WIN32)
    RECT rc;
    ::GetWindowRect(wnd.val()._val, &rc);
    ::MapWindowPoints(HWND_DESKTOP, ::GetParent(wnd.val()._val), (LPPOINT) &rc, 2);
    return z::Window::Position()
            ._x<z::Window::Position>(rc.left)
            ._y<z::Window::Position>(rc.top)
            ._w<z::Window::Position>(rc.right - rc.left)
            ._h<z::Window::Position>(rc.bottom - rc.top);
#elif defined(GTK)
    GtkRequisition req;
    gtk_widget_size_request(wnd.val()._val, &req);
    return z::Window::Position()
            ._x<z::Window::Position>(0)
            ._y<z::Window::Position>(0)
            ._w<z::Window::Position>(req.width)
            ._h<z::Window::Position>(req.height);
#elif defined(QT)
    UNIMPL();
    return z::Window::Position();
#elif defined(OSX)
    NSSize sz = wnd.val()._val.frame.size;
    return z::Window::Position()
            ._x<z::Window::Position>(0)
            ._y<z::Window::Position>(0)
            ._w<z::Window::Position>(sz.width)
            ._h<z::Window::Position>(sz.height);
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void z::Window::Delete::run(const z::widget& wnd) {
    wnd.clear();
}

void z::Window::SetTitle::run(const z::widget& wnd, const z::string& title) {
#if defined(WIN32)
    ::SetWindowText(wnd.val()._val, z::s2e(title).c_str());
#elif defined(GTK)
    gtk_window_set_title (GTK_WINDOW (wnd.val()._val), z::s2e(title).c_str());
#elif defined(QT)
    UNIMPL();
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void z::Window::SetFocus::run(const z::widget& frame, const z::widget& wnd) {
#if defined(WIN32)
    z::unused_t(frame);
    ::SetFocus(wnd.val()._val);
#elif defined(GTK)
    unused(frame);
    unused(wnd);
    UNIMPL();
#elif defined(QT)
    UNIMPL();
#elif defined(OSX)
    [frame.val()._frame makeFirstResponder:wnd.val()._val];
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void z::Window::Show::run(const z::widget& wnd) {
#if defined(WIN32)
    ::ShowWindow(wnd.val()._val, SW_SHOW);
#elif defined(GTK)
    gtk_widget_show(GTK_WIDGET(wnd.val()._val));
    gtk_window_deiconify(GTK_WINDOW(wnd.val()._val));
#elif defined(QT)
    UNIMPL();
#elif defined(OSX)
    UNIMPL();
//    [wnd.val()._val makeKeyAndOrderFront:wnd.val()._val];
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void z::Window::Hide::run(const z::widget& wnd) {
#if defined(WIN32)
    ::ShowWindow(wnd.val()._val, SW_HIDE);
#elif defined(GTK)
    gtk_widget_hide(GTK_WIDGET(wnd.val()._val));
#elif defined(QT)
    UNIMPL();
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void z::Window::Move::run(const z::widget& wnd, const z::Window::Position& position) {
#if defined(WIN32)
    ::MoveWindow(wnd.val()._val, position.x, position.y, position.w, position.h, TRUE);
#elif defined(GTK)
    unused(wnd); unused(position);
    //gtk_widget_set_uposition(wnd.val()._val, position.x, position.y);
    //gtk_window_set_default_size (wnd.val()._val, position.w, position.h);
#elif defined(QT)
    UNIMPL();
#elif defined(OSX)
    UNIMPL();
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

void z::Window::Size::run(const z::widget& wnd, const int& w, const int& h) {
#if defined(WIN32)
    RECT rc;
    ::GetWindowRect(wnd.val()._val, &rc);
    int tw = (w == -1)?(rc.right - rc.left): w;
    int th = (h == -1)?(rc.bottom - rc.top): h;
    ::MoveWindow(wnd.val()._val, rc.left, rc.top, tw, th, TRUE);
#elif defined(GTK)
    gtk_widget_set_size_request(wnd.val()._val, w, h);
#elif defined(QT)
    UNIMPL();
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
        z::Window::OnResize::Handler* handler = static_cast<z::Window::OnResize::Handler*>(phandler);
        z::Window::OnResize::Handler::_In in;
        z::ref(handler)._run(in);
        return FALSE;
    }
}
#endif

#if defined(OSX)
@interface CResizer : NSObject {
    z::Window::OnResize::Handler* handler;
    NSView* view;
}
@end

@implementation CResizer
-(void) initResizer: (z::Window::OnResize::Handler*) h withView:(NSView*) v {
    handler = h;
    view = v;
}

-(void) targetMethod:(NSTimer*)theTimer {
    NSLog(@"onresize");
}
@end

@interface CCloser : NSObject {
    z::Window::OnClose::Handler* handler;
    NSView* view;
}
@end

@implementation CCloser
-(void) initCloser: (z::Window::OnClose::Handler*) h withView:(NSView*) v {
    handler = h;
    view = v;
}

-(void) targetMethod:(NSTimer*)theTimer {
    NSLog(@"onclose");
}
@end
#endif

void z::Window::OnResize::addHandler(const z::widget& wnd, const z::pointer<Handler>& handler) {
#if defined(WIN32)
#elif defined(GTK)
    g_signal_connect (G_OBJECT (wnd.val()._val), "configure-event", G_CALLBACK (zz::onConfigureEvent), z::ptr(handler.get()) );
#elif defined(QT)
    UNIMPL();
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
        z::Window::OnClose::Handler* handler = static_cast<z::Window::OnClose::Handler*>(phandler);
        z::Window::OnClose::Handler::_In in;
        z::ref(handler)._run(in);
        return FALSE;
    }
}
#endif

void z::Window::OnClose::addHandler(const z::widget& wnd, const z::pointer<Handler>& handler) {
#if defined(WIN32)
#elif defined(GTK)
    g_signal_connect (G_OBJECT (wnd.val()._val), "closed", G_CALLBACK (zz::onWindowCloseEvent), z::ptr(handler.get()) );
#elif defined(QT)
    UNIMPL();
#elif defined(OSX)
    CCloser* closer = [CCloser alloc];
    [closer initCloser:z::ptr(handler.get()) withView:wnd.val()._val];
#elif defined(IOS)
    UNIMPL();
#else
#error "Unimplemented GUI mode"
#endif
}

//!! WEB-GUI CODE
#if defined(WIN32)
#define MUST_BE_IMPLEMENTED(f) return E_NOTIMPL;
class Storage : public IStorage {
    HRESULT STDMETHODCALLTYPE QueryInterface( REFIID /*riid*/, LPVOID FAR* /*ppvObj*/) { MUST_BE_IMPLEMENTED("QueryInterface"); }
    ULONG   STDMETHODCALLTYPE AddRef() { return(1); }
    ULONG   STDMETHODCALLTYPE Release() { return(1); }
    HRESULT STDMETHODCALLTYPE CreateStream(const WCHAR* /*pwcsName*/, DWORD /*grfMode*/, DWORD /*reserved1*/, DWORD /*reserved2*/, IStream** /*ppstm*/ ) { MUST_BE_IMPLEMENTED("CreateStream"); }
    HRESULT STDMETHODCALLTYPE OpenStream(const WCHAR* /*pwcsName*/, void* /*reserved1*/, DWORD /*grfMode*/, DWORD /*reserved2*/, IStream** /*ppstm*/) { MUST_BE_IMPLEMENTED("OpenStream"); }
    HRESULT STDMETHODCALLTYPE CreateStorage(const WCHAR* /*pwcsName*/, DWORD /*grfMode*/, DWORD /*reserved1*/, DWORD /*reserved2*/, IStorage** /*ppstg*/) {MUST_BE_IMPLEMENTED("CreateStorage"); }
    HRESULT STDMETHODCALLTYPE OpenStorage(const WCHAR* /*pwcsName*/, IStorage* /*pstgPriority*/, DWORD /*grfMode*/, SNB /*snbExclude*/, DWORD  /*reserved*/, IStorage** /*ppstg*/) { MUST_BE_IMPLEMENTED("OpenStorage"); }
    HRESULT STDMETHODCALLTYPE CopyTo(DWORD /*ciidExclude*/, IID const* /*rgiidExclude*/, SNB /*snbExclude*/,IStorage* /*pstgDest*/){ MUST_BE_IMPLEMENTED("CopyTo"); }
    HRESULT STDMETHODCALLTYPE MoveElementTo(const OLECHAR* /*pwcsName*/, IStorage* /*pstgDest*/, const OLECHAR* /*pwcsNewName*/, DWORD /*grfFlags*/) { MUST_BE_IMPLEMENTED("MoveElementTo"); }
    HRESULT STDMETHODCALLTYPE Commit(DWORD /*grfCommitFlags*/) { MUST_BE_IMPLEMENTED("Commit"); }  
    HRESULT STDMETHODCALLTYPE Revert() { MUST_BE_IMPLEMENTED("Revert"); }
    HRESULT STDMETHODCALLTYPE EnumElements(DWORD /*reserved1*/, void* /*reserved2*/, DWORD /*reserved3*/, IEnumSTATSTG** /*ppenum*/) { MUST_BE_IMPLEMENTED("EnumElements"); }
    HRESULT STDMETHODCALLTYPE DestroyElement(const OLECHAR* /*pwcsName*/) { MUST_BE_IMPLEMENTED("DestroyElement"); }
    HRESULT STDMETHODCALLTYPE RenameElement(const WCHAR* /*pwcsOldName*/, const WCHAR* /*pwcsNewName*/) { MUST_BE_IMPLEMENTED("RenameElement"); }
    HRESULT STDMETHODCALLTYPE SetElementTimes(const WCHAR* /*pwcsName*/, FILETIME const* /*pctime*/, FILETIME const* /*patime*/, FILETIME const* /*pmtime*/) { MUST_BE_IMPLEMENTED("SetElementTimes") }
    HRESULT STDMETHODCALLTYPE SetClass(REFCLSID /*clsid*/) {return S_OK;}
    HRESULT STDMETHODCALLTYPE SetStateBits(DWORD /*grfStateBits*/, DWORD /*grfMask*/) { MUST_BE_IMPLEMENTED("SetStateBits"); }
    HRESULT STDMETHODCALLTYPE Stat(STATSTG* /*pstatstg*/, DWORD /*grfStatFlag*/) { MUST_BE_IMPLEMENTED("Stat"); }
};

class OleInPlaceFrame : public IOleInPlaceFrame {
    HWND hwnd_;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID /*riid*/, LPVOID FAR* /*ppvObj*/) {MUST_BE_IMPLEMENTED("QueryInterface");}
    ULONG   STDMETHODCALLTYPE AddRef() {return 1;}
    ULONG   STDMETHODCALLTYPE Release() {return 1;}
    HRESULT STDMETHODCALLTYPE GetWindow(HWND FAR* lphwnd) {*lphwnd = hwnd_;return S_OK;}
    HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL /*fEnterMode*/) { MUST_BE_IMPLEMENTED("ContextSensitiveHelp");}
    HRESULT STDMETHODCALLTYPE GetBorder(LPRECT /*lprectBorder*/) { MUST_BE_IMPLEMENTED("GetBorder");}
    HRESULT STDMETHODCALLTYPE RequestBorderSpace(LPCBORDERWIDTHS /*pborderwidths*/) { MUST_BE_IMPLEMENTED("RequestBorderSpace");}
    HRESULT STDMETHODCALLTYPE SetBorderSpace(LPCBORDERWIDTHS /*pborderwidths*/) { MUST_BE_IMPLEMENTED("SetBorderSpace");}
    HRESULT STDMETHODCALLTYPE SetActiveObject(IOleInPlaceActiveObject* /*pActiveObject*/, LPCOLESTR /*pszObjName*/) { return S_OK;}
    HRESULT STDMETHODCALLTYPE InsertMenus(HMENU /*hmenuShared*/, LPOLEMENUGROUPWIDTHS /*lpMenuWidths*/) {MUST_BE_IMPLEMENTED("InsertMenus");}
    HRESULT STDMETHODCALLTYPE SetMenu(HMENU /*hmenuShared*/, HOLEMENU /*holemenu*/, HWND /*hwndActiveObject*/) { return(S_OK);}
    HRESULT STDMETHODCALLTYPE RemoveMenus(HMENU /*hmenuShared*/) {MUST_BE_IMPLEMENTED("RemoveMenus");}
    HRESULT STDMETHODCALLTYPE SetStatusText(LPCOLESTR /*pszStatusText*/) {return S_OK;}
    HRESULT STDMETHODCALLTYPE EnableModeless(BOOL /*fEnable*/) { return S_OK;}
    HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG /*lpmsg*/, WORD /*wID*/) {MUST_BE_IMPLEMENTED("TranslateAccelerator");}
public:
    inline OleInPlaceFrame(HWND h) : hwnd_(h) {}
};

class OleClientSite : public IOleClientSite {
    IOleInPlaceSite* in_place_;
    IDocHostUIHandler* doc_host_ui_handler_;
    DWebBrowserEvents2* web_browser_events_;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject) {
        if (!memcmp((const void*) &riid, (const void*)&IID_IUnknown, sizeof(GUID))) {
            *ppvObject = static_cast<IOleClientSite*>(this);
            return S_OK;
        }
        if (!memcmp((const void*) &riid, (const void*)&IID_IOleClientSite, sizeof(GUID))) {
            *ppvObject = static_cast<IOleClientSite*>(this);
            return S_OK;
        }
        if (!memcmp((const void*)&riid, &IID_IOleInPlaceSite, sizeof(GUID))) {
            *ppvObject = in_place_;
            return S_OK;
        }
        if (!memcmp((const void*)&riid, &IID_IDocHostUIHandler, sizeof(GUID))) {
            *ppvObject = doc_host_ui_handler_;
            return S_OK;
        }
        if (riid == DIID_DWebBrowserEvents2) {
            *ppvObject = web_browser_events_;
            return S_OK;
        }
        if (riid == IID_IDispatch) {
            *ppvObject = web_browser_events_;
            return S_OK;
        }
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
    ULONG   STDMETHODCALLTYPE AddRef() {return 1;}
    ULONG   STDMETHODCALLTYPE Release() {return 1;}
    HRESULT STDMETHODCALLTYPE SaveObject() {MUST_BE_IMPLEMENTED("SaveObject");}
    HRESULT STDMETHODCALLTYPE GetMoniker(DWORD /*dwAssign*/, DWORD /*dwWhichMoniker*/, IMoniker** /*ppmk*/) { MUST_BE_IMPLEMENTED("GetMoniker");}
    HRESULT STDMETHODCALLTYPE GetContainer(LPOLECONTAINER FAR* ppContainer) {*ppContainer = 0;return E_NOINTERFACE;}
    HRESULT STDMETHODCALLTYPE ShowObject() {return NOERROR;}
    HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL /*fShow*/) {MUST_BE_IMPLEMENTED("OnShowWindow");}
    HRESULT STDMETHODCALLTYPE RequestNewObjectLayout() {MUST_BE_IMPLEMENTED("RequestNewObjectLayout");}

public:
    inline OleClientSite(IOleInPlaceSite* in_place, IDocHostUIHandler* doc_host_ui_handler, DWebBrowserEvents2* web_browser_events) : in_place_(in_place), doc_host_ui_handler_(doc_host_ui_handler), web_browser_events_(web_browser_events ) {}
};

class DocHostUiHandler : public IDocHostUIHandler {
    IOleClientSite* ole_client_site_;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR* ppvObj) {
        if (ole_client_site_ == 0) return E_NOINTERFACE;
        return ole_client_site_->QueryInterface(riid, ppvObj);
    }

    ULONG   STDMETHODCALLTYPE AddRef() {return 1;}
    ULONG   STDMETHODCALLTYPE Release() {return 1;}
    HRESULT STDMETHODCALLTYPE ShowContextMenu(DWORD /*dwID*/, POINT __RPC_FAR* /*ppt*/, IUnknown __RPC_FAR* /*pcmdtReserved*/, IDispatch __RPC_FAR* /*pdispReserved*/) {return S_OK;}
    HRESULT STDMETHODCALLTYPE ShowUI(DWORD /*dwID*/, IOleInPlaceActiveObject __RPC_FAR* /*pActiveObject*/, IOleCommandTarget __RPC_FAR* /*pCommandTarget*/, IOleInPlaceFrame __RPC_FAR* /*pFrame*/, IOleInPlaceUIWindow __RPC_FAR* /*pDoc*/) {return S_OK;}
    HRESULT STDMETHODCALLTYPE GetHostInfo(DOCHOSTUIINFO __RPC_FAR* /*pInfo*/);
    HRESULT STDMETHODCALLTYPE HideUI() {return S_OK;}
    HRESULT STDMETHODCALLTYPE UpdateUI() {return S_OK;}
    HRESULT STDMETHODCALLTYPE EnableModeless(BOOL /*fEnable*/) {return S_OK;}
    HRESULT STDMETHODCALLTYPE OnDocWindowActivate(BOOL /*fActivate*/) {return S_OK;}
    HRESULT STDMETHODCALLTYPE OnFrameWindowActivate(BOOL /*fActivate*/) {return S_OK;}
    HRESULT STDMETHODCALLTYPE ResizeBorder(LPCRECT /*prcBorder*/, IOleInPlaceUIWindow __RPC_FAR* /*pUIWindow*/, BOOL /*fRameWindow*/) {return S_OK;}
    HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG /*lpMsg*/, const GUID __RPC_FAR* /*pguidCmdGroup*/, DWORD /*nCmdID*/){return S_FALSE;}
    HRESULT STDMETHODCALLTYPE GetOptionKeyPath(LPOLESTR __RPC_FAR* /*pchKey*/, DWORD /*dw*/) {return S_FALSE;}
    HRESULT STDMETHODCALLTYPE GetDropTarget(IDropTarget __RPC_FAR* /*pDropTarget*/, IDropTarget __RPC_FAR* __RPC_FAR* /*ppDropTarget*/) {return S_FALSE;}
    HRESULT STDMETHODCALLTYPE GetExternal(IDispatch __RPC_FAR* __RPC_FAR* ppDispatch) {*ppDispatch = 0;return S_FALSE;}
    HRESULT STDMETHODCALLTYPE TranslateUrl(DWORD /*dwTranslate*/, OLECHAR __RPC_FAR* /*pchURLIn*/, OLECHAR __RPC_FAR* __RPC_FAR* ppchURLOut) {*ppchURLOut = 0;return S_FALSE;}
    HRESULT STDMETHODCALLTYPE FilterDataObject(IDataObject __RPC_FAR* /*pDO*/, IDataObject __RPC_FAR* __RPC_FAR* ppDORet) {*ppDORet = 0;return S_FALSE;}

public:
    inline DocHostUiHandler() : ole_client_site_(0){}
    virtual ~DocHostUiHandler() {}
    inline void ClientSite(IOleClientSite* o) {ole_client_site_ = o;}
};

class OleInPlaceSite : public IOleInPlaceSite {
    IOleClientSite*   ole_client_site_;
    IOleInPlaceFrame* ole_in_place_frame_;
    IOleObject	* browser_object_;
    HWND              hwnd_;

    HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, LPVOID FAR* ppvObj) {return ole_client_site_->QueryInterface(riid, ppvObj);}
    ULONG   STDMETHODCALLTYPE AddRef() { return(1); }
    ULONG   STDMETHODCALLTYPE Release() { return(1); }
    HRESULT STDMETHODCALLTYPE GetWindow( HWND FAR* lphwnd) {*lphwnd = hwnd_;return(S_OK);}
    HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL /* fEnterMode*/) {MUST_BE_IMPLEMENTED("ContextSensitiveHelp");}
    HRESULT STDMETHODCALLTYPE CanInPlaceActivate() {return S_OK;}
    HRESULT STDMETHODCALLTYPE OnInPlaceActivate() {return S_OK;}
    HRESULT STDMETHODCALLTYPE OnUIActivate() {return(S_OK);}
    HRESULT STDMETHODCALLTYPE GetWindowContext(LPOLEINPLACEFRAME FAR* lplpFrame, LPOLEINPLACEUIWINDOW FAR* lplpDoc, LPRECT /*lprcPosRect*/, LPRECT /*lprcClipRect*/, LPOLEINPLACEFRAMEINFO lpFrameInfo) {
        *lplpFrame = ole_in_place_frame_;
        *lplpDoc = 0;
        lpFrameInfo->fMDIApp       = FALSE;
        lpFrameInfo->hwndFrame     = hwnd_;
        lpFrameInfo->haccel        = 0;
        lpFrameInfo->cAccelEntries = 0;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Scroll(SIZE /*scrollExtent*/) { MUST_BE_IMPLEMENTED("Scroll");}
    HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL /*fUndoable*/) { return(S_OK);}
    HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate() { return(S_OK);}
    HRESULT STDMETHODCALLTYPE DiscardUndoState() {MUST_BE_IMPLEMENTED("DiscardUndoState");}
    HRESULT STDMETHODCALLTYPE DeactivateAndUndo() {MUST_BE_IMPLEMENTED("DeactivateAndUndo");}

    // Called when the position of the browser object is changed
    HRESULT STDMETHODCALLTYPE OnPosRectChange(LPCRECT lprcPosRect) {
        IOleInPlaceObject* inplace = 0;
        if (browser_object_->QueryInterface(IID_IOleInPlaceObject, (void**)&inplace) == S_OK) {
            inplace->SetObjectRects(lprcPosRect, lprcPosRect);
        }
        return(S_OK);
    }

public:
    inline OleInPlaceSite( IOleInPlaceFrame* ole_in_place_frame, HWND h) : ole_client_site_(0), ole_in_place_frame_(ole_in_place_frame), browser_object_(0), hwnd_(h) {}
    void BrowserObject(IOleObject* o){browser_object_ = o;}
    void ClientSite(IOleClientSite* o) {ole_client_site_ = o;}
};

struct z::window::impl : public virtual DWebBrowserEvents2 {
    DocHostUiHandler*  _ui;
    IOleObject*        _bo;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);
    ULONG   STDMETHODCALLTYPE AddRef()  {return 1;}
    ULONG   STDMETHODCALLTYPE Release() {return 1;}
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int* /*pctinfo*/) {MUST_BE_IMPLEMENTED("GetTypeInfoCount");}
    HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int /*iTInfo*/,LCID /*lcid*/, ITypeInfo** /*ppTInfo*/) {MUST_BE_IMPLEMENTED("GetTypeInfo");}
    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID /*riid*/, OLECHAR** /*rgszNames*/, unsigned int /*cNames*/, LCID /*lcid*/, DISPID * /*rgDispId*/) {MUST_BE_IMPLEMENTED("GetIDsOfNames");}
    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD /*wFlags*/, DISPPARAMS* /*pDispParams*/, VARIANT* /*pVarResult*/, EXCEPINFO* /*pExcepInfo*/, unsigned int* /*puArgErr*/);
public:
    long EmbedBrowserObject(HWND hwnd_);
    void UnEmbedBrowserObject();
    void ResizeBrowser(DWORD width, DWORD height);
    long DisplayHTMLPage(const z::string& url);
};

HRESULT DocHostUiHandler::GetHostInfo(DOCHOSTUIINFO __RPC_FAR *pInfo) {
    pInfo->cbSize = sizeof(DOCHOSTUIINFO);
    pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_NO3DOUTERBORDER;
    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
    return S_OK ;
}

HRESULT z::window::impl::QueryInterface(REFIID riid, void ** ppvObject) {
    if (!memcmp((const void*) &riid, (const void*)&IID_IUnknown,          sizeof(GUID)) || 
        !memcmp((const void*) &riid, (const void*)&IID_IDispatch,         sizeof(GUID)) ||
        !memcmp((const void*) &riid, (const void*)&IID_IDocHostUIHandler, sizeof(GUID))) {
            *ppvObject = _ui;
            return S_OK;
    }

    ppvObject = 0;
    return E_NOINTERFACE;
}

HRESULT z::window::impl::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/, WORD /*wFlags */, DISPPARAMS FAR* pDispParams, VARIANT FAR* /*pVarResult*/, EXCEPINFO FAR* /*pExcepInfo*/, unsigned int FAR* /*puArgErr*/) {
    switch (dispIdMember) {
        case DISPID_BEFORENAVIGATE     :   // this is sent before navigation to give a chance to abort
            return S_OK;
        case DISPID_NAVIGATECOMPLETE   :   // in async, this is sent when we have enough to show
            return S_OK;
        case DISPID_STATUSTEXTCHANGE   :
        case DISPID_QUIT               :
        case DISPID_DOWNLOADCOMPLETE: 
            return S_OK;
        case DISPID_COMMANDSTATECHANGE :
            return S_OK;
        case DISPID_DOWNLOADBEGIN      :
            return S_OK;
        case DISPID_NEWWINDOW          :   // sent when a new window should be created
        case DISPID_PROGRESSCHANGE     :   // sent when download progress is updated
        case DISPID_WINDOWMOVE         :   // sent when main window has been moved
        case DISPID_WINDOWRESIZE       :   // sent when main window has been sized
        case DISPID_WINDOWACTIVATE     :   // sent when main window has been activated
        case DISPID_PROPERTYCHANGE     : {   // sent when the PutProperty method is called
            //VARIANT a = pDispParams->rgvarg[0];
            return S_OK;
        }
        case DISPID_TITLECHANGE        :   // sent when the document title changes
        case DISPID_TITLEICONCHANGE    :   // sent when the top level window icon may have changed.
        case DISPID_FRAMEBEFORENAVIGATE    :
        case DISPID_FRAMENAVIGATECOMPLETE  :
        case DISPID_FRAMENEWWINDOW         :
            return S_OK;

        case DISPID_BEFORENAVIGATE2: {   // hyperlink clicked on
            return S_OK;
         }
        case DISPID_NEWWINDOW2:          
            return S_OK;
        case DISPID_NAVIGATECOMPLETE2:       // UIActivate new document
            return S_OK;
            break;
        case DISPID_ONQUIT               :
        case DISPID_ONVISIBLE            :   // sent when the window goes visible/hidden
        case DISPID_ONTOOLBAR            :   // sent when the toolbar should be shown/hidden
        case DISPID_ONMENUBAR            :   // sent when the menubar should be shown/hidden
        case DISPID_ONSTATUSBAR          :   // sent when the statusbar should be shown/hidden
        case DISPID_ONFULLSCREEN         :   // sent when kiosk mode should be on/off
        case DISPID_DOCUMENTCOMPLETE     :   // new document goes ReadyState_Complete
            //AddSink();
            return S_OK;
        case DISPID_ONTHEATERMODE        :   // sent when theater mode should be on/off
        case DISPID_ONADDRESSBAR         :   // sent when the address bar should be shown/hidden
        case DISPID_WINDOWSETRESIZABLE   :   // sent to set the style of the host window frame
        case DISPID_WINDOWCLOSING        :   // sent before script window.close closes the window 
        case DISPID_WINDOWSETLEFT        :   // sent when the put_left method is called on the WebOC
        case DISPID_WINDOWSETTOP         :   // sent when the put_top method is called on the WebOC
        case DISPID_WINDOWSETWIDTH       :   // sent when the put_width method is called on the WebOC
        case DISPID_WINDOWSETHEIGHT      :   // sent when the put_height method is called on the WebOC 
        case DISPID_CLIENTTOHOSTWINDOW   :   // sent during window.open to request conversion of dimensions
        case DISPID_SETSECURELOCKICON    :   // sent to suggest the appropriate security icon to show
        case DISPID_FILEDOWNLOAD         :   // Fired to indicate the File Download dialog is opening
            return S_OK;
        case DISPID_PRIVACYIMPACTEDSTATECHANGE   :  // Fired when the user's browsing experience is impacted
        case DISPID_NAVIGATEERROR: {   // Fired to indicate the a binding error has occured
            return S_OK;
       }
    }
    return DISP_E_MEMBERNOTFOUND;
}

long z::window::impl::EmbedBrowserObject(HWND hwnd) {
    IStorage* storage = new Storage;
    OleInPlaceFrame* ole_in_place_frame = new OleInPlaceFrame(hwnd);
    OleInPlaceSite* ole_in_place_site = new OleInPlaceSite(ole_in_place_frame, hwnd);
    _ui = new DocHostUiHandler();

    OleClientSite* ole_client_site = new OleClientSite(ole_in_place_site, _ui, static_cast<DWebBrowserEvents2*>(this));
    _ui->ClientSite(ole_client_site);
    ole_in_place_site->ClientSite(ole_client_site);

    HRESULT hr = ::OleCreate(CLSID_WebBrowser, IID_IOleObject, OLERENDER_DRAW, 0, ole_client_site, storage, (void**)&_bo);
    if(hr != S_OK) {
        return -2 ;
    }

    ole_in_place_site->BrowserObject(_bo);
    _bo->SetHostNames(L"Some_host_name", 0);

    RECT rect;
    ::GetClientRect(hwnd, &rect);

    IWebBrowser2    *webBrowser2;
    if (! ::OleSetContainedObject(static_cast<IUnknown*>(_bo), TRUE) && !_bo->DoVerb(OLEIVERB_SHOW, NULL, ole_client_site, -1, hwnd, &rect) && !_bo->QueryInterface(IID_IWebBrowser2, reinterpret_cast<void**> (&webBrowser2))) {
        webBrowser2->put_Left  (0);
        webBrowser2->put_Top   (0);
        webBrowser2->put_Width (rect.right);
        webBrowser2->put_Height(rect.bottom);
        webBrowser2->Release();
        return 0;
    }
    return(-3);
}

void z::window::impl::ResizeBrowser(DWORD width, DWORD height) {
	IWebBrowser2* webBrowser2 = 0;
	if (_bo->QueryInterface(IID_IWebBrowser2, (void**)&webBrowser2) != S_OK)
        return;
	webBrowser2->put_Width(width);
	webBrowser2->put_Height(height);
	webBrowser2->Release();
}

void z::window::impl::UnEmbedBrowserObject() {
	_bo->Close(OLECLOSE_NOSAVE);
	_bo->Release();
    _bo = 0;
}

long z::window::impl::DisplayHTMLPage(const z::string& url1) {
    z::string16 url = z::c32to16(url1);
	IWebBrowser2	*webBrowser2;
	VARIANT			myURL;

    if (_bo->QueryInterface(IID_IWebBrowser2, (void**)&webBrowser2) != S_OK) {
	    return(-5);
    }
	VariantInit(&myURL);
	myURL.vt = VT_BSTR;
    assert(sizeof(z::char16_t) == sizeof(OLECHAR));
	myURL.bstrVal = SysAllocString((OLECHAR*)url.c_str());
	if (!myURL.bstrVal) {
    	webBrowser2->Release();
		return(-6);
	}

	// Call the Navigate2() function to actually display the page.
	webBrowser2->Navigate2(&myURL, 0, 0, 0, 0);
	VariantClear(&myURL);
	webBrowser2->Release();
	return(0);
}

//////////////////////////////////////////////////////////////////////////////
namespace zz {
    LPCTSTR szWindowClass = "zenlang_webwin";
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    //	int wmId, wmEvent;
        z::window::impl* This = 0;
        if(message == WM_NCCREATE) {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            This = reinterpret_cast<z::window::impl*>(lpcs->lpCreateParams);
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(This));
        } else {
            This = reinterpret_cast<z::window::impl*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
        }
	    switch (message) {
            case WM_CREATE:
			    if (This->EmbedBrowserObject(hWnd))
                    return(-1);
                break;
		    case WM_SIZE:
			    This->ResizeBrowser(LOWORD(lParam), HIWORD(lParam));
                break;
	        case WM_COMMAND:
	            return DefWindowProc(hWnd, message, wParam, lParam);
		        //wmId    = LOWORD(wParam);
		        //wmEvent = HIWORD(wParam);
		        //switch (wmId) {
		        //    //case IDM_EXIT:
			       //    // DestroyWindow(hWnd);
			       //    // break;
		        //    default:
			       //     return DefWindowProc(hWnd, message, wParam, lParam);
		        //}
		        //break;
	        case WM_DESTROY:
		        This->UnEmbedBrowserObject();
		        PostQuitMessage(0);
		        break;
	        default:
		        return DefWindowProc(hWnd, message, wParam, lParam);
	    }
	    return 0;
    }

    ATOM MyRegisterClass() {
	    WNDCLASSEX wcex;
	    wcex.cbSize = sizeof(WNDCLASSEX);
	    wcex.style			= CS_HREDRAW | CS_VREDRAW;
	    wcex.lpfnWndProc	= WndProc;
	    wcex.cbClsExtra		= 0;
	    wcex.cbWndExtra		= 0;
	    wcex.hInstance		= z::app().instance();
	    wcex.hIcon			= 0; //LoadIcon(z::app().instance(), MAKEINTRESOURCE(IDI_MINIE));
	    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	    wcex.lpszMenuName	= ""; //MAKEINTRESOURCE(IDC_MINIE);
        wcex.lpszClassName	= zz::szWindowClass;
	    wcex.hIconSm		= 0 ;//LoadIcon(z::app().instance(), MAKEINTRESOURCE(IDI_SMALL));
	    return RegisterClassEx(&wcex);
    }
}

z::window z::Window::Open::run(const z::string& page) {
    zz::MyRegisterClass();
    z::window::impl* impl = new z::window::impl();
    HWND hWnd = ::CreateWindow(zz::szWindowClass, "Zenlang", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, z::app().instance(), impl);
    if (!hWnd) {
        throw z::Exception("Window::Open", z::string("Unable to create window"));
    }

    impl->DisplayHTMLPage(page);
    ::ShowWindow(hWnd, SW_SHOW);
    ::UpdateWindow(hWnd);
    return z::window(impl);
}

#elif defined(GTK)
static void destroyWindowCb(GtkWidget* widget, GtkWidget* window) {
    gtk_main_quit();
}

static gboolean closeWebViewCb(WebKitWebView* webView, GtkWidget* window) {
    gtk_widget_destroy(window);
    return TRUE;
}

struct z::window::impl {
    inline impl(WebKitWebView* win) : _win(win) {}
    WebKitWebView* _win;
};

z::window z::Window::Open::run(const z::string& page) {
    // Create an 800x600 window that will contain the browser instance
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);

    // Create a browser instance
    WebKitWebView* webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Create a scrollable area, and put the browser instance into it
    GtkWidget *scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolledWindow), GTK_WIDGET(webView));

    // Set up callbacks so that if either the main window or the browser instance is
    // closed, the program will exit
    g_signal_connect(main_window, "destroy", G_CALLBACK(destroyWindowCb), NULL);
    g_signal_connect(webView, "close-web-view", G_CALLBACK(closeWebViewCb), main_window);

    // Put the scrollable area into the main window
    gtk_container_add(GTK_CONTAINER(main_window), scrolledWindow);

    // Load a web page into the browser instance
    z::estring es = z::s2e(page);
    webkit_web_view_load_uri(webView, es.c_str());

    // Make sure that when the browser area becomes visible, it will get mouse
    // and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(webView));
    gtk_widget_show_all(main_window);

    z::window::impl* impl = new z::window::impl(webView);
    return z::window(impl);
}

#elif defined(OSX)
struct z::window::impl {
    inline impl(WebView* win) : _win(win) {}
    WebView* _win;
};
z::window z::Window::Open::run(const z::string& page) {
    NSRect rc = NSMakeRect(0, 0, 800, 600);
//    NSRect rc1 = [[NSScreen mainScreen] frame];
    int styleMask = NSTitledWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask | NSClosableWindowMask;
    NSWindow* win = [[NSWindow alloc] initWithContentRect:rc styleMask:styleMask backing:NSBackingStoreBuffered defer:false];
    [win setBackgroundColor:[NSColor blueColor]];

    WebView* webView = [[WebView alloc] initWithFrame:rc];
    NSString* npage = [NSString stringWithUTF8String:z::s2e(page).c_str()];
    NSURL* url = [NSURL URLWithString:npage];
    NSURLRequest* req = [NSURLRequest requestWithURL:url];
    [[webView mainFrame] loadRequest:req];

    [win setContentView:webView];
    [win makeKeyAndOrderFront:win];

    z::window::impl* impl = new z::window::impl(webView);
    return z::window(impl);
}
#elif defined(IOS)
struct z::window::impl {
    inline impl(UIWebView* win) : _win(win) {}
    UIWebView* _win;
};
z::window z::Window::Open::run(const z::string& page) {
    AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication]delegate];
    appDelegate.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    appDelegate.window.rootViewController = [[UIViewController alloc] init];
    appDelegate.window.rootViewController.view.backgroundColor = [UIColor greenColor];

    CGRect rc = [[UIScreen mainScreen] bounds];
    rc = CGRectInset(rc, 10, 10);
    UIWebView* webView = [[UIWebView alloc] initWithFrame:rc];
    webView.delegate = appDelegate;
    webView.scalesPageToFit = YES;
    webView.userInteractionEnabled = YES;
    webView.backgroundColor = [UIColor redColor];

    NSString* npage = [NSString stringWithUTF8String:z::s2e(page).c_str()];
    NSURL* url = [NSURL URLWithString:npage];
    NSURLRequest* req = [NSURLRequest requestWithURL:url];
    [webView loadRequest:req];

    [appDelegate.window.rootViewController.view addSubview:webView];
    [appDelegate.window makeKeyAndVisible];

    z::window::impl* impl = new z::window::impl(webView);
    return z::window(impl);
}
#endif
