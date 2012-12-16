# include "zenlang.hpp"
#include "base/base.hpp"

// all webcontrol header files are included here.
#if defined(GUI)
    #if defined(WIN32)
        # include <exdisp.h>
        # include <mshtml.h>
        # include <mshtmhst.h>
        # include <ExDispId.h>
        # include <mshtmdid.h>
        # include <DispEx.h>
    #elif defined(QT)
        # include <QtGui/QApplication>
        # include <QtCore/QTimer>
        # include <QtWebKit/QWebView>
        # include <QtGui/QCloseEvent>
    #elif defined(OSX)
        // Note that this is import, not include. Which is why this must be here, not in pch.hpp
        // In OSX mode, this file *must* be compiled as a Obj-C++ file, not C++ file. In Xcode, go to
        // Project/Build Phases/Compile Sources/ and select the zenlang.cpp file.
        // Add "-x objective-c++" to the "Compiler Flags" column.
        # import <Cocoa/Cocoa.h>
        # import <AppKit/AppKit.h>
        # import <WebKit/WebKit.h>
    #elif defined(IOS)
        // Same as above note. This is an import.
        # import <UIKit/UIKit.h>
    #else
        #error "Unimplemented GUI mode"
    #endif
#else
    #if defined(OSX) || defined(IOS)
        // Same as above note. This is an import.
        # import <Cocoa/Cocoa.h>
    #endif
#endif

///////////////////////////////////////////////////////////////
#if defined(__APPLE__)
# include <mach-o/dyld.h>
#endif

///////////////////////////////////////////////////////////////
/// \brief Global singleton application instance
static z::application* g_app = 0;

////////////////////////////////////////////////////////////////////////////
void z::writelog(const z::string& msg) {
#if defined(OSX) || defined(IOS)
    std::cout << msg << std::endl;
#endif
    if(g_app) {
        z::app().writeLog(msg);
    } else {
        std::cout << msg << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////
// utf8 conversion code adapted
// from http://www.codeguru.com/cpp/misc/misc/multi-lingualsupport/article.php/c10451
#define MASKBITS   (uint64_t)0x3F
#define MASKBYTE   (uint64_t)0x80
#define MASK2BYTES (uint64_t)0xC0
#define MASK3BYTES (uint64_t)0xE0
#define MASK4BYTES (uint64_t)0xF0
#define MASK5BYTES (uint64_t)0xF8
#define MASK6BYTES (uint64_t)0xFC

z::string08 z::c32to08(const z::string32& in) {
    z::string08 rv;
    for(z::string32::size_type i = 0; i < in.size(); i++) {
        if(in.at(i) < 0x80) {
            // 0xxxxxxx
            rv.append((char08_t)in.at(i));
        } else if(in.at(i) < 0x800) {
            // 110xxxxx 10xxxxxx
            rv.append((char08_t)(MASK2BYTES | (in.at(i) >> 6)));
            rv.append((char08_t)(MASKBYTE   | (in.at(i) & MASKBITS)));
        } else if(in.at(i) < 0x10000) {
            // 1110xxxx 10xxxxxx 10xxxxxx
            rv.append((char08_t)(MASK3BYTES | (in.at(i) >> 12)));
            rv.append((char08_t)(MASKBYTE   | ((in.at(i) >> 6) & MASKBITS)));
            rv.append((char08_t)(MASKBYTE   | (in.at(i) & MASKBITS)));
        } else if(in.at(i) < 0x200000) {
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            rv.append((char08_t)(MASK4BYTES | (in.at(i) >> 18)));
            rv.append((char08_t)(MASKBYTE   | ((in.at(i) >> 12) & MASKBITS)));
            rv.append((char08_t)(MASKBYTE   | ((in.at(i) >> 6) & MASKBITS)));
            rv.append((char08_t)(MASKBYTE   | (in.at(i) & MASKBITS)));
        } else if(in.at(i) < 0x4000000) {
            // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            rv.append((char08_t)(MASK5BYTES | (in.at(i) >> 24)));
            rv.append((char08_t)(MASKBYTE   | ((in.at(i) >> 18) & MASKBITS)));
            rv.append((char08_t)(MASKBYTE   | ((in.at(i) >> 12) & MASKBITS)));
            rv.append((char08_t)(MASKBYTE   | ((in.at(i) >> 6) & MASKBITS)));
            rv.append((char08_t)(MASKBYTE   | (in.at(i) & MASKBITS)));
        } else if(in.at(i) < 0x8000000) {
            // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            rv.append((char08_t)(MASK6BYTES | (in.at(i) >> 30)));
            rv.append((char08_t)(MASKBYTE   | ((in.at(i) >> 18) & MASKBITS)));
            rv.append((char08_t)(MASKBYTE   | ((in.at(i) >> 12) & MASKBITS)));
            rv.append((char08_t)(MASKBYTE   | ((in.at(i) >> 6) & MASKBITS)));
            rv.append((char08_t)(MASKBYTE   | (in.at(i) & MASKBITS)));
        }
    }
    return rv;
}

z::string32 z::c08to32(const z::string08& in) {
    z::string32 rv;
    for(z::string08::size_type i = 0; i < in.size();) {
        int64_t ch = 0;
        if(((uint64_t)in.at(i) & MASK6BYTES) == MASK6BYTES) {
            // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            ch = ((in.at(i) & 0x01) << 30) | ((in[i+1] & MASKBITS) << 24) | ((in[i+2] & MASKBITS) << 18) | ((in[i+3] & MASKBITS) << 12) | ((in[i+4] & MASKBITS) << 6) | (in[i+5] & MASKBITS);
            i += 6;
        } else if(((uint64_t)in.at(i) & MASK5BYTES) == MASK5BYTES) {
            // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            ch = ((in.at(i) & 0x03) << 24) | ((in[i+1] & MASKBITS) << 18) | ((in[i+2] & MASKBITS) << 12) | ((in[i+3] & MASKBITS) << 6) | (in[i+4] & MASKBITS);
            i += 5;
        } else if(((uint64_t)in.at(i) & MASK4BYTES) == MASK4BYTES) {
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            ch = ((in.at(i) & 0x07) << 18) | ((in[i+1] & MASKBITS) << 12) | ((in[i+2] & MASKBITS) << 6) | (in[i+3] & MASKBITS);
            i += 4;
        } else if(((uint64_t)in.at(i) & MASK3BYTES) == MASK3BYTES) {
            // 1110xxxx 10xxxxxx 10xxxxxx
            ch = ((in.at(i) & 0x0F) << 12) | ((in[i+1] & MASKBITS) << 6) | (in[i+2] & MASKBITS);
            i += 3;
        } else if(((uint64_t)in.at(i) & MASK2BYTES) == MASK2BYTES) {
            // 110xxxxx 10xxxxxx
            ch = ((in.at(i) & 0x1F) << 6) | (in[i+1] & MASKBITS);
            i += 2;
        } else if((uint64_t)in.at(i) < MASKBYTE) {
            // 0xxxxxxx
            ch = in.at(i);
            i += 1;
        } else {
            assert(false);
        }
        rv.append((char32_t)ch);
    }
    return rv;
}

z::string16 z::c08to16(const z::string08& in) {
    z::string16 rv;
    for(z::string08::size_type i = 0; i < in.size();) {
        char16_t ch;
        if((in.at(i) & MASK3BYTES) == MASK3BYTES) {
            // 1110xxxx 10xxxxxx 10xxxxxx
            ch = ((in.at(i) & 0x0F) << 12) | ( (in[i+1] & MASKBITS) << 6) | (in[i+2] & MASKBITS);
            i += 3;
        } else if((in.at(i) & MASK2BYTES) == MASK2BYTES) {
            // 110xxxxx 10xxxxxx
            ch = ((in.at(i) & 0x1F) << 6) | (in[i+1] & MASKBITS);
            i += 2;
        } else if((uint64_t)in.at(i) < MASKBYTE) {
            // 0xxxxxxx
            ch = in.at(i);
            i += 1;
        }
        rv.append(ch);
    }
    return rv;
}

z::string08 z::c16to08(const z::string16& in) {
    z::string08 rv;
    for(z::string16::size_type i = 0; i < in.size(); i++) {
        if((uint16_t)in.at(i) < (uint16_t)0x80) {
            // 0xxxxxxx
            rv.append((char08_t)in.at(i));
        } else if((uint16_t)in.at(i) < (uint16_t)0x0800) {
            // 110xxxxx 10xxxxxx
            rv.append((char08_t)(MASK2BYTES | (in.at(i) >> 6)));
            rv.append((char08_t)(MASKBYTE | (in.at(i) & MASKBITS)));
//@        } else if((uint64_t)in.at(i) < (uint64_t)0x10000) {
//            // 1110xxxx 10xxxxxx 10xxxxxx
//            rv.append((char08_t)(MASK3BYTES | (in.at(i) >> 12)));
//            rv.append((char08_t)(MASKBYTE | ((in.at(i) >> 6) & MASKBITS)));
//            rv.append((char08_t)(MASKBYTE | (in.at(i) & MASKBITS)));
        }
    }
    return rv;
}

z::string32 z::c16to32(const z::string16& in) {
    z::string32 rv;
    for(z::string16::size_type i = 0; i < in.size(); i++) {
        const char16_t& ch = in.at(i);
        rv.append(ch);
    }
    return rv;
}

z::string16 z::c32to16(const z::string32& in) {
    z::string16 rv;
    for(z::string32::size_type i = 0; i < in.size(); i++) {
        const char32_t& ch = in.at(i);
        rv.append((char16_t)ch);
    }
    return rv;
}

////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////
void z::regex::compile(const z::string& re) {
#if defined(WIN32)
    unused(re);
    UNIMPL();
#else
    int res = regcomp(&_val, s2e(re).c_str(), 0);
    if(res != 0) {
        throw z::Exception("z::regex", z::string("regcomp failed for: %{s}").arg("s", re));
    }
#endif
}

void z::regex::match(const z::string& str) {
#if defined(WIN32)
    unused(str);
    UNIMPL();
#else
    int res = regexec(&_val, s2e(str).c_str(), 0, 0, 0);
    char buf[128];
    regerror(res, &_val, buf, 128);
#endif
}

////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
const z::string z::dir::sep = "\\";
#else
const z::string z::dir::sep = "/";
#endif

z::string z::dir::cleanPath(const z::string& path) {
    z::string r = path;
    r.replace("//", "/");
    r.replace("/./", "/");
    return r;
}

bool z::dir::exists(const z::string& path) {
    struct stat b;
    return (0 == stat(s2e(path).c_str(), &b));
}

int z::dir::mkdir(const z::string& path) {
#if defined(WIN32)
    int rv = ::_mkdir(s2e(path).c_str());
#else
    int rv = ::mkdir(s2e(path).c_str(), S_IRWXU | S_IRGRP | S_IROTH);
#endif
    if(rv == -1) {
        if(errno == EEXIST)
            return 0;
    }
    return rv;
}

void z::dir::mkpath(const z::string& filename) {
    z::string base = "";
    z::string::size_type prev = 0;
    for(z::string::size_type next = filename.find("/"); next != z::string::npos;next = filename.find("/", next+1)) {
        z::string sdir = filename.substr(prev, next - prev);
        if((base.size() == 0) || (sdir.size() > 0)) { // if base is empty or multiple / in path
            base += sdir;
            base += "/";
            if(!z::dir::exists(base)) {
                int rv = mkdir(base);
                if(rv != 0) {
                    throw z::Exception("z::dir", z::string("mkdir failed for: %{s}").arg("s", base));
                }
            }
        }
        prev = next + 1;
    }
}

z::string z::dir::getPath(const z::string& filename) {
    z::string basename = filename;

    // strip path, if any
    z::string::size_type idx = basename.rfind('/');
    if(idx != z::string::npos)
        basename = basename.substr(0, idx + 1);

    return basename;
}

z::string z::dir::getFilename(const z::string& filename) {
    z::string basename = filename;

    // strip path, if any
    z::string::size_type idx = basename.rfind('/');
    if(idx != z::string::npos)
        basename = basename.substr(idx + 1);

    return basename;
}

z::string z::dir::getBaseName(const z::string& filename) {
    z::string basename = filename;
    z::string::size_type idx = z::string::npos;

    // strip last extension, if any
    idx = basename.rfind('.');
    if(idx != z::string::npos)
        basename = basename.substr(0, idx);

    // strip path, if any
    idx = basename.rfind('/');
    if(idx != z::string::npos)
        basename = basename.substr(idx + 1);

    return basename;
}

z::string z::dir::getExtention(const z::string& filename) {
    z::string::size_type idx = z::string::npos;

    // find last extension, if any
    idx = filename.rfind('.');
    if(idx != z::string::npos)
        return filename.substr(idx + 1);
    return "";
}

z::string z::dir::cwd() {
    static const size_t MAXBUF = 1024;
    char buff[MAXBUF];
#if defined(WIN32)
    ::_getcwd( buff, MAXBUF);
#else
    char* p = ::getcwd( buff, MAXBUF);
    unused(p);
#endif
    z::string rv(buff);
    return rv;
}

////////////////////////////////////////////////////////////////////////////
z::ofile::ofile(const z::string& filename) {
    _name = filename;
    _os.open(s2e(_name).c_str());
    if(!_os.is_open()) {
        throw Exception("z::ofile", z::string("Error opening %{s}").arg("s", filename));
    }
}

////////////////////////////////////////////////////////////////////////
#if defined(UNIT_TEST)
static int s_totalTests = 0;
static int s_passedTests = 0;

z::TestResult::~TestResult() {
    std::cout << "PASSED " << s_passedTests << "/" << s_totalTests << std::endl;
}

void z::TestResult::begin(const z::string& name) {
    std::cout << name << std::endl;
    ++s_totalTests;
}

void z::TestResult::end(const z::string& name, const bool& passed) {
    unused(name);
    if(passed)
        ++s_passedTests;

    const z::string r = passed?" - PASS":" - FAIL ******************";
    std::cout << r << "\n" << std::endl;
}

///////////////////////////////////////////////////////////////
namespace z {
    template<> z::TestInstance* z::InitList<z::TestInstance>::_head = 0;
    template<> z::TestInstance* z::InitList<z::TestInstance>::_tail = 0;
    template<> z::TestInstance* z::InitList<z::TestInstance>::_next = 0;
    static z::InitList<z::TestInstance> s_testList;

    TestInstance::TestInstance() : _next(0) {
        s_testList.push(this);
    }
}
#endif

///////////////////////////////////////////////////////////////
namespace z {
    template<> z::MainInstance* z::InitList<z::MainInstance>::_head = 0;
    template<> z::MainInstance* z::InitList<z::MainInstance>::_tail = 0;
    template<> z::MainInstance* z::InitList<z::MainInstance>::_next = 0;
    static z::InitList<z::MainInstance> s_mainList;
    z::MainInstance::MainInstance() : _next(0) {
        s_mainList.push(this);
    }
}

/// \brief Return reference to global application instance.
const z::application& z::app() {
    return z::ref(g_app);
}

///////////////////////////////////////////////////////////////
/// \brief This is a static instance.
/// It should be stored in the TLS when running in multithreaded mode.
static z::ThreadContext* s_tctx = 0;

///////////////////////////////////////////////////////////////
static const size_t MaxPumpCount = 10;
static const size_t MaxPollTimeout = 10;

///////////////////////////////////////////////////////////////
/// \brief run queue
struct z::RunQueue {
private:
    typedef z::queue<z::Future*> InvocationList;
    InvocationList _list;
public:
    inline size_t run(const size_t& cnt) {
        assert(s_tctx != 0);
        for(size_t i = 0; i < cnt; ++i) {
            if(_list.size() == 0) {
                break;
            }
            z::autoptr<z::Future> ptr(_list.dequeue());
            ptr->run();
        }
        return _list.size();
    }

    inline void add(Future* future) {
        _list.enqueue(future);
    }

    inline size_t size() const {return _list.size();}
};

///////////////////////////////////////////////////////////////
/// \brief Maintains the run queue's
struct z::GlobalContext {
private:
    typedef z::list<z::RunQueue*> QueueList;
    QueueList _queueList;

private:
    typedef z::list<z::device*> DeviceList;
    DeviceList _deviceList;
    DeviceList _newDeviceList;

public:
    inline GlobalContext() {
        _queueList.add(new z::RunQueue());
    }

    inline ~GlobalContext() {
        for(QueueList::iterator it = _queueList.begin(); it != _queueList.end(); ++it) {
            z::RunQueue* queue = *it;
            delete queue;
        }
    }

    inline z::RunQueue& at(const size_t& idx) {
        return z::ref(_queueList.at(idx));
    }

    inline z::device& startPoll(z::device* d) {
        _newDeviceList.add(d);
        return z::ref(d);
    }

    inline void stopPoll(z::device& d) {
        for(DeviceList::iterator it = _deviceList.begin(); it != _deviceList.end(); ++it) {
            z::device* d1 = *it;
            if(z::ptr(d) == d1) {
                _deviceList.erase(it);
                delete d1;
                break;
            }
        }
    }

    inline size_t run(const size_t& cnt) {
        size_t bal = 0;
        for(QueueList::iterator it = _queueList.begin(); it != _queueList.end(); ++it) {
            z::RunQueue& queue = z::ref(*it);
            bal += queue.run(cnt);
        }
        if(bal == 0) {
            _deviceList.append(_newDeviceList);
            _newDeviceList.clear();
            for(DeviceList::iterator it = _deviceList.begin(); it != _deviceList.end();) {
                z::device& d = z::ref(*it);
                // if run() returns true, end polling for this device
                if(d.run(MaxPollTimeout)) {
                    z::device* ed = *it;
                    it = _deviceList.erase(it); // right way to erase while iterating
                    delete ed;
                } else {
                    ++it;
                }
            }
        }
        return bal;
    }
};

///////////////////////////////////////////////////////////////
/// \brief Return reference to global application object
inline z::GlobalContext& gctx() {
    return z::ref(g_app).ctx();
}

/// \brief Run global pump until empty.
static void pump() {
    while(gctx().run(MaxPumpCount) > 0) {}
}

///////////////////////////////////////////////////////////////
z::ThreadContext& z::ctx() {
    return z::ref(s_tctx);
}

z::ThreadContext::ThreadContext(z::RunQueue& queue) : _queue(queue) {
    assert(s_tctx == 0);
    s_tctx = this;
}

z::ThreadContext::~ThreadContext() {
    assert(s_tctx != 0);
    s_tctx = 0;
}

void z::ThreadContext::add(z::Future* future) {
    _queue.add(future);
}

z::device& z::ThreadContext::startPoll(z::device* d) {
    return gctx().startPoll(d);
}

void z::ThreadContext::stopPoll(z::device& d) {
    return gctx().stopPoll(d);
}

size_t z::ThreadContext::wait() {
    pump();
    return 0;
}

#if defined(GUI)
#if defined(WIN32)
//static int lastWM = WM_APP;
static int lastRes = 1000;

//int z::win32::getNextWmID() {
//    return lastWM++;
//}

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
#if defined(QT)
class ZTimer : public QObject {
    Q_OBJECT;
public slots:
    void OnTimer() {
        pump();
    }
};

////////////////////////////////////////////////////////////////////////
// The code between QT_BEGIN_MOC_NAMESPACE and QT_END_MOC_NAMESPACE is auto-generated by the qt moc.
// For most purposes this should not change, but if it needs to be regenerated, create a Qt-Gui
// project in QtCreator, copy the above ZTimer class into it, build it and then copy the contents of
// the generated moc_ZTimer.cpp file here.
QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ZTimer[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ZTimer[] = {
    "ZTimer\0\0OnTimer()\0"
};

const QMetaObject ZTimer::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ZTimer,
      qt_meta_data_ZTimer, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ZTimer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ZTimer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ZTimer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ZTimer))
        return static_cast<void*>(const_cast< ZTimer*>(this));
    return QObject::qt_metacast(_clname);
}

int ZTimer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: OnTimer(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
#elif defined(OSX) || defined(IOS)
@interface CTimer : NSObject {
    NSTimer* timer;
}
@end

@implementation CTimer
-(CTimer*) initTimer {
    timer = [NSTimer scheduledTimerWithTimeInterval: 1.0 target:self selector:@selector(targetMethod:) userInfo:nil repeats: YES];
    return self;
}

//define the targetmethod
-(void) targetMethod:(NSTimer*)theTimer {
    pump();
}
@end
#if defined(IOS)
#endif // IOS
#endif // OSX|IOS
#endif // GUI

#if defined(GUI) && defined(Z_EXE)
#if defined(WIN32)
namespace zz {
    // The following IE container code adapted from Tobbe Lundberg's project at: https://github.com/Tobbe/CppIEEmbed
    #define NOTIMPLEMENTED _ASSERT(0); return E_NOTIMPL
    class MainWindow : public IOleClientSite, IDispatch, IDocHostShowUI, IDocHostUIHandler, IOleInPlaceSite, IOleInPlaceFrame {
    private:
	    long ref;
	    unsigned int isnaving;    // bitmask: 4=haven't yet finished Navigate call, 2=haven't yet received DocumentComplete, 1=haven't yet received BeforeNavigate

	    UINT id;
	    IWebBrowser2 *ibrowser;   // Our pointer to the browser itself. Released in Close().
	    DWORD cookie;             // By this cookie shall the watcher be known

	    bool hasScrollbars;       // This is read from WS_VSCROLL|WS_HSCROLL at WM_CREATE
	    TCHAR *url;               // This was the url that the user just clicked on
	    TCHAR *kurl;              // Key\0Value\0Key2\0Value2\0\0 arguments for the url just clicked on

	    IHTMLDocument2 *GetDoc();
	    HWND hWnd;

    private:
	    static const DISPID DISPID_USER_EXECUTE = DISPID_VALUE + 1;
	    static const DISPID DISPID_USER_WRITEFILE = DISPID_VALUE + 2;
	    static const DISPID DISPID_USER_READFILE = DISPID_VALUE + 3;
	    static const DISPID DISPID_USER_GETVAL = DISPID_VALUE + 4;
	    static const DISPID DISPID_USER_SETVAL = DISPID_VALUE + 5;

	    std::map<std::wstring, DISPID> idMap;
	    std::map<std::string, std::string> values;

    public:
	    MainWindow();
	    ~MainWindow();
	    void create(HWND hWndParent, HINSTANCE hInstance, UINT id, bool showScrollbars);
	    void CloseThread();
	    void Close();
	    void Resize(const UINT& w, const UINT& h);
        void Go(const z::string& fn);
	    void Forward();
	    void Back();
	    void Refresh(bool clearCache);
	    void RunJSFunction(std::string cmd);
	    void AddCustomObject(IDispatch *custObj, std::string name);
	    static LRESULT CALLBACK WebformWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	    LRESULT InstanceWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	    void setupOle();

	    // IUnknown
	    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv);
	    ULONG STDMETHODCALLTYPE AddRef();
	    ULONG STDMETHODCALLTYPE Release();

	    // IOleClientSite
	    HRESULT STDMETHODCALLTYPE SaveObject(){NOTIMPLEMENTED;}
	    HRESULT STDMETHODCALLTYPE GetMoniker(DWORD /*dwAssign*/, DWORD /*dwWhichMoniker*/, IMoniker** /*ppmk*/){NOTIMPLEMENTED;}
	    HRESULT STDMETHODCALLTYPE GetContainer(IOleContainer** ppContainer){*ppContainer = NULL;return E_NOINTERFACE;}
	    HRESULT STDMETHODCALLTYPE ShowObject(){return S_OK;}
	    HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL /*fShow*/){NOTIMPLEMENTED;}
	    HRESULT STDMETHODCALLTYPE RequestNewObjectLayout(){NOTIMPLEMENTED;}

        // IDispatch
	    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctinfo){*pctinfo = 0; return S_OK;}
	    HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT /*iTInfo*/, LCID /*lcid*/, ITypeInfo** /*ppTInfo*/){return E_FAIL;}
        HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);
	    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid,LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

	    // IDocHostUIHandler
        HRESULT STDMETHODCALLTYPE ShowContextMenu(DWORD /*dwID*/, POINT* /*ppt*/, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/){return S_OK;}
	    HRESULT STDMETHODCALLTYPE ShowUI(DWORD /*dwID*/, IOleInPlaceActiveObject* /*pActiveObject*/, IOleCommandTarget* /*pCommandTarget*/, IOleInPlaceFrame* /*pFrame*/, IOleInPlaceUIWindow* /*pDoc*/){return S_OK;}
	    HRESULT STDMETHODCALLTYPE HideUI(){return S_OK;}
	    HRESULT STDMETHODCALLTYPE UpdateUI(){return S_OK;}
	    HRESULT STDMETHODCALLTYPE EnableModeless(BOOL /*fEnable*/){return S_OK;}
	    HRESULT STDMETHODCALLTYPE OnDocWindowActivate(BOOL /*fActivate*/){return S_OK;}
	    HRESULT STDMETHODCALLTYPE OnFrameWindowActivate(BOOL /*fActivate*/){return S_OK;}
	    HRESULT STDMETHODCALLTYPE ResizeBorder(LPCRECT /*prcBorder*/, IOleInPlaceUIWindow* /*pUIWindow*/, BOOL /*fRameWindow*/){return S_OK;}
	    HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG /*lpMsg*/, const GUID* /*pguidCmdGroup*/, DWORD /*nCmdID*/){return S_FALSE;}
	    HRESULT STDMETHODCALLTYPE GetOptionKeyPath(LPOLESTR* /*pchKey*/, DWORD /*dw*/){return S_FALSE;}
	    HRESULT STDMETHODCALLTYPE GetDropTarget(IDropTarget* /*pDropTarget*/, IDropTarget** /*ppDropTarget*/){return S_FALSE;}
	    HRESULT STDMETHODCALLTYPE TranslateUrl(DWORD /*dwTranslate*/, OLECHAR* /*pchURLIn*/, OLECHAR** ppchURLOut){*ppchURLOut = 0; return S_FALSE;}
	    HRESULT STDMETHODCALLTYPE FilterDataObject(IDataObject* /*pDO*/, IDataObject** ppDORet){*ppDORet = 0; return S_FALSE;}
	    HRESULT STDMETHODCALLTYPE GetHostInfo(DOCHOSTUIINFO *pInfo);
	    HRESULT STDMETHODCALLTYPE GetExternal(IDispatch **ppDispatch);

	    // IOleWindow (TOleInPlaceSite)
	    HRESULT STDMETHODCALLTYPE GetWindow(HWND *phwnd);
	    HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL /*fEnterMode*/){return E_NOTIMPL;}

        //IOleInPlaceSite
	    HRESULT STDMETHODCALLTYPE GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO info);
	    HRESULT STDMETHODCALLTYPE OnPosRectChange(LPCRECT lprcPosRect);
	    HRESULT STDMETHODCALLTYPE CanInPlaceActivate(){return S_OK;}
	    HRESULT STDMETHODCALLTYPE OnInPlaceActivate(){return S_OK;}
	    HRESULT STDMETHODCALLTYPE OnUIActivate(){return S_OK;}
	    HRESULT STDMETHODCALLTYPE Scroll(SIZE /*scrollExtant*/){return E_NOTIMPL;}
	    HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL /*fUndoable*/){return S_OK;}
	    HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate(){return S_OK;}
	    HRESULT STDMETHODCALLTYPE DiscardUndoState(){return E_NOTIMPL;}
	    HRESULT STDMETHODCALLTYPE DeactivateAndUndo(){return E_NOTIMPL;}

	    //IDocHostShowUI
	    HRESULT STDMETHODCALLTYPE ShowHelp(HWND /*hwnd*/, LPOLESTR /*pszHelpFile*/, UINT /*uCommand*/, DWORD /*dwData*/, POINT /*ptMouse*/, IDispatch* /*pDispatchObjectHit*/){return S_OK;}
	    HRESULT STDMETHODCALLTYPE ShowMessage(HWND /*hwnd*/, LPOLESTR /*lpstrText*/, LPOLESTR /*lpstrCaption*/, DWORD /*dwType*/, LPOLESTR /*lpstrHelpFile*/, DWORD /*dwHelpContext*/, LRESULT* /*plResult*/){return S_FALSE;}

	    // IOleInPlaceUIWindow
	    HRESULT STDMETHODCALLTYPE GetBorder(LPRECT /*lprectBorder*/){return E_NOTIMPL;}
	    HRESULT STDMETHODCALLTYPE RequestBorderSpace(LPCBORDERWIDTHS /*pborderwidths*/){return E_NOTIMPL;}
	    HRESULT STDMETHODCALLTYPE SetBorderSpace(LPCBORDERWIDTHS /*pborderwidths*/){return E_NOTIMPL;}
	    HRESULT STDMETHODCALLTYPE SetActiveObject(IOleInPlaceActiveObject* /*pActiveObject*/, LPCOLESTR /*pszObjName*/){return S_OK;}

	    // IOleInPlaceFrame
	    HRESULT STDMETHODCALLTYPE InsertMenus(HMENU /*hmenuShared*/, LPOLEMENUGROUPWIDTHS /*lpMenuWidths*/){return E_NOTIMPL;}
	    HRESULT STDMETHODCALLTYPE SetMenu(HMENU /*hmenuShared*/, HOLEMENU /*holemenu*/, HWND /*hwndActiveObject*/){return S_OK;}
	    HRESULT STDMETHODCALLTYPE RemoveMenus(HMENU /*hmenuShared*/){return E_NOTIMPL;}
        HRESULT STDMETHODCALLTYPE SetStatusText(LPCOLESTR /*pszStatusText*/){return S_OK;}
	    HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG /*lpmsg*/, WORD /*wID*/){return E_NOTIMPL;}

    private:
	    void BeforeNavigate2(const wchar_t *url, short *cancel);
	    void DocumentComplete(const wchar_t *url); 
    };

    const LPCTSTR WEBFORM_CLASS = _T("WebformClass");
    const UINT WEBFN_LOADED = 3;

    inline char* BSTRToLPSTR(BSTR bStr, LPSTR lpstr) {
	    int lenW = SysStringLen(bStr);
	    int lenA = WideCharToMultiByte(CP_ACP, 0, bStr, lenW, 0, 0, NULL, NULL);

	    if (lenA > 0) {
		    lpstr = new char[lenA + 1]; // allocate a final null terminator as well
		    WideCharToMultiByte(CP_ACP, 0, bStr, lenW, lpstr, lenA, NULL, NULL);
		    lpstr[lenA] = '\0'; // Set the null terminator yourself
	    } else {
		    lpstr = NULL;
	    }

	    return lpstr;
    }

    MainWindow::MainWindow() : ref(0), ibrowser(NULL), cookie(0), isnaving(0), url(NULL), kurl(NULL), hasScrollbars(false), hWnd(NULL) {
	    idMap.insert(std::make_pair(L"execute", DISPID_USER_EXECUTE));
	    idMap.insert(std::make_pair(L"writefile", DISPID_USER_WRITEFILE));
	    idMap.insert(std::make_pair(L"readfile", DISPID_USER_READFILE));
	    idMap.insert(std::make_pair(L"getevar", DISPID_USER_GETVAL));
	    idMap.insert(std::make_pair(L"setevar", DISPID_USER_SETVAL));
    }

    void MainWindow::setupOle() {
	    RECT rc;
	    GetClientRect(hWnd, &rc);

	    HRESULT hr;
	    IOleObject* iole = 0;
	    hr = CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_INPROC_SERVER, IID_IOleObject, (void**)&iole);
	    if (iole == 0) {
		    return;
	    }

	    hr = iole->SetClientSite(this);
	    if (hr != S_OK) {
		    iole->Release();
		    return;
	    }

	    hr = iole->SetHostNames(L"MyHost", L"MyDoc");
	    if (hr != S_OK) {
		    iole->Release();
		    return;
	    }

	    hr = OleSetContainedObject(iole, TRUE);
	    if (hr != S_OK) {
		    iole->Release();
		    return;
	    }

	    hr = iole->DoVerb(OLEIVERB_SHOW, 0, this, 0, hWnd, &rc);
	    if (hr != S_OK) {
		    iole->Release();
		    return;
	    }

	    bool connected = false;
	    IConnectionPointContainer *cpc = 0;
	    iole->QueryInterface(IID_IConnectionPointContainer, (void**)&cpc);
	    if (cpc != 0) {
		    IConnectionPoint *cp = 0;
		    cpc->FindConnectionPoint(DIID_DWebBrowserEvents2, &cp);

		    if (cp != 0) {
			    cp->Advise((IDispatch*)this, &cookie);
			    cp->Release();
			    connected = true;
		    }

		    cpc->Release();
	    }

	    if (!connected) {
		    iole->Release();
		    return;
	    }

	    iole->QueryInterface(IID_IWebBrowser2, (void**)&ibrowser);
	    iole->Release();
    }

    void MainWindow::Close() {
	    if (ibrowser != 0) {
		    IConnectionPointContainer *cpc = 0;
		    ibrowser->QueryInterface(IID_IConnectionPointContainer, (void**)&cpc);

		    if (cpc != 0) {
			    IConnectionPoint *cp = 0;
			    cpc->FindConnectionPoint(DIID_DWebBrowserEvents2, &cp);

			    if (cp != 0) {
				    cp->Unadvise(cookie);
				    cp->Release();
			    }

			    cpc->Release();
		    }

		    IOleObject *iole = 0;
		    ibrowser->QueryInterface(IID_IOleObject, (void**)&iole);
		    UINT refCount = ibrowser->Release();
            (void)refCount;
		    ibrowser = 0;

		    if (iole != 0) {
			    iole->Close(OLECLOSE_NOSAVE);
			    iole->Release();
		    }
	    }
    }

    MainWindow::~MainWindow() {
	    if (url != 0) {
		    delete[] url;
	    }

	    if (kurl != 0) {
		    delete[] kurl;
	    }
    }

    HRESULT STDMETHODCALLTYPE MainWindow::GetIDsOfNames(REFIID /*riid*/, LPOLESTR *rgszNames, UINT cNames, LCID /*lcid*/, DISPID *rgDispId) {
	    HRESULT hr = S_OK;

	    for (UINT i = 0; i < cNames; i++) {
		    std::map<std::wstring, DISPID>::iterator iter = idMap.find(rgszNames[i]);
		    if (iter != idMap.end()) {
			    rgDispId[i] = iter->second;
		    } else {
			    rgDispId[i] = DISPID_UNKNOWN;
			    hr = DISP_E_UNKNOWNNAME;
		    }
	    }

	    return hr;
    }

    typedef std::vector<std::string> StringList;
    inline StringList GetArgList(DISPPARAMS* pDispParams) {
        StringList argList;
	    for (size_t i = 0; i < pDispParams->cArgs; ++i) {
            /*
		    BSTR bstrArg = pDispParams->rgvarg[i].bstrVal;
		    LPSTR arg = NULL;
		    arg = BSTRToLPSTR(bstrArg, arg);
		    args[pDispParams->cArgs - 1 - i] = arg; // also re-reverse order of arguments
		    delete [] arg;
            */
	    }
        return argList;
    }

    HRESULT STDMETHODCALLTYPE MainWindow::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID /*lcid*/, WORD /*wFlags*/, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* /*pExcepInfo*/, UINT* /*puArgErr*/) {
	    HRESULT hr = S_OK;
	    switch (dispIdMember) {
	        case DISPID_BEFORENAVIGATE2: {
		        BSTR bstrUrl = pDispParams->rgvarg[5].pvarVal->bstrVal;
		        char *lpstrUrl = NULL;

		        lpstrUrl = BSTRToLPSTR(bstrUrl, lpstrUrl);
		        if (lpstrUrl == NULL) {
			        break;
		        }

		        std::string url = lpstrUrl;
		        delete [] lpstrUrl;

		        bool cancel = false;

                // Set Cancel parameter to TRUE to cancel the current event
		        *(((*pDispParams).rgvarg)[0].pboolVal) = cancel ? TRUE : FALSE;

		        break;
	        }
	        case DISPID_DOCUMENTCOMPLETE:
		        DocumentComplete(pDispParams->rgvarg[0].pvarVal->bstrVal);
		        break;
	        case DISPID_NAVIGATECOMPLETE2: {
		        BSTR bstrUrl = pDispParams->rgvarg[0].pvarVal->bstrVal;
		        char *lpstrUrl = NULL;

		        lpstrUrl = BSTRToLPSTR(bstrUrl, lpstrUrl);
		        if (lpstrUrl == NULL) {
			        break;
		        }

		        std::string url = lpstrUrl;
		        delete [] lpstrUrl;

    	        AddCustomObject(this, "JSObject");

		        break;
	        }
	        case DISPID_AMBIENT_DLCONTROL:
		        pVarResult->vt = VT_I4;
		        pVarResult->lVal = DLCTL_DLIMAGES | DLCTL_VIDEOS | DLCTL_BGSOUNDS | DLCTL_SILENT;
		        break;
		    case DISPID_USER_EXECUTE: {
			    //LSExecute(NULL, args[0].c_str(), SW_NORMAL);

			    break;
		    }
		    case DISPID_USER_WRITEFILE: {
    		    StringList args = GetArgList(pDispParams);
			    std::ofstream outfile;
			    std::ios_base::openmode mode = std::ios_base::out;

			    if (args[1] == "overwrite") {
				    mode |= std::ios_base::trunc;
			    } else if (args[1] == "append") {
				    mode |= std::ios_base::app;
			    }

			    outfile.open(args[0].c_str());
			    outfile << args[2];
			    outfile.close();
			    break;
		    }
		    case DISPID_USER_READFILE: {
    		    StringList args = GetArgList(pDispParams);
			    std::string buffer;
			    std::string line;
			    std::ifstream infile;
			    infile.open(args[0].c_str());

			    while(std::getline(infile, line)) {
				    buffer += line;
				    buffer += "\n";
			    }

			    int lenW = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buffer.c_str(), -1, NULL, 0);
			    BSTR bstrRet = SysAllocStringLen(0, lenW - 1);
			    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buffer.c_str(), -1, bstrRet, lenW);

			    pVarResult->vt = VT_BSTR;
			    pVarResult->bstrVal = bstrRet;

			    break;
		    }
		    case DISPID_USER_GETVAL: {
    		    StringList args = GetArgList(pDispParams);
			    char *buf = new char[256];
			    strncpy_s(buf, 256, values[args[0]].c_str(), 256);

			    int lenW = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buf, -1, NULL, 0);
			    BSTR bstrRet = SysAllocStringLen(0, lenW - 1);
			    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buf, -1, bstrRet, lenW);

			    pVarResult->vt = VT_BSTR;
			    pVarResult->bstrVal = bstrRet;

			    break;
		    }
		    case DISPID_USER_SETVAL: {
    		    StringList args = GetArgList(pDispParams);
			    std::map<std::string, std::string>::iterator itr = values.find(args[0]);
			    if (itr == values.end()) {
				    values.insert(std::make_pair(args[0], args[1]));
			    } else {
				    values[args[0]] = args[1];
			    }

			    break;
		    }
		    default:
			    hr = DISP_E_MEMBERNOTFOUND;
	    }
	    return hr;
    }

    HRESULT STDMETHODCALLTYPE MainWindow::GetHostInfo(DOCHOSTUIINFO *pInfo) {
	    pInfo->dwFlags = (hasScrollbars ? 0 : DOCHOSTUIFLAG_SCROLL_NO) | DOCHOSTUIFLAG_NO3DOUTERBORDER;
	    return S_OK;
    }

    HRESULT STDMETHODCALLTYPE MainWindow::GetExternal(IDispatch **ppDispatch) {
	    *ppDispatch = static_cast<IDispatch*>(this);
	    return S_OK;
    }

    HRESULT STDMETHODCALLTYPE MainWindow::GetWindow(HWND* phwnd) {
	    *phwnd = hWnd;
	    return S_OK;
    }

    HRESULT STDMETHODCALLTYPE MainWindow::GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO info) {
	    *ppFrame = static_cast<IOleInPlaceFrame*>(this);
	    AddRef();
	    *ppDoc = NULL;
	    info->fMDIApp = FALSE;
	    info->hwndFrame = hWnd;
	    info->haccel = 0;
	    info->cAccelEntries = 0;
	    GetClientRect(hWnd, lprcPosRect);
	    GetClientRect(hWnd, lprcClipRect);
	    return S_OK;
    }

    HRESULT STDMETHODCALLTYPE MainWindow::OnPosRectChange(LPCRECT lprcPosRect) {
	    IOleInPlaceObject *iole = NULL;
	    ibrowser->QueryInterface(IID_IOleInPlaceObject, (void**)&iole);

	    if (iole != NULL) {
		    iole->SetObjectRects(lprcPosRect, lprcPosRect);
		    iole->Release();
	    }

	    return S_OK;
    }

    void MainWindow::Resize(const UINT& w, const UINT& h) {
        ::MoveWindow(hWnd, 0, 0, w, h, TRUE);
    }

    void MainWindow::Go(const z::string& url) {
	    if (url.length() == 0 || ibrowser == 0) {
		    return;
	    }

        z::string16 url16 = z::c32to16(url);
        assert(sizeof(wchar_t) == sizeof(z::string16::scharT));
	    isnaving = 7;
	    VARIANT v;
	    v.vt = VT_I4;
	    v.lVal = 0; // v.lVal = navNoHistory;
	    ibrowser->Navigate((BSTR)url16.c_str(), &v, NULL, NULL, NULL);

	    // (Special case: maybe it's already loaded by the time we get here!)
	    if ((isnaving & 2) == 0) {
		    WPARAM w = (GetWindowLong(hWnd, GWL_ID) & 0xFFFF) | ((WEBFN_LOADED & 0xFFFF) << 16);
		    PostMessage(GetParent(hWnd), WM_COMMAND, w, (LPARAM)hWnd);
	    }

	    isnaving &= ~4;
    }

    void MainWindow::Forward() {
	    ibrowser->GoForward();
    }

    void MainWindow::Back() {
	    ibrowser->GoBack();
    }

    void MainWindow::Refresh(bool clearCache) {
	    if (clearCache) {
		    VARIANT v;
		    v.vt = VT_I4;
		    v.lVal = REFRESH_COMPLETELY;
		    ibrowser->Refresh2(&v);
	    } else {
		    ibrowser->Refresh();
	    }
    }

    IHTMLDocument2 *MainWindow::GetDoc() {
	    IDispatch *dispatch = 0;
	    ibrowser->get_Document(&dispatch);
    	
	    if (dispatch == NULL) {
		    return NULL;
	    }

	    IHTMLDocument2 *doc;
	    dispatch->QueryInterface(IID_IHTMLDocument2, (void**)&doc);
	    dispatch->Release();
    	
	    return doc;
    }

    void MainWindow::RunJSFunction(std::string cmd) {
	    IHTMLDocument2 *doc = GetDoc();
	    if (doc != NULL) {
		    IHTMLWindow2 *win = NULL;
		    doc->get_parentWindow(&win);

		    if (win != NULL) {
			    int lenW = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cmd.c_str(), -1, NULL, 0);
			    BSTR bstrCmd = SysAllocStringLen(0, lenW);
			    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cmd.c_str(), -1, bstrCmd, lenW);

			    VARIANT v;
			    VariantInit(&v);
			    win->execScript(bstrCmd, NULL, &v);

			    VariantClear(&v);
			    SysFreeString(bstrCmd);
			    win->Release();
		    }

		    doc->Release();
	    }
    }

    void MainWindow::AddCustomObject(IDispatch *custObj, std::string name) {
	    IHTMLDocument2 *doc = GetDoc();

	    if (doc == NULL) {
		    return;
	    }

	    IHTMLWindow2 *win = NULL;
	    doc->get_parentWindow(&win);
	    doc->Release();

	    if (win == NULL) {
		    return;
	    }

	    IDispatchEx *winEx;
	    win->QueryInterface(&winEx);
	    win->Release();

	    if (winEx == NULL) {
		    return;
	    }

	    int lenW = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, name.c_str(), -1, NULL, 0);
	    BSTR objName = SysAllocStringLen(0, lenW);
	    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, name.c_str(), -1, objName, lenW);

	    DISPID dispid; 
	    HRESULT hr = winEx->GetDispID(objName, fdexNameEnsure, &dispid);

	    SysFreeString(objName);

	    if (FAILED(hr)) {
		    return;
	    }

	    DISPID namedArgs[] = {DISPID_PROPERTYPUT};
	    DISPPARAMS params;
	    params.rgvarg = new VARIANT[1];
	    params.rgvarg[0].pdispVal = custObj;
	    params.rgvarg[0].vt = VT_DISPATCH;
	    params.rgdispidNamedArgs = namedArgs;
	    params.cArgs = 1;
	    params.cNamedArgs = 1;

	    hr = winEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, NULL, NULL, NULL); 
	    winEx->Release();

	    if (FAILED(hr)) {
		    return;
	    }
    }

    void MainWindow::DocumentComplete(const wchar_t *) {
	    isnaving &= ~2;

	    if (isnaving & 4) {
		    return; // "4" means that we're in the middle of Go(), so the notification will be handled there
	    }

	    WPARAM w = (GetWindowLong(hWnd, GWL_ID) & 0xFFFF) | ((WEBFN_LOADED & 0xFFFF) << 16);
	    PostMessage(GetParent(hWnd), WM_COMMAND, w, (LPARAM)hWnd);
    }

    HRESULT STDMETHODCALLTYPE MainWindow::QueryInterface(REFIID riid, void **ppv) {
	    *ppv = NULL;

	    if (riid == IID_IUnknown || riid == IID_IOleClientSite) {
		    *ppv = static_cast<IOleClientSite*>(this);
	    } else if (riid == IID_IOleWindow || riid == IID_IOleInPlaceSite) {
		    *ppv = static_cast<IOleInPlaceSite*>(this);
	    } else if (riid == IID_IOleInPlaceUIWindow) {
		    *ppv = static_cast<IOleInPlaceUIWindow*>(this);
	    } else if (riid == IID_IOleInPlaceFrame) {
		    *ppv = static_cast<IOleInPlaceFrame*>(this);
	    } else if (riid == IID_IDispatch) {
		    *ppv = static_cast<IDispatch*>(this);
	    } else if (riid == IID_IDocHostUIHandler) {
		    *ppv = static_cast<IDocHostUIHandler*>(this);
	    } else if (riid == IID_IDocHostShowUI) {
		    *ppv = static_cast<IDocHostShowUI*>(this);
	    }

	    if (*ppv != NULL) {
		    AddRef();
		    return S_OK;
	    }

	    return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE MainWindow::AddRef() {
	    return InterlockedIncrement(&ref);
    }

    ULONG STDMETHODCALLTYPE MainWindow::Release() {
	    int tmp = InterlockedDecrement(&ref);
    	
	    if (tmp == 0) {
		    OutputDebugStringA("MainWindow::Release(): delete this");
    //		delete this; /// \todo Debug this.
	    }
    	
	    return tmp;
    }

    LRESULT CALLBACK MainWindow::WebformWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	    if (msg == WM_NCCREATE) {
		    MainWindow *webf = (MainWindow*)((LPCREATESTRUCT(lParam))->lpCreateParams);
		    webf->hWnd = hwnd;
		    webf->setupOle();
		    if (webf->ibrowser == 0) {
			    MessageBoxA(NULL, "web->ibrowser is NULL", "WM_CREATE", MB_OK);
			    delete webf;
			    webf = NULL;
		    } else {
			    webf->AddRef();
		    }

		    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)webf);

		    return DefWindowProc(hwnd, msg, wParam, lParam);
	    }

	    MainWindow *webf = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	    if (webf == NULL) {
		    return DefWindowProc(hwnd, msg, wParam, lParam);
	    }

	    return webf->InstanceWndProc(msg, wParam, lParam);
    }

    LRESULT MainWindow::InstanceWndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
	    switch (msg) {
		    case WM_CREATE: {
			    CREATESTRUCT *cs = (CREATESTRUCT*)lParam;

			    if (cs->style & (WS_HSCROLL | WS_VSCROLL)) {
				    SetWindowLongPtr(hWnd, GWL_STYLE, cs->style & ~(WS_HSCROLL | WS_VSCROLL));
			    }

			    break;
		    }
		    case WM_DESTROY:
			    Close();
			    Release();
			    SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
                ::PostQuitMessage(0);
			    break;
		    case WM_SETTEXT:
			    Go((TCHAR*)lParam);
			    break;
		    case WM_SIZE:
			    if (ibrowser != NULL) {
				    ibrowser->put_Width(LOWORD(lParam));
				    ibrowser->put_Height(HIWORD(lParam));
			    }
			    break;
		    case WM_PAINT: {
			    PAINTSTRUCT ps;
			    BeginPaint(hWnd, &ps);
			    HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));

			    FillRect(ps.hdc, &ps.rcPaint, brush);

			    DeleteObject(brush);
			    EndPaint(hWnd, &ps);

			    return 0;
		    }
	    }

	    return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    void MainWindow::create(HWND hWndParent, HINSTANCE hInstance, UINT id, bool showScrollbars) {
	    hasScrollbars = showScrollbars;
	    this->id = id;

	    WNDCLASSEX wcex = {0};
        if (!::GetClassInfoEx(hInstance, WEBFORM_CLASS, &wcex)) {
		    wcex.cbSize = sizeof(WNDCLASSEX);
		    wcex.style = CS_HREDRAW | CS_VREDRAW;
		    wcex.lpfnWndProc = (WNDPROC)MainWindow::WebformWndProc;
		    wcex.hInstance = hInstance;
		    wcex.lpszClassName = WEBFORM_CLASS;
		    wcex.cbWndExtra = sizeof(MainWindow*);

            if(!::RegisterClassEx(&wcex)) {
                ::MessageBoxA(NULL, "Could not register wndcls", "MainWindow::create", MB_OK);
			    return;
		    }
	    }

        hWnd = ::CreateWindow(WEBFORM_CLASS, _T("zenlang"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWndParent, (HMENU)(LONG_PTR)id, hInstance, (LPVOID)this);
        ::ShowWindow(hWnd, SW_SHOW);
        ::UpdateWindow(hWnd);
    }

    void OpenWindow() {
        MainWindow* mw = new MainWindow();
        mw->create(0, z::app().instance(), 0, true);
        z::string url = "res://" + z::app().name() + "/index.html";
        mw->Go(url);
    }
} // namespace zz
#elif defined(QT)
namespace zz {
    class MainWindow : public QWebView {
        void closeEvent(QCloseEvent* ev) {
            z::ref(QApplication::instance()).exit(0);
            z::ref(ev).ignore();
        }
    private slots:
        void onLoadFinished(bool ok) {
            assert(ok);
        }
    public:
        inline MainWindow() : QWebView(0) {
            connect(this, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));
        }
    };
    void OpenWindow() {
        MainWindow* view = new MainWindow();
        view->load(QUrl("qrc:/res/index.html"));
        view->show();
    }
} // namespace zz
#elif defined(OSX)
@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    NSRect rc = NSMakeRect(0, 0, 800, 600);
//    NSRect rc1 = [[NSScreen mainScreen] frame];
    int styleMask = NSTitledWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask | NSClosableWindowMask;
    NSWindow* win = [[NSWindow alloc] initWithContentRect:rc styleMask:styleMask backing:NSBackingStoreBuffered defer:false];
    [win setBackgroundColor:[NSColor blueColor]];

    WebView* webView = [[WebView alloc] initWithFrame:rc];

    NSString* fpath = [[NSBundle mainBundle] pathForResource:@"index" ofType:@"html" inDirectory:@""];
    NSURL* url = [NSURL URLWithString:fpath];
    NSURLRequest* req = [NSURLRequest requestWithURL:url];
    [[webView mainFrame] loadRequest:req];

    [win setContentView:webView];
    [win makeKeyAndOrderFront:win];
}
@end
namespace zz {
    void OpenWindow() {
    }
} // namespace zz
#elif defined(IOS)
@interface AppDelegate : UIResponder <UIApplicationDelegate, UIWebViewDelegate>
    @property (strong, nonatomic) UIWindow *window;
@end

@implementation AppDelegate
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.rootViewController = [[UIViewController alloc] init];
    self.window.rootViewController.view.backgroundColor = [UIColor greenColor];

    CGRect rc = [[UIScreen mainScreen] bounds];
    rc = CGRectInset(rc, 10, 10);
    UIWebView* webView = [[UIWebView alloc] initWithFrame:rc];
    webView.delegate = self;
    webView.scalesPageToFit = YES;
    webView.userInteractionEnabled = YES;
    webView.backgroundColor = [UIColor redColor];

    NSString* fpath = [[NSBundle mainBundle] pathForResource:@"index" ofType:@"html" inDirectory:@""];
    NSURL* url = [NSURL URLWithString:fpath];
    NSURLRequest* req = [NSURLRequest requestWithURL:url];
    [webView loadRequest:req];

    [self.window.rootViewController.view addSubview:webView];
    [self.window makeKeyAndVisible];
    return YES;
}

-(void)webView:(UIWebView*)webView didFailLoadWithError:(NSError *)error
{
    NSLog(@"Error: %@", error);
}
@end
namespace zz {
    inline void OpenWindow() {
    }
} // namespace zz
#endif
#endif

z::application::application(int argc, char** argv) : _argc(argc), _argv(argv), _isExit(false), _log(0) {
    if(g_app != 0) {
        throw z::Exception("z::application", z::string("Multiple instances of application not permitted"));
    }
    g_app = this;

    _ctx.reset(new GlobalContext());

    // store path to executable
    static const int len = 1024;
    char path[len] = "";
#if defined(WIN32)
    DWORD rv = GetModuleFileName(NULL, path, len);
    if(rv == 0) {
        DWORD ec = GetLastError();
        assert(ec != ERROR_SUCCESS);
        throw z::Exception("z::application", z::string("Internal error retrieving process path: %{s}").arg("s", ec));
    }
#elif defined(__APPLE__)
    uint32_t sz = len;
    if(_NSGetExecutablePath(path, &sz) != 0) {
        throw z::Exception("z::application", z::string("Internal error retrieving process path: %{s}").arg("s", path));
    }
#else
    if (readlink ("/proc/self/exe", path, len) == -1) {
        throw z::Exception("z::application", z::string("Internal error retrieving process path: %{s}").arg("s", path));
    }
#endif
    _path = z::string(path);
#if defined(WIN32)
    _path.replace("\\", "/");
#endif

    // store name of application (this depends on _path)
    _name = z::dir::getBaseName(_path);
    z::string ext = z::dir::getExtention(_path);
    if(ext.length() > 0) {
        _name = _name + "." + ext;
    }

    // store path to application data directory (this is dependent on _name)
#if defined(WIN32)
    char chPath[MAX_PATH];
    /// \todo Use SHGetKnownFolderPath for vista and later.
    HRESULT hr = ::SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, chPath);
    if(!SUCCEEDED(hr)) {
        throw z::Exception("application", z::string("Internal error retrieving data directory: %{s}").arg("s", hr));
    }
    _data = z::string(chPath);
    _data.replace("\\", "/");
#elif defined(OSX) || defined(IOS)
#if !__has_feature(objc_arc)
    // OSX system not initialized yet, so we need an explicit pool
    NSAutoreleasePool* pool = [NSAutoreleasePool new];
#endif
    // get the home directory
    NSString* p = NSHomeDirectory();
    const char* instr = [p UTF8String];
    _data = z::string(instr) + "/Library";

    NSString* b = [[NSBundle mainBundle] bundlePath];
    const char* bstr = [b UTF8String];
    z::string bpath = z::string(bstr);
#if !__has_feature(objc_arc)
    // release the pool
    [pool drain];
#endif
#else
    const char* p = ::getenv("HOME");
    if(p) {
        _data = z::string(p);
    } else {
        _data = "./";
    }
#endif
    _data = _data + "/" + z::app().name() + "/";
    z::dir::mkpath(_data);

    // store path to application resource directory
#if defined(OSX) || defined(IOS)
    _base = bpath;
#else
    _base = z::dir::getPath(_path);
#endif

    // convert all argv to argl
    for(int i = 0; i < _argc; ++i) {
        std::string a = _argv[i];
        if((a.length() >= 10) && (a.substr(0, 10) == "---logfile") && (i < (_argc-1))) {
            std::string f = _argv[++i];
            if(f == "-") {
                _log = new std::ofstream();
                z::ref(_log).setstate(std::ios_base::badbit);
            } else {
                _log = new std::ofstream(f.c_str());
            }
        } else {
            _argl.add(argv[i]);
        }
    }

    if(_log == 0) {
#if defined(GUI)
#if defined(DEBUG)
        z::string logfile = "./default.log";
#else
        z::string logfile = _data + "/default.log";
#endif
        std::cout << "log: " << logfile << std::endl;
        _log = new std::ofstream(z::s2e(logfile).c_str());
#else
        _log = z::ptr(std::cout);
#endif
    }

#if defined(WIN32)
    ::_tzset();
#else
    ::tzset();
#endif

#if defined(WIN32)
#if defined(GUI)
    if (::OleInitialize(NULL) != S_OK) {
        throw z::Exception("z::application", z::string("OleInitialize failed"));
    }
    // init common controls.
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_USEREX_CLASSES;
    InitCommonControlsEx(&icex);

    // required for rich edit control
    HMODULE libmod = ::LoadLibrary("msftedit.dll");
    if(libmod == NULL) {
        z::elog("main", "Unable to load library msftedit.dll required by win32 edit control.");
    }
#endif
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = ::WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        throw z::Exception("z::application", z::string("WSAStartup failed"));
    }
#endif
}

z::application::~application() {
#if defined(WIN32)
#if defined(GUI)
    ::OleUninitialize();
#endif
    WSACleanup();
#endif
    if(_log != z::ptr(std::cout)) {
        delete _log;
    }
    _log = 0;
}

#if defined(WIN32)
static HINSTANCE s_hInstance = 0;
HINSTANCE z::application::instance() const {
    assert(0 != s_hInstance);
    return s_hInstance;
}
#endif

inline int z::application::execExx() {
#if defined(UNIT_TEST)
    TestResult tr; unused(tr);
#endif
    int code = 0;

    // this is the local context for thread-0.
    // all UI events will run in this context
    ThreadContext tctx(gctx().at(0)); unused(tctx);

#if defined(GUI)
    // start timer and message pump
#if defined(WIN32)
    // create timer
    int timerID = win32::getNextResID();
    UINT timer = SetTimer(NULL, timerID, 0, IdleProc);

    // spin main loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KillTimer(NULL, timer);
    code = (int)msg.wParam;
#elif defined(QT)
    // create timer object
    QTimer ts;
    ZTimer td;
    QObject::connect(&ts, SIGNAL(timeout()), &td, SLOT(OnTimer()));
    ts.start(0);

    // spin main loop
    code = z::ref(QApplication::instance()).exec();
#elif defined(OSX) || defined(IOS)
    // create timer object
    CTimer* ctimer = [[CTimer alloc] initTimer];
    unused(ctimer);
#if defined(OSX) //COCOA_NIB
    NSApplication* app = [NSApplication sharedApplication];
    AppDelegate* del = [[AppDelegate alloc] init];
    [app setDelegate:del];
    [NSApp run];
    code = EXIT_SUCCESS;
    //code = NSApplicationMain(_argc, (const char**)_argv);
#elif defined(IOS)
    @autoreleasepool {
        return UIApplicationMain(_argc, _argv, nil, NSStringFromClass([AppDelegate class]));
    }
#else
#error "Unknown MacOs mode"
#endif
#else
#error "Unimplemented GUI mode"
#endif
#else // GUI
#if defined(SERVER)
    while(!_isExit) {
        pump();
    }
#else
        pump();
#endif
#endif // GUI

    return code;
}

int z::application::exec() {
    int rv = execExx();
    return rv;
}

int z::application::exit(const int& code) const {
    // Enqueue all at-exit functions.
    // NOTE: This does not get called under OSX/iOS, because the main loop is terminated
    // using exit(). This leads to memory leaks. The only way to call this would be using _atexit().
    onExit();

#if defined(GUI)
#if defined(WIN32)
    ::PostQuitMessage(code);
#elif defined(QT)
    z::ref(QApplication::instance()).exit(code);
#elif defined(OSX)
    [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
#elif defined(IOS)
    // no option to exit IOS app programmatically, no encouraged in Apple HIG
    // using terminateWithSuccess will get the iOS app rejected from AppStore
    //[[UIApplication sharedApplication] terminateWithSuccess];
#else
#error "Unimplemented GUI mode"
#endif
#endif

    z::application& self = const_cast<z::application&>(z::ref(this));
    self._isExit = true;
    return code;
}

void z::application::writeLog(const z::string& msg) const {
    z::ref(_log) << msg << std::endl;
#if defined(DEBUG) && defined(GUI) && defined(WIN32)
    OutputDebugStringA(z::s2e(msg + "\n").c_str());
#endif
}

#if defined(Z_EXE)
void initMain(const z::stringlist& argl) {
    z::ThreadContext tctx(gctx().at(0));

#if defined(UNIT_TEST)
    z::s_testList.begin();
    z::TestInstance* ti = z::s_testList.next();
    while(ti != 0) {
        ref(ti).enque(z::ref(s_tctx));
        ti = z::s_testList.next();
    }
#endif

    z::s_mainList.begin();
    z::MainInstance* mi = z::s_mainList.next();
    while(mi != 0) {
        ref(mi).enque(z::ref(s_tctx), argl);
        mi = z::s_mainList.next();
    }
#if defined(GUI)
    zz::OpenWindow();
#endif
}

#if defined(GUI) && defined(WIN32)
    int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
        unused(hPrevInstance);unused(lpCmdLine);unused(nCmdShow);
        s_hInstance = hInstance;
        z::application a(__argc, (char**)__argv);
        initMain(a.argl());
        return a.exec();
    }
#else // GUI && WIN32
    #if !defined(ZPP_EXE) // special case for ZPP compiler, which defines its own main()
        int main(int argc, char* argv[]) {
        #if defined(GUI) && defined(QT)
            // This cannot be in initMain() since it must have application-level lifetime.
            QApplication qapp(argc, argv);
        #endif
            z::application a(argc, argv);
            initMain(a.argl());
            return a.exec();
        }
    #endif // ZPP_EXE
#endif // GUI && WIN32
#endif // Z_EXE
