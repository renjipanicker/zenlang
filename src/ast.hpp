#pragma once
#include "token.hpp"

namespace Ast {
    /// \brief Outer struct for AccessType enumeration.
    struct AccessType {
        /// \brief The access type for any user-defined TypeSpec.
        enum T {
            Private,     /// TypeSpec is visible only within current compilation unit (default)
            Public,      /// TypeSpec is visible outside current compilation unit
            Internal,    /// TypeSpec is visible anywhere within current module
            External,    /// TypeSpec is visible outside current module (dllexport)
            Parent       /// TypeSpec inherits the access type of its parent
        };
    };

    /// \brief Outer struct for DefinitionType enumeration.
    struct DefinitionType {
        /// \brief The Definition type for any user-defined TypeSpec.
        enum T {
            Native,     /// TypeSpec is implemented in native code
            Abstract,   /// TypeSpec is an abstract type (struct and function)
            Final       /// TypeSpec cannot be overridden and is implemented in zenlang code
        };
    };

    /// \brief Outer struct for HeaderType enumeration.
    struct HeaderType {
        /// \brief The type for any header file.
        enum T {
            Import,     /// import system file
            Include     /// include application file
        };
    };

    /// \brief Outer struct for ScopeType enumeration.
    struct ScopeType {
        /// \brief The type for any scope
        enum T {
            Member,      /// Member of enum or struct
            XRef,        /// XRef scope
            Param,       /// Param scope
            VarArg,      /// VarArg in-param
            Local        /// local scope
        };
    };

    /// \brief Outer struct for RefType enumeration.
    struct RefType {
        /// \brief The type for any scope
        enum T {
            Global,      /// Variable is in global scope (unused for now)
            XRef,        /// Variable is from outside current function
            Param,       /// Reference to current function parameter
            Local        /// variable is in local scope
        };
    };

    //////////////////////////////////////////////////////////////////
    template <typename T>
    struct Ptr {
        inline void dec() {
            if(_value) {
                typename T::RefCnt_t rc = z::ref(_value).dec();
//                z::ref(_value).dump("P::dec", "-");
                if(rc == 0) {
//                    z::ref(_value).dump("P::del", " ");
                    delete _value;
                }
                _value = 0;
            }
        }

        inline void inc(T& val) {
            _value = z::ptr(val);
            z::ref(_value).inc();
//            z::ref(_value).dump("P::inc", "+");
        }

        inline T& get() const {
            return z::ref(_value);
        }

        template <typename X> inline bool check() const {
            return (dynamic_cast<const X*>(_value) != 0);
        }

        template <typename X> inline const X& getT() const {
            return z::ref(dynamic_cast<const X*>(_value));
        }

        inline void reset(T& val) {
            if(z::ptr(val) != _value) {
                dec();
                inc(val);
            }
        }

        inline bool empty() const {return (_value == 0);}
        inline Ptr() : _value(0) {
        }

        inline Ptr(T& value) : _value(0) {
            inc(value);
        }

        inline ~Ptr() {
            dec();
        }
        inline Ptr(const Ptr& src) : _value(0) {
            if(src._value)
                inc(z::ref(src._value));
        }
    private:
        T* _value;
    };

    //////////////////////////////////////////////////////////////////
    template <typename T, typename ListT, typename ItemT>
    struct Lst {
        typedef ListT List;
        typedef typename List::size_type size_type;
        typedef typename List::const_iterator const_iterator;
        typedef typename List::const_reverse_iterator const_reverse_iterator;
        typedef typename List::reverse_iterator reverse_iterator;

        inline const_iterator begin() const {return _list.begin();}
        inline const_iterator end() const {return _list.end();}
        inline const_reverse_iterator rbegin() const {return _list.rbegin();}
        inline const_reverse_iterator rend() const {return _list.rend();}
        inline reverse_iterator rbegin() {return _list.rbegin();}
        inline reverse_iterator rend() {return _list.rend();}
        inline T& front() const {return _list.front().get();}
        inline T& at(const size_type& idx) const {assert(idx < _list.size()); return _list.at(idx).get();}
        inline size_type size() const {return _list.size();}
        inline T& top() const {assert(_list.size() > 0); return _list.back().get();}

        inline ItemT& add(T& val) {
            _list.push_back(ItemT(val));
            return _list.back();
        }
        inline void push(T& val) {
            _list.push_back(ItemT(val));
        }
        inline void pop() {
            _list.pop_back();
        }
    protected:
        List _list;
    };

    //////////////////////////////////////////////////////////////////
    template <typename T>
    struct SLst : public Lst<T, std::vector< Ptr<T> >, Ptr<T> > {
    };

    //////////////////////////////////////////////////////////////////
    template <typename T>
    struct WLst : public Lst<T, std::vector< T* >, T* > {
    };

    //////////////////////////////////////////////////////////////////
    class Token {
    public:
        inline Token(const z::string& filename, const int row, const int col, const z::string& text) : _filename(filename), _row(row), _col(col), _text(text) {}
        inline Token(TokenData& td) : _filename(td.filename()), _row(td.row()), _col(td.col()), _text(td.text()) {TokenData::deleteT(td);}
        inline const z::string& filename() const {return _filename;}
        inline const int& row() const {return _row;}
        inline const int& col() const {return _col;}
        inline const char* text() const {return _text.c_str();}
        inline const z::string& string() const {return _text;}
    private:
        const z::string _filename;
        const int _row;
        const int _col;
        const z::string _text;
    };

    //////////////////////////////////////////////////////////////////
    class Node {
    public:
        typedef size_t RefCnt_t;
    public:
        inline const Token& pos() const {return _pos;}
        inline void dump(const z::string& src, const z::string& act) const {
            trace("%lu %s refCount%s %lu, ", (unsigned long)this, src.c_str(), act.c_str(), _refCount);
            fflush(stdout);
            z::string x = z::type_name(*this);
            trace("<%s>\n", x.c_str());
            fflush(stdout);
        }

        inline void inc() const {
            ++_refCount;
//            dump("N::inc", "+");
        }

        inline RefCnt_t dec() const {
            --_refCount;
//            dump("N::dec", "-");
            return _refCount;
        }

        inline const RefCnt_t& refCount() const {return _refCount;}

    protected:
        inline Node(const Token& pos) : _pos(pos), _refCount(0) {
//            dump("N::ctr", "*");
        }

        virtual ~Node() {
//            dump("N::dtr", "~");
            assert(_refCount == 0);
        }

    private:
        inline Node(const Node& src) : _pos(src._pos), _refCount(0) {}
    private:
        const Token _pos;
        mutable RefCnt_t _refCount;
    };

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    class ChildTypeSpec;
    class TypeSpec : public Node {
    public:
        struct Visitor;
    private:
        typedef SLst<ChildTypeSpec> ChildTypeSpecList;
        typedef std::map<z::string, ChildTypeSpec* > ChildTypeSpecMap;
    public:
        inline TypeSpec(const Token& name) : Node(name), _accessType(AccessType::Private), _name(name) {}
    public:
        inline TypeSpec& accessType(const AccessType::T& val) {_accessType = val; return z::ref(this);}
        inline const Token& name() const {return _name;}
        inline const AccessType::T& accessType() const {return _accessType;}
        inline const ChildTypeSpecList& childTypeSpecList() const {return _childTypeSpecList;}
        inline ChildTypeSpecList::size_type childCount() const {return _childTypeSpecList.size();}
    public:
        template <typename T>
        inline void addChild(T& typeSpec) {\
            /// When a routine/function is pre-declared, it already exists in this map. Hence we cannot
            /// check for its absence before adding it. \todo Find out way to ensure that we are not creating
            /// a different type.
            //ChildTypeSpecMap::const_iterator it = _childTypeSpecMap.find(typeSpec.name().string());
            //assert(it == _childTypeSpecMap.end());
            assert(z::ptr(typeSpec.parent()) == this);

            Ptr<ChildTypeSpec>& sptr = _childTypeSpecList.add(typeSpec);
            _childTypeSpecMap[typeSpec.name().text()] = z::ptr(sptr.get());
        }

        template <typename T>
        inline T* hasChild(const z::string& name) const {
            ChildTypeSpecMap::const_iterator it = _childTypeSpecMap.find(name);
            if(it == _childTypeSpecMap.end()) {
                return 0;
            }
            return dynamic_cast<T*>(it->second);
        }

    public:
        virtual void visit(Visitor& visitor) const = 0;
    private:
        AccessType::T _accessType;
        const Token _name;
    private:
        ChildTypeSpecList _childTypeSpecList;
        ChildTypeSpecMap _childTypeSpecMap;
    };

    class QualifiedTypeSpec : public Node {
    public:
        inline QualifiedTypeSpec(const Token& pos, const bool& isConst, const TypeSpec& typeSpec, const bool& isRef)
            : Node(pos), _isConst(isConst), _typeSpec(typeSpec), _isRef(isRef) {}
        inline const bool& isConst() const {return _isConst;}
        inline const TypeSpec& typeSpec() const {return _typeSpec;}
        inline const bool& isRef() const {return _isRef;}
    private:
        const bool _isConst;
        const TypeSpec& _typeSpec;
        const bool _isRef;
    };

    class Expr;
    class VariableDefn : public Node {
    public:
        inline VariableDefn(const QualifiedTypeSpec& qualifiedTypeSpec, const Token& name, const Ast::Expr& initExpr) : Node(name), _qualifiedTypeSpec(qualifiedTypeSpec), _name(name), _initExpr(initExpr) {}
        inline const QualifiedTypeSpec& qTypeSpec() const {return _qualifiedTypeSpec.get();}
        inline const Token& name() const {return _name;}
        inline const Expr& initExpr() const {return _initExpr.get();}
    private:
        const Ptr<const QualifiedTypeSpec> _qualifiedTypeSpec;
        const Token _name;
        const Ptr<const Ast::Expr> _initExpr;
    };

    class Scope : public Node {
    public:
        typedef SLst<const VariableDefn> List;
    public:
        inline Scope(const Token& pos, const ScopeType::T& type) : Node(pos), _type(type), _isTuple(true) {}
        inline Scope& addVariableDef(const VariableDefn& variableDef) {_list.add(variableDef); return z::ref(this);}
        inline const ScopeType::T& type() const {return _type;}
        inline const List& list() const {return _list;}
        inline void posParam(const Scope& val) {_posParam.reset(val);}
        inline bool hasPosParam() const {return false;}
        inline const Scope& posParam() const {assert(false); return _posParam.get();}
        inline void isTuple(const bool& val) {_isTuple = val;}
        inline const bool& isTuple() const {return _isTuple;}
    private:
        const ScopeType::T _type;
        List _list;
        Ptr<const Scope> _posParam;
        bool _isTuple;
    };

    class RootTypeSpec : public TypeSpec {
    public:
        inline RootTypeSpec(const Token& name) : TypeSpec(name) {}
    };

    class ChildTypeSpec : public TypeSpec {
    public:
        inline ChildTypeSpec(const TypeSpec& parent, const Token& name) : TypeSpec(name), _parent(parent) {}
    public:
        inline const TypeSpec& parent() const {return _parent;}
    private:
        // parent already holds a ref to child, so using Ptr here will create a cyclic ref.
        const TypeSpec& _parent;
    };

    class UserDefinedTypeSpec : public ChildTypeSpec {
    public:
        inline UserDefinedTypeSpec(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType) : ChildTypeSpec(parent, name), _defType(defType) {}
        inline const DefinitionType::T& defType() const {return _defType;}
    private:
        const DefinitionType::T _defType;
    };

    class TypedefDecl : public UserDefinedTypeSpec {
    public:
        inline TypedefDecl(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class TypedefDefn : public UserDefinedTypeSpec {
    public:
        inline TypedefDefn(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const Ast::QualifiedTypeSpec& qTypeSpec) : UserDefinedTypeSpec(parent, name, defType), _qTypeSpec(qTypeSpec) {}
        inline const Ast::QualifiedTypeSpec& qTypeSpec() const {return _qTypeSpec.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Ast::QualifiedTypeSpec> _qTypeSpec;
    };

    class TemplatePartList : public Node {
    public:
        typedef std::list<Token> List;
    public:
        inline TemplatePartList(const Token& pos) : Node(pos) {}
    public:
        inline TemplatePartList& addPart(const Token& name) {_list.push_back(name); return z::ref(this);}
        inline const List& list() const {return _list;}
    private:
        List _list;
    };

    class TemplateDecl : public UserDefinedTypeSpec {
    public:
        inline TemplateDecl(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const TemplatePartList& list) : UserDefinedTypeSpec(parent, name, defType), _list(list) {}
    public:
        inline const TemplatePartList::List& list() const {return _list.get().list();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        Ptr<const TemplatePartList> _list;
    };

    class TemplateTypePartList : public Node {
    public:
        typedef SLst<const QualifiedTypeSpec> List;
    public:
        inline TemplateTypePartList(const Token& pos) : Node(pos) {}
    public:
        inline void addType(const QualifiedTypeSpec& qTypeSpec) {_list.add(qTypeSpec);}
        inline const List& list() const {return _list;}
    private:
        List _list;
    };

    class TemplateDefn : public UserDefinedTypeSpec {
    public:
        typedef TemplateTypePartList::List::size_type size_type;
    public:
        inline TemplateDefn(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const TemplateDecl& templateDecl, const TemplateTypePartList& list)
            : UserDefinedTypeSpec(parent, name, defType), _templateDecl(templateDecl), _list(list) {}
    public:
        inline const TemplateTypePartList::List& list() const {return _list.get().list();}
        inline const QualifiedTypeSpec& at(const size_type& idx) const {return list().at(idx);}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const TemplateDecl& _templateDecl;
        const Ptr<const TemplateTypePartList> _list;
    };

    class EnumDefn : public UserDefinedTypeSpec {
    public:
        inline EnumDefn(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const Scope& list) : UserDefinedTypeSpec(parent, name, defType), _list(list) {}
        inline const Scope::List& list() const {return _list.get().list();}
        inline const Ast::Scope& scope() const {return _list.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Scope> _list;
    };

    class CompoundStatement;
    class PropertyDecl : public UserDefinedTypeSpec {
    protected:
        inline PropertyDecl(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const Ast::QualifiedTypeSpec& propertyType) : UserDefinedTypeSpec(parent, name, defType), _propertyType(propertyType) {}
    public:
        inline const Ast::QualifiedTypeSpec& qTypeSpec() const {return _propertyType.get();}
    public:
        inline const Ast::CompoundStatement& getBlock() const {return _getBlock.get();}
        inline void setGetBlock(const Ast::CompoundStatement& val) {_getBlock.reset(val);}
    private:
        const Ptr<const QualifiedTypeSpec> _propertyType;
        Ptr<const Ast::CompoundStatement> _getBlock;
    };

    class PropertyDeclRW : public PropertyDecl {
    public:
        inline PropertyDeclRW(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const Ast::QualifiedTypeSpec& propertyType) : PropertyDecl(parent, name, defType, propertyType) {}
    public:
        inline const Ast::CompoundStatement& setBlock() const {return _setBlock.get();}
        inline void setSetBlock(const Ast::CompoundStatement& val) {_setBlock.reset(val);}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        Ptr<const Ast::CompoundStatement> _setBlock;
    };

    class PropertyDeclRO : public PropertyDecl {
    public:
        inline PropertyDeclRO(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const Ast::QualifiedTypeSpec& propertyType) : PropertyDecl(parent, name, defType, propertyType) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class StructDecl : public UserDefinedTypeSpec {
    public:
        inline StructDecl(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class StructDefn : public UserDefinedTypeSpec {
    public:
        typedef SLst<const PropertyDecl> PropertyList;
    protected:
        inline StructDefn(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, Ast::Scope& list, Ast::CompoundStatement& block) : UserDefinedTypeSpec(parent, name, defType), _list(list), _block(block) {}
    public:
        inline const PropertyList& propertyList() const {return _propertyList;}
        inline const Ast::Scope::List& list() const {return _list.get().list();}
        inline const Ast::Scope& scope() const {return _list.get();}
        inline Ast::CompoundStatement& block() {return _block.get();}
        inline const Ast::CompoundStatement& block() const {return _block.get();}
        inline void addVariable(const VariableDefn& val) { _list.get().addVariableDef(val);}
        inline void addProperty(const PropertyDecl& val) { _propertyList.add(val);}
    private:
        const Ptr<Ast::Scope> _list;
        PropertyList _propertyList;
        const Ptr<Ast::CompoundStatement> _block;
    };

    class RootStructDefn : public StructDefn {
    public:
        inline RootStructDefn(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, Ast::Scope& list, Ast::CompoundStatement& block) : StructDefn(parent, name, defType, list, block) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class ChildStructDefn : public StructDefn {
    public:
        inline ChildStructDefn(const TypeSpec& parent, const StructDefn& base, const Token& name, const DefinitionType::T& defType, Ast::Scope& list, Ast::CompoundStatement& block) : StructDefn(parent, name, defType, list, block), _base(base) {}
        inline const StructDefn& base() const {return _base.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const StructDefn> _base;
    };

    class Routine : public UserDefinedTypeSpec {
    protected:
        inline Routine(const TypeSpec& parent, const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType), _outType(outType), _in(in) {}
    public:
        inline const Ast::QualifiedTypeSpec& outType() const {return _outType.get();}
        inline const Ast::Scope::List& in()  const {return _in.get().list();}
    public:
        inline Ast::Scope& inScope() const {return _in.get();}
    private:
        const Ptr<const Ast::QualifiedTypeSpec> _outType;
        Ptr<Ast::Scope> _in;
    };

    class RoutineDecl : public Routine {
    public:
        inline RoutineDecl(const TypeSpec& parent, const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const DefinitionType::T& defType) : Routine(parent, outType, name, in, defType) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class RoutineDefn : public Routine {
    public:
        inline RoutineDefn(const TypeSpec& parent, const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const DefinitionType::T& defType)
            : Routine(parent, outType, name, in, defType) {}
    public:
        inline const Ast::CompoundStatement& block() const {return _block.get();}
        inline void setBlock(const Ast::CompoundStatement& block) {_block.reset(block);}
    private:
        virtual void visit(Visitor& visitor) const;
        Ptr<const Ast::CompoundStatement> _block;
    };

    class FunctionSig : public Node {
    public:
        inline FunctionSig(const Ast::Scope& out, const Ast::Token& name, Ast::Scope& in) : Node(name), _out(out), _name(name), _in(in) {}
    public:
        inline const Ast::Scope::List& out() const {return _out.get().list();}
        inline const Token& name() const {return _name;}
        inline const Ast::Scope::List& in() const {return _in.get().list();}
    public:
        inline const Ast::Scope& outScope()  const {return _out.get();}
        inline Ast::Scope& inScope()  const {return _in.get();}
    private:
        const Ptr<const Ast::Scope> _out;
        const Token _name;
        Ptr<Ast::Scope> _in;
    };

    class Function : public UserDefinedTypeSpec {
    public:
        inline Function(const TypeSpec& parent, const Ast::Token& name, const DefinitionType::T& defType, const Ast::FunctionSig& sig, Ast::Scope& xref) : UserDefinedTypeSpec(parent, name, defType), _xref(xref), _sig(sig) {}
        inline const Ast::Scope::List& xref() const {return _xref.get().list();}
        inline Ast::Scope& xrefScope()  const {return _xref.get();}
        inline const Ast::FunctionSig& sig() const {return _sig.get();}
    private:
        const Ptr<Ast::Scope> _xref;
        const Ptr<const Ast::FunctionSig> _sig;
    };

    class FunctionDecl : public Function {
    public:
        inline FunctionDecl(const TypeSpec& parent, const Ast::Token& name, const DefinitionType::T& defType, const Ast::FunctionSig& sig, Ast::Scope& xref)
            : Function(parent, name, defType, sig, xref) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class FunctionDefn : public Function {
    protected:
        inline FunctionDefn(const TypeSpec& parent, const Ast::Token& name, const DefinitionType::T& defType, const Ast::FunctionSig& sig, Ast::Scope& xref)
            : Function(parent, name, defType, sig, xref) {}
    public:
        inline const Ast::CompoundStatement& block() const {return _block.get();}
        inline void setBlock(const Ast::CompoundStatement& block) {_block.reset(block);}
    private:
        Ptr<const Ast::CompoundStatement> _block;
    };

    class RootFunctionDefn : public FunctionDefn {
    public:
        inline RootFunctionDefn(const TypeSpec& parent, const Ast::Token& name, const DefinitionType::T& defType, const Ast::FunctionSig& sig, Ast::Scope& xref)
            : FunctionDefn(parent, name, defType, sig, xref) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class ChildFunctionDefn : public FunctionDefn {
    public:
        inline ChildFunctionDefn(const TypeSpec& parent, const Ast::Token& name, const DefinitionType::T& defType, const Ast::FunctionSig& sig, Ast::Scope& xref, const Ast::Function& base)
            : FunctionDefn(parent, name, defType, sig, xref), _base(base) {}
        inline const Ast::Function& base() const {return _base.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Ast::Function> _base;
    };

    class FunctionRetn : public ChildTypeSpec {
    public:
        inline FunctionRetn(const TypeSpec& parent, const Ast::Token& name, const Ast::Scope& out) : ChildTypeSpec(parent, name), _out(out) {}
        inline const Ast::Scope::List& out() const {return _out.get().list();}
        inline const Ast::Scope& outScope() const {return _out.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Ast::Scope> _out;
    };

    class EventDecl : public UserDefinedTypeSpec {
    public:
        inline EventDecl(const TypeSpec& parent, const Ast::Token& name, const Ast::FunctionSig& sig, const Ast::VariableDefn& in, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType), _sig(sig), _in(in) {}
    public:
        inline const Ast::VariableDefn& in()  const {return _in.get();}
    public:
        inline void setHandler(Ast::FunctionDecl& funDecl) {
            addChild(funDecl);
            _funDecl.reset(funDecl);
        }
        inline const Ast::FunctionDecl& handler() const {return _funDecl.get();}
    public:
        inline void setAddFunction(Ast::FunctionDecl& funDecl) {
            addChild(funDecl);
            _addDecl.reset(funDecl);
        }
        inline const Ast::FunctionDecl& addFunction() const {return _addDecl.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Ast::FunctionSig> _sig;
        const Ptr<const Ast::VariableDefn> _in;
        Ptr<Ast::FunctionDecl> _funDecl;
        Ptr<Ast::FunctionDecl> _addDecl;
    };

    class Namespace : public ChildTypeSpec {
    public:
        inline Namespace(const TypeSpec& parent, const Token& name) : ChildTypeSpec(parent, name) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class Root : public RootTypeSpec {
    public:
        inline Root(const z::string& name) : RootTypeSpec(Token("", 0, 0, name)) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    struct TypeSpec::Visitor {
        inline void visitNode(const TypeSpec& node) {
            node.visit(z::ref(this));
        }

        inline void visitChildren(const TypeSpec& typeSpec) {
            for(TypeSpec::ChildTypeSpecList::const_iterator it = typeSpec.childTypeSpecList().begin(); it != typeSpec.childTypeSpecList().end(); ++it) {
                const TypeSpec& childTypeSpec = it->get();
                visitNode(childTypeSpec);
            }
        }

        virtual void visit(const TypedefDecl& node) = 0;
        virtual void visit(const TypedefDefn& node) = 0;
        virtual void visit(const TemplateDecl& node) = 0;
        virtual void visit(const TemplateDefn& node) = 0;
        virtual void visit(const EnumDefn& node) = 0;
        virtual void visit(const StructDecl& node) = 0;
        virtual void visit(const RootStructDefn& node) = 0;
        virtual void visit(const ChildStructDefn& node) = 0;
        virtual void visit(const PropertyDeclRW& node) = 0;
        virtual void visit(const PropertyDeclRO& node) = 0;
        virtual void visit(const RoutineDecl& node) = 0;
        virtual void visit(const RoutineDefn& node) = 0;
        virtual void visit(const FunctionDecl& node) = 0;
        virtual void visit(const RootFunctionDefn& node) = 0;
        virtual void visit(const ChildFunctionDefn& node) = 0;
        virtual void visit(const FunctionRetn& node) = 0;
        virtual void visit(const EventDecl& node) = 0;
        virtual void visit(const Namespace& node) = 0;
        virtual void visit(const Root& node) = 0;
    };

    //////////////////////////////////////////////////////////////////
    class NamespaceList : public Node {
    public:
        typedef SLst<Ast::Namespace> List;
        inline NamespaceList(const Ast::Token& pos) : Node(pos) {}
        inline void addNamespace(Ast::Namespace& ns) {_list.add(ns);}
        inline const List& list() const {return _list;}
    private:
        List _list;
    };

    //////////////////////////////////////////////////////////////////
    class CoerceList : public Node {
    public:
        typedef SLst<const Ast::TypeSpec> List;
        inline CoerceList(const Ast::Token& pos) : Node(pos) {}
        inline void addTypeSpec(const Ast::TypeSpec& typeSpec) {_list.add(typeSpec);}
        inline const List& list() const {return _list;}
    private:
        List _list;
    };

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    class Expr : public Node {
    public:
        struct Visitor;
    protected:
        inline Expr(const Token& pos, const QualifiedTypeSpec& qTypeSpec) : Node(pos), _qTypeSpec(qTypeSpec) {}
    public:
        inline const QualifiedTypeSpec& qTypeSpec() const {return _qTypeSpec.get();}
    public:
        virtual void visit(Visitor& visitor) const = 0;
    private:
        const Ptr<const QualifiedTypeSpec> _qTypeSpec;
    };

    class ExprList : public Node {
    public:
        typedef SLst<const Expr> List;
    public:
        inline ExprList(const Token& pos) : Node(pos) {}
        inline ExprList& addExpr(const Expr& expr) {_list.add(expr); return z::ref(this);}
        inline const List& list() const {return _list;}
        inline const Expr& at(const List::size_type& idx) const {return _list.at(idx);}
    private:
        List _list;
    };

    class TernaryOpExpr : public Expr {
    public:
        inline TernaryOpExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op1, const Token& op2, const Expr& lhs, const Expr& rhs1, const Expr& rhs2)
            : Expr(op1, qTypeSpec), _op1(op1), _op2(op2), _lhs(lhs), _rhs1(rhs1), _rhs2(rhs2) {}
        inline const Token& op1() const {return _op1;}
        inline const Token& op2() const {return _op2;}
        inline const Expr& lhs() const {return _lhs.get();}
        inline const Expr& rhs1() const {return _rhs1.get();}
        inline const Expr& rhs2() const {return _rhs2.get();}
    private:
        const Token _op1;
        const Token _op2;
        const Ptr<const Expr> _lhs;
        const Ptr<const Expr> _rhs1;
        const Ptr<const Expr> _rhs2;
    };

    class ConditionalExpr : public TernaryOpExpr {
    public:
        inline ConditionalExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op1, const Token& op2, const Expr& lhs, const Expr& rhs1, const Expr& rhs2)
            : TernaryOpExpr(qTypeSpec, op1, op2, lhs, rhs1, rhs2) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryExpr : public Expr {
    public:
        inline BinaryExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : Expr(op, qTypeSpec), _op(op), _lhs(lhs), _rhs(rhs) {}
        inline const Token& op() const {return _op;}
        inline const Expr& lhs() const {return _lhs.get();}
        inline const Expr& rhs() const {return _rhs.get();}
    private:
        const Token _op;
        const Ptr<const Expr> _lhs;
        const Ptr<const Expr> _rhs;
    };

    class BooleanAndExpr : public BinaryExpr {
    public:
        inline BooleanAndExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BooleanOrExpr : public BinaryExpr {
    public:
        inline BooleanOrExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BooleanEqualExpr : public BinaryExpr {
    public:
        inline BooleanEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BooleanNotEqualExpr : public BinaryExpr {
    public:
        inline BooleanNotEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BooleanLessThanExpr : public BinaryExpr {
    public:
        inline BooleanLessThanExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BooleanGreaterThanExpr : public BinaryExpr {
    public:
        inline BooleanGreaterThanExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BooleanLessThanOrEqualExpr : public BinaryExpr {
    public:
        inline BooleanLessThanOrEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BooleanGreaterThanOrEqualExpr : public BinaryExpr {
    public:
        inline BooleanGreaterThanOrEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BooleanHasExpr : public BinaryExpr {
    public:
        inline BooleanHasExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryAssignEqualExpr : public BinaryExpr {
    public:
        inline BinaryAssignEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryPlusEqualExpr : public BinaryExpr {
    public:
        inline BinaryPlusEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryMinusEqualExpr : public BinaryExpr {
    public:
        inline BinaryMinusEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryTimesEqualExpr : public BinaryExpr {
    public:
        inline BinaryTimesEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryDivideEqualExpr : public BinaryExpr {
    public:
        inline BinaryDivideEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryModEqualExpr : public BinaryExpr {
    public:
        inline BinaryModEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryBitwiseAndEqualExpr : public BinaryExpr {
    public:
        inline BinaryBitwiseAndEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryBitwiseOrEqualExpr : public BinaryExpr {
    public:
        inline BinaryBitwiseOrEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryBitwiseXorEqualExpr : public BinaryExpr {
    public:
        inline BinaryBitwiseXorEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryShiftLeftEqualExpr : public BinaryExpr {
    public:
        inline BinaryShiftLeftEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryShiftRightEqualExpr : public BinaryExpr {
    public:
        inline BinaryShiftRightEqualExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryPlusExpr : public BinaryExpr {
    public:
        inline BinaryPlusExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryMinusExpr : public BinaryExpr {
    public:
        inline BinaryMinusExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryTimesExpr : public BinaryExpr {
    public:
        inline BinaryTimesExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryDivideExpr : public BinaryExpr {
    public:
        inline BinaryDivideExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryModExpr : public BinaryExpr {
    public:
        inline BinaryModExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryBitwiseAndExpr : public BinaryExpr {
    public:
        inline BinaryBitwiseAndExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryBitwiseOrExpr : public BinaryExpr {
    public:
        inline BinaryBitwiseOrExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryBitwiseXorExpr : public BinaryExpr {
    public:
        inline BinaryBitwiseXorExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryShiftLeftExpr : public BinaryExpr {
    public:
        inline BinaryShiftLeftExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BinaryShiftRightExpr : public BinaryExpr {
    public:
        inline BinaryShiftRightExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : BinaryExpr(qTypeSpec, op, lhs, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class PostfixExpr : public Expr {
    public:
        inline PostfixExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs) : Expr(op, qTypeSpec), _op(op), _lhs(lhs) {}
        inline const Token& op() const {return _op;}
        inline const Expr& lhs() const {return _lhs.get();}
    private:
        const Token _op;
        const Ptr<const Expr> _lhs;
    };

    class PostfixIncExpr : public PostfixExpr {
    public:
        inline PostfixIncExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs) : PostfixExpr(qTypeSpec, op, lhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class PostfixDecExpr : public PostfixExpr {
    public:
        inline PostfixDecExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs) : PostfixExpr(qTypeSpec, op, lhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class PrefixExpr : public Expr {
    public:
        inline PrefixExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& rhs) : Expr(op, qTypeSpec), _op(op), _rhs(rhs) {}
        inline const Token& op() const {return _op;}
        inline const Expr& rhs() const {return _rhs.get();}
    private:
        const Token _op;
        const Ptr<const Expr> _rhs;
    };

    class PrefixNotExpr : public PrefixExpr {
    public:
        inline PrefixNotExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& rhs) : PrefixExpr(qTypeSpec, op, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class PrefixPlusExpr : public PrefixExpr {
    public:
        inline PrefixPlusExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& rhs) : PrefixExpr(qTypeSpec, op, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class PrefixMinusExpr : public PrefixExpr {
    public:
        inline PrefixMinusExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& rhs) : PrefixExpr(qTypeSpec, op, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class PrefixIncExpr : public PrefixExpr {
    public:
        inline PrefixIncExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& rhs) : PrefixExpr(qTypeSpec, op, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class PrefixDecExpr : public PrefixExpr {
    public:
        inline PrefixDecExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& rhs) : PrefixExpr(qTypeSpec, op, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class PrefixBitwiseNotExpr : public PrefixExpr {
    public:
        inline PrefixBitwiseNotExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& rhs) : PrefixExpr(qTypeSpec, op, rhs) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class ListItem : public Node {
    public:
        inline ListItem(const Token& pos, const Expr& valueExpr) : Node(pos), _valueExpr(valueExpr) {}
        inline const Expr& valueExpr() const {return _valueExpr.get();}
    private:
        const Ptr<const Expr> _valueExpr;
    };

    class ListList : public Node {
    public:
        typedef SLst<const ListItem> List;
    public:
        inline ListList(const Token& pos) : Node(pos) {}
    public:
        inline ListList& dValueType(const QualifiedTypeSpec& val) { _dValueType.reset(val); return z::ref(this);}
        inline ListList& valueType(const QualifiedTypeSpec& val) { _valueType.reset(val); return z::ref(this);}
        inline const QualifiedTypeSpec& valueType() const {return _valueType.get();}
        inline ListList& addItem(const ListItem& item) { _list.add(item); return z::ref(this);}
        inline const List& list() const {return _list;}
    private:
        Ptr<const QualifiedTypeSpec> _dValueType;
        Ptr<const QualifiedTypeSpec> _valueType;
        List _list;
    };

    class ListExpr : public Expr {
    public:
        inline ListExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const ListList& list) : Expr(pos, qTypeSpec), _list(list) {}
        inline const ListList& list() const {return _list.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const ListList> _list;
    };

    class DictItem : public Node {
    public:
        inline DictItem(const Token& pos, const Expr& keyExpr, const Expr& valueExpr) : Node(pos), _keyExpr(keyExpr), _valueExpr(valueExpr) {}
        inline const Expr& keyExpr() const {return _keyExpr.get();}
        inline const Expr& valueExpr() const {return _valueExpr.get();}
    private:
        const Ptr<const Expr> _keyExpr;
        const Ptr<const Expr> _valueExpr;
    };

    class DictList : public Node {
    public:
        typedef SLst<const DictItem> List;
    public:
        inline DictList(const Token& pos) : Node(pos) {}
    public:
        inline DictList& dKeyType(const QualifiedTypeSpec& val) { _dKeyType.reset(val); return z::ref(this);}
        inline DictList& dValueType(const QualifiedTypeSpec& val) { _dValueType.reset(val); return z::ref(this);}
        inline DictList& keyType(const QualifiedTypeSpec& val) { _keyType.reset(val); return z::ref(this);}
        inline const QualifiedTypeSpec& keyType() const {return _keyType.get();}
        inline DictList& valueType(const QualifiedTypeSpec& val) { _valueType.reset(val); return z::ref(this);}
        inline const QualifiedTypeSpec& valueType() const {return _valueType.get();}
        inline DictList& addItem(const DictItem& item) { _list.add(item); return z::ref(this);}
        inline const List& list() const {return _list;}
    private:
        Ptr<const QualifiedTypeSpec> _dKeyType;
        Ptr<const QualifiedTypeSpec> _dValueType;
        Ptr<const QualifiedTypeSpec> _keyType;
        Ptr<const QualifiedTypeSpec> _valueType;
        List _list;
    };

    class DictExpr : public Expr {
    public:
        inline DictExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const DictList& dict) : Expr(pos, qTypeSpec), _list(dict) {}
        inline const DictList& list() const {return _list.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const DictList> _list;
    };

    class FormatExpr : public Expr {
    public:
        inline FormatExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Ast::Expr& stringExpr, const Ast::DictExpr& dictExpr) : Expr(pos, qTypeSpec), _stringExpr(stringExpr), _dictExpr(dictExpr) {}
        inline const Expr& stringExpr() const {return _stringExpr.get();}
        inline const DictExpr& dictExpr() const {return _dictExpr.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Ast::Expr> _stringExpr;
        const Ptr<const Ast::DictExpr> _dictExpr;
    };

    class OrderedExpr : public Expr {
    public:
        inline OrderedExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Expr& expr) : Expr(pos, qTypeSpec), _expr(expr) {}
        inline const Expr& expr() const {return _expr.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Expr> _expr;
    };

    class IndexExpr : public Expr {
    public:
        inline IndexExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr, const Ast::Expr& index) : Expr(pos, qTypeSpec), _expr(expr), _index(index) {}
        inline const Expr& expr() const {return _expr.get();}
        inline const Expr& index() const {return _index.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Expr> _expr;
        const Ptr<const Expr> _index;
    };

    class SpliceExpr : public Expr {
    public:
        inline SpliceExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr, const Ast::Expr& from, const Ast::Expr& to) : Expr(pos, qTypeSpec), _expr(expr), _from(from), _to(to) {}
        inline const Expr& expr() const {return _expr.get();}
        inline const Expr& from() const {return _from.get();}
        inline const Expr& to() const {return _to.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Expr> _expr;
        const Ptr<const Expr> _from;
        const Ptr<const Expr> _to;
    };

    class SetIndexExpr : public Expr {
    public:
        inline SetIndexExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const IndexExpr& lhs, const Expr& rhs) : Expr(pos, qTypeSpec), _lhs(lhs), _rhs(rhs) {}
        inline const IndexExpr& lhs() const {return _lhs.get();}
        inline const Expr& rhs() const {return _rhs.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const IndexExpr> _lhs;
        const Ptr<const Expr> _rhs;
    };

    class TypeofExpr : public Expr {
    protected:
        inline TypeofExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec) : Expr(pos, qTypeSpec) {}
    };

    class TypeofTypeExpr : public TypeofExpr {
    public:
        inline TypeofTypeExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Ast::QualifiedTypeSpec& typeSpec) : TypeofExpr(pos, qTypeSpec), _typeSpec(typeSpec) {}
        inline const QualifiedTypeSpec& typeSpec() const {return _typeSpec.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Ast::QualifiedTypeSpec> _typeSpec;
    };

    class TypeofExprExpr : public TypeofExpr {
    public:
        inline TypeofExprExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr) : TypeofExpr(pos, qTypeSpec), _expr(expr) {}
        inline const Expr& expr() const {return _expr.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Ast::Expr> _expr;
    };

    class TypecastExpr : public Expr {
    protected:
        inline TypecastExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const QualifiedTypeSpec& qSrcTypeSpec, const Ast::Expr& expr)
            : Expr(pos, qTypeSpec), _qSrcTypeSpec(qSrcTypeSpec), _expr(expr) {}
    public:
        inline const Expr& expr() const {return _expr.get();}
    private:
        const Ptr<const QualifiedTypeSpec> _qSrcTypeSpec;
        const Ptr<const Ast::Expr> _expr;
    };

    class StaticTypecastExpr : public TypecastExpr {
    public:
        inline StaticTypecastExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const QualifiedTypeSpec& qSrcTypeSpec, const Ast::Expr& expr) : TypecastExpr(pos, qTypeSpec, qSrcTypeSpec, expr) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class DynamicTypecastExpr : public TypecastExpr {
    public:
        inline DynamicTypecastExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const QualifiedTypeSpec& qSrcTypeSpec, const Ast::Expr& expr) : TypecastExpr(pos, qTypeSpec, qSrcTypeSpec, expr) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class TemplateDefnInstanceExpr : public Expr {
    protected:
        inline TemplateDefnInstanceExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Ast::TemplateDefn& srcTemplateDefn, const Ast::TemplateDefn& templateDefn, const ExprList& exprList)
            : Expr(pos, qTypeSpec), _srcTemplateDefn(srcTemplateDefn), _templateDefn(templateDefn), _exprList(exprList) {}
    public:
        inline const TemplateDefn& templateDefn() const {return _templateDefn.get();}
        inline const ExprList& exprList() const {return _exprList.get();}
    private:
        const Ptr<const Ast::TemplateDefn> _srcTemplateDefn;
        const Ptr<const Ast::TemplateDefn> _templateDefn;
        const Ptr<const ExprList> _exprList;
    };

    class PointerInstanceExpr : public TemplateDefnInstanceExpr {
    public:
        inline PointerInstanceExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Ast::TemplateDefn& srcTemplateDefn, const Ast::TemplateDefn& templateDefn, const ExprList& exprList)
            : TemplateDefnInstanceExpr(pos, qTypeSpec, srcTemplateDefn, templateDefn, exprList) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class ValueInstanceExpr : public TemplateDefnInstanceExpr {
    public:
        inline ValueInstanceExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Ast::TemplateDefn& srcTemplateDefn, const Ast::TemplateDefn& templateDefn, const ExprList& exprList)
            : TemplateDefnInstanceExpr(pos, qTypeSpec, srcTemplateDefn, templateDefn, exprList) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class CallExpr : public Expr {
    protected:
        inline CallExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const ExprList& exprList) : Expr(pos, qTypeSpec), _exprList(exprList) {}
    public:
        inline const ExprList& exprList() const {return _exprList.get();}
    private:
        const Ptr<const ExprList> _exprList;
    };

    class RoutineCallExpr : public CallExpr {
    public:
        inline RoutineCallExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Routine& routine, const ExprList& exprList) : CallExpr(pos, qTypeSpec, exprList), _routine(routine) {}
        inline const Routine& routine() const {return _routine.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Routine> _routine;
    };

    class FunctorCallExpr : public CallExpr {
    public:
        inline FunctorCallExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Expr& expr, const ExprList& exprList) : CallExpr(pos, qTypeSpec, exprList), _expr(expr) {}
        inline const Expr& expr() const {return _expr.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Expr> _expr;
    };

    class RunExpr : public Expr {
    public:
        inline RunExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const FunctorCallExpr& callExpr) : Expr(pos, qTypeSpec), _callExpr(callExpr) {}
    public:
        inline const FunctorCallExpr& callExpr() const {return _callExpr.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const FunctorCallExpr> _callExpr;
    };

    class VariableRefExpr : public Expr {
    public:
        inline VariableRefExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const VariableDefn& vref, const RefType::T& refType) : Expr(pos, qTypeSpec), _vref(vref), _refType(refType) {}
        inline const VariableDefn& vref() const {return _vref.get();}
        inline const RefType::T& refType() const {return _refType;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const VariableDefn> _vref;
        const RefType::T _refType;
    };

    class MemberExpr : public Expr {
    protected:
        inline MemberExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Expr& expr) : Expr(pos, qTypeSpec), _expr(expr) {}
    public:
        inline const Expr& expr() const {return _expr.get();}
    private:
        const Ptr<const Expr> _expr;
    };

    class MemberVariableExpr : public MemberExpr {
    public:
        inline MemberVariableExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Expr& expr, const VariableDefn& vref) : MemberExpr(pos, qTypeSpec, expr), _vref(vref) {}
        inline const VariableDefn& vref() const {return _vref.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const VariableDefn> _vref;
    };

    class MemberPropertyExpr : public MemberExpr {
    public:
        inline MemberPropertyExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Expr& expr, const PropertyDecl& vref) : MemberExpr(pos, qTypeSpec, expr), _vref(vref) {}
        inline const PropertyDecl& pref() const {return _vref.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const PropertyDecl> _vref;
    };

    class TypeSpecMemberExpr : public Expr {
    protected:
        inline TypeSpecMemberExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const TypeSpec& typeSpec, const VariableDefn& vref) : Expr(pos, qTypeSpec), _typeSpec(typeSpec), _vref(vref) {}
    public:
        inline const TypeSpec& typeSpec() const {return _typeSpec.get();}
        inline const VariableDefn& vref() const {return _vref.get();}
    private:
        const Ptr<const TypeSpec> _typeSpec;
        const Ptr<const VariableDefn> _vref;
    };

    class EnumMemberExpr : public TypeSpecMemberExpr {
    public:
        inline EnumMemberExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const TypeSpec& typeSpec, const VariableDefn& vref) : TypeSpecMemberExpr(pos, qTypeSpec, typeSpec, vref) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class StructMemberExpr : public TypeSpecMemberExpr {
    public:
        inline StructMemberExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const TypeSpec& typeSpec, const VariableDefn& vref) : TypeSpecMemberExpr(pos, qTypeSpec, typeSpec, vref) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class TypeSpecInstanceExpr : public Expr {
    protected:
        inline TypeSpecInstanceExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const ExprList& exprList) : Expr(pos, qTypeSpec), _exprList(exprList) {}
    public:
        inline const ExprList& exprList() const {return _exprList.get();}
    private:
        const Ptr<const ExprList> _exprList;
    };

    class StructInitPart : public Node {
    public:
        inline StructInitPart(const Token& pos, const VariableDefn& vdef, const Expr& expr) : Node(pos), _vdef(vdef), _expr(expr) {}
        inline const VariableDefn& vdef() const {return _vdef.get();}
        inline const Expr& expr() const {return _expr.get();}
    private:
        const Ptr<const VariableDefn> _vdef;
        const Ptr<const Expr> _expr;
    };

    class StructInitPartList : public Node {
    public:
        typedef SLst<const StructInitPart> List;
    public:
        inline StructInitPartList(const Token& pos) : Node(pos) {}
        inline const List& list() const {return _list;}
        inline void addPart(const StructInitPart& part) { _list.add(part);}
    private:
        List _list;
    };

    class StructInstanceExpr : public Expr {
    public:
        inline StructInstanceExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const StructDefn& structDefn, const StructInitPartList& list) : Expr(pos, qTypeSpec), _structDefn(structDefn), _list(list) {}
        inline const StructDefn& structDefn() const {return _structDefn.get();}
        inline const StructInitPartList& list() const {return _list.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const StructDefn> _structDefn;
        const Ptr<const StructInitPartList> _list;
    };

    class FunctionTypeInstanceExpr : public TypeSpecInstanceExpr {
    public:
        inline FunctionTypeInstanceExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const ExprList& exprList) : TypeSpecInstanceExpr(pos, qTypeSpec, exprList) {}
    };

    class FunctionInstanceExpr : public FunctionTypeInstanceExpr {
    public:
        inline FunctionInstanceExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const Function& function, const ExprList& exprList) : FunctionTypeInstanceExpr(pos, qTypeSpec, exprList), _function(function) {}
        inline const Function& function() const {return _function.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Function> _function;
    };

    class AnonymousFunctionExpr : public FunctionTypeInstanceExpr {
    public:
        inline AnonymousFunctionExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, Ast::ChildFunctionDefn& function, const ExprList& exprList)
            : FunctionTypeInstanceExpr(pos, qTypeSpec, exprList), _function(function) {}
        inline const ChildFunctionDefn& function() const {return _function.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        Ptr<Ast::ChildFunctionDefn> _function;
    };

    class ConstantExpr : public Expr {
    public:
        inline ConstantExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec) : Expr(pos, qTypeSpec) {}
    };

    class ConstantFloatExpr : public ConstantExpr {
    public:
        inline ConstantFloatExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const float& value) : ConstantExpr(pos, qTypeSpec), _value(value) {}
        inline const float& value() const {return _value;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const float _value;
    };

    class ConstantDoubleExpr : public ConstantExpr {
    public:
        inline ConstantDoubleExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const double& value) : ConstantExpr(pos, qTypeSpec), _value(value) {}
        inline const double& value() const {return _value;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const double _value;
    };

    class ConstantBooleanExpr : public ConstantExpr {
    public:
        inline ConstantBooleanExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const bool& value) : ConstantExpr(pos, qTypeSpec), _value(value) {}
        inline const bool& value() const {return _value;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const bool _value;
    };

    class ConstantStringExpr : public ConstantExpr {
    public:
        inline ConstantStringExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const z::string& value) : ConstantExpr(pos, qTypeSpec), _value(value) {}
        inline const z::string& value() const {return _value;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const z::string _value;
    };

    class ConstantCharExpr : public ConstantExpr {
    public:
        inline ConstantCharExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const z::string& value) : ConstantExpr(pos, qTypeSpec), _value(value) {}
        inline const z::string& value() const {return _value;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const z::string _value;
    };

    class ConstantLongExpr : public ConstantExpr {
    public:
        inline ConstantLongExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const long& value) : ConstantExpr(pos, qTypeSpec), _value(value) {}
        inline const long& value() const {return _value;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const long _value;
    };

    class ConstantIntExpr : public ConstantExpr {
    public:
        inline ConstantIntExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const int& value) : ConstantExpr(pos, qTypeSpec), _value(value) {}
        inline const int& value() const {return _value;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const int _value;
    };

    class ConstantShortExpr : public ConstantExpr {
    public:
        inline ConstantShortExpr(const Token& pos, const QualifiedTypeSpec& qTypeSpec, const short& value) : ConstantExpr(pos, qTypeSpec), _value(value) {}
        inline const short& value() const {return _value;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const short _value;
    };

    struct Expr::Visitor {
        inline void visitNode(const Expr& node) {
            node.visit(z::ref(this));
        }

        inline void visitList(const ExprList& exprList) {
            for(ExprList::List::const_iterator it = exprList.list().begin(); it != exprList.list().end(); ++it) {
                const Expr& expr = it->get();
                sep();
                visitNode(expr);
            }
        }

        virtual void visit(const ConditionalExpr& node) = 0;
        virtual void visit(const BooleanAndExpr& node) = 0;
        virtual void visit(const BooleanOrExpr& node) = 0;
        virtual void visit(const BooleanEqualExpr& node) = 0;
        virtual void visit(const BooleanNotEqualExpr& node) = 0;
        virtual void visit(const BooleanLessThanExpr& node) = 0;
        virtual void visit(const BooleanGreaterThanExpr& node) = 0;
        virtual void visit(const BooleanLessThanOrEqualExpr& node) = 0;
        virtual void visit(const BooleanGreaterThanOrEqualExpr& node) = 0;
        virtual void visit(const BooleanHasExpr& node) = 0;
        virtual void visit(const BinaryAssignEqualExpr& node) = 0;
        virtual void visit(const BinaryPlusEqualExpr& node) = 0;
        virtual void visit(const BinaryMinusEqualExpr& node) = 0;
        virtual void visit(const BinaryTimesEqualExpr& node) = 0;
        virtual void visit(const BinaryDivideEqualExpr& node) = 0;
        virtual void visit(const BinaryModEqualExpr& node) = 0;
        virtual void visit(const BinaryBitwiseAndEqualExpr& node) = 0;
        virtual void visit(const BinaryBitwiseOrEqualExpr& node) = 0;
        virtual void visit(const BinaryBitwiseXorEqualExpr& node) = 0;
        virtual void visit(const BinaryShiftLeftEqualExpr& node) = 0;
        virtual void visit(const BinaryShiftRightEqualExpr& node) = 0;

        virtual void visit(const BinaryPlusExpr& node) = 0;
        virtual void visit(const BinaryMinusExpr& node) = 0;
        virtual void visit(const BinaryTimesExpr& node) = 0;
        virtual void visit(const BinaryDivideExpr& node) = 0;
        virtual void visit(const BinaryModExpr& node) = 0;
        virtual void visit(const BinaryBitwiseAndExpr& node) = 0;
        virtual void visit(const BinaryBitwiseOrExpr& node) = 0;
        virtual void visit(const BinaryBitwiseXorExpr& node) = 0;
        virtual void visit(const BinaryShiftLeftExpr& node) = 0;
        virtual void visit(const BinaryShiftRightExpr& node) = 0;

        virtual void visit(const PostfixIncExpr& node) = 0;
        virtual void visit(const PostfixDecExpr& node) = 0;

        virtual void visit(const PrefixNotExpr& node) = 0;
        virtual void visit(const PrefixPlusExpr& node) = 0;
        virtual void visit(const PrefixMinusExpr& node) = 0;
        virtual void visit(const PrefixIncExpr& node) = 0;
        virtual void visit(const PrefixDecExpr& node) = 0;
        virtual void visit(const PrefixBitwiseNotExpr& node) = 0;

        virtual void visit(const SetIndexExpr& node) = 0;
        virtual void visit(const ListExpr& node) = 0;
        virtual void visit(const DictExpr& node) = 0;
        virtual void visit(const FormatExpr& node) = 0;
        virtual void visit(const RunExpr& node) = 0;
        virtual void visit(const RoutineCallExpr& node) = 0;
        virtual void visit(const FunctorCallExpr& node) = 0;
        virtual void visit(const OrderedExpr& node) = 0;
        virtual void visit(const IndexExpr& node) = 0;
        virtual void visit(const SpliceExpr& node) = 0;
        virtual void visit(const TypeofTypeExpr& node) = 0;
        virtual void visit(const TypeofExprExpr& node) = 0;
        virtual void visit(const StaticTypecastExpr& node) = 0;
        virtual void visit(const DynamicTypecastExpr& node) = 0;
        virtual void visit(const PointerInstanceExpr& node) = 0;
        virtual void visit(const ValueInstanceExpr& node) = 0;
        virtual void visit(const VariableRefExpr& node) = 0;
        virtual void visit(const MemberVariableExpr& node) = 0;
        virtual void visit(const MemberPropertyExpr& node) = 0;
        virtual void visit(const EnumMemberExpr& node) = 0;
        virtual void visit(const StructMemberExpr& node) = 0;
        virtual void visit(const StructInstanceExpr& node) = 0;
        virtual void visit(const FunctionInstanceExpr& node) = 0;
        virtual void visit(const AnonymousFunctionExpr& node) = 0;

        virtual void visit(const ConstantFloatExpr& node) = 0;
        virtual void visit(const ConstantDoubleExpr& node) = 0;
        virtual void visit(const ConstantBooleanExpr& node) = 0;
        virtual void visit(const ConstantStringExpr& node) = 0;
        virtual void visit(const ConstantCharExpr& node) = 0;
        virtual void visit(const ConstantLongExpr& node) = 0;
        virtual void visit(const ConstantIntExpr& node) = 0;
        virtual void visit(const ConstantShortExpr& node) = 0;

        virtual void sep() = 0;
    };

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    class Statement : public Node {
    public:
        struct Visitor;
    protected:
        inline Statement(const Token& pos) : Node(pos) {}
    public:
        virtual void visit(Visitor& visitor) const = 0;
    };

    class ImportStatement : public Statement {
    public:
        typedef std::list<Token> Part;
    public:
        inline ImportStatement(const Token& pos, const Ast::AccessType::T& accessType, const Ast::HeaderType::T& headerType, const Ast::DefinitionType::T& defType, Ast::NamespaceList& list)
            : Statement(pos), _accessType(accessType), _headerType(headerType), _defType(defType), _list(list) {}
    public:
        inline const AccessType::T& accessType() const {return _accessType;}
        inline const HeaderType::T& headerType() const {return _headerType;}
        inline const DefinitionType::T& defType() const {return _defType;}
    public:
        inline const NamespaceList::List& list() const {return _list.get().list();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const AccessType::T _accessType;
        const HeaderType::T _headerType;
        const DefinitionType::T _defType;
        const Ptr<const NamespaceList> _list;
    };

    class EnterNamespaceStatement : public Statement {
    public:
        inline EnterNamespaceStatement(const Token& pos, NamespaceList& list) : Statement(pos), _list(list) {}
        inline const NamespaceList::List& list() const {return _list.get().list();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<NamespaceList> _list;
    };

    class LeaveNamespaceStatement : public Statement {
    public:
        inline LeaveNamespaceStatement(const Token& pos, const EnterNamespaceStatement& statement) : Statement(pos), _statement(statement) {}
        inline const EnterNamespaceStatement& statement() const {return _statement.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const EnterNamespaceStatement> _statement;
    };

    class UserDefinedTypeSpecStatement : public Statement {
    public:
        inline UserDefinedTypeSpecStatement(const Token& pos, const UserDefinedTypeSpec& typeSpec) : Statement(pos), _typeSpec(typeSpec) {}
        inline const UserDefinedTypeSpec& typeSpec() const {return _typeSpec.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const UserDefinedTypeSpec> _typeSpec;
    };

    class StructMemberStatement : public Statement {
    protected:
        inline StructMemberStatement(const Token& pos, const StructDefn& structDefn) : Statement(pos), _structDefn(structDefn) {}
    public:
        inline const StructDefn& structDefn() const {return _structDefn;}
    private:
        // StructDefn._block holds a ref to this, so no need
        // to hold a ref back to it.
        const StructDefn& _structDefn;
    };

    class StructMemberVariableStatement : public StructMemberStatement {
    public:
        inline StructMemberVariableStatement(const Token& pos, const StructDefn& structDefn, const VariableDefn& defn) :StructMemberStatement(pos, structDefn), _defn(defn) {}
        inline const VariableDefn& defn() const {return _defn.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const VariableDefn> _defn;
    };

    class StructInitStatement : public StructMemberStatement {
    public:
        inline StructInitStatement(const Token& pos, const StructDefn& structDefn) : StructMemberStatement(pos, structDefn) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class EmptyStatement : public Statement {
    public:
        inline EmptyStatement(const Token& pos) : Statement(pos) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class AutoStatement : public Statement {
    public:
        inline AutoStatement(const Token& pos, const VariableDefn& defn) : Statement(pos), _defn(defn) {}
        inline const VariableDefn& defn() const {return _defn.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const VariableDefn> _defn;
    };

    class ExprStatement : public Statement {
    public:
        inline ExprStatement(const Token& pos, const Expr& expr) : Statement(pos), _expr(expr) {}
        inline const Expr& expr() const {return _expr.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Expr> _expr;
    };

    class PrintStatement : public Statement {
    public:
        inline PrintStatement(const Token& pos, const Expr& expr) : Statement(pos), _expr(expr) {}
        inline const Expr& expr() const {return _expr.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Expr> _expr;
    };

    class ConditionalStatement : public Statement {
    protected:
        inline ConditionalStatement(const Token& pos, const Expr& expr, const CompoundStatement& tblock) : Statement(pos), _expr(expr), _tblock(tblock) {}
    public:
        inline const Expr& expr() const {return _expr.get();}
        inline const CompoundStatement& tblock() const {return _tblock.get();}
    private:
        const Ptr<const Expr> _expr;
        const Ptr<const CompoundStatement> _tblock;
    };

    class IfStatement : public ConditionalStatement {
    public:
        inline IfStatement(const Token& pos, const Expr& expr, const CompoundStatement& tblock) : ConditionalStatement(pos, expr, tblock) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class IfElseStatement : public ConditionalStatement {
    public:
        inline IfElseStatement(const Token& pos, const Expr& expr, const CompoundStatement& tblock, const CompoundStatement& fblock) : ConditionalStatement(pos, expr, tblock), _fblock(fblock) {}
        inline const CompoundStatement& fblock() const {return _fblock.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const CompoundStatement> _fblock;
    };

    class LoopStatement : public Statement {
    protected:
        inline LoopStatement(const Token& pos, const Expr& expr, const CompoundStatement& block) : Statement(pos), _expr(expr), _block(block) {}
    public:
        inline const Expr& expr() const {return _expr.get();}
        inline const CompoundStatement& block() const {return _block.get();}
    private:
        const Ptr<const Expr> _expr;
        const Ptr<const CompoundStatement> _block;
    };

    class WhileStatement : public LoopStatement {
    public:
        inline WhileStatement(const Token& pos, const Expr& expr, const CompoundStatement& block) : LoopStatement(pos, expr, block) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class DoWhileStatement : public LoopStatement {
    public:
        inline DoWhileStatement(const Token& pos, const Expr& expr, const CompoundStatement& block) : LoopStatement(pos, expr, block) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class ForStatement : public LoopStatement {
    protected:
        inline ForStatement(const Token& pos, const Expr& expr, const Expr& incr, const CompoundStatement& block) : LoopStatement(pos, expr, block), _incr(incr) {}
    public:
        inline const Expr& incr() const {return _incr.get();}
    private:
        const Ptr<const Expr> _incr;
    };

    class ForExprStatement : public ForStatement {
    public:
        inline ForExprStatement(const Token& pos, const Expr& init, const Expr& expr, const Expr& incr, const CompoundStatement& block) : ForStatement(pos, expr, incr, block), _init(init) {}
        inline const Expr& init() const {return _init.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Expr> _init;
    };

    class ForInitStatement : public ForStatement {
    public:
        inline ForInitStatement(const Token& pos, const VariableDefn& init, const Expr& expr, const Expr& incr, const CompoundStatement& block) : ForStatement(pos, expr, incr, block), _init(init) {}
        inline const VariableDefn& init() const {return _init.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const VariableDefn> _init;
    };

    class ForeachStatement : public Statement {
    protected:
        inline ForeachStatement(const Token& pos, const Ast::VariableDefn& valDef, const Expr& expr) : Statement(pos), _valDef(valDef), _expr(expr) {}
    public:
        inline const VariableDefn& valDef() const {return _valDef.get();}
        inline const Expr& expr() const {return _expr.get();}
        inline const CompoundStatement& block() const {return _block.get();}
        inline void setBlock(const CompoundStatement& val) {_block.reset(val);}
    private:
        const Ptr<const VariableDefn> _valDef;
        const Ptr<const Expr> _expr;
        Ptr<const CompoundStatement> _block;
    };

    class ForeachStringStatement : public ForeachStatement {
    public:
        inline ForeachStringStatement(const Token& pos, const Ast::VariableDefn& valDef, const Expr& expr) : ForeachStatement(pos, valDef, expr) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class ForeachListStatement : public ForeachStatement {
    public:
        inline ForeachListStatement(const Token& pos, const Ast::VariableDefn& valDef, const Expr& expr) : ForeachStatement(pos, valDef, expr) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class ForeachDictStatement : public ForeachStatement {
    public:
        inline ForeachDictStatement(const Token& pos, const Ast::VariableDefn& keyDef, const Ast::VariableDefn& valDef, const Expr& expr) : ForeachStatement(pos, valDef, expr), _keyDef(keyDef) {}
        inline const VariableDefn& keyDef() const {return _keyDef.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const VariableDefn> _keyDef;
    };


    class CaseStatement : public Statement {
    protected:
        inline CaseStatement(const Token& pos, const CompoundStatement& block) : Statement(pos), _block(block) {}
    public:
        inline const CompoundStatement& block() const {return _block.get();}
    private:
        const Ptr<const CompoundStatement> _block;
    };

    class CaseExprStatement : public CaseStatement {
    public:
        inline CaseExprStatement(const Token& pos, const Expr& expr, const CompoundStatement& block) : CaseStatement(pos, block), _expr(expr) {}
    public:
        inline const Expr& expr() const {return _expr.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Expr> _expr;
    };

    class CaseDefaultStatement : public CaseStatement {
    public:
        inline CaseDefaultStatement(const Token& pos, const CompoundStatement& block) : CaseStatement(pos, block) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class SwitchStatement : public Statement {
    protected:
        inline SwitchStatement(const Token& pos, const CompoundStatement& block) : Statement(pos), _block(block) {}
    public:
        inline const CompoundStatement& block() const {return _block.get();}
    private:
        const Ptr<const CompoundStatement> _block;
    };

    class SwitchValueStatement : public SwitchStatement {
    public:
        inline SwitchValueStatement(const Token& pos, const Expr& expr, const CompoundStatement& block) : SwitchStatement(pos, block), _expr(expr) {}
        inline const Expr& expr() const {return _expr.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Expr> _expr;
    };

    class SwitchExprStatement : public SwitchStatement {
    public:
        inline SwitchExprStatement(const Token& pos, const CompoundStatement& block) : SwitchStatement(pos, block) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BreakStatement : public Statement {
    public:
        inline BreakStatement(const Token& pos) : Statement(pos) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class ContinueStatement : public Statement {
    public:
        inline ContinueStatement(const Token& pos) : Statement(pos) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class AddEventHandlerStatement : public Statement {
    public:
        inline AddEventHandlerStatement(const Token& pos, const Ast::EventDecl& event, const Ast::Expr& source, Ast::FunctionTypeInstanceExpr& functor) : Statement(pos), _event(event), _source(source), _functor(functor) {}
    public:
        inline const Ast::EventDecl& event() const {return _event;}
        inline const Ast::Expr& source() const {return _source.get();}
        inline const Ast::FunctionTypeInstanceExpr& functor() const {return _functor.get();}
        inline Ast::FunctionTypeInstanceExpr& functor() {return _functor.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::EventDecl& _event;
        const Ptr<const Ast::Expr> _source;
        const Ptr<Ast::FunctionTypeInstanceExpr> _functor;
    };

    class ReturnStatement : public Statement {
    protected:
        inline ReturnStatement(const Token& pos, const ExprList& exprList) : Statement(pos), _exprList(exprList) {}
    public:
        inline const ExprList& exprList() const {return _exprList.get();}
    private:
        Ptr<const ExprList> _exprList;
    };

    class RoutineReturnStatement : public ReturnStatement {
    public:
        inline RoutineReturnStatement(const Token& pos, const ExprList& exprList) : ReturnStatement(pos, exprList) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class FunctionReturnStatement : public ReturnStatement {
    public:
        inline FunctionReturnStatement(const Token& pos, const ExprList& exprList) : ReturnStatement(pos, exprList) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class CompoundStatement : public Statement {
    public:
        typedef SLst<const Statement> List;
    public:
        inline CompoundStatement(const Token& pos) : Statement(pos) {}
        inline CompoundStatement& addStatement(const Statement& statement) {_list.add(statement); return z::ref(this);}
        inline const List& list() const {return _list;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        List _list;
    };

    struct Statement::Visitor {
        inline void visitNode(const Statement& node) {
            node.visit(z::ref(this));
        }

        virtual void visit(const ImportStatement& node) = 0;
        virtual void visit(const EnterNamespaceStatement& node) = 0;
        virtual void visit(const LeaveNamespaceStatement& node) = 0;
        virtual void visit(const UserDefinedTypeSpecStatement& node) = 0;
        virtual void visit(const StructMemberVariableStatement& node) = 0;
        virtual void visit(const StructInitStatement& node) = 0;
        virtual void visit(const EmptyStatement& node) = 0;
        virtual void visit(const AutoStatement& node) = 0;
        virtual void visit(const ExprStatement& node) = 0;
        virtual void visit(const PrintStatement& node) = 0;
        virtual void visit(const IfStatement& node) = 0;
        virtual void visit(const IfElseStatement& node) = 0;
        virtual void visit(const WhileStatement& node) = 0;
        virtual void visit(const DoWhileStatement& node) = 0;
        virtual void visit(const ForExprStatement& node) = 0;
        virtual void visit(const ForInitStatement& node) = 0;
        virtual void visit(const ForeachStringStatement& node) = 0;
        virtual void visit(const ForeachListStatement& node) = 0;
        virtual void visit(const ForeachDictStatement& node) = 0;
        virtual void visit(const CaseExprStatement& node) = 0;
        virtual void visit(const CaseDefaultStatement& node) = 0;
        virtual void visit(const SwitchValueStatement& node) = 0;
        virtual void visit(const SwitchExprStatement& node) = 0;
        virtual void visit(const BreakStatement& node) = 0;
        virtual void visit(const ContinueStatement& node) = 0;
        virtual void visit(const AddEventHandlerStatement& node) = 0;
        virtual void visit(const RoutineReturnStatement& node) = 0;
        virtual void visit(const FunctionReturnStatement& node) = 0;
        virtual void visit(const CompoundStatement& node) = 0;
    };

    //////////////////////////////////////////////////////////////////
    class Body : public Node {
    public:
        struct Visitor;
    protected:
        inline Body(const Token& pos, const CompoundStatement& block) : Node(pos), _block(block) {}
    public:
        inline const CompoundStatement& block() const {return _block.get();}
    public:
        virtual void visit(Visitor& visitor) const = 0;
    private:
        Ptr<const CompoundStatement> _block;
    };

    class RoutineBody : public Body {
    public:
        inline RoutineBody(const Token& pos, const Routine& routine, const CompoundStatement& block) : Body(pos, block), _routine(routine) {}
        inline const Routine& routine() const {return _routine.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Routine> _routine;
    };

    class FunctionBody : public Body {
    public:
        inline FunctionBody(const Token& pos, const Function& function, const CompoundStatement& block) : Body(pos, block), _function(function) {}
        inline const Function& function() const {return _function.get();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ptr<const Function> _function;
    };

    struct Body::Visitor {
        inline void visitNode(const Body& node) {
            node.visit(z::ref(this));
        }

        virtual void visit(const RoutineBody& node) = 0;
        virtual void visit(const FunctionBody& node) = 0;
    };

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    class Config {
    public:
        struct Mode {
            enum T {
                Compile,
                Executable,
                Shared,
                Static
            };
        };
        typedef std::list<z::string> PathList;
    public:
        inline Config(const z::string& name) : _name(name), _mode(Mode::Executable), _gui(false), _debug(true), _test(true), _olanguage("stlcpp"), _zlibPath("../../zenlang/lib") {}
    public:
        inline const z::string& name() const {return _name;}
    public:
        inline Config& mode(const Mode::T& val) { _mode = val; return z::ref(this);}
        inline const Mode::T& mode() const {return _mode;}
    public:
        inline Config& gui(const bool& val) { _gui = val; return z::ref(this);}
        inline const bool& gui() const {return _gui;}
        inline Config& debug(const bool& val) { _debug = val; return z::ref(this);}
        inline const bool& debug() const {return _debug;}
        inline Config& test(const bool& val) { _test = val; return z::ref(this);}
        inline const bool& test() const {return _test;}
    public:
        inline Config& olanguage(const z::string& val) { _olanguage = val; return z::ref(this);}
        inline const z::string& olanguage() const {return _olanguage;}
    public:
        inline Config& zexePath(const z::string& val) { _zexePath = val; return z::ref(this);}
        inline const z::string& zexePath() const {return _zexePath;}
    public:
        inline Config& zlibPath(const z::string& val) { _zlibPath = val; return z::ref(this);}
        inline const z::string& zlibPath() const {return _zlibPath;}
    public:
        inline Config& addIncludePath(const z::string& dir) { _includePathList.push_back(dir); return z::ref(this);}
        inline const PathList& includePathList() const {return _includePathList;}
    public:
        inline Config& addIncludeFile(const z::string& file) { _includeFileList.push_back(file); return z::ref(this);}
        inline const PathList& includeFileList() const {return _includeFileList;}
    public:
        inline Config& addSourceFile(const z::string& file) { _sourceFileList.push_back(file); return z::ref(this);}
        inline const PathList& sourceFileList() const {return _sourceFileList;}
    private:
        const z::string _name;
        Mode::T _mode;
        bool _gui;
        bool _debug;
        bool _test;
        z::string _olanguage;
        z::string _zexePath;
        z::string _zlibPath;
        PathList _includePathList;
        PathList _includeFileList;
        PathList _sourceFileList;
    };

    class Project {
    public:
        typedef std::map<z::string, Config*> ConfigList;
        struct Verbosity {
            enum T {
                Silent,
                Normal,
                Detailed
            };
        };

    public:
        inline Project() : _name("main"), _oproject("cmake"), _hppExt(".h;.hpp;"), _cppExt(".c;.cpp;"), _zppExt(".zpp;"), _verbosity(Verbosity::Normal) {}
        inline ~Project() {
            for(ConfigList::iterator it = _configList.begin(); it != _configList.end(); ++it) {
                Config* cfg = it->second;
                delete cfg;
            }
        }

    public:
        inline Project& name(const z::string& val) { _name = val; return z::ref(this);}
        inline const z::string& name() const {return _name;}
    public:
        inline Project& oproject(const z::string& val) { _oproject = val; return z::ref(this);}
        inline const z::string& oproject() const {return _oproject;}
    public:
        inline Project& verbosity(const Verbosity::T& val) { _verbosity = val; return z::ref(this);}
        inline const Verbosity::T& verbosity() const {return _verbosity;}
    public:
        inline Config& config(const z::string& name) {
            ConfigList::iterator it = _configList.find(name);
            if(it == _configList.end()) {
                throw z::Exception("Config", z::fmt("Config does not exist"));
            }
            return z::ref(it->second);
        }

        inline Config& addConfig(const z::string& name) {
            ConfigList::iterator it = _configList.find(name);
            if(it != _configList.end()) {
                throw z::Exception("Config", z::fmt("Config already exists"));
            }
            _configList[name] = new Config(name);
            return z::ref(_configList[name]);
        }

    public:
        inline const ConfigList& configList() const {return _configList;}
    public:
        inline const z::string& hppExt() const {return _hppExt;}
        inline const z::string& cppExt() const {return _cppExt;}
        inline const z::string& zppExt() const {return _zppExt;}
    private:
        z::string _name;
        z::string _oproject;
        ConfigList _configList;
    private:
        z::string _hppExt;
        z::string _cppExt;
        z::string _zppExt;
        Verbosity::T _verbosity;
    };
}

inline std::ostream& operator << (std::ostream& os, const Ast::Token& val) {
    os << val.string();
    return os;
}
