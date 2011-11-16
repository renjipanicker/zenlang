#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "window.hpp"
#include "windowimpl.hpp"

#if defined(WIN32)
static int lastWM = WM_APP;
static int lastRes = 1000;
static int lastclassId = 1;

int getNextWmID() {
    return lastWM++;
}

int getNextResID() {
    return lastRes++;
}

static std::string getNextClassID() {
    char name[128];
    snprintf(name, 128, "classX%d", lastclassId++);
    return name;
}

HINSTANCE instance() {
    return Application::Instance::Impl::instance()._hInstance;
}

static HandlerList<int, MenuItem::SelectHandler> menuItemSelectHandlerList;
static HandlerList<int, SysTray::OnActivationHandler> sysTrayActivationHandlerList;
static HandlerList<int, SysTray::OnContextMenuHandler> sysTrayContextMenuHandlerList;

static HandlerList<HWND, Button::OnClickHandler> onButtonClickHandlerList;
static HandlerList<HWND, Window::OnResize> onResizeHandlerList;
static HandlerList<HWND, Window::OnCloseHandler> onCloseHandlerList;

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

// Message handler for the app
static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if(lParam == WM_LBUTTONDOWN) {
        if(sysTrayActivationHandlerList.runHandler(message))
            return 1;
    }

    if((lParam == WM_RBUTTONDOWN) || (lParam == WM_CONTEXTMENU)) {
        if(sysTrayContextMenuHandlerList.runHandler(message))
            return 1;
    }

    switch (message) {
        case WM_COMMAND:
            if(menuItemSelectHandlerList.runHandler(LOWORD(wParam)))
                return 1;
            if(LOWORD(wParam) == BN_CLICKED) {
                if(onButtonClickHandlerList.runHandler((HWND)lParam))
                    return 1;
            }
            break;

        case WM_SIZE:
            if(onResizeHandlerList.runHandler(hWnd))
                return 1;
            break;

        case WM_CLOSE:
            if(onCloseHandlerList.runHandler(hWnd))
                return 1;
            break;
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
    wcx.cbWndExtra = 0;                // no extra window memory
    wcx.hInstance = instance();         // handle to instance
    wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);              // predefined app. icon
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);                    // predefined arrow
    wcx.hbrBackground = bg;
    wcx.lpszMenuName =  _T("MainMenu");    // name of menu resource
    wcx.lpszClassName = className.c_str();  // name of window class
    wcx.hIconSm = (HICON)LoadImage(instance(), MAKEINTRESOURCE(5), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

    // Register the window class.
    if(!::RegisterClassEx(&wcx)) {
        throw Exception("Unable to register class");
    }

    return className;
}

void addMenuItemSelectHandler(const int& wm, MenuItem::SelectHandler* handler) {
    menuItemSelectHandlerList.addHandler(wm, handler);
}

void addSysTrayActivationHandler(const int& wm, SysTray::OnActivationHandler* handler) {
    sysTrayActivationHandlerList.addHandler(wm, handler);
}

void addSysTrayContextMenuHandler(const int& wm, SysTray::OnContextMenuHandler* handler) {
    sysTrayContextMenuHandlerList.addHandler(wm, handler);
}

void addOnResizeHandler(HWND hwnd, Window::OnResize *handler) {
    onResizeHandlerList.addHandler(hwnd, handler);
}

void addOnCloseHandler(HWND hwnd, Window::OnCloseHandler *handler) {
    onCloseHandlerList.addHandler(hwnd, handler);
}

void addOnButtonClickHandler(HWND hwnd, Button::OnClickHandler *handler) {
    onButtonClickHandlerList.addHandler(hwnd, handler);
}
#endif

#if defined(WIN32)
void Window::Native::createWindow(Window::Create& action, const std::string& className, const int& style, const int& xstyle, HWND parent) {
    Position pos(CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT);
    if(action._position.x != -1)
        pos.x = action._position.x;
    if(action._position.y != -1)
        pos.y = action._position.y;
    if(action._position.w != -1)
        pos.w = action._position.w;
    if(action._position.h != -1)
        pos.h = action._position.h;

    action._window._impl->_hWindow = ::CreateWindowEx(xstyle,
                                                      className.c_str(),
                                                      action._title.c_str(),
                                                      style,
                                                      pos.x, pos.y, pos.w, pos.h,
                                                      parent, (HMENU)NULL,
                                                      instance(), (LPVOID)NULL);
}

void Window::Native::createMainFrame(Window::Create& action, const int& style, const int& xstyle) {
    HBRUSH brush = (action._style == Window::Style::Dialog)?(HBRUSH)GetSysColorBrush(COLOR_3DFACE):(HBRUSH)GetStockObject(WHITE_BRUSH);
    std::string className = registerClass(brush);
    return createWindow(action, className, style, xstyle, (HWND)NULL);
}

void Window::Native::createChildFrame(Window::Create& action, const int &style, const int &xstyle, const Window::Instance &parent) {
    HBRUSH brush = (action._style == Window::Style::Dialog)?(HBRUSH)GetSysColorBrush(COLOR_3DFACE):(HBRUSH)GetStockObject(WHITE_BRUSH);
    std::string className = registerClass(brush);
    return createWindow(action, className, style, xstyle, parent._impl->_hWindow);
}

void Window::Native::createChildWindow(Window::Create& action, const std::string& className, const int& style, const int& xstyle, const Window::Instance& parent) {
    return createWindow(action, className, style, xstyle, parent._impl->_hWindow);
}
#endif

#if defined(GTK)
Window::Instance::Impl Window::Native::createWindow(const Window::Definition& def, GtkWidget *parent) {
    unused(parent);
    Window::Instance::Impl impl;
    impl._hWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (impl._hWindow), def.title.c_str());
    impl._hFixed = 0;
    return impl;
}

void Window::Native::createChildWindow(Window::Instance::Impl& impl, const Window::Definition& def, const Window::Instance& parent) {
    trace("createChildWindow: %d\n", impl._hWindow);
    gtk_fixed_put (GTK_FIXED (parent._impl->_hFixed), impl._hWindow, def.position.x, def.position.y);
    gtk_widget_show(impl._hWindow);
}
#endif

////////////////////////////////////////////////////////////////////////////////
Window::Position Window::getWindowPosition(const Instance& window) {
#if defined(WIN32)
    RECT rc;
    ::GetWindowRect(window._impl->_hWindow, &rc);
    return Window::Position()
            ._x<Window::Position>(rc.left)
            ._y<Window::Position>(rc.top)
            ._w<Window::Position>(rc.right - rc.left)
            ._h<Window::Position>(rc.bottom - rc.top);
#endif
#if defined(GTK)
    GtkRequisition req;
    gtk_widget_size_request(window._impl->_hWindow, &req);
    return Window::Position()
            ._x<Window::Position>(0)
            ._y<Window::Position>(0)
            ._w<Window::Position>(req.width)
            ._h<Window::Position>(req.height);
#endif
}

Window::Position Window::getChildPosition(const Instance& window) {
#if defined(WIN32)
    RECT rc;
    ::GetClientRect(window._impl->_hWindow, &rc);
    ::MapWindowPoints(window._impl->_hWindow, ::GetParent(window._impl->_hWindow), (LPPOINT) &rc, 2);
    return Window::Position()
            ._x<Window::Position>(rc.left)
            ._y<Window::Position>(rc.top)
            ._w<Window::Position>(rc.right - rc.left)
            ._h<Window::Position>(rc.bottom - rc.top);
#endif
#if defined(GTK)
    GtkRequisition req;
    gtk_widget_size_request(window._impl->_hWindow, &req);
    return Window::Position()
            ._x<Window::Position>(0)
            ._y<Window::Position>(0)
            ._w<Window::Position>(req.width)
            ._h<Window::Position>(req.height);
#endif
}

const Window::Delete::_Out& Window::Delete::run(const _In& _in) {
    delete _in.window._impl;
   //_in.window._impl = 0;
   return out(new _Out());
}

const Window::SetTitle::_Out& Window::SetTitle::run(const _In& _in) {
#if defined(WIN32)
    ::SetWindowText(_in.window._impl->_hWindow, _in.title.c_str());
#endif
#if defined(GTK)
    gtk_window_set_title (GTK_WINDOW (_in.window._impl->_hWindow), _in.title.c_str());
#endif
   return out(new _Out());
}

const Window::Show::_Out& Window::Show::run(const _In& _in) {
#if defined(WIN32)
    ::ShowWindow(_in.window._impl->_hWindow, SW_SHOW);
#endif
#if defined(GTK)
    gtk_widget_show(GTK_WIDGET(_in.window._impl->_hWindow));
    gtk_window_deiconify(GTK_WINDOW(_in.window._impl->_hWindow));
#endif
   return out(new _Out());
}

const Window::Hide::_Out& Window::Hide::run(const _In& _in) {
#if defined(WIN32)
    ::ShowWindow(_in.window._impl->_hWindow, SW_HIDE);
#endif
#if defined(GTK)
    gtk_widget_hide(GTK_WIDGET(_in.window._impl->_hWindow));
#endif
   return out(new _Out());
}

const Window::Move::_Out& Window::Move::run(const _In& _in) {
#if defined(WIN32)
    ::MoveWindow(_in.window._impl->_hWindow, _in.position.x, _in.position.y, _in.position.w, _in.position.h, TRUE);
#endif
#if defined(GTK)
    unused(_in);
    //gtk_widget_set_uposition(_window._impl->_hWindow, _position.x, _position.y);
    //gtk_window_set_default_size (_window._impl->_hWindow, _position.w, _position.h);
#endif
   return out(new _Out());
}

const Window::Size::_Out& Window::Size::run(const _In& _in) {
#if defined(WIN32)
    RECT rc;
    ::GetWindowRect(_in.window._impl->_hWindow, &rc);
    int w = (_in.w == -1)?(rc.right - rc.left): _in.w;
    int h = (_in.h == -1)?(rc.bottom - rc.top): _in.h;
    ::MoveWindow(_in.window._impl->_hWindow, rc.left, rc.top, w, h, TRUE);
#endif
#if defined(GTK)
    gtk_widget_set_size_request(_in.window._impl->_hWindow, _in.w, _in.h);
#endif
   return out(new _Out());
}

#if defined(GTK)
static gboolean onConfigureEvent(GtkWindow* window, GdkEvent* event, gpointer phandler) {
    unused(window);
    unused(event);
    Window::OnResize::Handler* handler = static_cast<Window::OnResize::Handler*>(phandler);
    Window::OnResize::Handler::_In in;
    ref(handler).run(in);
    return FALSE;
}
#endif

const Window::OnResize::Add::_Out& Window::OnResize::Add::run(const _In& _in) {
    Window::OnResize::Handler& h = Window::OnResize::add(_in.handler);
#if defined(WIN32)
    onResizeHandlerList.addHandler(_in.window._impl->_hWindow, ptr(h));
#endif
#if defined(GTK)
    trace("Window::OnResize::Handler::run(): handler = %lu, window = %d\n", ptr(h), _in.window._impl->_hWindow);
    g_signal_connect (G_OBJECT (_in.window._impl->_hWindow), "configure-event", G_CALLBACK (onConfigureEvent), ptr(h));
#endif
   return out(new _Out());
}

#if defined(GTK)
static gboolean onWindowCloseEvent(GtkWindow* window, gpointer phandler) {
    unused(window);
    trace("onWindowCloseEvent\n");
    Window::OnClose::Handler* handler = static_cast<Window::OnClose::Handler*>(phandler);
    Window::OnClose::Handler::_In in;
    ref(handler).run(in);
    return FALSE;
}
#endif

const Window::OnClose::Add::_Out& Window::OnClose::Add::run(const _In& _in) {
    Window::OnClose::Handler& h = Window::OnClose::add(_in.handler);
#if defined(WIN32)
    onCloseHandlerList.addHandler(_in.window._impl->_hWindow, ptr(h));
#endif
#if defined(GTK)
    g_signal_connect (G_OBJECT (_in.window._impl->_hWindow), "closed", G_CALLBACK (onWindowCloseEvent), ptr(h));
#endif
   return out(new _Out());
}


//////////////////////////
int Main(const std::list<std::string>& argl) {
    return 0;
}
