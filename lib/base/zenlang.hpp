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

#if defined(DEBUG)
#if defined(GUI) && defined(WIN32)
inline void trace(const char* txt, ...) {
    va_list vlist;
    va_start(vlist, txt);
    std::string buf = ssprintfv(txt, vlist);
    OutputDebugStringA(buf.c_str());
}
#else
#define trace printf
#endif
#else
#define trace(x)
#endif

class Exception {
public:
    inline Exception(const char* txt, ...) {
        va_list vlist;
        va_start(vlist, txt);
        _msg = ssprintfv(txt, vlist);
        trace("%s\n", _msg.c_str());
    }

private:
    std::string _msg;
};

template <typename V>
struct Pointer {
    struct value {
    protected:
        inline value(){}
    public:
        virtual V& get() = 0;
        virtual value* clone() const = 0;
    };

    template <typename DerT>
    struct valueT : public value {
        inline valueT(const DerT& v) : _v(v) {const V& x = v;unused(x);}
        virtual V& get() {return _v;}
        virtual value* clone() const {return new valueT<DerT>(_v);}
    private:
        DerT _v;
    };

    inline void set(value* val) {
        delete _val;
        _val = val;
    }

    inline V& get() const {
        valueT<V>* vt = dynamic_cast<valueT<V>*>(_val);
        if(vt == 0) {
            throw Exception("Typecast error in pointer");
        }
        return ref(vt).get();
    }

    inline Pointer() : _val(0) {}

    inline ~Pointer() {delete _val;}

    template <typename DerT>
    inline Pointer(const DerT& val) : _val(0) {
        value* v = new valueT<DerT>(val);
        set(v);
    }

    inline Pointer(const Pointer& src) : _val(0) {
        if(src._val) {
            value* v = src._val->clone();
            set(v);
        }
    }

//    template <typename DerT>
//    inline Pointer(const Pointer<DerT>& src) : _val(0) {
//        if(src._val) {
//            value* v = src._val->clone();
//            set(v);
//        }
//    }

    template <typename DerT>
    inline Pointer& setVal(const DerT& val) {
        value* v = new valueT<DerT>(val);
        set(v);
        return ref(this);
    }

    inline Pointer& operator=(const Pointer& src) {
        value* v = src._val->clone();
        set(v);
        return ref(this);
    }

    template <typename DerT>
    inline Pointer& operator=(const DerT& val) {
        return setVal(val);
    }

//    template <typename DerT>
//    inline Pointer& operator=(const Pointer<DerT>& src) {
//        value* v = src._val->clone();
//        set(v);
//        return ref(this);
//    }

private:
    value* _val;
};

template <typename V>
struct pointer : public Pointer<V> {
    inline pointer(const std::string& tname) : Pointer<V>(), _tname(tname) {}

    template <typename DerT>
    inline pointer(const std::string& tname, const DerT& val) : Pointer<V>(val), _tname(tname) {}

    template <typename DerT>
    inline pointer(const pointer<DerT>& src) : Pointer<V>(src.get()), _tname(src.tname()) {}

    template <typename DerT>
    inline pointer& operator=(const pointer<DerT>& val) {
        _tname = val.tname();
        setVal(val);
        return ref(this);
    }

    inline const std::string& tname() const {return _tname;}
private:
    std::string _tname;
};

template <typename V, typename ListT>
struct container {
    typedef ListT List;
    typedef typename List::iterator iterator;
    inline iterator begin() {return _list.begin();}
    inline iterator end() {return _list.end();}
protected:
    List _list;
};

template <typename V>
struct list : public container<V, std::list<V> > {
    typedef container<V, std::list<V> > BaseT;

    inline void add(const V& v) {BaseT::_list.push_back(v);}
    struct creator {
        inline creator& add(const V& v) {
            _list.add(v);
            return ref(this);
        }
        inline list get() {return _list;}
        list _list;
    };
};

template <typename K, typename V>
struct dict : public container<V, std::map<K, V> > {
    typedef container<V, std::map<K, V> > BaseT;

    inline void add(const K& k, V v) {BaseT::_list.insert(std::pair<K, V>(k, v));}
    inline void clone(const dict& src) {
        for(typename BaseT::List::const_iterator it = src._list.begin(); it != src._list.end(); ++it) {
            const K& k = it->first;
            const V& v = it->second;
            add(k, v);
        }
    }

    struct creator {
        inline creator& add(const K& k, const V& v) {
            _list.add(k, v);
            return ref(this);
        }

        template <typename DerT>
        inline creator& add(const K& k, const DerT& dv) {
            V v(dv);
            _list.add(k, v);
            return ref(this);
        }
        inline dict get() {return _list;}
        dict _list;
    };
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
    inline std::string get() {return _text;}
private:
    std::string _text;
};

//template <typename FunctionT>
//struct FunctorT {
//    inline FunctorT(const FunctionT& function) : _function() {_function = new FunctionT(function);}
//    inline FunctionT* function() const {return _function;}
//private:
//    FunctionT* _function;
//};

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
struct FunctorList {
    inline FunctionT& add(FunctionT* h) {
        _list.push_back(h);
        return ref(h);
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
    inline FutureT<FunctionT>& add(FunctionT function, const typename FunctionT::_In& in) {
        return _list.push(function, in);
    }

//    template <typename FunctionT>
//    inline FutureT<FunctionT>& add(FunctorT<FunctionT> functor, const typename FunctionT::_In& in) {
//        return _list.push(ref(functor.function()), in);
//    }

    inline size_t size() const {return _list.size();}
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

    inline bool runHandler(const KeyT& key, typename ValT::_In in) {
        typename Map::const_iterator it = map.find(key);
        if(it == map.end())
            return false;

        const List& list = it->second;
        for(typename List::const_iterator itl = list.begin(); itl != list.end(); ++itl) {
            ValT* handler = *itl;
            handler->run(in);
        }
        return true;
    }
};

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

    inline void begin() {
        _next = _head;
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

struct Log {
    struct Out{};
    static Log& get();
    Log& operator <<(Out);
    template <typename T> inline Log& operator <<(const T& val) {_ss << val; return ref(this);}
private:
    std::stringstream _ss;
};

#if defined(UNIT_TEST)
struct TestResult {
    ~TestResult();
    static void begin(const std::string& name);
    static void end(const std::string& name, const bool& passed);
};

template <typename T>
struct test_ {
public:
    struct _Out {
        inline _Out(const bool& passed) : _passed(passed) {}
        bool _passed;
    };
public:
    struct _In {
        inline _In() {}
    };
public:
    Pointer<_Out> _out;
    inline const _Out& out(const _Out& val) {_out = val; return _out.get();}
    virtual const _Out& run(const _In& _in) {
        T& t = static_cast<T&>(ref(this));
        TestResult::begin(t.name());
        const _Out& out = t.test(_in);
        TestResult::end(t.name(), out._passed);
        return out;
    }
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

#if defined(WIN32)
struct win32 {
#if defined(GUI)
    static int getNextWmID();
    static int getNextResID();
#endif
};
#endif

#if defined(Z_EXE)
struct Application {
    Application(int argc, char* argv[]);
    ~Application();
#if defined(WIN32)
    static HINSTANCE instance();
#endif
    int exec();
    int exit(const int& code);
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
