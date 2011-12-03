#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "Window.hpp"
#include "WindowImpl.hpp"
#include "Button.hpp"

#if defined(WIN32)
static int lastclassId = 1;
static std::string getNextClassID() {
    char name[128];
    snprintf(name, 128, "classX%d", lastclassId++);
    return name;
}

//static HandlerList<int, MenuItem::SelectHandler> menuItemSelectHandlerList;
//static HandlerList<int, SysTray::OnActivationHandler> sysTrayActivationHandlerList;
//static HandlerList<int, SysTray::OnContextMenuHandler> sysTrayContextMenuHandlerList;

static HandlerList<HWND, Window::OnResize::Handler> onResizeHandlerList;
static HandlerList<HWND, Window::OnClose::Handler> onCloseHandlerList;

static ULONGLONG GetDllVersion(LPCTSTR lpszDllName) {
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

// Message handler for the app
static InitList<Window::Native::WndProc> s_WndProcList;
Window::Native::WndProc::WndProc() : _next(0) {
    s_WndProcList.push(this);
}

struct WndProc : public Window::Native::WndProc {
    virtual LRESULT handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_SIZE:
            {
                Window::OnResize::Handler::_In in;
                if(onResizeHandlerList.runHandler(hWnd, in))
                    return 1;
                break;
            }

            case WM_CLOSE:
            {
                Window::OnClose::Handler::_In in;
                if(onCloseHandlerList.runHandler(hWnd, in))
                    return 1;
                break;
            }
        }
        return 0;
    }
};
static WndProc s_winProc;

static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if(message == WM_NCCREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        void* p = ref(pcs).lpCreateParams;
        Window::Instance::Impl* impl = reinterpret_cast<Window::Instance::Impl*>(p);
        ::SetWindowLongPtr(hWnd, GWL_USERDATA, reinterpret_cast<long>(impl));
    }

    if(lParam == WM_LBUTTONDOWN) {
//        if(sysTrayActivationHandlerList.runHandler(message))
//            return 1;
    }

    if((lParam == WM_RBUTTONDOWN) || (lParam == WM_CONTEXTMENU)) {
//        if(sysTrayContextMenuHandlerList.runHandler(message))
//            return 1;
    }

    s_WndProcList.begin();
    Window::Native::WndProc* wp = s_WndProcList.next();

    while(wp != 0) {
        LRESULT lr = ref(wp).handle(hWnd, message, wParam, lParam);
        if(lr != 0)
            return lr;
        wp = s_WndProcList.next();
    }

    return ::DefWindowProc(hWnd, message, wParam, lParam);
}

std::string registerClass(HBRUSH bg) {
    std::string className = getNextClassID();
    WNDCLASSEX wcx;

    // Fill in the window class structure with parameters
    // that describe the main window.

    wcx.cbSize = sizeof(wcx);          // size of structure
    wcx.style = CS_HREDRAW | CS_VREDRAW;                    // redraw if size changes
    wcx.lpfnWndProc = WinProc;     // points to window procedure
    wcx.cbClsExtra = 0;                // no extra class memory
    wcx.cbWndExtra = sizeof(Window::Instance*);        // store window data
    wcx.hInstance = Application::instance();           // handle to instance
    wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);              // predefined app. icon
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);                    // predefined arrow
    wcx.hbrBackground = bg;
    wcx.lpszMenuName =  _T("MainMenu");    // name of menu resource
    wcx.lpszClassName = className.c_str();  // name of window class
    wcx.hIconSm = (HICON)LoadImage(Application::instance(), MAKEINTRESOURCE(5), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

    // Register the window class.
    if(!::RegisterClassEx(&wcx)) {
        throw Exception("Unable to register class");
    }

    return className;
}

//void addMenuItemSelectHandler(const int& wm, MenuItem::SelectHandler* handler) {
//    menuItemSelectHandlerList.addHandler(wm, handler);
//}

//void addSysTrayActivationHandler(const int& wm, SysTray::OnActivationHandler* handler) {
//    sysTrayActivationHandlerList.addHandler(wm, handler);
//}

//void addSysTrayContextMenuHandler(const int& wm, SysTray::OnContextMenuHandler* handler) {
//    sysTrayContextMenuHandlerList.addHandler(wm, handler);
//}
#endif

#if defined(WIN32)
Window::Instance::Impl& Window::Native::createWindow(const Window::Definition& def, const std::string& className, const int& style, const int& xstyle, HWND parent) {
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

    Window::Instance::Impl* impl = new Window::Instance::Impl();
    ref(impl)._hWindow = ::CreateWindowEx(xstyle,
                                     className.c_str(),
                                     def.title.c_str(),
                                     style,
                                     pos.x, pos.y, pos.w, pos.h,
                                     parent, (HMENU)NULL,
                                     Application::instance(), (LPVOID)impl);
    return ref(impl);
}

Window::Instance::Impl& Window::Native::createMainFrame(const Window::Definition& def, const int& style, const int& xstyle) {
    HBRUSH brush = (def.style == Window::Style::Dialog)?(HBRUSH)GetSysColorBrush(COLOR_3DFACE):(HBRUSH)GetStockObject(WHITE_BRUSH);
    std::string className = registerClass(brush);
    return createWindow(def, className, style, xstyle, (HWND)NULL);
}

Window::Instance::Impl& Window::Native::createChildFrame(const Window::Definition& def, const int &style, const int &xstyle, const Window::Instance &parent) {
    HBRUSH brush = (def.style == Window::Style::Dialog)?(HBRUSH)GetSysColorBrush(COLOR_3DFACE):(HBRUSH)GetStockObject(WHITE_BRUSH);
    std::string className = registerClass(brush);
    return createWindow(def, className, style, xstyle, ref(parent..wdata)._hWindow);
}

Window::Instance::Impl& Window::Native::createChildWindow(const Window::Definition& def, const std::string& className, const int& style, const int& xstyle, const Window::Instance& parent) {
    return createWindow(def, className, style, xstyle, ref(parent.wdata)._hWindow);
}
#endif

#if defined(GTK)
Window::Instance::Impl& Window::Native::initWindowImpl(GtkWidget* hwnd) {
    Window::Instance::Impl* impl = new Window::Instance::Impl();
    ref(impl)._hWindow = hwnd;
    ref(impl)._hFixed = 0;
    g_object_set_data(G_OBJECT(ref(impl)._hWindow), "impl", impl);
    return ref(impl);
}

Window::Instance::Impl& Window::Native::createWindow(const Window::Definition& def, GtkWidget *parent) {
    unused(parent);
    GtkWidget* hwnd = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    Window::Instance::Impl& impl = initWindowImpl(hwnd);
    return impl;
}

Window::Instance::Impl& Window::Native::createChildWindow(GtkWidget* hwnd, const Window::Definition& def, const Window::Instance& parent) {
    gtk_fixed_put (GTK_FIXED (ref(parent.wdata)._hFixed), hwnd, def.position.x, def.position.y);
    Window::Instance::Impl& impl = initWindowImpl(hwnd);
    gtk_widget_show(impl._hWindow);
    return impl;
}
#endif

////////////////////////////////////////////////////////////////////////////////
Window::Position Window::getWindowPosition(const Instance& window) {
#if defined(WIN32)
    RECT rc;
    ::GetWindowRect(ref(window.wdata)._hWindow, &rc);
    const Window::Position pos = Window::Position()
            ._x<Window::Position>(rc.left)
            ._y<Window::Position>(rc.top)
            ._w<Window::Position>(rc.right - rc.left)
            ._h<Window::Position>(rc.bottom - rc.top);
#endif
#if defined(GTK)
    GtkRequisition req;
    gtk_widget_size_request(ref(window.wdata)._hWindow, &req);
    const Window::Position pos = Window::Position()
            ._x<Window::Position>(0)
            ._y<Window::Position>(0)
            ._w<Window::Position>(req.width)
            ._h<Window::Position>(req.height);
#endif
    return pos;
}

Window::Position Window::getChildPosition(const Instance& window) {
#if defined(WIN32)
    RECT rc;
    ::GetClientRect(ref(window.wdata)._hWindow, &rc);
    ::MapWindowPoints(ref(window.wdata)._hWindow, ::GetParent(ref(window.wdata)._hWindow), (LPPOINT) &rc, 2);
    return Window::Position()
            ._x<Window::Position>(rc.left)
            ._y<Window::Position>(rc.top)
            ._w<Window::Position>(rc.right - rc.left)
            ._h<Window::Position>(rc.bottom - rc.top);
#endif
#if defined(GTK)
    GtkRequisition req;
    gtk_widget_size_request(ref(window.wdata)._hWindow, &req);
    return Window::Position()
            ._x<Window::Position>(0)
            ._y<Window::Position>(0)
            ._w<Window::Position>(req.width)
            ._h<Window::Position>(req.height);
#endif
}

Window::ChildList& Window::childList(const Window::Instance& window) {
    return ref(window.wdata)._childList;
}

const Window::Delete::_Out& Window::Delete::run(const Window::Instance& window) {
    delete window.wdata;
   //window.wdata = 0;
   return out(_Out());
}

const Window::SetTitle::_Out& Window::SetTitle::run(const Window::Instance& window, const std::string& title) {
#if defined(WIN32)
    ::SetWindowText(ref(window.wdata)._hWindow, title.c_str());
#endif
#if defined(GTK)
    gtk_window_set_title (GTK_WINDOW (ref(window.wdata)._hWindow), title.c_str());
#endif
   return out(_Out());
}

const Window::Show::_Out& Window::Show::run(const Window::Instance& window) {
#if defined(WIN32)
    ::ShowWindow(ref(window.wdata)._hWindow, SW_SHOW);
#endif
#if defined(GTK)
    gtk_widget_show(GTK_WIDGET(ref(window.wdata)._hWindow));
    gtk_window_deiconify(GTK_WINDOW(ref(window.wdata)._hWindow));
#endif
   return out(_Out());
}

const Window::Hide::_Out& Window::Hide::run(const Window::Instance& window) {
#if defined(WIN32)
    ::ShowWindow(ref(window.wdata)._hWindow, SW_HIDE);
#endif
#if defined(GTK)
    gtk_widget_hide(GTK_WIDGET(ref(window.wdata)._hWindow));
#endif
   return out(_Out());
}

const Window::Move::_Out& Window::Move::run(const Window::Instance& window, const Window::Position& position) {
#if defined(WIN32)
    ::MoveWindow(ref(window.wdata)._hWindow, position.x, position.y, position.w, position.h, TRUE);
#endif
#if defined(GTK)
    unused(window); unused(position);
    //gtk_widget_set_uposition(ref(window.wdata)._hWindow, position.x, position.y);
    //gtk_window_set_default_size (ref(window.wdata)._hWindow, position.w, position.h);
#endif
   return out(_Out());
}

const Window::Size::_Out& Window::Size::run(const Window::Instance& window, const int& w, const int& h) {
#if defined(WIN32)
    RECT rc;
    ::GetWindowRect(ref(window.wdata)._hWindow, &rc);
    int tw = (w == -1)?(rc.right - rc.left): w;
    int th = (h == -1)?(rc.bottom - rc.top): h;
    ::MoveWindow(ref(window.wdata)._hWindow, rc.left, rc.top, tw, th, TRUE);
#endif
#if defined(GTK)
    gtk_widget_set_size_request(ref(window.wdata)._hWindow, w, h);
#endif
   return out(_Out());
}

#if defined(GTK)
static gboolean onConfigureEvent(GtkWindow* window, GdkEvent* event, gpointer phandler) {
    unused(window);
    unused(event);
    Window::OnResize::Handler* handler = static_cast<Window::OnResize::Handler*>(phandler);
    Window::OnResize::Handler::_In in;
    ref(handler)._run(in);
    return FALSE;
}
#endif

void Window::OnResize::addHandler(const Window::Instance& window, Handler* handler) {
    Window::OnResize::add(handler);
#if defined(WIN32)
    onResizeHandlerList.addHandler(ref(window.wdata)._hWindow, handler);
#endif
#if defined(GTK)
    g_signal_connect (G_OBJECT (ref(window.wdata)._hWindow), "configure-event", G_CALLBACK (onConfigureEvent), handler);
#endif
}

#if defined(GTK)
static gboolean onWindowCloseEvent(GtkWindow* window, gpointer phandler) {
    unused(window);
    Window::OnClose::Handler* handler = static_cast<Window::OnClose::Handler*>(phandler);
    Window::OnClose::Handler::_In in;
    ref(handler)._run(in);
    return FALSE;
}
#endif

void Window::OnClose::addHandler(const Window::Instance& window, Handler* handler) {
    Window::OnClose::add(handler);
#if defined(WIN32)
    onCloseHandlerList.addHandler(ref(window.wdata)._hWindow, handler);
#endif
#if defined(GTK)
    g_signal_connect (G_OBJECT (ref(window.wdata)._hWindow), "closed", G_CALLBACK (onWindowCloseEvent), handler);
#endif
}
