#include "pch.hpp"
#include "zenlang.hpp"

template <typename InitT>
struct InitList {
    inline InitList() : _head(0), _tail(0), _next(0) {}

    inline void push(InitT* inst) {
        if(_tail == 0) {
            assert(_head == 0);
            _head = inst;
            _next = _head;
        } else {
            ref(_tail)._next = inst;
        }
        _tail = inst;
    }

    inline InitT* next() {
        InitT* n = _next;
        if(n != 0) {
            _next = ref(_next)._next;
        }
        return n;
    }

private:
    InitT* _head;
    InitT* _tail;
    InitT* _next;
};

#if defined(UNIT_TEST)
static InitList<TestInstance> s_testList;
TestInstance::TestInstance() : _next(0) {
    s_testList.push(this);
}
#endif

static InitList<MainInstance> s_mainList;

MainInstance::MainInstance() : _next(0) {
    s_mainList.push(this);
}

CallContext g_context = CallContext();
CallContext& CallContext::get() {
    return g_context;
}

void CallContext::run() {
    while(_list.size() > 0) {
        FunctionList::Ptr ptr;
        if(_list.pop(ptr))
            ptr->run();
    }
}

#if defined(Z_EXE)
static void pump() {
    g_context.run();
}

#if defined(GUI)
#if defined(WIN32)
static int lastWM = WM_APP;
static int lastRes = 1000;

int win32::getNextWmID() {
    return lastWM++;
}

int win32::getNextResID() {
    return lastRes++;
}

static void CALLBACK IdleProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD time) {
    unused(hwnd);
    unused(uMsg);
    unused(idEvent);
    unused(time);
    pump();
}
#endif
#if defined(GTK)
static gboolean onIdle(gpointer data) {
    unused(data);
    pump();
    return TRUE;
}
#endif
#endif

Application::Application(int argc, char* argv[]) {
#if defined(GUI)
#if defined(WIN32)
#endif
#if defined(GTK)
    gtk_init(&argc, &argv);
#endif
#endif
}

Application::~Application() {
}

#if defined(WIN32)
static HINSTANCE s_hInstance = 0;
HINSTANCE Application::instance() {
    return s_hInstance;
}
#endif

int Application::exec() {
    int code = 0;
#if defined(GUI)
#if defined(WIN32)
    int timerID = win32::getNextResID();
    UINT timer = SetTimer(NULL, timerID, 0, IdleProc);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KillTimer(NULL, timer);
    code = (int)msg.wParam;
#endif
#if defined(GTK)
    g_idle_add(onIdle, 0);
    gtk_main();
#endif
#else
    pump();
#endif
    return code;
}

int Application::exit(const int& code) {
#if defined(GUI)
#if defined(WIN32)
    ::PostQuitMessage(code);
#endif
#if defined(GTK)
    gtk_main_quit();
#endif
#endif
    return code;
}

static void initMain(int argc, char* argv[]) {
#if defined(UNIT_TEST)
    TestInstance* ti = s_testList.next();
    while(ti != 0) {
        ref(ti).enque(g_context);
        ti = s_testList.next();
    }
#endif

    ArgList argl;
    MainInstance* mi = s_mainList.next();
    while(mi != 0) {
        ref(mi).enque(g_context, argl);
        mi = s_mainList.next();
    }
}

#if defined(GUI) && defined(WIN32)
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    unused(hPrevInstance);
    unused(lpCmdLine);
    unused(nCmdShow);
    Application a(0, 0);
    InitCommonControls();
    s_hInstance = hInstance;
    initMain(0, 0);
    return a.exec();
}
#else
int main(int argc, char* argv[]) {
    Application a(argc, argv);
    initMain(argc, argv);
    return a.exec();
}
#endif
#endif
