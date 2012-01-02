#include "pch.hpp"
#include "zenlang.hpp"

#if defined(UNIT_TEST)
static int s_totalTests = 0;
static int s_passedTests = 0;

z::TestResult::~TestResult() {
//    Log::get() << "PASSED: " << s_passedTests << "/" << s_totalTests << "\n" << Log::Out();
    printf("PASSED %d/%d\n", s_passedTests, s_totalTests);
}

void z::TestResult::begin(const std::string& name) {
    Log::get() << name << Log::Out();
    ++s_totalTests;
}

void z::TestResult::end(const std::string& name, const bool& passed) {
    if(passed)
        ++s_passedTests;

    const std::string r = passed?" - PASS":" - FAIL ******************";
    Log::get() << r << "\n" << Log::Out();
}

static z::InitList<z::TestInstance> s_testList;
z::TestInstance::TestInstance() : _next(0) {
    s_testList.push(this);
}
#endif

static z::InitList<z::MainInstance> s_mainList;

z::MainInstance::MainInstance() : _next(0) {
    s_mainList.push(this);
}

z::CallContext g_context = z::CallContext();
z::CallContext& z::CallContext::get() {
    return g_context;
}

void z::CallContext::run() {
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

z::Application::Application(int argc, char* argv[]) {
#if defined(GUI)
#if defined(WIN32)
#endif
#if defined(GTK)
    gtk_init(&argc, &argv);
#endif
#endif
}

z::Application::~Application() {
}

#if defined(WIN32)
static HINSTANCE s_hInstance = 0;
HINSTANCE z::Application::instance() {
    return s_hInstance;
}
#endif

int z::Application::exec() {
#if defined(UNIT_TEST)
    TestResult tr; unused(tr);
#endif
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

int z::Application::exit(const int& code) {
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
    z::TestInstance* ti = s_testList.next();
    while(ti != 0) {
        ref(ti).enque(g_context);
        ti = s_testList.next();
    }
#endif

    z::ArgList argl;
    z::MainInstance* mi = s_mainList.next();
    while(mi != 0) {
        ref(mi).enque(g_context, argl);
        mi = s_mainList.next();
    }
}

#if defined(GUI) && defined(WIN32)
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    unused(hPrevInstance);unused(lpCmdLine);unused(nCmdShow);

    InitCommonControls();
    s_hInstance = hInstance;

    Application a(__argc, __argv);
    initMain(__argc, __argv);
    return a.exec();
}
#else
int main(int argc, char* argv[]) {
    z::Application a(argc, argv);
    initMain(argc, argv);
    return a.exec();
}
#endif
#endif

static z::Log s_log;
z::Log& z::Log::get() {
    return s_log;
}

z::Log& z::Log::operator<<(z::Log::Out) {
    printf("%s", _ss.str().c_str());
    _ss.str("");
    return ref(this);
}
