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

class CallContext {
public:
    void run();
    inline size_t size() const {return _invocationList.size();}

private:
    struct Invocation {
        virtual void run() = 0;
    };

    template <typename FunctionT>
    struct InvocationT : public Invocation {
        InvocationT(const FunctionT& function, const typename FunctionT::_In& in) : _function(function), _in(in) {}
        FunctionT _function;
        typename FunctionT::_In _in;
        virtual void run() {_function.run(_in);}
    };
    typedef std::list<Invocation*> InvocationList;
    InvocationList _invocationList;

public:
    template <typename FunctionT>
    FunctionT add(FunctionT function, const typename FunctionT::_In& in) {
        _invocationList.push_back(new InvocationT<FunctionT>(function, in));
        return function;
    }
};

template <typename T>
struct Event {
    struct Handler {
        struct Item {
            inline Item(Handler* x) : t(x) {}
            Handler* t;
        };
        typedef std::list<Item> List;

        struct Ptr {
            Item& _item;
            inline Ptr(Item& item) : _item(item) {}
            inline Ptr(const Ptr& src) : _item(src._item) {}
        };

        inline Handler() {}
        virtual ~Handler(){}
    };

    template <typename FnT>
    class AddHandler {
    protected:
        typename Handler::Ptr _handler;
    public:
        inline AddHandler(typename Handler::Ptr handler) : _handler(handler) {}
    };

    typename Handler::List list;
    inline ~Event() {
    }

    inline typename Handler::Ptr add_(Handler* handler) {
        list.push_back(typename Handler::Item(handler));
        typename Handler::Item& item = list.back();
        return typename Handler::Ptr(item);
    }

    static T instance;
    static inline typename Handler::Ptr add(Handler* handler) {return instance.add_(handler);}
};

struct TestInstance {
    TestInstance();
    virtual void enque(CallContext& context) = 0;
    TestInstance* _next;
};

template <typename T>
struct test {
    struct _Out {
        inline _Out(const int& passed) : _passed(passed) {}
        int _passed;
    };
    _Out* _out;
    inline void ret(_Out* val) {_out = val;}
public:
    struct _In {
        inline _In() {}
    };

    inline test() : _instance(ref(static_cast<T*>(this))) {}
    struct Instance : public TestInstance {
        inline Instance(T& t) : _t(t) {}
        virtual void enque(CallContext& context) {
            context.add(_t, _In());
        }
        T& _t;
    } _instance;
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
