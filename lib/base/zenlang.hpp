#pragma once

#define unused(x) ((void)x)

template <typename T>
inline T& ref(T* t) {
    assert(t);
    return *t;
}

template <typename T>
inline T* ptr(T& t) {
    assert(&t);
    return &t;
}

inline void _trace(const std::string& txt) {
#if defined(_WIN32)
    OutputDebugStringA(txt.c_str());
#else
    std::cout << txt.c_str() << std::flush;
#endif
}

inline std::string ssprintfv(const char* txt, va_list vlist) {
    const int len = 1024;
    char buf[len];
#if defined(_WIN32)
    vsnprintf_s(buf, len, _TRUNCATE, txt, vlist);
#else
    vsnprintf(buf, len, txt, vlist);
#endif
    va_end(vlist);
    return buf;
}

inline std::string ssprintf(const char* txt, ...) {
    va_list vlist;
    va_start(vlist, txt);
    return ssprintfv(txt, vlist);
}

inline void trace(const char* txt, ...) {
#if defined(DEBUG)
    va_list vlist;
    va_start(vlist, txt);
    std::string buf = ssprintfv(txt, vlist);
    _trace(buf);
#else
    unused(txt);
#endif
}

class Exception {
public:
    inline Exception(const char* txt, ...) {
        va_list vlist;
        va_start(vlist, txt);
        _msg = ssprintfv(txt, vlist);
        printf("%s\n", _msg.c_str());
    }

private:
    std::string _msg;
};

template <typename T>
struct Pointer {
    inline void reset(T* ptr) {delete _ptr; _ptr = 0; _ptr = ptr;}
    inline T* clone(const Pointer& src) {
        if(src._ptr == 0)
            return 0;
        return new T(ref(src._ptr));
    }

    inline Pointer() : _ptr(0) {}
    inline ~Pointer() {reset(0);}
    inline Pointer(const Pointer& src) : _ptr(0) {reset(clone(src));}
    inline Pointer& operator=(const Pointer& src) {reset(clone(src)); return ref(this);}
    inline Pointer& operator=(T* ptr) {reset(ptr); return ref(this);}
    inline Pointer& operator=(T t) {reset(new T(t)); return ref(this);}
    inline T& operator*() {return ref(_ptr);}
    inline T* operator->() {assert(_ptr); return _ptr;}
private:
    T* _ptr;
};

template <typename V>
struct ListCreator {
    inline ListCreator& add(V v) {
        _list.push_back(v);
        return ref(this);
    }
    inline std::list<V> value() {return _list;}
    std::list<V> _list;
};

template <typename K, typename V>
struct DictCreator {
    inline DictCreator& add(K k, V v) {
        _list[k] = v;
        return ref(this);
    }
    inline std::map<K, V> value() {return _list;}
    std::map<K, V> _list;
};

namespace String {
    inline void replace(std::string& text, const std::string& search, const std::string& replace) {
        for(std::string::size_type next = text.find(search); next != std::string::npos;next = text.find(search, next)) {
            text.replace(next, search.length(), replace);
            next += replace.length();
        }
    }
}

struct Formatter {
    inline Formatter(const std::string& text) : _text(text) {}
    template <typename T>
    inline Formatter& add(const std::string& key, T value) {
        std::stringstream ss;
        ss << value;
        std::string replace = ss.str();
        std::string search = "%{" + key + "}";
        String::replace(_text, search, replace);
        return ref(this);
    }
    inline std::string value() {return _text;}
private:
    std::string _text;
};

struct Future {
    virtual void run() = 0;
};

template <typename FunctionT>
struct FutureT : public Future {
    FutureT(const FunctionT& function, const typename FunctionT::_In& in) : _function(function), _in(in) {}
    FunctionT _function;
    typename FunctionT::_In _in;
    virtual void run() {_function.run(_in);}
};

template <typename FunctionT>
struct FunctorT {
    inline FunctorT(FunctionT* function) : _function(function) {}
    inline FunctionT* function() const {return _function;}
private:
    FunctionT* _function;
};

template <typename FunctionT>
struct FunctorList {
    inline FunctionT& add(const FunctorT<FunctionT>& h) {
        _list.push_back(h.function());
        return ref(h.function());
    }

private:
    typedef std::list<FunctionT*> List;
    List _list;
};

class FunctionList {
private:
    typedef std::list<Future*> InvocationList;
    InvocationList _invocationList;
public:
    struct Ptr {
        inline Ptr() : _ptr(0) {}
        inline ~Ptr() {delete _ptr; _ptr = 0;}
        inline void set(Future* ptr) {delete _ptr; _ptr = 0; _ptr = ptr;}
        inline Future* operator->() {return _ptr;}
     private:
        inline Ptr(const Ptr& src) : _ptr(0) {unused(src);}
        inline Ptr& operator=(const Ptr& src) {unused(src); return ref(this);}
        Future* _ptr;
    };

    inline size_t size() const {return _invocationList.size();}

    template <typename FunctionT>
    FutureT<FunctionT>& push(FunctionT function, const typename FunctionT::_In& in) {
        FutureT<FunctionT>* inv = new FutureT<FunctionT>(function, in);
        _invocationList.push_back(inv);
        return ref(inv);
    }

    inline bool pop(Ptr& ptr) {
        if(size() == 0)
            return false;
        ptr.set(_invocationList.front());
        _invocationList.pop_front();
        return true;
    }
};

class CallContext {
public:
    static CallContext& get();
public:
    void run();
public:
    template <typename FunctionT>
    FutureT<FunctionT>& add(FunctionT function, const typename FunctionT::_In& in) {
        return _list.push(function, in);
    }
private:
    FunctionList _list;
};

template <typename KeyT, typename ValT>
struct HandlerList {
    typedef std::list<ValT*> List;
    typedef std::map<KeyT, List> Map;
    Map map;

    inline void addHandler(const KeyT& key, ValT* val) {
        List& list = map[key];
        list.push_back(val);
    }

    inline bool runHandler(const KeyT& key) {
        typename Map::const_iterator it = map.find(key);
        if(it == map.end())
            return false;

        const List& list = it->second;
        for(typename List::const_iterator itl = list.begin(); itl != list.end(); ++itl) {
            ValT* handler = *itl;
            handler->run();
        }
        return true;
    }
};

#if defined(UNIT_TEST)
template <typename T>
struct test_ {
    struct _Out {
        inline _Out(const int& passed) : _passed(passed) {}
        int _passed;
    };
public:
    struct _In {
        inline _In() {}
    };
public:
    Pointer<_Out> _out;
    inline const _Out& out(_Out* val) {_out = val;return *_out;}
};

struct TestInstance {
    TestInstance();
    virtual void enque(CallContext& context) = 0;
    TestInstance* _next;
};

template <typename T>
struct TestInstanceT : public TestInstance {
    virtual void enque(CallContext& context) {
        T t;
        typename T::_In in;
        context.add(t, in);
    }
};
#endif

typedef std::list<std::string> ArgList;

template <typename T>
struct main_ {
    struct _Out {
        inline _Out(const int& code) : _code(code) {}
        int _code;
    };
public:
    struct _In {
        inline _In(const ArgList& argl) : _argl(argl) {}
        ArgList _argl;
    };
public:
    Pointer<_Out> _out;
    inline const _Out& out(_Out* val) {_out = val;return *_out;}
};

struct MainInstance {
    MainInstance();
    virtual void enque(CallContext& context, const ArgList& argl) = 0;
    MainInstance* _next;
};

template <typename T>
struct MainInstanceT : public MainInstance {
    virtual void enque(CallContext& context, const ArgList& argl) {
        T t;
        typename T::_In in(argl);
        context.add(t, in);
    }
};

#if defined(Z_EXE)
struct Application {
    Application(int argc, char* argv[]);
    ~Application();
    int exec();
};
#endif

#if 0
/// \todo helpers to invoke function objects
template <typename MethodT, typename ReturnT >
struct Method<ReturnT(*)(MethodT& This)> : public MethodX<ReturnT(*)(MethodT& This)> {
    inline Method(const typename Method<MethodT>::Impl& impl) : MethodX<ReturnT(*)(MethodT& This)>(impl) {}
    inline ReturnT run() {return (*(ref(this)._impl))(ref(static_cast<MethodT*>(this)));}
};

template <typename MethodT, typename ReturnT, typename P1 >
struct Method<ReturnT(*)(MethodT&,P1)> : public MethodX<ReturnT(*)(MethodT&, P1)> {
    inline Method(const typename Method<MethodT>::Impl& impl) : MethodX<ReturnT(*)(MethodT&, P1)>(impl) {}
    inline ReturnT run(P1 p1) {return (*(ref(this)._impl))(ref(static_cast<MethodT*>(this)), p1);}
};

template <typename MethodT, typename ReturnT, typename P1, typename P2 >
struct Method<ReturnT(*)(MethodT&,P1,P2)> : public MethodX<ReturnT(*)(MethodT&,P1,P2)> {
    inline Method(const typename Method<MethodT>::Impl& impl) : MethodX<ReturnT(*)(MethodT&,P1,P2)>(impl) {}
    inline ReturnT run(P1 p1, P2 p2) {return (*(ref(this)._impl))(ref(static_cast<MethodT*>(this)), p1, p2);}
};

template <typename MethodT, typename ReturnT, typename P1, typename P2, typename P3 >
struct Method<ReturnT(*)(MethodT&,P1,P2,P3)> : public MethodX<ReturnT(*)(MethodT&,P1,P2,P3)> {
    inline Method(const typename Method<MethodT>::Impl& impl) : MethodX<ReturnT(*)(MethodT&,P1,P2,P3)>(impl) {}
    inline ReturnT run(P1 p1, P2 p2, P3 p3) {return (*(ref(this)._impl))(ref(static_cast<MethodT*>(this)), p1, p2, p3);}
};

template <typename MethodT, typename ReturnT, typename P1, typename P2, typename P3, typename P4 >
struct Method<ReturnT(*)(MethodT&,P1,P2,P3,P4)> : public MethodX<ReturnT(*)(MethodT&,P1,P2,P3,P4)> {
    inline Method(const typename Method<MethodT>::Impl& impl) : MethodX<ReturnT(*)(MethodT&,P1,P2,P3,P4)>(impl) {}
    inline ReturnT run(P1 p1, P2 p2, P3 p3, P4 p4) {return (*(ref(this)._impl))(ref(static_cast<MethodT*>(this)), p1, p2, p3, p4);}
};

template <typename MethodT, typename ReturnT, typename P1, typename P2, typename P3, typename P4, typename P5 >
struct Method<ReturnT(*)(MethodT&,P1,P2,P3,P4,P5)> : public MethodX<ReturnT(*)(MethodT&,P1,P2,P3,P4,P5)> {
    inline Method(const typename Method<MethodT>::Impl& impl) : MethodX<ReturnT(*)(MethodT&,P1,P2,P3,P4,P5)>(impl) {}
    inline ReturnT run(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {return (*(ref(this)._impl))(ref(static_cast<MethodT*>(this)), p1, p2, p3, p4, p5);}
};

template <typename MethodT, typename ReturnT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
struct Method<ReturnT(*)(MethodT&,P1,P2,P3,P4,P5,P6)> : public MethodX<ReturnT(*)(MethodT&,P1,P2,P3,P4,P5,P6)> {
    inline Method(const typename Method<MethodT>::Impl& impl) : MethodX<ReturnT(*)(MethodT&,P1,P2,P3,P4,P5,P6)>(impl) {}
    inline ReturnT run(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) {return (*(ref(this)._impl))(ref(static_cast<MethodT*>(this)), p1, p2, p3, p4, p5, p6);}
};

template <typename MethodT, typename ReturnT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7 >
struct Method<ReturnT(*)(MethodT&,P1,P2,P3,P4,P5,P6,P7)> : public MethodX<ReturnT(*)(MethodT&,P1,P2,P3,P4,P5,P6,P7)> {
    inline Method(const typename Method<MethodT>::Impl& impl) : MethodX<ReturnT(*)(MethodT&,P1,P2,P3,P4,P5,P6,P7)>(impl) {}
    inline ReturnT run(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) {return (*(ref(this)._impl))(ref(static_cast<MethodT*>(this)), p1, p2, p3, p4, p5, p6, p7);}
};

template <typename MethodT, typename ReturnT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8 >
struct Method<ReturnT(*)(MethodT&,P1,P2,P3,P4,P5,P6,P7,P8)> : public MethodX<ReturnT(*)(MethodT&,P1,P2,P3,P4,P5,P6,P7,P8)> {
    inline Method(const typename Method<MethodT>::Impl& impl) : MethodX<ReturnT(*)(MethodT&,P1,P2,P3,P4,P5,P6,P7,P8)>(impl) {}
    inline ReturnT run(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) {return (*(ref(this)._impl))(ref(static_cast<MethodT*>(this)), p1, p2, p3, p4, p5, p6, p7, p8);}
};
#endif
