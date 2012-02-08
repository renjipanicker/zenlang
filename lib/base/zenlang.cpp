#include "pch.hpp"
#include "zenlang.hpp"

#if defined(DEBUG) && defined(GUI) && defined(WIN32)
void trace(const char* txt, ...) {
    va_list vlist;
    va_start(vlist, txt);
    static const size_t MAXBUF = 1024;
    char buf[MAXBUF];
    vsnprintf_s(buf, MAXBUF, txt, vlist);
    OutputDebugStringA(buf);
}
#endif

z::mutex::mutex() {
#if defined(WIN32)
    _val = CreateMutex(0, FALSE, 0);
#else
    pthread_mutex_init (&_val, NULL);
#endif
}

z::mutex::~mutex() {
#if defined(WIN32)
    ReleaseMutex(_val);
#else
    pthread_mutex_destroy(&_val);
#endif
}

int z::mutex::enter() {
#if defined(WIN32)
    return (WaitForSingleObject(_val, INFINITE)==WAIT_FAILED?1:0);
#else
    return pthread_mutex_lock(&_val);
#endif
}

int z::mutex::leave() {
#if defined(WIN32)
    return (ReleaseMutex(_val)==0);
#else
    return pthread_mutex_unlock(&_val);
#endif
}

void z::regex::compile(const z::string& re) {
#if !defined(WIN32)
    int res = regcomp(&_val, re.c_str(), 0);
    if(res != 0) {
        throw z::Exception("z::regex", z::fmt("regcomp failed for: %{s}").add("s", re));
    }
#endif
}

void z::regex::match(const z::string& str) {
#if !defined(WIN32)
    int res = regexec(&_val, str.c_str(), 0, 0, 0);
    char buf[128];
    regerror(res, &_val, buf, 128);
#endif
}

#ifdef WIN32
const z::string z::file::sep = "\\";
#else
const z::string z::file::sep = "/";
#endif

bool z::file::exists(const z::string& path) {
    struct stat b;
    return (0 == stat(path.c_str(), &b));
}

int z::file::mkdir(const z::string& path) {
#if defined(WIN32)
    int rv = ::_mkdir(path.c_str());
#else
    int rv = ::mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IROTH);
#endif
    if(rv == -1) {
        if(errno == EEXIST)
            return 0;
    }
    return rv;
}

z::string z::file::cwd() {
    static const size_t MAXBUF = 1024;
    char buff[MAXBUF];
#if defined(WIN32)
    ::_getcwd( buff, MAXBUF);
#else
    ::getcwd( buff, MAXBUF);
#endif
    z::string rv(buff);
    return rv;
}

void z::file::open(const z::string& filename, const z::string& mode, const MkPathT& mkpath) {
    if(_val != 0) {
        close();
    }
    assert(_val == 0);

    if(mkpath == makePath) {
        z::string base = "";
        z::string::size_type prev = 0;
        for(z::string::size_type next = filename.find(sep); next != z::string::npos;next = filename.find(sep, next+1)) {
            z::string sdir = filename.substr(prev, next - prev);
            base += sdir;
            base += sep;
            if(!exists(base)) {
                int rv = mkdir(base.c_str());
                if(rv != 0) {
                    throw z::Exception("z::file", z::fmt("mkdir failed for: %{s}").add("s", base));
                }
            }
            prev = next + 1;
        }

        // check if fname is empty
        assert(filename.substr(prev).size() > 0);
    }

#if defined(WIN32)
    errno_t en = fopen_s(&_val, filename.c_str(), mode.c_str());
    if(en != 0) {
        _val = 0;
    }
#else
    _val = fopen(filename.c_str(), mode.c_str());
#endif
    if(_val == 0) {
        throw z::Exception("z::file", z::fmt("fopen failed for: %{s}").add("s", filename));
    }
    _name = filename;
}

void z::file::open(const z::string& dir, const z::string& filename, const z::string& mode, const MkPathT& mkpath) {
    return open(dir + sep + filename, mode, mkpath);
}

void z::file::close() {
    if(_val == 0)
        return;
    fclose(_val);
    _val = 0;
}

////////////////////////////////////////////////////////////////////////
#if defined(UNIT_TEST)
static int s_totalTests = 0;
static int s_passedTests = 0;

z::TestResult::~TestResult() {
//    Log::get() << "PASSED: " << s_passedTests << "/" << s_totalTests << "\n" << Log::Out();
    printf("PASSED %d/%d\n", s_passedTests, s_totalTests);
}

void z::TestResult::begin(const z::string& name) {
    Log::get() << name << Log::Out();
    ++s_totalTests;
}

void z::TestResult::end(const z::string& name, const bool& passed) {
    if(passed)
        ++s_passedTests;

    const z::string r = passed?" - PASS":" - FAIL ******************";
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

z::size z::CallContext::run(const z::size& cnt) {
    for(z::size i = 0; i < cnt; ++i) {
        if(_list.size() == 0) {
            break;
        }
        FunctionList::Ptr ptr;
        if(_list.pop(ptr))
            ptr->run();
    }
    return _list.size();
}

static const z::size MaxPumpCount = 10;
static void pump() {
    while(g_context.run(MaxPumpCount) > 0) {}
}
#if defined(GUI)
#if defined(WIN32)
static int lastWM = WM_APP;
static int lastRes = 1000;

int z::win32::getNextWmID() {
    return lastWM++;
}

int z::win32::getNextResID() {
    return lastRes++;
}

static void CALLBACK IdleProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD time) {
    unused(hwnd);
    unused(uMsg);
    unused(idEvent);
    unused(time);
    pump();
}
#endif // WIN32
#if defined(GTK)
static gboolean onIdle(gpointer data) {
    unused(data);
    pump();
    return TRUE;
}
#endif // GTK
#endif // GUI

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

#if defined(Z_EXE)
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

    z::Application a(__argc, __argv);
    initMain(__argc, __argv);
    return a.exec();
}
#else // GUI
int main(int argc, char* argv[]) {
    z::Application a(argc, argv);
    initMain(argc, argv);
    return a.exec();
}
#endif // GUI
#endif // Z_EXE

static z::Log s_log;
z::Log& z::Log::get() {
    return s_log;
}

z::Log& z::Log::operator<<(z::Log::Out) {
    printf("%s", _ss.str().c_str());
    _ss.str("");
    return ref(this);
}
