#pragma once

#define NULL_PTR_ERROR (false)
#define NULL_REF_ERROR (false)

namespace z {
    template <typename T>
    inline void unused_t(const T&) {}

#if defined(DEBUG)
    inline void assert_t(const bool& cond){assert(cond);}
#else
    inline void assert_t(const bool& /*cond*/){}
#endif

    template <typename T>
    inline T& ref(T* t) {
        // this if-syntax (instead of assert(t)) makes it easier to set breakpoints for debugging.
        if(t == 0) {
            assert(NULL_PTR_ERROR);
        }
        return (*t);
    }

    template <typename T>
    inline T* ptr(T& t) {
        if((&t) == 0) {
            assert(NULL_REF_ERROR);
        }
        return &t;
    }

    template <typename T>
    inline unsigned long pad(T& t) {
        if((&t) == 0) {
            assert(NULL_REF_ERROR);
        }
        return (unsigned long)(&t);
    }

    typedef ::int8_t  int8_t;
    typedef ::int16_t int16_t;
    typedef ::int32_t int32_t;
    typedef ::int64_t int64_t;

    typedef ::uint8_t  uint8_t;
    typedef ::uint16_t uint16_t;
    typedef ::uint32_t uint32_t;
    typedef ::uint64_t uint64_t;

    typedef std::size_t size;

    typedef char     char08_t;
    typedef uint16_t char16_t;
    typedef uint32_t char32_t;

    typedef void void_t;
    typedef bool bool_t;

    typedef float float_t;
    typedef double double_t;

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
        typedef charT scharT;
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

        inline int compare(const stringT& rhs) const {return _val.compare(rhs._val);}
        inline bool operator==(const stringT& rhs) const {return (_val.compare(rhs._val) == 0);}
        inline bool operator!=(const stringT& rhs) const {return (_val.compare(rhs._val) != 0);}
        inline bool operator< (const stringT& rhs) const {return (_val.compare(rhs._val) < 0);}
        inline bool operator> (const stringT& rhs) const {return (_val.compare(rhs._val) > 0);}

        inline charT at(const size_type& idx) const {return _val.at(idx);}
        inline stringT substr(const size_type& from, const size_type& len) const {return _val.substr(from, len);}
        inline stringT substr(const size_type& from) const {return _val.substr(from);}

        inline stringT slice(const size_type& from, const size_type& len) const {return substr(from, len);}

        inline size_type find(const charT& s, const size_type& from) const {return _val.find(s, from);}
        inline size_type find(const charT& s) const {return _val.find(s);}
        inline size_type find(const stringT& s) const {return _val.find(s._val);}
        inline size_type find(const stringT& s, const size_type& from) const {return _val.find(s._val, from);}

        inline size_type rfind(const charT& s) const {return _val.rfind(s);}
        inline size_type rfind(const stringT& s) const {return _val.rfind(s._val);}

        inline stringT replace(const size_type& from, const size_type& len, const stringT& to) {return _val.replace(from, len, to._val);}

        inline void replace(const stringT& search, const stringT& replace) {
            for(typename sstringT::size_type next = _val.find(search._val); next != sstringT::npos;next = _val.find(search._val, next)) {
                _val.replace(next, search.length(), replace._val);
                next += replace.length();
            }
        }

        inline void append08(const char* str) {
            size_t len = ::strlen(str);
            charT* b = (charT*)::malloc((len + 1) * sizeof(charT));
            for(size_t i = 0;i < len; ++i) {
                b[i] = str[i];
            }
            b[len] = 0; // valid because we malloc'ed len+1 size.
            _val.append(b);
            ::free(b);
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
        inline T to(const char& fmt = 0) const;

        template <typename T>
        inline stringT& from(const T& t, const char& fmt = 0);

        inline const sstringT& val() const {return _val;}
        inline const charT* c_str() const {return _val.c_str();}

        explicit inline bstring() {}
        inline bstring(const charT* s) : _val(s) {}
        inline bstring(const charT* s, const size_t& n) : _val(s, n) {}
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
        inline string08(const char* s, const size_t& n) : BaseT(s, n) {}
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

    // define wstring as 16 bit string
    typedef string16 wstring;

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

    /// \brief Function to replace occurences of %{key} with value
    template <typename charT, typename stringT>
    template <typename T>
    inline stringT& z::bstring<charT, stringT>::arg(const stringT& key, const T& value) {
        // first stream the val into a regular std::stringstream
        std::stringstream sval;
        sval << value;

        // then use e2s to convert it to current string type
        stringT sv = e2s(sval.str());

        std::stringstream skey;

        // replace %{X}
        skey << "%{" << s2e(key) << "}";
        stringT sk = e2s(skey.str());
        replace(sk, sv);

        // replace %X
        std::stringstream skey2;
        skey2 << "%" << s2e(key);
        sk = e2s(skey2.str());
        replace(sk, sv);

        return z::ref(static_cast<stringT*>(this));
    }

    /// \brief for converting strings to native types.
    /// In case of integers, converts to C++-style string
    template <typename charT, typename stringT>
    template <typename T>
    inline stringT& z::bstring<charT, stringT>::from(const T& t, const char& fmt) {
        stringT& This = z::ref(static_cast<stringT*>(this));
        z::string prefix;
        std::stringstream ss;
        switch(fmt) {
            case 'x':
                ss << std::hex;
                prefix = "0x";
                break;
            case 'd':
                ss << std::dec;
                break;
            case 'o':
                ss << std::oct;
                prefix = "0";
                break;
        }
        ss << t;
        This = (prefix + z::e2s(ss.str()));
        return This;
    }

    template <typename charT, typename stringT>
    template <typename T>
    inline T z::bstring<charT, stringT>::to(const char& fmt) const {
        const stringT& This = z::ref(static_cast<const stringT*>(this));
        z::estring es = s2e(This);
        std::stringstream ss(es.val());
        switch(fmt) {
            case 'x':
                ss >> std::hex;
                break;
            case 'd':
                ss >> std::dec;
                break;
            case 'o':
                ss >> std::oct;
                break;
        }
        // converting to utf8 and then to expected type.
        /// \todo: find out better way to convert from 16 and 32 bit string to expected type
        //std::basic_stringstream<charT> ss(_val);
        T val;
        ss >> val;
        return val;
    }

    /// \brief Converts integers to zen-style strings ("oxfful")
    template <typename T>
    inline z::string n2s(const T& s, const char& fmt) {
        z::string v = z::string().from(s, fmt);
        /// \todo: add ul, etc postfix
        return v;
    }

    template <typename T>
    inline T s2n(const z::string& s) {
        z::estring es = s2e(s);

        T val = 0;
        char fmt = 'd';
        z::estring::const_iterator it = es.begin();
        z::estring::const_iterator ite = es.end();
        if(*it == '0') {
            ++it;
            if(it == ite)
                return val;
            if(*it == 'x') {
                fmt = 'x';
                ++it;
                if(it == ite)
                    return val;
            } else {
                fmt = 'o';
            }
        }

        for(;it != ite; ++it) {
            const char& c = *it;
            switch(fmt) {
                case 'x':
                    if((c >= '0') && (c <= '9')) {
                        val = (val * 16) + (c - '0');
                    } else if((c >= 'A') && (c <= 'F')) {
                        val = (val * 16) + (c - 'A' + 10);
                    } else if((c >= 'a') && (c <= 'f')) {
                        val = (val * 16) + (c - 'a' + 10);
                    } else {
                        return val;
                    }
                    break;
                case 'd':
                    if((c >= '0') && (c <= '9')) {
                        val = (val * 10) + (c - '0');
                    } else {
                        return val;
                    }
                    break;
                case 'o':
                    if((c >= '0') && (c <= '7')) {
                        val = (val * 8) + (c - '0');
                    } else {
                        return val;
                    }
                    break;
            }
        }

        return val;
    }

    /////////////////////////////
    // bstring helpers
    template <typename charT, typename stringT>
    inline typename z::bstring<charT, stringT>::size_type length(const z::bstring<charT, stringT>& s) {
        return s.size();
    }

    inline int compare(const z::string& lhs, const z::string& rhs) {
        return lhs.compare(rhs);
    }

    ////////////////////////////////////////////////////////////////////////////
    struct datetime {
        inline datetime() : _val(0) {}
        inline datetime(const time_t& val) : _val(val) {}
        inline datetime& operator=(const time_t& val) {_val = val; return z::ref(this);}
        inline const time_t& val() const {return _val;}
    private:
        time_t _val;
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

    ////////////////////////////////////////////////////////////////////////////
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

    void writelog(const z::string& msg);
    inline void writelog(const z::string& src, const z::string& msg) {
        writelog(src + ":" + msg);
    }

    inline void mlog(const z::string& msg)                       {writelog(msg);}
    inline void mlog(const z::string& src, const z::string& msg) {writelog(src, msg);}
    inline void elog(const z::string& msg)                       {writelog(msg);}
    inline void elog(const z::string& src, const z::string& msg) {writelog(src, msg);}

    class Exception /*  : public std::runtime_error */ { /// \todo:
    public:
        explicit inline Exception(const z::string& src, const z::string& msg) : _msg(msg) {
            elog(src, _msg);
            assert(false);
        }
        explicit inline Exception(const z::string& msg) : _msg(msg) {
            elog(_msg);
            assert(false);
        }
    private:
        const z::string _msg;
    };

    ////////////////////////////////////////////////////////////////////////////
    /// \brief simple smart pointer
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

    ////////////////////////////////////////////////////////////////////////////
    struct data : public bstring< uint8_t, data > {
        typedef bstring< uint8_t, data > BaseT;

        explicit inline data() : BaseT() {}
        inline data(const uint8_t* s, const size_t& n) : BaseT(s, n) {}
        inline data(const BaseT::sstringT& s) : BaseT(s) {}
        inline data(const size_type& count, const char_t& ch) : BaseT(count, (uint8_t)ch) {}
        inline data(const data& src) : BaseT(src) {}
    };

    /////////////////////////////
    // data helpers
    /// \brief Convert data to specified type T
    template <typename T>
    inline T map(const data& d, const size_t& from, const size_t& len ) {
        if((from + len) > d.length()) {
            throw Exception("z::map", z::string("%{k} out of bounds\n").arg("k", from));
        }
        const uint8_t* v = d.c_str() + from;
        assert(v != 0);
        const T* t = reinterpret_cast<const T*>(v);
        return z::ref(t);
    }

    /// \brief Converts data to string.
    template <>
    inline z::string map<z::string>(const data& d, const size_t& from, const size_t& len) {
        if((from + len) > d.length()) {
            throw Exception("z::map", z::string("%{k} out of bounds\n").arg("k", from));
        }
        const uint8_t* v = d.c_str() + from;
        assert(v != 0);
        z::estring es((const char*)v, len);
        z::string r = z::e2s(es);
        return r;
    }

    /// \brief Converts any type to a raw data type.
    template <typename T>
    inline z::data raw(const T& t) {
        z::data d((const z::data::scharT*)z::ptr(t), sizeof(t));
        return d;
    }

    /// \brief Represents a type
    /// Thsi struct represents a type as a value.
    struct type {
        explicit inline type(const z::string& name) : _name(name) {}
        inline const z::string& name() const {return _name;}
        inline bool operator==(const type& rhs) const {return (_name == rhs._name);}
        inline bool operator!=(const type& rhs) const {return (_name != rhs._name);}
    private:
        z::string _name;
    };

    /// \brief smart pointer class with typename
    /// This class manages type-casting to/from derived values
    template <typename V>
    struct pointer {
    private:
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

        template <typename DerT>
        struct valueT : public value {
            inline valueT(const DerT& v) : _v(v) {const V& dummy = v;z::unused_t(dummy);}
            virtual V& get() {return _v;}
            virtual value* clone() const {return new valueT<DerT>(_v);}
            virtual z::string tname() const {return type_name<DerT>();}
            template <typename VisT>
            inline void visit(VisT& vis) {vis.visit(_v);}
        private:
            DerT _v;
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

    private:
        type _tname;
        value* _val;

    public:
        template <typename DerT>
        inline DerT& getT() const {
            V& v = get();
            DerT& r = static_cast<DerT&>(v);
            const V& dummy = r; z::unused_t(dummy); // to check that DerT is a derived class of V
            return r;
        }

        inline pointer& operator=(const pointer& src) {
            reset();
            if(src.has()) {
                value* v = src.clone();
                set(src.tname(), v);
            }
            return ref(this);
        }

        template <typename DerT>
        inline pointer& operator=(const pointer<DerT>& src) {
            reset();
            if(src.has()) {
                const DerT& val = src.template getT<DerT>();
                const V& dummy = val; z::unused_t(dummy); // to check that DerT acn be derived from V
                value* v = new valueT<DerT>(val);
                set(src.tname(), v);
            }
            return ref(this);
        }

        template <typename VisT>
        inline void visit(VisT& vis) {
            assert(false);
//            vis.visit(_v);
        }

        /// \brief default-ctor
        /// Required when pointer is used as the value in a dict.
        /// \todo Find out way to avoid it.
        inline pointer() : _tname(""), _val(0) {}

        /// \brief The primary ctor
        /// This ctor is the one to be invoked when creating a pointer-object.
        template <typename DerT>
        explicit inline pointer(const z::string& tname, const DerT& val) : _tname(""), _val(0) {
            const V& dummy = val; z::unused_t(dummy); // to check that DerT is a derived class of V
            value* v = new valueT<DerT>(val);
            set(type(tname), v);
        }

        /// \brief default-cctor
        inline pointer(const pointer& src) : _tname(""), _val(0) { (*this) = src;}

        /// \brief template-cctor
        /// when copying from an object of a different type
        template <typename DerT>
        inline pointer(const pointer<DerT>& src) : _tname(""), _val(0) { (*this) = src;}

        /// \brief default-dtor
        inline ~pointer() {reset();}
    };

    /////////////////////////////////////////////////////////////////
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
    protected:
        List _list;
    };

    /// \brief Base class for all list-like classes
    template <typename V, typename ListT>
    struct listbase : public z::container<V, ListT > {
        typedef z::container<V, ListT > BaseT;

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

        inline V& back() {
            assert(BaseT::_list.size() > 0);
            V& v = BaseT::_list.back();
            return v;
        }

        inline const V& back() const {
            assert(BaseT::_list.size() > 0);
            const V& v = BaseT::_list.back();
            return v;
        }

        inline V& add(const V& v) {
            BaseT::_list.push_back(v);
            return back();
        }

        inline void append(const listbase& src) {
            for(typename BaseT::const_iterator it = src._list.begin(); it != src._list.end(); ++it) {
                const V& iv = *it;
                add(iv);
            }
        }

        inline typename BaseT::iterator last() {return --BaseT::_list.end();}
        inline typename BaseT::iterator erase(typename BaseT::iterator& it) {return BaseT::_list.erase(it);}
    };

    /// \brief Stack class
    template <typename V>
    struct stack : public z::listbase<V, std::list<V> > {
        typedef z::listbase<V, std::list<V> > BaseT;

        inline V& top() {
            assert(BaseT::_list.size() > 0);
            V& v = BaseT::_list.back();
            return v;
        }

        inline const V& top() const {
            assert(BaseT::_list.size() > 0);
            const V& v = BaseT::_list.back();
            return v;
        }

        inline V& push(const V& v) {
            BaseT::_list.push_back(v);
            return top();
        }

        inline V pop() {
            assert(BaseT::_list.size() > 0);
            V v = BaseT::_list.back();
            BaseT::_list.pop_back();
            return v;
        }
    };

    /// \brief Queue class
    template <typename V>
    struct queue : public z::listbase<V, std::list<V> > {
        typedef z::listbase<V, std::list<V> > BaseT;

        inline V& enqueue(const V& v) {
            BaseT::_list.push_back(v);
            return BaseT::_list.back();
        }

        inline V dequeue() {
            assert(BaseT::_list.size() > 0);
            V v = BaseT::_list.front();
            BaseT::_list.pop_front();
            return v;
        }
    };

    /// \brief List class
    template <typename V>
    struct list : public z::listbase<V, std::list<V> > {
        typedef z::listbase<V, std::list<V> > BaseT;

        /// \todo This function is inefficient
        inline V at(const typename BaseT::size_type& k) const {
            if(k >= BaseT::_list.size()) {
                throw Exception("z::list", z::string("%{k} out of list bounds\n").arg("k", k));
            }
            typename BaseT::const_iterator it = BaseT::_list.begin();
            std::advance(it, k);
            return *it;
//            return BaseT::_list.at(k);
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
            typename BaseT::iterator it = BaseT::_list.begin();
            std::advance(it, k);
            BaseT::_list.insert(it, v);
//            BaseT::_list.at(k) = v;
        }

        template <typename CmpFnT>
        inline void sort(CmpFnT fn) {
            std::sort(BaseT::_list.begin(), BaseT::_list.end(), fn);
        }

        inline list<V> slice(const typename BaseT::size_type& from, const typename BaseT::size_type& len) const {
            list<V> nl;
            typename BaseT::const_iterator it = BaseT::_list.begin();
            std::advance(it, from);
            for(typename BaseT::size_type i = 0; i < len; ++i, ++it) {
                const V& v = *it;
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
                    BaseT::erase(it);
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

        inline V& remove(const K& k) {
            typename BaseT::iterator it = BaseT::_list.find(k);
            if(it == BaseT::_list.end()) {
                throw Exception("dict", z::string("%{k} not found\n").arg("k", k));
            }
            V& v = it->second;
            BaseT::_list.erase(it);
            return v;
        }

        inline void erase(typename BaseT::iterator& it) {BaseT::_list.erase(it);}

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
                throw Exception("dict", z::string("key: %{k} not found").arg("k", k));
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

        inline void append(const dict& src) {
            for(typename BaseT::const_iterator it = src._list.begin(); it != src._list.end(); ++it) {
                const K& k = it->first;
                const V& v = it->second;
//                if(BaseT::_list.find(k) == BaseT::_list.end()) {
//                    throw Exception("dict", z::string("Duplicate key: %{k}").arg("k", k));
//                }
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
            if(BaseT::has(k)) {
                V* v = BaseT::at(k);
                delete v;
            }
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
    // slice helpers
    template <typename T>
    inline T slice(const T& t, const int64_t& from, const int64_t& len) {
        int64_t f = from;
        while(f < 0) {
            f = t.size() + f;
        }

        int64_t l = len;
        while(l < 0) {
            l = t.size() + l;
        }

        if(l == 0)
            l = t.size() - 1;

        return t.slice((size_t)f, (size_t)l);
    }

    /////////////////////////////
    // generic helpers
    template <typename T>
    inline void clear(T& l) {
        return l.clear();
    }

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
    inline V& append(list<V>& l, const V& v) {
        return l.add(v);
    }

    template <typename T>
    inline void append(T& l, const T& r) {
        return l.append(r);
    }

    template <typename V>
    inline typename list<V>::size_type length(const list<V>& l) {
        return l.size();
    }

    template <typename V>
    inline V& first(list<V>& l) {
        return *(l.first());
    }

    template <typename V>
    inline V& last(list<V>& l) {
        typename list<V>::iterator it = l.last();
        return *it;
    }

    template <typename V>
    inline V& push(stack<V>& l, const V& v) {
        return l.push(v);
    }

    template <typename V>
    inline V& top(stack<V>& l) {
        return l.top();
    }

    template <typename V>
    inline V pop(stack<V>& l) {
        return l.pop();
    }

    template <typename V>
    inline V& enqueue(queue<V>& l, const V& v) {
        return l.enqueue(v);
    }

    template <typename V>
    inline V& head(queue<V>& l) {
        return l.front();
    }

    template <typename V>
    inline V dequeue(queue<V>& l) {
        return l.dequeue();
    }

    template <typename V>
    inline typename stack<V>::size_type length(const stack<V>& l) {
        return l.size();
    }

    template <typename V>
    inline typename queue<V>::size_type length(const queue<V>& l) {
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

    template <typename K, typename V>
    inline void append(dict<K, V>& l, const dict<K, V>& v) {
        return l.append(v);
    }

    template <typename K, typename V>
    inline const V& remove(dict<K, V>& l, const K& idx) {
        return l.remove(idx);
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
    /// \brief Base class for function and in-param to be enqueued for execution
    struct Future {
        virtual ~Future(){}
        virtual void run() = 0;
    };

    ///////////////////////////////////////////////////////////////
    /// \brief Spcialization of function instance and in-param. Holds out-param after invocation
    template <typename FunctionT>
    struct FutureT : public Future {
        inline FutureT(const z::pointer<FunctionT>& function, const typename FunctionT::_In& in) : _function(function), _in(in) {}
    private:
        /// \brief the function instance
        z::pointer<FunctionT> _function;

        /// \brief the in-params to invoke the function with
        typedef typename FunctionT::_In In;
        In _in;

        /// \brief the out-params after the function was invoked
        typedef typename FunctionT::_Out Out;
        z::autoptr<Out> _out;

        /// \brief the actual invocation
        virtual void run() {Out out = _function.get()._run(_in); _out.reset(new Out(out));}
    };

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Base class for devices.
    /// Devices are async objects that get periodically polled for input, such as files and sockets.
    struct device {
        virtual ~device(){}
        struct _Out {
            inline _Out(const bool& done) : _done(done) {}
            bool _done;
        };
    public:
        struct _In {
            inline _In(const int& ptimeout) : timeout(ptimeout) {}
            int timeout;
        };
    public:
        virtual bool run(const int& timeout) = 0;
        inline _Out _run(const _In& _in) {bool r = run(_in.timeout); return _Out(r);}
    };


    ////////////////////////////////////////////////////////////////////////////
    /// \brief Queue for a single thread
    struct RunQueue;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief thread-local-context
    struct ThreadContext {
        ThreadContext(RunQueue& queue);
        ~ThreadContext();

    public:
        template <typename FunctionT>
        inline FutureT<FunctionT>& addT(const z::pointer<FunctionT>& function, const typename FunctionT::_In& in) {
            FutureT<FunctionT>* inv = new FutureT<FunctionT>(function, in);
            add(inv);
            return ref(inv);
        }

    public:
        z::device& startPoll(z::device* device);
        void stopPoll(z::device& device);

    public:
        z::size wait();

    private:
        void add(z::Future* future);

    private:
        RunQueue& _queue;
    };

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Returns the local context instance
    /// This instance encapsulates the current run-queue
    /// and is unique for every thread. Uses TLS internally.
    ThreadContext& ctx();

    ///////////////////////////////////////////////////////////////
    /// \brief MultiMap of event source to event-handler-list
    template <typename KeyT, typename ValT, typename EventT>
    struct HandlerList {
    private:
        typedef z::list< pointer<ValT> > OList;
        OList _olist;
        typedef z::rlist< pointer<ValT> > List;
        typedef z::dict<KeyT, List> Dict;
        Dict _dict;

    public:
        inline ValT& insertHandler(const KeyT& key, z::pointer<ValT> val) {
            z::pointer<ValT>& vv = _olist.add(val);
            List& list = _dict[key];
            list.add(vv);
            EventT::addHandler(key, vv);
            return vv.get();
        }

        inline void removeHandler() {} /// \todo: later

        inline bool runHandler(const KeyT& key, typename ValT::_In in) {
            typename Dict::const_iterator it = _dict.find(key);
            if(it == _dict.end())
                return false;

            const List& list = it->second;
            for(typename List::const_iterator itl = list.begin(); itl != list.end(); ++itl) {
                const z::pointer<ValT>& handler = z::ref(*itl);
                ctx().addT(handler, in);
            }
            return true;
        }
    };

    #if defined(UNIT_TEST)
    ////////////////////////////////////////////////////////////////////////////
    struct TestResult {
        ~TestResult();
        static void begin(const z::string& name);
        static void end(const z::string& name, const bool& passed);
    };

    ////////////////////////////////////////////////////////////////////////////
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
            z::unused_t(_in);
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

    ////////////////////////////////////////////////////////////////////////////
    struct TestInstance {
        TestInstance();
        virtual void enque(ThreadContext& ctx) = 0;
        TestInstance* _next;
    };

    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct TestInstanceT : public TestInstance {
        virtual void enque(ThreadContext& ctx) {
            T t;
            typename T::_In in;
            ctx.addT(z::pointer<T>("test", t), in);
        }
    };
    #endif

    ////////////////////////////////////////////////////////////////////////////
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

    ////////////////////////////////////////////////////////////////////////////
    struct MainInstance {
        MainInstance();
        virtual void enque(ThreadContext& ctx, const z::stringlist& argl) = 0;
        MainInstance* _next;
    };

    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct MainInstanceT : public MainInstance {
        virtual void enque(ThreadContext& ctx, const z::stringlist& argl) {
            T t;
            typename T::_In in(argl);
            ctx.addT(z::pointer<T>("main", t), in);
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

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Internal global context
    struct GlobalContext;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Represents a single running application instance.
    struct application {
        // application object can be instantiated only from an executable
#if defined(Z_EXE)
    public:
#else
    private:
#endif
        application(int argc, char** argv);
        ~application();

    public:
#if defined(WIN32)
        HINSTANCE instance() const;
#elif defined(IOS)
        void appClass(z::string& cn) const;
#endif
        int exec();
        int exit(const int& code) const;

    public:
        void writeLog(const z::string& msg) const;

    public:
        inline const z::stringlist& argl() const {return _argl;}
        inline const int& argc() const {return _argc;}
        inline char** argv() const {return _argv;}

    public:
        /// \brief Return path to executable
        inline z::string path() const {return _path;}

        /// \brief Return application name
        inline z::string name() const {return _name;}

        /// \brief Return path to data directory
        inline z::string data() const {return _data;}

        /// \brief Return path to base directory
        inline z::string base() const {return _base;}

    public:
        // This application instance can only be accessed through the app() function below.
        // Since that function returns a const-ref, this ctx() function cannot be called by
        // any outside function other than those in zenlang.cpp
        inline z::GlobalContext& ctx() {return _ctx.get();}

    private:
        /// \brief runs the main loop, throws exceptions
        inline int execExx();

        /// \brief Called on exit
        /// This function is implemented in ApplicationImpl.cpp
        void onExit() const;

    private:
        z::autoptr<z::GlobalContext> _ctx;
        z::string _path;
        z::string _name;
        z::string _data;
        z::string _base;
        z::stringlist _argl;
        int _argc;
        char** _argv;
        bool _isExit;
        std::ostream* _log;
    };

    /// \brief Provides read-only access to singleton app-instance
    const z::application& app();

    ////////////////////////////////////////////////////////////////////////////
    /// intrinsic types that represent system objects
    ////////////////////////////////////////////////////////////////////////////
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

    ////////////////////////////////////////////////////////////////////////////
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

    ////////////////////////////////////////////////////////////////////////////
    struct dir {
        static const z::string sep;
        static bool exists(const z::string& path);
        static int mkdir(const z::string& path);

        static z::string cleanPath(const z::string& path);

        /// \brief Makes a path upto the second-last component, unless filename is terminated by a /
        static void mkpath(const z::string& filename);

        static z::string cwd();

        static z::string getPath(const z::string& filename);
        static z::string getFilename(const z::string& filename);
        static z::string getBaseName(const z::string& filename);
        static z::string getExtention(const z::string& filename);
    };

    ////////////////////////////////////////////////////////////////////////////
    struct file {
        inline const z::string& name() const {return _name;}
        inline void close() {
            if(_val == 0) {
                return;
            }
            fclose(_val);
            _val = 0;
        }

        inline file() : _val(0) {}
        inline file(const z::string& name, FILE* val) : _name(name), _val(val) {}
        inline ~file() {close();}
        inline FILE* val() const {return _val;}
    private:
        z::string _name;
        FILE* _val;
    };

    ////////////////////////////////////////////////////////////////////////////
    struct ofile {
        inline operator bool() {return _os.is_open();}
        inline std::ostream& operator()() {return _os;}
        inline const z::string& name() const {return _name;}
        ofile(const z::string& filename);
    private:
        z::string _name;
        std::ofstream _os;
    };

    ////////////////////////////////////////////////////////////////////////////
    struct socket {
        inline socket() : _val(0) {}
        inline socket(const SOCKET& val) : _val(val) {}
        inline const SOCKET& val() const {return _val;}
        inline bool operator<(const socket& rhs) const {return (_val < rhs._val);}
    private:
        SOCKET _val;
    };

#if defined(GUI)
    ////////////////////////////////////////////////////////////////////////////
    struct widget {
        struct impl;
    public:
        inline widget() : _val(0) {}
        inline widget(impl* i) : _val(i) {}
        inline widget(impl& i) : _val(&i) {}
        inline const impl& val() const {return z::ref(_val);}
        void clear() const;
        void set(const z::string& key, const z::widget& v);
        z::widget at(const z::string& key) const;
    public:
    #if defined(WIN32)
        NOTIFYICONDATA& ni() const;
    #endif
        inline bool operator<(const widget& rhs) const {return (_val < rhs._val);}
    private:
        mutable impl* _val;
    };
#endif

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Simple mechanism for tracing function calls.
    struct tracer {
        inline tracer(const z::string& cName, const z::string& fName) : _cName(cName), _fName(fName) {
            z::mlog(_cName, z::string("%{n} enter").arg("n", _fName));
        }

        inline ~tracer() {
            z::mlog(_cName, z::string("%{n} leave").arg("n", _fName));
        }

    private:
        z::string _cName;
        z::string _fName;
    };

}

#define _TRACE(c, f) z::tracer _s_(c, f)
#define DISABLE_ASSIGNMENT(c) private: inline c& operator=(const c& /*src*/){throw z::Exception("", z::string(#c));}
#define DISABLE_COPYCTOR(c) private: inline c(const c& /*src*/){throw z::Exception("", z::string(#c));}
#define UNIMPL() assert(false)
