#pragma once

#define unused(x) ((void)(&x))

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
    #define trace(f)
#endif

namespace z {
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

    template <typename T>
    inline unsigned long pad(T& t) {
        assert(&t);
        return (unsigned long)(&t);
    }

    struct string {
        static inline char_t lower(const char_t& ch) {
            if((ch >= 'A') && (ch <= 'Z'))
                return 'a' + (ch - 'A');
            return ch;
        }

        explicit inline string() {}
        inline string(const char* s) : _val(s) {}
        inline string(const std::string& s) : _val(s) {}

        typedef typename std::string::iterator iterator;
        inline iterator begin() {return _val.begin();}
        inline iterator end() {return _val.end();}

        typedef typename std::string::const_iterator const_iterator;
        inline const_iterator begin() const {return _val.begin();}
        inline const_iterator end() const {return _val.end();}

        typedef typename std::string::size_type size_type;
        static const size_type npos  = std::string::npos;

        inline size_type size() const {return _val.size();}
        inline size_type length() const {return _val.length();}

        inline bool operator==(const z::string& rhs) const {return (_val == rhs._val);}
        inline bool operator!=(const z::string& rhs) const {return (_val != rhs._val);}
        inline bool operator< (const z::string& rhs) const {return (_val <  rhs._val);}
        inline bool operator> (const z::string& rhs) const {return (_val >  rhs._val);}

        inline char_t at(const size_type& idx) const {return _val.at(idx);}
        inline z::string substr(const size_type& from, const size_type& len) const {return _val.substr(from, len);}
        inline z::string substr(const size_type& from) const {return _val.substr(from);}

        inline size_type find(const char& s) const {return _val.find(s);}
        inline size_type find(const z::string& s) const {return _val.find(s._val);}
        inline size_type find(const z::string& s, const size_type& from) const {return _val.find(s._val, from);}
        inline size_type rfind(const char& s) const {return _val.rfind(s);}
        inline size_type rfind(const z::string& s) const {return _val.rfind(s._val);}
        inline z::string replace(const size_type& from, const size_type& len, const z::string& to) {return _val.replace(from, len, to._val);}

        inline void replace(const z::string& search, const z::string& replace) {
            for(z::string::size_type next = _val.find(search._val); next != z::string::npos;next = _val.find(search._val, next)) {
                _val.replace(next, search.length(), replace._val);
                next += replace.length();
            }
        }

        inline void append(const char_t& rhs) {_val += rhs;}
        inline void append(const z::string& rhs) {_val.append(rhs._val);}
        inline void clear() {_val.clear();}

        inline z::string& operator= (const char_t& rhs) {_val = rhs; return *this;}
        inline z::string& operator= (const char_t* rhs) {_val = rhs; return *this;}
        inline z::string& operator= (const z::string& rhs) {_val = rhs._val; return *this;}
        inline z::string& operator+=(const z::string& rhs) {append(rhs); return *this;}
        inline z::string& operator+=(const char_t& rhs) {append(rhs); return *this;}
        inline char_t operator[](const size_type& idx) const {return _val[idx];}

        inline z::string lower() const {
            z::string r;
            for(std::string::const_iterator it = _val.begin(); it != _val.end(); ++it) {
                const char_t& ch = *it;
                r += lower(ch);
            }
            return r;
        }

        inline const std::string& val() const {return _val;}
        inline const char* c_str() const {return _val.c_str();}
        inline const char* toUtf8() const {return _val.c_str();}

        template <typename T> inline z::string& arg(const z::string& key, T value);
        template <typename T> inline T to() const {
            std::stringstream ss(_val);
            T val;
            ss >> val;
            return val;
        }

    private:
        std::string _val;
    };

    struct datetime {
        inline datetime() : _val(0) {}
        inline datetime(const int64_t& val) : _val(val) {}
        inline datetime& operator=(const int64_t& val) {_val = val; return z::ref(this);}
        inline const int64_t& val() const {return _val;}
    private:
        int64_t _val;
    };
}

inline z::string operator+(const char* lhs, const z::string& rhs) {return (lhs + rhs.val());}
inline z::string operator+(const z::string& lhs, const char* rhs) {return (lhs.val() + rhs);}
inline z::string operator+(const z::string& lhs, const char_t rhs) {return (lhs.val() + rhs);}
inline z::string operator+(const z::string& lhs, const z::string& rhs) {return (lhs.val() + rhs.val());}

inline std::ostream& operator<<(std::ostream& os, const z::string& val) {
    os << val.val();
    return os;
}

template <typename T>
inline z::string& z::string::arg(const z::string& key, T value) {
    std::stringstream skey;
    skey << "%{" << key << "}";
    std::stringstream sval;
    sval << value;
    replace(skey.str(), sval.str());
    return z::ref(this);
}

namespace z {
    template <typename T>
    inline z::string type_name() {
        const char* name = typeid(T).name();
        z::string uname = name;
    #if defined(_WIN32)
        // msvc-cl returns unmangled name by default, so do nothing
    #else
        int status = -4;
        char* dname = abi::__cxa_demangle(name, NULL, NULL, &status);
        if(dname) {
            if(status == 0)
                uname = dname;
            free(dname);
        }
    #endif
        return uname;
    }

    template <typename T>
    inline z::string type_name(const T& t) {
        unused(t);
        return type_name<T>();
    }

    struct fmt {
        explicit inline fmt(const z::string& text) : _text(text) {}

        template <typename T>
        inline fmt& add(const z::string& key, T value) {
            _text.arg(key, value);
            return z::ref(this);
        }
        inline const z::string& get() const {return _text;}
    private:
        z::string _text;
    };

    inline void mlog(const z::string& src, const z::fmt& msg) {std::cout << src << ":" << msg.get() << std::endl;}
    inline void elog(const z::string& src, const z::fmt& msg) {std::cout << src << ":" << msg.get() << std::endl;}

    class Exception {
    public:
        explicit inline Exception(const z::string& src, const fmt& msg) : _msg(msg) {elog(src, _msg);}

    private:
        const fmt _msg;
    };

    template <typename V>
    struct Pointer {
        struct value {
        protected:
            inline value(){}
        public:
            virtual V& get() = 0;
            virtual value* clone() const = 0;
            virtual z::string tname() const = 0;
        };

        template <typename DerT>
        struct valueT : public value {
            inline valueT(const DerT& v) : _v(v) {const V& x = v;unused(x);}
            virtual V& get() {return _v;}
            virtual value* clone() const {return new valueT<DerT>(_v);}
            virtual z::string tname() const {return type_name<DerT>();}
        private:
            DerT _v;
        };

        inline void set(value* val) {
            delete _val;
            _val = val;
        }

        inline bool has() const {return (_val != 0);}
        inline value* clone() const {return ref(_val).clone();}

        inline V& get() const {
            return ref(_val).get();
        }

        inline Pointer() : _val(0) {
        }

        inline Pointer(value* val) : _val(val) {
        }

        inline ~Pointer() {
            delete _val;
        }

        inline Pointer(const Pointer<V>& src) : _val(0) {
            if(src.has()) {
                value* v = src.clone();
                set(v);
            }
        }

        template <typename DerT>
        inline Pointer(const Pointer<DerT>& src) : _val(0) {
            if(src.has()) {
                const DerT& d = src.get();
                value* v = new valueT<DerT>(d);
                set(v);
            }
        }

        inline Pointer& operator=(const Pointer& src) {
            value* v = src.clone();
            set(v);
            return ref(this);
        }

    private:
        value* _val;
    };

    struct type {
        explicit inline type(const z::string& name) : _name(name) {}
        inline const z::string& name() const {return _name;}
        inline bool operator==(const type& rhs) const {return (_name == rhs._name);}
        inline bool operator!=(const type& rhs) const {return (_name != rhs._name);}
    private:
        z::string _name;
    };

    template <typename V>
    struct pointer : public z::Pointer<V> {
        /// default-ctor is required when this struct is used as the value in a dict.
        /// \todo Find out way to avoid it.
        inline pointer() : z::Pointer<V>(), _tname("") {}
        inline pointer(const type& tname, typename z::Pointer<V>::value* val) : z::Pointer<V>(val), _tname(tname) {}
        inline pointer(const pointer& src) : z::Pointer<V>(src), _tname(src._tname) {}
        inline pointer& operator=(const pointer& val) {
            _tname = val._tname;
            return ref(this);
        }

        template <typename DerT>
        inline pointer(const pointer<DerT>& src) : z::Pointer<V>(src), _tname(src.tname()) {}

        template <typename DerT>
        inline DerT& getT() const {
            V& v = z::Pointer<V>::get();
            return static_cast<DerT&>(v);
        }

        inline const type& tname() const {return _tname;}
    private:
        type _tname;
    };

    template <typename V, typename DerT>
    struct PointerCreator {
        typedef typename z::Pointer<V>::template valueT<DerT> VT;
        static inline z::Pointer<V> get(const DerT& val) {
            return z::Pointer<V>(new VT(val));
        }
        static inline pointer<V> get(const type& tname, const DerT& val) {
            return pointer<V>(tname, new VT(val));
        }
    };

    template <typename V, typename ListT>
    struct container {
        typedef ListT List;
        typedef typename List::size_type size_type;

        typedef typename List::iterator iterator;
        inline iterator begin() {return _list.begin();}
        inline iterator end() {return _list.end();}

        typedef typename List::const_iterator const_iterator;
        inline const_iterator begin() const {return _list.begin();}
        inline const_iterator end() const {return _list.end();}

        inline size_type size() const {return _list.size();}
        inline bool empty() const {return (_list.size() == 0);}

        inline void clear() {_list.clear();}
        inline void erase(iterator& it) {_list.erase(it);}
    protected:
        List _list;
    };

    template <typename V, typename ListT>
    struct listbase : public z::container<V, ListT > {
        typedef z::container<V, ListT > BaseT;
        inline V& back() {return BaseT::_list.back();}
    };

    template <typename V>
    struct stack : public z::listbase<V, std::list<V> > {
        typedef z::listbase<V, std::list<V> > BaseT;

        inline V& top() {
            assert(BaseT::_list.size() > 0);
            V& v = BaseT::_list.front();
            return v;
        }

        inline const V& top() const {
            assert(BaseT::_list.size() > 0);
            const V& v = BaseT::_list.front();
            return v;
        }

        inline V& push(const V& v) {
            BaseT::_list.push_front(v);
            return top();
        }

        inline V pop() {
            assert(BaseT::_list.size() > 0);
            V v = BaseT::_list.front();
            BaseT::_list.pop_front();
            return v;
        }
    };

    template <typename V>
    struct queue : public z::listbase<V, std::list<V> > {
        typedef z::listbase<V, std::list<V> > BaseT;

        inline V& front() {
            assert(BaseT::_list.size() > 0);
            V& v = BaseT::_list.front();
            return v;
        }

        inline const V& front() const {
            assert(BaseT::_list.size() > 0);
            const V& v = BaseT::_list.front();
            return v;
        }

        inline void enqueue(const V& v) {
            BaseT::_list.push_back(v);
        }

        inline V dequeue() {
            assert(BaseT::_list.size() > 0);
            V v = BaseT::_list.front();
            BaseT::_list.pop_front();
            return v;
        }

        inline typename BaseT::iterator erase(typename BaseT::iterator it) {
            return BaseT::_list.erase(it);
        }
    };

    template <typename V>
    struct list : public z::listbase<V, std::vector<V> > {
        typedef z::listbase<V, std::vector<V> > BaseT;

        inline V at(const typename BaseT::size_type& k) const {
            if(k >= BaseT::_list.size()) {
                throw Exception("z::list", fmt("%{k} out of list bounds\n").add("k", k));
            }
            return BaseT::_list.at(k);
        }

        inline bool has(const V& v) const {
            for(typename BaseT::const_iterator it = BaseT::_list.begin(); it != BaseT::_list.end(); ++it) {
                const V& iv = *it;
                if(v == iv)
                    return true;
            }
            return false;
        }

        inline void set(const typename BaseT::size_type& k, V v) {
            if(k >= BaseT::_list.size()) {
                BaseT::_list.resize(k+4);
            }
            BaseT::_list.at(k) = v;
        }

        inline V add(const V& v) {
            BaseT::_list.push_back(v);
            return BaseT::back();
        }

        inline void append(const list<V>& src) {
            for(typename BaseT::const_iterator it = src._list.begin(); it != src._list.end(); ++it) {
                const V& iv = *it;
                add(iv);
            }
        }

        template <typename CmpFnT>
        inline void sort(CmpFnT fn) {
            std::sort(BaseT::_list.begin(), BaseT::_list.end(), fn);
        }

        inline list<V> splice(const typename BaseT::size_type& from, const typename BaseT::size_type& to) const {
            list<V> nl;
            for(typename BaseT::size_type i = from; i < to; ++i) {
                const V& v = BaseT::_list.at(i);
                nl.add(v);
            }
            return nl;
        }

        struct creator {
            inline creator& add(const V& v) {
                _list.add(v);
                return ref(this);
            }
            inline list get() {return _list;}
            list _list;
        };
    };

    template <typename V>
    struct olist : public z::list<V*> {
        typedef z::list<V*> BaseT;
        inline olist() {}
        inline ~olist() {
            for(typename BaseT::iterator it = BaseT::_list.begin(); it != BaseT::_list.end(); ++it) {
                V* v = *it;
                delete v;
            }
        }

        inline V& add(V* v) {
            V* r = BaseT::add(v);
            return z::ref(r);
        }

    private:
        inline olist(const olist& /*src*/) {}
    };

    template <typename V>
    struct rlist : public z::list<V*> {
        typedef z::list<V*> BaseT;
        inline V& add(V& v) {
            V* r = BaseT::add(z::ptr(v));
            return z::ref(r);
        }

        inline V& at(const typename BaseT::size_type& k) const {
            V* v = BaseT::at(k);
            return z::ref(v);
        }

        inline bool has(const V& v) const {
            return BaseT::has(z::ptr(v));
        }
    };

    template <typename K, typename V>
    struct dict : public z::container<V, std::map<K, V> > {
        typedef z::container<V, std::map<K, V> > BaseT;

        inline V& set(const K& k, V v) {
            if(BaseT::_list.find(k) == BaseT::_list.end())
                BaseT::_list.insert(std::pair<K, V>(k, v));
            else
                BaseT::_list[k] = v;
            return BaseT::_list[k];
        }

        inline V& at(const K& k) {
            typename BaseT::iterator it = BaseT::_list.find(k);
            if(it == BaseT::_list.end()) {
                throw Exception("dict", z::fmt("%{k} not found\n").add("k", k));
            }
            return it->second;
        }

        inline const V& at(const K& k) const {
            typename BaseT::const_iterator it = BaseT::_list.find(k);
            if(it == BaseT::_list.end()) {
                throw Exception("dict", z::fmt("%{k} not found\n").add("k", k));
            }
            return it->second;
        }

        inline typename BaseT::const_iterator find(const K& k) const {
            return BaseT::_list.find(k);
        }

        inline typename BaseT::iterator find(const K& k) {
            return BaseT::_list.find(k);
        }

        inline bool has(const K& k) const {
            typename BaseT::const_iterator it = BaseT::_list.find(k);
            return (it != BaseT::_list.end());
        }

        inline void clone(const dict& src) {
            for(typename BaseT::const_iterator it = src._list.begin(); it != src._list.end(); ++it) {
                const K& k = it->first;
                const V& v = it->second;
                set(k, v);
            }
        }

        inline V& operator[](const K& k) {
            typename BaseT::iterator it = BaseT::_list.find(k);
            if(it == BaseT::_list.end()) {
                return set(k, V());
            }
            return it->second;
        }

        struct creator {
            inline creator& add(const K& k, const V& v) {
                _list.set(k, v);
                return ref(this);
            }

            inline dict get() {return _list;}
            dict _list;
        };
    };

    template <typename K, typename V>
    struct odict : public z::dict<K, V*> {
        typedef z::dict<K, V*> BaseT;
        inline ~odict() {
            for(typename BaseT::iterator it = BaseT::_list.begin(); it != BaseT::_list.end(); ++it) {
                V* v = it->second;
                delete v;
            }

        }

        inline V& set(const K& k, V* v) {
            V* r = BaseT::set(k, v);
            return z::ref(r);
        }

        inline const V& at(const K& k) const {
            const V* v = BaseT::at(k);
            return z::ref(v);
        }

        inline V& at(const K& k) {
            V* v = BaseT::at(k);
            return z::ref(v);
        }

        inline odict() {}
    private:
        inline odict(const odict& /*src*/) {}
    };

    template <typename K, typename V>
    struct rdict : public z::dict<K, V*> {
        typedef z::dict<K, V*> BaseT;
        inline V& set(const K& k, V& v) {
            V* r = BaseT::set(k, z::ptr(v));
            return z::ref(r);
        }

        inline V& at(const K& k) {
            V* v = BaseT::at(k);
            return z::ref(v);
        }

        inline const V& at(const K& k) const {
            const V* v = BaseT::at(k);
            return z::ref(v);
        }
    };

    /////////////////////////////
    // list helpers
    template <typename V>
    inline const V& at(const list<V>& l, const typename list<V>::size_type& idx) {
        return l.at(idx);
    }

    template <typename V>
    inline V& at(list<V>& l, const typename list<V>::size_type& idx) {
        return l.at(idx);
    }

    template <typename V>
    inline list<V> splice(const list<V>& l, const typename list<V>::size_type& from, const typename list<V>::size_type& to) {
        return l.splice(from, to);
    }

    template <typename V>
    inline typename list<V>::size_type length(const list<V>& l) {
        return l.size();
    }

    /////////////////////////////
    // dict helpers
    template <typename K, typename V>
    inline V& at(dict<K, V>& l, const K& idx) {
        return l.at(idx);
    }

    template <typename K, typename V>
    inline const V& at(const dict<K, V>& l, const K& idx) {
        return l.at(idx);
    }

    template <typename K, typename V>
    inline typename dict<K,V>::size_type length(const dict<K, V>& l) {
        return l.size();
    }

    ///////////////////////////////////////////////////////////////
    struct Future {
        virtual void run() = 0;
    };

    template <typename FunctionT>
    struct FutureT : public Future {
        FutureT(const FunctionT& function, const typename FunctionT::_In& in) : _function(function), _in(in) {}
        FunctionT _function;
        typename FunctionT::_In _in;
        virtual void run() {_function._run(_in);}
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
        typedef InvocationList::size_type size_type;
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

        inline size_type size() const {return _invocationList.size();}

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

        inline FunctionList::size_type size() const {return _list.size();}
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
                handler->_run(in);
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
        static void begin(const z::string& name);
        static void end(const z::string& name, const bool& passed);
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
        inline const _Out& out(const _Out& val) {_out = z::PointerCreator<_Out, _Out>::get(val); return _out.get();}
        virtual const _Out& _run(const _In& _in) {
            unused(_in);
            T& t = static_cast<T&>(ref(this));
            TestResult::begin(t.name());
            const _Out& out = t.test();
            TestResult::end(t.name(), out._passed);
            return out;
        }
    protected:
        inline bool verify(const bool& cond) {
            return cond;
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

    typedef std::list<z::string> ArgList;

    template <typename T>
    struct main_ {
        struct _Out {
            inline _Out(const int& code) : _code(code) {}
            int _code;
        };
    public:
        struct _In {
            inline _In(const ArgList& pargl) : argl(pargl) {}
            ArgList argl;
        };
    public:
        Pointer<_Out> _out;
        inline const _Out& out(_Out* val) {_out = PointerCreator<_Out, _Out>::get(val); return *_out;}
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
}
