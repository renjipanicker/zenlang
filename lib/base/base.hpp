#pragma once

#define unused(x) ((void)(&x))

#if defined(DEBUG)
    #if defined(GUI) && defined(WIN32)
        void trace(const char* txt, ...);
    #else
        #define trace printf
    #endif
#else
inline void trace(const char* txt, ...) {unused(txt);} // empty inline function gets optimized away
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

    typedef std::size_t size;

    typedef char     char08_t;
    typedef uint16_t char16_t;
    typedef uint32_t char32_t;

    // define char_t as 8, 16 or 32 bit
#if defined(CHAR_WIDTH_08)
    typedef char08_t char_t;
#elif defined(CHAR_WIDTH_16)
    typedef char16_t char_t;
#elif defined(CHAR_WIDTH_32)
    typedef char32_t char_t;
#else
#error CHAR_WIDTH not defined
#endif

    ////////////////////////////////////////////////////////////////////////////
    // base class for all string types
    template <typename charT, typename stringT>
    struct bstring {
        typedef std::basic_string<charT> sstringT;

        typedef typename sstringT::size_type size_type;
#if defined(WIN32)
        static const size_type npos  = (size_type)(-1);
#else
        static const size_type npos  = sstringT::npos;
#endif

        typedef typename sstringT::iterator iterator;
        inline iterator begin() {return _val.begin();}
        inline iterator end() {return _val.end();}

        typedef typename sstringT::const_iterator const_iterator;
        inline const_iterator begin() const {return _val.begin();}
        inline const_iterator end() const {return _val.end();}

        inline size_type size() const {return _val.size();}
        inline size_type length() const {return _val.length();}

        inline bool operator==(const stringT& rhs) const {return (_val.compare(rhs._val) == 0);}
        inline bool operator!=(const stringT& rhs) const {return (_val.compare(rhs._val) != 0);}
        inline bool operator< (const stringT& rhs) const {return (_val.compare(rhs._val) < 0);}
        inline bool operator> (const stringT& rhs) const {return (_val.compare(rhs._val) > 0);}

        inline charT at(const size_type& idx) const {return _val.at(idx);}
        inline stringT substr(const size_type& from, const size_type& len) const {return _val.substr(from, len);}
        inline stringT substr(const size_type& from) const {return _val.substr(from);}

        inline size_type find(const charT& s) const {return _val.find(s);}
        inline size_type find(const stringT& s) const {return _val.find(s._val);}
        inline size_type find(const stringT& s, const size_type& from) const {return _val.find(s._val, from);}
        inline size_type rfind(const charT& s) const {return _val.rfind(s);}
        inline size_type rfind(const stringT& s) const {return _val.rfind(s._val);}
        inline stringT   replace(const size_type& from, const size_type& len, const stringT& to) {return _val.replace(from, len, to._val);}

        inline void replace(const stringT& search, const stringT& replace) {
            for(typename sstringT::size_type next = _val.find(search._val); next != sstringT::npos;next = _val.find(search._val, next)) {
                _val.replace(next, search.length(), replace._val);
                next += replace.length();
            }
        }

        inline void append08(const char* str) {
            for(const char* c = str; *c != 0; ++c) {
                _val.push_back((z::char_t)(*c));
            }
        }

        inline void append(const charT& rhs) {_val += rhs;}
        inline void append(const stringT& rhs) {_val.append(rhs._val);}
        inline void clear() {_val.clear();}

        inline stringT& operator= (const charT& rhs) {_val = rhs; return static_cast<stringT&>(*this);}
        inline stringT& operator= (const charT* rhs) {_val = rhs; return static_cast<stringT&>(*this);}
        inline stringT& operator= (const stringT& rhs) {_val = rhs._val; return static_cast<stringT&>(*this);}
        inline stringT& operator= (const sstringT& rhs) {_val = rhs; return static_cast<stringT&>(*this);}
        inline stringT& operator+=(const stringT& rhs) {append(rhs); return static_cast<stringT&>(*this);}
        inline stringT& operator+=(const charT& rhs) {append(rhs); return static_cast<stringT&>(*this);}

        inline charT operator[](const size_type& idx) const {return _val[idx];}

        template <typename T>
        inline stringT& arg(const stringT& key, const T& value);

        template <typename T>
        inline T to() const;

        inline const sstringT& val() const {return _val;}
        inline const charT* c_str() const {return _val.c_str();}

        explicit inline bstring() {}
        inline bstring(const charT* s) : _val(s) {}
        inline bstring(const sstringT& s) : _val(s) {}
        inline bstring(const bstring& src) : _val(src._val) {}
        inline bstring(const size_type& count, const charT& ch) : _val(count, ch) {}
    protected:
        sstringT _val;
    };

    // utf8 string
    struct string08 : public bstring<char08_t, string08 > {
        typedef bstring<char08_t, string08 > BaseT;

        explicit inline string08() : BaseT() {}
        inline string08(const char* s) : BaseT() {append08(s);}
        inline string08(const BaseT::sstringT& s) : BaseT(s) {}
        inline string08(const size_type& count, const char_t& ch) : BaseT(count, (char08_t)ch) {}
        inline string08(const string08& src) : BaseT(src) {}
    };

    // 16 bit unicode string
    struct string16 : public bstring<char16_t, string16 > {
        typedef bstring<char16_t, string16 > BaseT;

        explicit inline string16() : BaseT() {}
        inline string16(const char* s) : BaseT() {append08(s);}
        inline string16(const char16_t* s) : BaseT(s) {}
        inline string16(const BaseT::sstringT& s) : BaseT(s) {}
        inline string16(const size_type& count, const char_t& ch) : BaseT(count, (char08_t)ch) {}
        inline string16(const string16& src) : BaseT(src) {}
    };

    // 32 bit unicode string
    struct string32 : public bstring<char32_t, string32 > {
        typedef bstring<char32_t, string32 > BaseT;

        explicit inline string32() : BaseT() {}
        inline string32(const char* s) : BaseT() {append08(s);}
        inline string32(const char32_t* s) : BaseT(s) {}
        inline string32(const BaseT::sstringT& s) : BaseT(s) {}
        inline string32(const size_type& count, const char_t& ch) : BaseT(count, (char08_t)ch) {}
        inline string32(const string32& src) : BaseT(src) {}
    };

    // define estring (encoded-string) as 8bit utf8 string
    typedef string08 estring;

    // define ustring as 32 bit string
    typedef string32 ustring;

    // define string as one of 8, 16 or 32 bit string
#if defined(CHAR_WIDTH_08)
    typedef string08 string;
#elif defined(CHAR_WIDTH_16)
    typedef string16 string;
#elif defined(CHAR_WIDTH_32)
    typedef string32 string;
#endif

    // convert between utf8 and 32 bit strings
    z::string08 c32to08(const z::string32& in);
    z::string32 c08to32(const z::string08& in);

    // convert between utf8 and 16 bit strings
    z::string08 c16to08(const z::string16& in);
    z::string16 c08to16(const z::string08& in);

    // convert between 32 bit and 16 bit strings (implemented as direct copy operations)
    z::string32 c16to32(const z::string16& in);
    z::string16 c32to16(const z::string32& in);

    // conversion from string to estring
    inline z::estring s2e(const z::string& in) {
#if defined(CHAR_WIDTH_08)
        return in;
#elif defined(CHAR_WIDTH_16)
        return c16to08(in);
#elif defined(CHAR_WIDTH_32)
        return c32to08(in);
#endif
    }

    // conversion from estring to string
    inline z::string e2s(const z::estring& in) {
#if defined(CHAR_WIDTH_08)
        return in;
#elif defined(CHAR_WIDTH_16)
        return c08to16(in);
#elif defined(CHAR_WIDTH_32)
        return c08to32(in);
#endif
    }

    // conversion from string to ustring
    inline z::ustring s2u(const z::string& in) {
#if defined(CHAR_WIDTH_08)
        return c08to32(in);
#elif defined(CHAR_WIDTH_16)
        return c16to32(in);
#elif defined(CHAR_WIDTH_32)
        return in;
#endif
    }

    // conversion from ustring to string
    inline z::string u2s(const z::ustring& in) {
#if defined(CHAR_WIDTH_08)
        return c32to08(in);
#elif defined(CHAR_WIDTH_16)
        return c32to16(in);
#elif defined(CHAR_WIDTH_32)
        return in;
#endif
    }

    typedef std::basic_ostream<char_t> ostream;

    inline std::ostream& operator<<(std::ostream& os, const z::estring& val) {
        os << val.val();
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const z::string& val) {
        os << z::s2e(val).val();
        return os;
    }

    inline z::string operator+(const z::char_t* lhs, const z::string& rhs) {return lhs + rhs.val();}
    inline z::string operator+(const z::string& lhs, const z::char_t* rhs) {return (lhs.val() + rhs);}
    inline z::string operator+(const z::string& lhs, const z::char_t rhs) {return (lhs.val() + rhs);}
    inline z::string operator+(const z::string& lhs, const z::string& rhs) {return (lhs.val() + rhs.val());}

    #if !defined(CHAR_WIDTH_08)
    inline z::string operator+(const z::string& lhs, const char* rhs) {
        z::string rv = lhs;
        rv.append08(rhs);
        return rv;
    }
    #endif

    template <typename charT, typename stringT>
    template <typename T>
    inline stringT& z::bstring<charT, stringT>::arg(const stringT& key, const T& value) {
        // first stream the key:val pair into a regular std::stringstream
        std::stringstream skey;
        skey << "%{" << s2e(key) << "}";
        std::stringstream sval;
        sval << value;

        // then use e2s to convert it to current string width
        stringT sk = e2s(skey.str());
        stringT sv = e2s(sval.str());

        // and replace
        replace(sk, sv);
        return z::ref(static_cast<stringT*>(this));
    }

    template <typename charT, typename stringT>
    template <typename T>
    inline T z::bstring<charT, stringT>::to() const {
        z::estring es = s2e(z::ref(static_cast<const stringT*>(this)));
        std::stringstream ss(es.val());
        // converting to utf8 and then to expected type.
        /// \todo: find out better way to convert from 16 and 32 bit string to expected type
        //std::basic_stringstream<charT> ss(_val);
        T val;
        ss >> val;
        return val;
    }

    ////////////////////////////////////////////////////////////////////////////
    struct datetime {
        inline datetime() : _val(0) {}
        inline datetime(const int64_t& val) : _val(val) {}
        inline datetime& operator=(const int64_t& val) {_val = val; return z::ref(this);}
        inline const int64_t& val() const {return _val;}
    private:
        int64_t _val;
    };

    struct regex {
        void compile(const z::string& re);
        void match(const z::string& str);
        inline regex() {}
        inline regex(const z::string& re) {compile(re);}

    private:
#if !defined(WIN32)
        regex_t _val;
#endif
    };

    struct file {
        static const z::string sep;
        static bool exists(const z::string& path);
        static int mkdir(const z::string& path);

        /// makes a path upto the second-last component, unless filename is terminated by a /
        static void mkpath(const z::string& filename);

        static z::string cwd();

        static z::string getPath(const z::string& filename);
        static z::string getFilename(const z::string& filename);
        static z::string getBaseName(const z::string& filename);
        static z::string getExtention(const z::string& filename);
    };

    struct ofile {
        inline operator bool() {return _os.is_open();}
        inline std::ostream& operator()() {return _os;}
        inline const z::string& name() const {return _name;}
        ofile(const z::string& filename);
    private:
        z::string _name;
        std::ofstream _os;
    };

    /// \brief Return typename of T as string
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

    /// \brief Return typename of t as string
    template <typename T>
    inline z::string type_name(const T& t) {
        unused(t);
        return type_name<T>();
    }

    inline void writelog(const z::string& src, const z::string& msg) {
        z::string s;
        if(src.length() > 0) {
            s += src;
            s += " : ";
        }
        s += msg;
        std::cout << s << std::endl;
#if defined(GUI) && defined(WIN32)
        trace("%s\n", z::s2e(s).c_str());
#endif
    }

    inline void mlog(const z::string& src, const z::string& msg) {writelog(src, msg);}
    inline void elog(const z::string& src, const z::string& msg) {writelog(src, msg);}

    class Exception {
    public:
        explicit inline Exception(const z::string& src, const z::string& msg) : _msg(msg) {elog(src, _msg); assert(false);}
    private:
        const z::string _msg;
    };

    template <typename T>
    struct autoptr {
        inline autoptr() : _val(0) {}
        inline autoptr(T* val) : _val(val) {}
        inline ~autoptr() {
            delete _val;
            _val = 0;
        }
        inline bool empty() const {return (_val == 0);}
        inline T* ptr() {return _val;}
        inline T& get() {assert(!empty()); return z::ref(_val);}
        inline T* operator->() {assert(!empty()); return ptr();}
        inline T& operator*() {return get();}
        inline T* take() {assert(!empty()); T* v = _val; _val = 0; return v;}
        inline void reset(T* val) {delete _val; _val = val;}
        inline void reset() {reset(0);}
    private:
        inline autoptr(const autoptr& /*src*/) : _val(0) {}
    private:
        T* _val;
    };

    struct mutex {
        mutex();
        ~mutex();
        int enter();
        int leave();
    private:
        inline mutex(const mutex& /*src*/) {}
    private:
        mutex_t _val;
    };

    struct mlock {
        inline mlock(mutex& m) : _mutex(m) {
            _mutex.enter();
        }

        inline ~mlock() {
            _mutex.leave();
        }

    private:
        inline mlock(const mlock& src) : _mutex(src._mutex) {}
    private:
        mutex& _mutex;
    };

    struct type {
        explicit inline type(const z::string& name) : _name(name) {}
        inline const z::string& name() const {return _name;}
        inline bool operator==(const type& rhs) const {return (_name == rhs._name);}
        inline bool operator!=(const type& rhs) const {return (_name != rhs._name);}
    private:
        z::string _name;
    };

    /// \brief Base class for pointer class
    /// This class holds the base value pointer
    template <typename V>
    struct bpointer {
    protected:
        struct value {
        protected:
            inline value(){}
        public:
            virtual ~value(){}
        public:
            virtual V& get() = 0;
            virtual value* clone() const = 0;
            virtual z::string tname() const = 0;
        };

        inline void reset() {
            _tname = type("");
            delete _val;
            _val = 0;
        }

        inline void set(const type& tname, value* val) {
            _tname = tname;
            _val = val;
        }

    public:
        inline bool has() const {return (_val != 0);}
        inline value* clone() const {return ref(_val).clone();}
        inline const type& tname() const {return _tname;}

        inline V& get() const {
            V& v = ref(_val).get();
            return (v);
        }

        inline bpointer() : _tname(""), _val(0) {}
        inline ~bpointer() {reset();}

    protected:
        type _tname;
        value* _val;
    };

    /// \brief smart pointer class with typename
    /// This class manages type-casting to/from derived values
    template <typename V>
    struct pointer : public bpointer<V> {
        typedef bpointer<V> BaseT;
        typedef typename bpointer<V>::value value;

        template <typename DerT>
        struct valueT : public value {
            inline valueT(const DerT& v) : _v(v) {const V& dummy = v;unused(dummy);}
            virtual V& get() {return _v;}
            virtual value* clone() const {return new valueT<DerT>(_v);}
            virtual z::string tname() const {return type_name<DerT>();}
        private:
            DerT _v;
        };

    public:
        template <typename DerT>
        inline DerT& getT() const {
            V& v = BaseT::get();
            DerT& r = static_cast<DerT&>(v);
            const V& dummy = r; unused(dummy); // to check that DerT is a derived class of V
            return r;
        }

        inline pointer& operator=(const pointer& src) {
            BaseT::reset();
            if(src.has()) {
                value* v = src.clone();
                BaseT::set(src.tname(), v);
            }
            return ref(this);
        }

        template <typename DerT>
        inline pointer& operator=(const pointer<DerT>& src) {
            BaseT::reset();
            if(src.has()) {
                const DerT& val = src.getT<DerT>();
                const V& dummy = val; unused(dummy); // to check that DerT acn be derived from V
                value* v = new valueT<DerT>(val);
                BaseT::set(src.tname(), v);
            }
            return ref(this);
        }

        /// \brief default-ctor
        /// Required when pointer is used as the value in a dict.
        /// \todo Find out way to avoid it.
        inline pointer() {}

        /// \brief The primary ctor
        /// This ctor is the one to be invoked when creating a pointer-object.
        template <typename DerT>
        explicit inline pointer(const z::string& tname, const DerT& val) {
            const V& dummy = val; unused(dummy); // to check that DerT is a derived class of V
            value* v = new valueT<DerT>(val);
            BaseT::set(type(tname), v);
        }

        inline pointer(const pointer& src) { (*this) = src;}

        template <typename DerT>
        inline pointer(const pointer<DerT>& src) { (*this) = src;}
    };

    /// \brief Base class for all container classes
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

    /// \brief Base class for all list-like classes
    template <typename V, typename ListT>
    struct listbase : public z::container<V, ListT > {
        typedef z::container<V, ListT > BaseT;
        inline V& back() {return BaseT::_list.back();}
        inline typename BaseT::iterator last() {return --BaseT::_list.end();}
    };

    /// \brief Stack class
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

    /// \brief Queue class
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

    /// \brief List class
    template <typename V>
    struct list : public z::listbase<V, std::vector<V> > {
        typedef z::listbase<V, std::vector<V> > BaseT;

        inline V at(const typename BaseT::size_type& k) const {
            if(k >= BaseT::_list.size()) {
                throw Exception("z::list", z::string("%{k} out of list bounds\n").arg("k", k));
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

        inline void remove(V* v) {
            for(typename BaseT::iterator it = BaseT::_list.begin(); it != BaseT::_list.end(); ++it) {
                V* vv = *it;
                if(vv == v) {
                    delete vv;
                    BaseT::_list.erase(it);
                    return;
                }
            }
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
                throw Exception("dict", z::string("%{k} not found\n").arg("k", k));
            }
            return it->second;
        }

        inline const V& at(const K& k) const {
            typename BaseT::const_iterator it = BaseT::_list.find(k);
            if(it == BaseT::_list.end()) {
                throw Exception("dict", z::string("%{k} not found\n").arg("k", k));
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

    /////////////////////////////
    typedef z::list<z::string> stringlist;

    ///////////////////////////////////////////////////////////////
    /// \brief list of items that get initialized via static ctors
    /// This includes main() and test() functions
    template <typename InitT>
    struct InitList {
        inline InitList() {}

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
        static InitT* _head;
        static InitT* _tail;
        static InitT* _next;
    };

    ///////////////////////////////////////////////////////////////
    /// \brief MultiMap of event source to event-handler-list
    /// This is a helper function for GUI classes
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

    ///////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////
    /// \brief List of event-handlers
    template <typename FunctionT>
    struct FunctorList {
        inline FunctionT& add(FunctionT* h) {
            _list.add(h);
            return ref(h);
        }

    private:
        typedef z::olist<FunctionT> List;
        List _list;
    };

    ///////////////////////////////////////////////////////////////
    /// \brief Base class for function and in-param to be enqueued for execution
    struct Future {
        virtual ~Future(){}
        virtual void run() = 0;
    };

    ///////////////////////////////////////////////////////////////
    /// \brief Spcialization of function instance and in-param. Holds out-param after invocation
    template <typename FunctionT>
    struct FutureT : public Future {
        inline FutureT(const FunctionT& function, const typename FunctionT::_In& in) : _function(function), _in(in) {}
    private:
        /// \brief the function instance
        FunctionT _function;

        /// \brief the in-params to invoke the function with
        typedef typename FunctionT::_In In;
        In _in;

        /// \brief the out-params after the function was invoked
        typedef typename FunctionT::_Out Out;
        z::autoptr<Out> _out;

        /// \brief the actual invocation
        virtual void run() {Out out = _function._run(_in); _out.reset(new Out(out));}
    };

    /// \brief Base class for devices.
    /// Devices are async objects that get periodically polled for input, such as files and sockets.
    struct Device {
        virtual void poll(const z::size& timeout) = 0;
    };

    /// \brief Queue for a single thread
    struct RunQueue;

    /// \brief thread-local-context
    struct ThreadContext {
        ThreadContext(RunQueue& queue);
        ~ThreadContext();

    public:
        template <typename FunctionT>
        inline FutureT<FunctionT>& add(FunctionT function, const typename FunctionT::_In& in) {
            FutureT<FunctionT>* inv = new FutureT<FunctionT>(function, in);
            add(inv);
            return ref(inv);
        }

    public:
        z::Device& start(z::Device& device);
        z::Device& stop(z::Device& device);

    public:
        z::size wait();

    private:
        void add(z::Future* future);

    private:
        RunQueue& _queue;
    };

    /// \brief Returns the local context instance
    /// This instance encapsulates the current run-queue
    /// and is unique for every thread. Uses TLS internally.
    ThreadContext& ctx();

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
        inline _Out _run(const _In& _in) {
            unused(_in);
            T& t = static_cast<T&>(ref(this));
            TestResult::begin(t.name());
            _Out out = t.run();
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
        virtual void enque(ThreadContext& ctx) = 0;
        TestInstance* _next;
    };

    template <typename T>
    struct TestInstanceT : public TestInstance {
        virtual void enque(ThreadContext& ctx) {
            T t;
            typename T::_In in;
            ctx.add(t, in);
        }
    };
    #endif

    template <typename T>
    struct main_ {
        struct _Out {
            inline _Out(const int& code) : _code(code) {}
            int _code;
        };
    public:
        struct _In {
            inline _In(const z::stringlist& pargl) : argl(pargl) {}
            z::stringlist argl;
        };
    public:
        inline _Out _run(const _In& _in) {
            T& t = static_cast<T&>(ref(this));
            _Out out = t.run(_in.argl);
            return out;
        }
    };

    struct MainInstance {
        MainInstance();
        virtual void enque(ThreadContext& ctx, const z::stringlist& argl) = 0;
        MainInstance* _next;
    };

    template <typename T>
    struct MainInstanceT : public MainInstance {
        virtual void enque(ThreadContext& ctx, const z::stringlist& argl) {
            T t;
            typename T::_In in(argl);
            ctx.add(t, in);
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

    /// \brief Internal global context
    struct GlobalContext;

    /// \brief Represents a single running application instance.
    struct Application {
        // Application object can be instantiated only from an executable
#if defined(Z_EXE)
    public:
#else
    private:
#endif
        Application(int argc, const char** argv);
        ~Application();

    public:
#if defined(WIN32)
        static HINSTANCE instance();
#endif
        int exec();
        int exit(const int& code) const;

    public:
        inline const z::stringlist& argl() const {return _argl;}
        inline const int& argc() const {return _argc;}
        inline const char** argv() const {return _argv;}

    public:
        // This Application instance can only be accessed through the app() function below.
        // Since that function returns a const-ref, this ctx() function cannot be called by
        // any outside function other than those in zenlang.cpp
        inline z::GlobalContext& ctx() {return _ctx.get();}

    private:
        /// \brief runs the main loop, throws exceptions
        inline int execEx();

        /// \brief Called on exit
        /// This function is implemented in ApplicationImpl.cpp
        void onExit();

    private:
        z::autoptr<z::GlobalContext> _ctx;
        z::stringlist _argl;
        int _argc;
        const char** _argv;
        bool _isExit;
    };

    /// \brief Provides read-only access to singleton app-instance
    const z::Application& app();

    /// \brief Simple mechanism for tracing function calls.
    struct Tracer {
        inline Tracer(const z::string& cName, const z::string& fName) : _cName(cName), _fName(fName) {
            z::mlog(_cName, z::string("%{n} enter").arg("n", _fName));
        }

        inline ~Tracer() {
            z::mlog(_cName, z::string("%{n} leave").arg("n", _fName));
        }

    private:
        z::string _cName;
        z::string _fName;
    };

}

#define _TRACE(c, f) z::Tracer _s_(c, f)
#define DISABLE_ASSIGNMENT(c) private: inline c& operator=(const c& /*src*/){throw z::Exception("", z::string(#c));}
#define DISABLE_COPYCTOR(c) private: inline c(const c& /*src*/){throw z::Exception("", z::string(#c));}
