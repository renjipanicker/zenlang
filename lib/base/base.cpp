# include "zenlang.hpp"
#include "base/base.hpp"

#if defined(GUI)
    #if defined(WIN32)
    #elif defined(GTK)
    #elif defined(QT)
    # include <QtGui/QApplication>
    # include <QtCore/QTimer>
    #elif defined(OSX)
        // Note that this is import, not include. Which is why this must be here, not in pch.hpp
        // In OSX mode, this file *must* be compiled as a Obj-C++ file, not C++ file. In Xcode, go to
        // Project/Build Phases/Compile Sources/ and select the zenlang.cpp file.
        // Add "-x objective-c++" to the "Compiler Flags" column.
        #import <Cocoa/Cocoa.h>
        #import <AppKit/AppKit.h>
    #elif defined(IOS)
        // Same as above note. This is an import.
        #import <UIKit/UIKit.h>
    #else
        #error "Unimplemented GUI mode"
    #endif
#else
    #if defined(OSX) || defined(IOS)
        // Same as above note. This is an import.
        #import <Cocoa/Cocoa.h>
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
void z::writelog(const z::string& src, const z::string& msg) {
    z::string s;
    if(src.length() > 0) {
        s += src;
        s += " : ";
    }
    s += msg;
#if defined(OSX) || defined(IOS)
    std::cout << s << std::endl;
#endif
    if(g_app) {
        z::app().writeLog(s);
    } else {
        std::cout << s << std::endl;
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
        char32_t ch;
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
        }
        rv.append(ch);
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
        const z::char16_t& ch = in.at(i);
        rv.append(ch);
    }
    return rv;
}

z::string16 z::c32to16(const z::string32& in) {
    z::string16 rv;
    for(z::string32::size_type i = 0; i < in.size(); i++) {
        const z::char32_t& ch = in.at(i);
        rv.append((z::char16_t)ch);
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
    z::unused_t(re);
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
    z::unused_t(str);
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
    z::unused_t(p);
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
#if defined(GUI)
struct z::widget::impl {
#if defined(WIN32)
    inline impl() : _val(0), _menu(0), _id(0) {}
    HWND _val;          /// contains HWND if this is a window
    HMENU _menu;         /// contains HMENU if this is a menu (\_val will point to parent window)
    uint32_t _id;        /// contains id if this is menuitem or systray.
    NOTIFYICONDATA _ni; /// if this is a systray. The member hWnd contains handle to parent window
#elif defined(GTK)
    inline impl() : _val(0), _fixed(0), _icon(0), _menu(0), _menuItem(0) {}
    GtkWidget* _val;    /// contains window or menu or menuitem
    GtkWidget* _fixed;   /// contains pointer to fixed layout child if _val is a parent frame
    GtkStatusIcon* _icon;
    GtkWidget* _menu;
    GtkWidget* _menuItem;
#elif defined(QT)
#elif defined(OSX)
    inline impl() {} // : _val(0), _frame(0) {}
    NSView* _val;
    NSWindow* _frame;
#elif defined(IOS)
#else
#error "Unimplemented GUI mode"
#endif
    typedef dict<string, widget> ChildList;
    ChildList _childList;
};

void z::widget::clear() const {
    delete _val;
    _val = 0;
}

void z::widget::set(const z::string& key, const z::widget& v) {
    z::ref(_val)._childList[key] = v;
}

z::widget z::widget::at(const z::string& key) const {
    return z::ref(_val)._childList.at(key);
}

#if defined(WIN32)
NOTIFYICONDATA& z::widget::ni() const {
    return z::ref(_val)._ni;
}
#endif
#endif

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
    z::unused_t(name);
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
static const z::size MaxPumpCount = 10;
static const z::size MaxPollTimeout = 10;

///////////////////////////////////////////////////////////////
/// \brief run queue
struct z::RunQueue {
private:
    typedef z::queue<z::Future*> InvocationList;
    InvocationList _list;
public:
    inline z::size run(const z::size& cnt) {
        assert(s_tctx != 0);
        for(z::size i = 0; i < cnt; ++i) {
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

z::size z::ThreadContext::wait() {
    pump();
    return 0;
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
    z::unused_t(hwnd);
    z::unused_t(uMsg);
    z::unused_t(idEvent);
    z::unused_t(time);
    pump();
}
#endif // WIN32
#if defined(GTK)
static gboolean onIdle(gpointer data) {
    z::unused_t(data);
    pump();
    return TRUE;
}
#endif // GTK

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
#endif // QT/OSX
#endif // GUI

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
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        throw z::Exception("z::application", z::string("WSAStartup failed"));
    }
#endif

#if defined(GUI) && defined(GTK)
    gtk_init(&argc, &argv);
#endif
}

z::application::~application() {
#if defined(WIN32)
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

inline int z::application::execEx() {
#if defined(UNIT_TEST)
    TestResult tr; z::unused_t(tr);
#endif
    int code = 0;

    // this is the local context for thread-0.
    // all UI events will run in this context
    ThreadContext tctx(gctx().at(0)); z::unused_t(tctx);

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
#elif defined(GTK)
    // create idle handler
    g_idle_add(onIdle, 0);

    // spin main loop
    gtk_main();
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
    z::unused_t(ctimer);
#if defined(OSX) //COCOA_NIB
    code = NSApplicationMain(_argc, (const char**)_argv);
#elif defined(IOS)
    @autoreleasepool {
        z::string ccn;
        appClass(ccn);
        z::estring es = z::s2e(ccn);
        NSString* ns = [NSString stringWithUTF8String:es.c_str()];
        return UIApplicationMain(_argc, _argv, nil, ns);
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
    int rv = execEx();
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
#elif defined(GTK)
    gtk_main_quit();
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
}

#if defined(GUI) && defined(WIN32)
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    z::unused_t(hPrevInstance);z::unused_t(lpCmdLine);z::unused_t(nCmdShow);
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
