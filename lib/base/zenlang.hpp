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

typedef std::string string;

inline void _trace(const std::string& txt) {
#if defined(WIN32)
    OutputDebugStringA(txt.c_str());
#else
    std::cout << txt.c_str() << std::flush;
#endif
}

inline std::string ssprintfv(const char* txt, va_list vlist) {
    const int len = 1024;
    char buf[len];
#if defined(WIN32)
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
        printf("Error: %s\n", _msg.c_str());
    }

private:
    std::string _msg;
};

template <typename ImplT>
struct MethodX {
    typedef ImplT Impl;
    inline MethodX(const Impl& impl) : _impl(impl) {}
protected:
    Impl _impl;
};

template <typename MethodT>
struct Method : public MethodX<MethodT& (*)(MethodT& This)> {
    inline Method(const typename Method<MethodT>::Impl& impl) : MethodX<MethodT& (*)(MethodT& This)>(impl) {}
    inline void run() {return (*(ref(this)._impl))(ref(static_cast<MethodT*>(this)));}
};

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

template <typename T>
struct AbstractFunction : public Method<T> {
    inline AbstractFunction(const typename AbstractFunction<T>::Impl& impl) : Method<T>(impl) {}
};

template <typename T>
struct Function : public AbstractFunction<T> {
    inline Function(const typename Function<T>::Impl& impl) : AbstractFunction<T>(impl) {}
};

template <typename T>
struct Event {
    struct Handler : public Method<Handler> {
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

        inline Handler(typename Handler::Impl impl) : Method<Handler>(impl) {}
        virtual ~Handler(){}
    };

    template <typename FnT>
    class AddHandler : public Function<FnT> {
    protected:
        typename Handler::Ptr _handler;
    public:
        inline AddHandler(const typename AddHandler::Impl& impl, typename Handler::Ptr handler) : Function<FnT>(impl), _handler(handler) {}
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
