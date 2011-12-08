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
            Direct      /// TypeSpec is implemented directly
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
            Global,      /// global scope (unused for now)
            Member,      /// Member of enum or struct
            XRef,        /// XRef scope
            Param,       /// Param scope
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
    class Token {
    public:
        inline Token(const int row, const int col, const std::string& text) : _row(row), _col(col), _text(text) {}
        inline Token(const ::TokenData& token) : _row(token.row()), _col(token.col()), _text(token.text()) {}
        inline const int& row() const {return _row;}
        inline const int& col() const {return _col;}
        inline const char* text() const {return _text.c_str();}
        inline const std::string& string() const {return _text;}
    private:
        const int _row;
        const int _col;
        const std::string _text;
    };

    class Node {
    protected:
        virtual ~Node(){}
    };

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    class ChildTypeSpec;
    class TypeSpec : public Node {
    public:
        struct Visitor;
    private:
        typedef std::list<ChildTypeSpec*> ChildTypeSpecList;
        typedef std::map<std::string, TypeSpec*> ChildTypeSpecMap;
    public:
        inline TypeSpec(const Token& name) : _accessType(AccessType::Private), _name(name) {}
    public:
        inline TypeSpec& accessType(const AccessType::T& val) {_accessType = val; return ref(this);}
        inline const Token& name() const {return _name;}
        inline const AccessType::T& accessType() const {return _accessType;}
        inline const ChildTypeSpecList& childTypeSpecList() const {return _childTypeSpecList;}
        inline size_t childCount() const {return _childTypeSpecList.size();}
    public:
        template <typename T>
        inline void addChild(T& typeSpec) {
            assert(ptr(typeSpec.parent()) == this);
            _childTypeSpecList.push_back(ptr(typeSpec));
            _childTypeSpecMap[typeSpec.name().text()] = ptr(typeSpec);
        }

        template <typename T>
        inline T* hasChild(const std::string& name) const {
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
        inline QualifiedTypeSpec(const bool& isConst, const TypeSpec& typeSpec, const bool& isRef) : _isConst(isConst), _typeSpec(typeSpec), _isRef(isRef) {}
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
        inline VariableDefn(const QualifiedTypeSpec& qualifiedTypeSpec, const Token& name, const Ast::Expr& initExpr) : _qualifiedTypeSpec(qualifiedTypeSpec), _name(name), _initExpr(initExpr) {}
        inline const QualifiedTypeSpec& qTypeSpec() const {return _qualifiedTypeSpec;}
        inline const Token& name() const {return _name;}
        inline const Expr& initExpr() const {return _initExpr;}
    private:
        const QualifiedTypeSpec& _qualifiedTypeSpec;
        const Token _name;
        const Ast::Expr& _initExpr;
    };

    class Scope : public Node {
    public:
        typedef std::list<const VariableDefn*> List;
    public:
        inline Scope(const ScopeType::T& type) : _type(type), _posParam(0), _isTuple(true) {}
        inline Scope& addVariableDef(const VariableDefn& variableDef) {_list.push_back(ptr(variableDef)); return ref(this);}
        inline const ScopeType::T& type() const {return _type;}
        inline const List& list() const {return _list;}
        inline void posParam(const Scope& val) {_posParam = ptr(val);}
        inline const Scope* posParam() const {return _posParam;}
        inline void isTuple(const bool& val) {_isTuple = val;}
        inline const bool& isTuple() const {return _isTuple;}
    private:
        const ScopeType::T _type;
        List _list;
        const Scope* _posParam;
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
        inline const Ast::QualifiedTypeSpec& qTypeSpec() const {return _qTypeSpec;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::QualifiedTypeSpec& _qTypeSpec;
    };

    class TemplatePartList : public Node {
    public:
        typedef std::list<Token> List;
    public:
        inline TemplatePartList() {}
    public:
        inline TemplatePartList& addPart(const Token& name) {_list.push_back(name); return ref(this);}
        inline const List& list() const {return _list;}
    private:
        List _list;
    };

    class TemplateDecl : public UserDefinedTypeSpec {
    public:
        inline TemplateDecl(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const TemplatePartList& list) : UserDefinedTypeSpec(parent, name, defType), _list(list) {}
    public:
        inline const TemplatePartList::List& list() const {return _list.list();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const TemplatePartList& _list;
    };

    class TemplateTypePartList : public Node {
    public:
        typedef std::vector<const QualifiedTypeSpec*> List;
    public:
        inline void addType(const QualifiedTypeSpec& qTypeSpec) {_list.push_back(ptr(qTypeSpec));}
        inline const List& list() const {return _list;}
    private:
        List _list;
    };

    class TemplateDefn : public UserDefinedTypeSpec {
    public:
        typedef std::vector<const QualifiedTypeSpec*> List;
    public:
        inline TemplateDefn(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const TemplateDecl& templateDecl) : UserDefinedTypeSpec(parent, name, defType), _templateDecl(templateDecl) {}
    public:
        inline void addType(const QualifiedTypeSpec& qTypeSpec) {_list.push_back(ptr(qTypeSpec));}
        inline const List& list() const {return _list;}
        inline const QualifiedTypeSpec& at(const size_t& idx) const {assert(idx < _list.size()); return ref(_list.at(idx));}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const TemplateDecl& _templateDecl;
        List _list;
    };

    class EnumDefn : public UserDefinedTypeSpec {
    public:
        inline EnumDefn(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const Scope& list) : UserDefinedTypeSpec(parent, name, defType), _list(list) {}
        inline const Scope::List& list() const {return _list.list();}
        inline const Ast::Scope& scope() const {return _list;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Scope& _list;
    };

    class CompoundStatement;
    class PropertyDecl : public UserDefinedTypeSpec {
    protected:
        inline PropertyDecl(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const Ast::QualifiedTypeSpec& propertyType) : UserDefinedTypeSpec(parent, name, defType), _propertyType(propertyType), _getBlock(0) {}
    public:
        inline const Ast::QualifiedTypeSpec& qTypeSpec() const {return _propertyType;}
    public:
        inline const Ast::CompoundStatement& getBlock() const {return ref(_getBlock);}
        inline void setGetBlock(const Ast::CompoundStatement& val) {_getBlock = ptr(val);}
    private:
        const QualifiedTypeSpec& _propertyType;
        const Ast::CompoundStatement* _getBlock;
    };

    class PropertyDeclRW : public PropertyDecl {
    public:
        inline PropertyDeclRW(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const Ast::QualifiedTypeSpec& propertyType) : PropertyDecl(parent, name, defType, propertyType), _setBlock(0) {}
    public:
        inline const Ast::CompoundStatement& setBlock() const {return ref(_setBlock);}
        inline void setSetBlock(const Ast::CompoundStatement& val) {_setBlock = ptr(val);}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::CompoundStatement* _setBlock;
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
        typedef std::list<const PropertyDecl*> PropertyList;
    protected:
        inline StructDefn(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, Ast::Scope& list, Ast::CompoundStatement& block) : UserDefinedTypeSpec(parent, name, defType), _list(list), _block(block) {}
    public:
        inline const PropertyList& propertyList() const {return _propertyList;}
        inline const Ast::Scope::List& list() const {return _list.list();}
        inline const Ast::Scope& scope() const {return _list;}
        inline Ast::CompoundStatement& block() {return _block;}
        inline const Ast::CompoundStatement& block() const {return _block;}
        inline void addVariable(const VariableDefn& val) { _list.addVariableDef(val);}
        inline void addProperty(const PropertyDecl& val) { _propertyList.push_back(ptr(val));}
    private:
        Ast::Scope& _list;
        PropertyList _propertyList;
        Ast::CompoundStatement& _block;
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
        inline const StructDefn& base() const {return _base;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const StructDefn& _base;
    };

    class Routine : public UserDefinedTypeSpec {
    protected:
        inline Routine(const TypeSpec& parent, const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, Ast::Scope& in, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType), _outType(outType), _in(in) {}
    public:
        inline const Ast::QualifiedTypeSpec& outType() const {return _outType;}
        inline const Ast::Scope::List& in()  const {return _in.list();}
    public:
        inline Ast::Scope& inScope() const {return _in;}
    private:
        const Ast::QualifiedTypeSpec& _outType;
        Ast::Scope& _in;
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
            : Routine(parent, outType, name, in, defType), _block(0) {}
    public:
        inline const Ast::CompoundStatement& block() const {return ref(_block);}
        inline void setBlock(const Ast::CompoundStatement& block) {_block = ptr(block);}
    private:
        virtual void visit(Visitor& visitor) const;
        const Ast::CompoundStatement* _block;
    };

    class FunctionSig : public Node {
    public:
        inline FunctionSig(const Ast::Scope& out, const Ast::Token& name, Ast::Scope& in) : _out(out), _name(name), _in(in) {}
    public:
        inline const Ast::Scope::List& out() const {return _out.list();}
        inline const Token& name() const {return _name;}
        inline const Ast::Scope::List& in() const {return _in.list();}
    public:
        inline const Ast::Scope& outScope()  const {return _out;}
        inline Ast::Scope& inScope()  const {return _in;}
    private:
        const Ast::Scope& _out;
        const Token _name;
        Ast::Scope& _in;
    };

    class Function : public UserDefinedTypeSpec {
    public:
        inline Function(const TypeSpec& parent, const Ast::Token& name, const DefinitionType::T& defType, const Ast::FunctionSig& sig, Ast::Scope& xref) : UserDefinedTypeSpec(parent, name, defType), _xref(xref), _sig(sig) {}
        inline const Ast::Scope::List& xref() const {return _xref.list();}
        inline Ast::Scope& xrefScope()  const {return _xref;}
        inline const Ast::FunctionSig& sig() const {return _sig;}
    private:
        Ast::Scope& _xref;
        const Ast::FunctionSig& _sig;
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
            : Function(parent, name, defType, sig, xref), _block(0) {}
    public:
        inline const Ast::CompoundStatement& block() const {return ref(_block);}
        inline void setBlock(const Ast::CompoundStatement& block) {_block = ptr(block);}
    private:
        const Ast::CompoundStatement* _block;
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
        inline const Ast::Function& base() const {return _base;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::Function& _base;
    };

    class FunctionRetn : public ChildTypeSpec {
    public:
        inline FunctionRetn(const TypeSpec& parent, const Ast::Token& name, const Ast::Scope& out) : ChildTypeSpec(parent, name), _out(out) {}
        inline const Ast::Scope::List& out() const {return _out.list();}
        inline const Ast::Scope& outScope() const {return _out;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::Scope& _out;
    };

    class EventDecl : public UserDefinedTypeSpec {
    public:
        inline EventDecl(const TypeSpec& parent, const Ast::Token& name, const Ast::VariableDefn& in, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType), _in(in), _funDecl(0), _addDecl(0) {}
    public:
        inline const Ast::VariableDefn& in()  const {return _in;}
    public:
        inline void setHandler(Ast::FunctionDecl& funDecl) {
            addChild(funDecl);
            _funDecl = ptr(funDecl);
        }
        inline const Ast::FunctionDecl& handler() const {return ref(_funDecl);}
    public:
        inline void setAddFunction(Ast::FunctionDecl& funDecl) {
            addChild(funDecl);
            _addDecl = ptr(funDecl);
        }
        inline const Ast::FunctionDecl& addFunction() const {return ref(_addDecl);}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::VariableDefn& _in;
        Ast::FunctionDecl* _funDecl;
        Ast::FunctionDecl* _addDecl;
    };

    class Namespace : public ChildTypeSpec {
    public:
        inline Namespace(const TypeSpec& parent, const Token& name) : ChildTypeSpec(parent, name) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class Root : public RootTypeSpec {
    public:
        inline Root(const std::string& name) : RootTypeSpec(Token(0, 0, name)) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    struct TypeSpec::Visitor {
        inline void visitNode(const TypeSpec& node) {
            node.visit(ref(this));
        }

        inline void visitChildren(const TypeSpec& typeSpec) {
            for(TypeSpec::ChildTypeSpecList::const_iterator it = typeSpec.childTypeSpecList().begin(); it != typeSpec.childTypeSpecList().end(); ++it) {
                const TypeSpec& childTypeSpec = ref(*it);
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

    inline void TypedefDecl::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void TypedefDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void TemplateDecl::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void TemplateDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void EnumDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void StructDecl::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void RootStructDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ChildStructDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void PropertyDeclRW::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void PropertyDeclRO::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void RoutineDecl::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void RoutineDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void FunctionDecl::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void RootFunctionDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ChildFunctionDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void FunctionRetn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void EventDecl::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void Namespace::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void Root::visit(Visitor& visitor) const {visitor.visit(ref(this));}

    //////////////////////////////////////////////////////////////////
    class CoerceList : public Node {
    public:
        typedef std::list<const Ast::TypeSpec*> List;
        inline void addTypeSpec(const Ast::TypeSpec& typeSpec) {_list.push_back(ptr(typeSpec));}
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
        inline Expr(const QualifiedTypeSpec& qTypeSpec) : _qTypeSpec(qTypeSpec) {}
    public:
        inline const QualifiedTypeSpec& qTypeSpec() const {return _qTypeSpec;}
    public:
        virtual void visit(Visitor& visitor) const = 0;
    private:
        const QualifiedTypeSpec& _qTypeSpec;
    };

    class ExprList : public Node {
    public:
        typedef std::vector<const Expr*> List;
    public:
        inline ExprList() {}
        inline ExprList& addExpr(const Expr& expr) {_list.push_back(ptr(expr)); return ref(this);}
        inline const List& list() const {return _list;}
        inline const Expr& at(const size_t& idx) const {assert(idx < _list.size()); return ref(_list.at(idx));}
    private:
        List _list;
    };

    class TernaryOpExpr : public Expr {
    public:
        inline TernaryOpExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op1, const Token& op2, const Expr& lhs, const Expr& rhs1, const Expr& rhs2)
            : Expr(qTypeSpec), _op1(op1), _op2(op2), _lhs(lhs), _rhs1(rhs1), _rhs2(rhs2) {}
        inline const Token& op1() const {return _op1;}
        inline const Token& op2() const {return _op2;}
        inline const Expr& lhs() const {return _lhs;}
        inline const Expr& rhs1() const {return _rhs1;}
        inline const Expr& rhs2() const {return _rhs2;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Token _op1;
        const Token _op2;
        const Expr& _lhs;
        const Expr& _rhs1;
        const Expr& _rhs2;
    };

    class BinaryOpExpr : public Expr {
    public:
        inline BinaryOpExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : Expr(qTypeSpec), _op(op), _lhs(lhs), _rhs(rhs) {}
        inline const Token& op() const {return _op;}
        inline const Expr& lhs() const {return _lhs;}
        inline const Expr& rhs() const {return _rhs;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Token _op;
        const Expr& _lhs;
        const Expr& _rhs;
    };

    class PostfixOpExpr : public Expr {
    public:
        inline PostfixOpExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& lhs) : Expr(qTypeSpec), _op(op), _lhs(lhs) {}
        inline const Token& op() const {return _op;}
        inline const Expr& lhs() const {return _lhs;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Token _op;
        const Expr& _lhs;
    };

    class PrefixOpExpr : public Expr {
    public:
        inline PrefixOpExpr(const QualifiedTypeSpec& qTypeSpec, const Token& op, const Expr& rhs) : Expr(qTypeSpec), _op(op), _rhs(rhs) {}
        inline const Token& op() const {return _op;}
        inline const Expr& rhs() const {return _rhs;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Token _op;
        const Expr& _rhs;
    };

    class ListItem : public Node {
    public:
        inline ListItem(const Expr& valueExpr) : _valueExpr(valueExpr) {}
        inline const Expr& valueExpr() const {return _valueExpr;}
    private:
        const Expr& _valueExpr;
    };

    class ListList : public Node {
    public:
        typedef std::list<const ListItem*> List;
    public:
        inline ListList() : _valueType(0) {}
    public:
        inline ListList& valueType(const QualifiedTypeSpec& val) { _valueType = ptr(val); return ref(this);}
        inline const QualifiedTypeSpec& valueType() const {return ref(_valueType);}
        inline ListList& addItem(const ListItem& item) { _list.push_back(ptr(item)); return ref(this);}
        inline const List& list() const {return _list;}
    private:
        const QualifiedTypeSpec* _valueType;
        List _list;
    };

    class ListExpr : public Expr {
    public:
        inline ListExpr(const QualifiedTypeSpec& qTypeSpec, const ListList& list) : Expr(qTypeSpec), _list(list) {}
        inline const ListList& list() const {return _list;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const ListList& _list;
    };

    class DictItem : public Node {
    public:
        inline DictItem(const Expr& keyExpr, const Expr& valueExpr) : _keyExpr(keyExpr), _valueExpr(valueExpr) {}
        inline const Expr& keyExpr() const {return _keyExpr;}
        inline const Expr& valueExpr() const {return _valueExpr;}
    private:
        const Expr& _keyExpr;
        const Expr& _valueExpr;
    };

    class DictList : public Node {
    public:
        typedef std::list<const DictItem*> List;
    public:
        inline DictList() : _keyType(0), _valueType(0) {}
    public:
        inline DictList& keyType(const QualifiedTypeSpec& val) { _keyType = ptr(val); return ref(this);}
        inline const QualifiedTypeSpec& keyType() const {return ref(_keyType);}
        inline DictList& valueType(const QualifiedTypeSpec& val) { _valueType = ptr(val); return ref(this);}
        inline const QualifiedTypeSpec& valueType() const {return ref(_valueType);}
        inline DictList& addItem(const DictItem& item) { _list.push_back(ptr(item)); return ref(this);}
        inline const List& list() const {return _list;}
    private:
        const QualifiedTypeSpec* _keyType;
        const QualifiedTypeSpec* _valueType;
        List _list;
    };

    class DictExpr : public Expr {
    public:
        inline DictExpr(const QualifiedTypeSpec& qTypeSpec, const DictList& dict) : Expr(qTypeSpec), _list(dict) {}
        inline const DictList& list() const {return _list;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const DictList& _list;
    };

    class FormatExpr : public Expr {
    public:
        inline FormatExpr(const QualifiedTypeSpec& qTypeSpec, const Ast::Expr& stringExpr, const Ast::DictExpr& dictExpr) : Expr(qTypeSpec), _stringExpr(stringExpr), _dictExpr(dictExpr) {}
        inline const Expr& stringExpr() const {return _stringExpr;}
        inline const DictExpr& dictExpr() const {return _dictExpr;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::Expr& _stringExpr;
        const Ast::DictExpr& _dictExpr;
    };

    class OrderedExpr : public Expr {
    public:
        inline OrderedExpr(const QualifiedTypeSpec& qTypeSpec, const Expr& expr) : Expr(qTypeSpec), _expr(expr) {}
        inline const Expr& expr() const {return _expr;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Expr& _expr;
    };

    class IndexExpr : public Expr {
    public:
        inline IndexExpr(const QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr, const Ast::Expr& index) : Expr(qTypeSpec), _expr(expr), _index(index) {}
        inline const Expr& expr() const {return _expr;}
        inline const Expr& index() const {return _index;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Expr& _expr;
        const Expr& _index;
    };

    class SetIndexExpr : public Expr {
    public:
        inline SetIndexExpr(const QualifiedTypeSpec& qTypeSpec, const IndexExpr& lhs, const Expr& rhs) : Expr(qTypeSpec), _lhs(lhs), _rhs(rhs) {}
        inline const IndexExpr& lhs() const {return _lhs;}
        inline const Expr& rhs() const {return _rhs;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const IndexExpr& _lhs;
        const Expr& _rhs;
    };

    class TypeofExpr : public Expr {
    protected:
        inline TypeofExpr(const QualifiedTypeSpec& qTypeSpec) : Expr(qTypeSpec) {}
    };

    class TypeofTypeExpr : public TypeofExpr {
    public:
        inline TypeofTypeExpr(const QualifiedTypeSpec& qTypeSpec, const Ast::QualifiedTypeSpec& typeSpec) : TypeofExpr(qTypeSpec), _typeSpec(typeSpec) {}
        inline const QualifiedTypeSpec& typeSpec() const {return _typeSpec;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::QualifiedTypeSpec& _typeSpec;
    };

    class TypeofExprExpr : public TypeofExpr {
    public:
        inline TypeofExprExpr(const QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr) : TypeofExpr(qTypeSpec), _expr(expr) {}
        inline const Expr& expr() const {return _expr;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::Expr& _expr;
    };

    class TypecastExpr : public Expr {
    protected:
        inline TypecastExpr(const QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr) : Expr(qTypeSpec), _expr(expr) {}
    public:
        inline const Expr& expr() const {return _expr;}
    private:
        const Ast::Expr& _expr;
    };

    class StaticTypecastExpr : public TypecastExpr {
    public:
        inline StaticTypecastExpr(const QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr) : TypecastExpr(qTypeSpec, expr) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class DynamicTypecastExpr : public TypecastExpr {
    public:
        inline DynamicTypecastExpr(const QualifiedTypeSpec& qTypeSpec, const Ast::Expr& expr) : TypecastExpr(qTypeSpec, expr) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class TemplateDefnInstanceExpr : public Expr {
    protected:
        inline TemplateDefnInstanceExpr(const QualifiedTypeSpec& qTypeSpec, const Ast::TemplateDefn& templateDefn, const ExprList& exprList) : Expr(qTypeSpec), _templateDefn(templateDefn), _exprList(exprList) {}
    public:
        inline const TemplateDefn& templateDefn() const {return _templateDefn;}
        inline const ExprList& exprList() const {return _exprList;}
    private:
        const Ast::TemplateDefn& _templateDefn;
        const ExprList& _exprList;
    };

    class PointerInstanceExpr : public TemplateDefnInstanceExpr {
    public:
        inline PointerInstanceExpr(const QualifiedTypeSpec& qTypeSpec, const Ast::TemplateDefn& templateDefn, const ExprList& exprList) : TemplateDefnInstanceExpr(qTypeSpec, templateDefn, exprList) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class ValueInstanceExpr : public TemplateDefnInstanceExpr {
    public:
        inline ValueInstanceExpr(const QualifiedTypeSpec& qTypeSpec, const Ast::TemplateDefn& templateDefn, const ExprList& exprList) : TemplateDefnInstanceExpr(qTypeSpec, templateDefn, exprList) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class CallExpr : public Expr {
    protected:
        inline CallExpr(const QualifiedTypeSpec& qTypeSpec, const ExprList& exprList) : Expr(qTypeSpec), _exprList(exprList) {}
    public:
        inline const ExprList& exprList() const {return _exprList;}
    private:
        const ExprList& _exprList;
    };

    class RoutineCallExpr : public CallExpr {
    public:
        inline RoutineCallExpr(const QualifiedTypeSpec& qTypeSpec, const Routine& routine, const ExprList& exprList) : CallExpr(qTypeSpec, exprList), _routine(routine) {}
        inline const Routine& routine() const {return _routine;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Routine& _routine;
    };

    class FunctorCallExpr : public CallExpr {
    public:
        inline FunctorCallExpr(const QualifiedTypeSpec& qTypeSpec, const Expr& expr, const ExprList& exprList) : CallExpr(qTypeSpec, exprList), _expr(expr) {}
        inline const Expr& expr() const {return _expr;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Expr& _expr;
    };

    class RunExpr : public Expr {
    public:
        inline RunExpr(const QualifiedTypeSpec& qTypeSpec, const FunctorCallExpr& callExpr) : Expr(qTypeSpec), _callExpr(callExpr) {}
    public:
        inline const FunctorCallExpr& callExpr() const {return _callExpr;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const FunctorCallExpr& _callExpr;
    };

    class VariableRefExpr : public Expr {
    public:
        inline VariableRefExpr(const QualifiedTypeSpec& qTypeSpec, const VariableDefn& vref, const RefType::T& refType) : Expr(qTypeSpec), _vref(vref), _refType(refType) {}
        inline const VariableDefn& vref() const {return _vref;}
        inline const RefType::T& refType() const {return _refType;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const VariableDefn& _vref;
        const RefType::T _refType;
    };

    class MemberExpr : public Expr {
    protected:
        inline MemberExpr(const QualifiedTypeSpec& qTypeSpec, const Expr& expr) : Expr(qTypeSpec), _expr(expr) {}
    public:
        inline const Expr& expr() const {return _expr;}
    private:
        const Expr& _expr;
    };

    class MemberVariableExpr : public MemberExpr {
    public:
        inline MemberVariableExpr(const QualifiedTypeSpec& qTypeSpec, const Expr& expr, const VariableDefn& vref) : MemberExpr(qTypeSpec, expr), _vref(vref) {}
        inline const VariableDefn& vref() const {return _vref;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const VariableDefn& _vref;
    };

    class MemberPropertyExpr : public MemberExpr {
    public:
        inline MemberPropertyExpr(const QualifiedTypeSpec& qTypeSpec, const Expr& expr, const PropertyDecl& vref) : MemberExpr(qTypeSpec, expr), _vref(vref) {}
        inline const PropertyDecl& pref() const {return _vref;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const PropertyDecl& _vref;
    };

    class TypeSpecMemberExpr : public Expr {
    protected:
        inline TypeSpecMemberExpr(const QualifiedTypeSpec& qTypeSpec, const TypeSpec& typeSpec, const VariableDefn& vref) : Expr(qTypeSpec), _typeSpec(typeSpec), _vref(vref) {}
    public:
        inline const TypeSpec& typeSpec() const {return _typeSpec;}
        inline const VariableDefn& vref() const {return _vref;}
    private:
        const TypeSpec& _typeSpec;
        const VariableDefn& _vref;
    };

    class EnumMemberExpr : public TypeSpecMemberExpr {
    public:
        inline EnumMemberExpr(const QualifiedTypeSpec& qTypeSpec, const TypeSpec& typeSpec, const VariableDefn& vref) : TypeSpecMemberExpr(qTypeSpec, typeSpec, vref) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class StructMemberExpr : public TypeSpecMemberExpr {
    public:
        inline StructMemberExpr(const QualifiedTypeSpec& qTypeSpec, const TypeSpec& typeSpec, const VariableDefn& vref) : TypeSpecMemberExpr(qTypeSpec, typeSpec, vref) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class TypeSpecInstanceExpr : public Expr {
    protected:
        inline TypeSpecInstanceExpr(const QualifiedTypeSpec& qTypeSpec, const ExprList& exprList) : Expr(qTypeSpec), _exprList(exprList) {}
    public:
        inline const ExprList& exprList() const {return _exprList;}
    private:
        const ExprList& _exprList;
    };

    class StructInitPart : public Node {
    public:
        inline StructInitPart(const VariableDefn& vdef, const Expr& expr) : _vdef(vdef), _expr(expr) {}
        inline const VariableDefn& vdef() const {return _vdef;}
        inline const Expr& expr() const {return _expr;}
    private:
        const VariableDefn& _vdef;
        const Expr& _expr;
    };

    class StructInitPartList : public Node {
    public:
        typedef std::list<const StructInitPart*> List;
    public:
        inline const List& list() const {return _list;}
        inline void addPart(const StructInitPart& part) { _list.push_back(ptr(part));}
    private:
        List _list;
    };

    class StructInstanceExpr : public Expr {
    public:
        inline StructInstanceExpr(const QualifiedTypeSpec& qTypeSpec, const StructDefn& structDefn, const StructInitPartList& list) : Expr(qTypeSpec), _structDefn(structDefn), _list(list) {}
        inline const StructDefn& structDefn() const {return _structDefn;}
        inline const StructInitPartList& list() const {return _list;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const StructDefn& _structDefn;
        const StructInitPartList& _list;
    };

    class FunctionTypeInstanceExpr : public TypeSpecInstanceExpr {
    public:
        inline FunctionTypeInstanceExpr(const QualifiedTypeSpec& qTypeSpec, const ExprList& exprList) : TypeSpecInstanceExpr(qTypeSpec, exprList) {}
    };

    class FunctionInstanceExpr : public FunctionTypeInstanceExpr {
    public:
        inline FunctionInstanceExpr(const QualifiedTypeSpec& qTypeSpec, const Function& function, const ExprList& exprList) : FunctionTypeInstanceExpr(qTypeSpec, exprList), _function(function) {}
        inline const Function& function() const {return _function;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Function& _function;
    };

    class AnonymousFunctionExpr : public FunctionTypeInstanceExpr {
    public:
        inline AnonymousFunctionExpr(const QualifiedTypeSpec& qTypeSpec, Ast::ChildFunctionDefn& function, const ExprList& exprList, const Ast::CompoundStatement& compoundStatement)
            : FunctionTypeInstanceExpr(qTypeSpec, exprList), _function(function) {}
        inline const ChildFunctionDefn& function() const {return _function;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        Ast::ChildFunctionDefn& _function;
    };

    class ConstantExpr : public Expr {
    public:
        inline ConstantExpr(const QualifiedTypeSpec& qTypeSpec, const Token& value) : Expr(qTypeSpec), _value(value) {}
        inline const Token& value() const {return _value;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Token _value;
    };

    struct Expr::Visitor {
        inline void visitNode(const Expr& node) {
            node.visit(ref(this));
        }

        inline void visitList(const ExprList& exprList) {
            for(ExprList::List::const_iterator it = exprList.list().begin(); it != exprList.list().end(); ++it) {
                const Expr& expr = ref(*it);
                sep();
                visitNode(expr);
            }
        }

        virtual void visit(const TernaryOpExpr& node) = 0;
        virtual void visit(const BinaryOpExpr& node) = 0;
        virtual void visit(const SetIndexExpr& node) = 0;
        virtual void visit(const PostfixOpExpr& node) = 0;
        virtual void visit(const PrefixOpExpr& node) = 0;
        virtual void visit(const ListExpr& node) = 0;
        virtual void visit(const DictExpr& node) = 0;
        virtual void visit(const FormatExpr& node) = 0;
        virtual void visit(const RunExpr& node) = 0;
        virtual void visit(const RoutineCallExpr& node) = 0;
        virtual void visit(const FunctorCallExpr& node) = 0;
        virtual void visit(const OrderedExpr& node) = 0;
        virtual void visit(const IndexExpr& node) = 0;
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
        virtual void visit(const ConstantExpr& node) = 0;
        virtual void sep() = 0;
    };

    inline void TernaryOpExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void BinaryOpExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void SetIndexExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void PostfixOpExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void PrefixOpExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ListExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void DictExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void FormatExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void RunExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void RoutineCallExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void FunctorCallExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void OrderedExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void IndexExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void TypeofTypeExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void TypeofExprExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void StaticTypecastExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void DynamicTypecastExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void PointerInstanceExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ValueInstanceExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void VariableRefExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void MemberVariableExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void MemberPropertyExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void EnumMemberExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void StructMemberExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void StructInstanceExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void FunctionInstanceExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void AnonymousFunctionExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ConstantExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    class Statement : public Node {
    public:
        struct Visitor;
    protected:
        inline Statement() {}
    public:
        virtual void visit(Visitor& visitor) const = 0;
    };

    class ImportStatement : public Statement {
    public:
        typedef std::list<Token> Part;
    public:
        inline ImportStatement() : _accessType(AccessType::Private), _headerType(HeaderType::Include), _defType(DefinitionType::Direct) {}
    public:
        inline const AccessType::T& accessType() const {return _accessType;}
        inline ImportStatement& accessType(const AccessType::T& val) { _accessType = val; return ref(this);}
    public:
        inline const HeaderType::T& headerType() const {return _headerType;}
        inline ImportStatement& headerType(const HeaderType::T& val) { _headerType = val; return ref(this);}
    public:
        inline const DefinitionType::T& defType() const {return _defType;}
        inline ImportStatement& defType(const DefinitionType::T& val) { _defType = val; return ref(this);}
    public:
        inline ImportStatement& addPart(const Token& name) {_part.push_back(name); return ref(this);}
        inline const Part& part() const {return _part;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        Part _part;
        AccessType::T _accessType;
        HeaderType::T _headerType;
        DefinitionType::T _defType;
    };

    class NamespaceStatement : public Statement {
    public:
        typedef std::list<const Namespace*> List;
        inline void addNamespace(const Namespace& val) {_list.push_back(ptr(val));}
        inline const List& list() const {return _list;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        List _list;
    };

    class LeaveNamespaceStatement : public Statement {
    public:
        inline LeaveNamespaceStatement(const NamespaceStatement& statement) : _statement(statement) {}
        inline const NamespaceStatement& statement() const {return _statement;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const NamespaceStatement& _statement;
    };

    class UserDefinedTypeSpecStatement : public Statement {
    public:
        inline UserDefinedTypeSpecStatement(const UserDefinedTypeSpec& typeSpec) : _typeSpec(typeSpec) {}
        inline const UserDefinedTypeSpec& typeSpec() const {return _typeSpec;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const UserDefinedTypeSpec& _typeSpec;
    };

    class StructMemberStatement : public Statement {
    protected:
        inline StructMemberStatement(const StructDefn& structDefn, const VariableDefn& defn) : _structDefn(structDefn), _defn(defn) {}
    public:
        inline const StructDefn& structDefn() const {return _structDefn;}
        inline const VariableDefn& defn() const {return _defn;}
    private:
        const StructDefn& _structDefn;
        const VariableDefn& _defn;
    };

    class StructMemberVariableStatement : public StructMemberStatement {
    public:
        inline StructMemberVariableStatement(const StructDefn& structDefn, const VariableDefn& defn) :StructMemberStatement(structDefn, defn) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class StructInitStatement : public Statement {
    public:
        inline StructInitStatement(const StructDefn& structDefn) : _structDefn(structDefn) {}
        inline const StructDefn& structDefn() const {return _structDefn;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const StructDefn& _structDefn;
    };

    class AutoStatement : public Statement {
    public:
        inline AutoStatement(const VariableDefn& defn) : _defn(defn) {}
        inline const VariableDefn& defn() const {return _defn;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const VariableDefn& _defn;
    };

    class ExprStatement : public Statement {
    public:
        inline ExprStatement(const Expr& expr) : _expr(expr) {}
        inline const Expr& expr() const {return _expr;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Expr& _expr;
    };

    class PrintStatement : public Statement {
    public:
        inline PrintStatement(const Expr& expr) : _expr(expr) {}
        inline const Expr& expr() const {return _expr;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Expr& _expr;
    };

    class IfStatement : public Statement {
    public:
        inline IfStatement(const Expr& expr, const CompoundStatement& tblock) : _expr(expr), _tblock(tblock) {}
        inline const Expr& expr() const {return _expr;}
        inline const CompoundStatement& tblock() const {return _tblock;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Expr& _expr;
        const CompoundStatement& _tblock;
    };

    class IfElseStatement : public Statement {
    public:
        inline IfElseStatement(const Expr& expr, const CompoundStatement& tblock, const CompoundStatement& fblock) : _expr(expr), _tblock(tblock), _fblock(fblock) {}
        inline const Expr& expr() const {return _expr;}
        inline const CompoundStatement& tblock() const {return _tblock;}
        inline const CompoundStatement& fblock() const {return _fblock;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Expr& _expr;
        const CompoundStatement& _tblock;
        const CompoundStatement& _fblock;
    };

    class LoopStatement : public Statement {
    protected:
        inline LoopStatement(const Expr& expr, const CompoundStatement& block) : _expr(expr), _block(block) {}
    public:
        inline const Expr& expr() const {return _expr;}
        inline const CompoundStatement& block() const {return _block;}
    private:
        const Expr& _expr;
        const CompoundStatement& _block;
    };

    class WhileStatement : public LoopStatement {
    public:
        inline WhileStatement(const Expr& expr, const CompoundStatement& block) : LoopStatement(expr, block) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class DoWhileStatement : public LoopStatement {
    public:
        inline DoWhileStatement(const Expr& expr, const CompoundStatement& block) : LoopStatement(expr, block) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class ForStatement : public LoopStatement {
    protected:
        inline ForStatement(const Expr& expr, const Expr& incr, const CompoundStatement& block) : LoopStatement(expr, block), _incr(incr) {}
    public:
        inline const Expr& incr() const {return _incr;}
    private:
        const Expr& _incr;
    };

    class ForExprStatement : public ForStatement {
    public:
        inline ForExprStatement(const Expr& init, const Expr& expr, const Expr& incr, const CompoundStatement& block) : ForStatement(expr, incr, block), _init(init) {}
        inline const Expr& init() const {return _init;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Expr& _init;
    };

    class ForInitStatement : public ForStatement {
    public:
        inline ForInitStatement(const VariableDefn& init, const Expr& expr, const Expr& incr, const CompoundStatement& block) : ForStatement(expr, incr, block), _init(init) {}
        inline const VariableDefn& init() const {return _init;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const VariableDefn& _init;
    };

    class ForeachStatement : public Statement {
    protected:
        inline ForeachStatement(const Expr& expr) : _expr(expr), _block(0) {}
    public:
        inline const Expr& expr() const {return _expr;}
        inline const CompoundStatement& block() const {return ref(_block);}
        inline void setBlock(const CompoundStatement& val) {_block = ptr(val);}
    private:
        const Expr& _expr;
        const CompoundStatement* _block;
    };

    class ForeachListStatement : public ForeachStatement {
    public:
        inline ForeachListStatement(const Ast::VariableDefn& valDef, const Expr& expr) : ForeachStatement(expr), _valDef(valDef) {}
        inline const VariableDefn& valDef() const {return _valDef;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const VariableDefn& _valDef;
    };

    class ForeachDictStatement : public ForeachStatement {
    public:
        inline ForeachDictStatement(const Ast::VariableDefn& keyDef, const Ast::VariableDefn& valDef, const Expr& expr) : ForeachStatement(expr), _keyDef(keyDef), _valDef(valDef) {}
        inline const VariableDefn& keyDef() const {return _keyDef;}
        inline const VariableDefn& valDef() const {return _valDef;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const VariableDefn& _keyDef;
        const VariableDefn& _valDef;
    };


    class CaseStatement : public Statement {
    protected:
        inline CaseStatement(const CompoundStatement& block) : _block(block) {}
    public:
        inline const CompoundStatement& block() const {return _block;}
    private:
        const CompoundStatement& _block;
    };

    class CaseExprStatement : public CaseStatement {
    public:
        inline CaseExprStatement(const Expr& expr, const CompoundStatement& block) : CaseStatement(block), _expr(expr) {}
    public:
        inline const Expr& expr() const {return _expr;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Expr& _expr;
    };

    class CaseDefaultStatement : public CaseStatement {
    public:
        inline CaseDefaultStatement(const CompoundStatement& block) : CaseStatement(block) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class SwitchStatement : public Statement {
    protected:
        inline SwitchStatement(const CompoundStatement& block) : _block(block) {}
    public:
        inline const CompoundStatement& block() const {return _block;}
    private:
        const CompoundStatement& _block;
    };

    class SwitchValueStatement : public SwitchStatement {
    public:
        inline SwitchValueStatement(const Expr& expr, const CompoundStatement& block) : SwitchStatement(block), _expr(expr) {}
        inline const Expr& expr() const {return _expr;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Expr& _expr;
    };

    class SwitchExprStatement : public SwitchStatement {
    public:
        inline SwitchExprStatement(const CompoundStatement& block) : SwitchStatement(block) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class BreakStatement : public Statement {
    public:
        inline BreakStatement() {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class ContinueStatement : public Statement {
    public:
        inline ContinueStatement() {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class AddEventHandlerStatement : public Statement {
    public:
        inline AddEventHandlerStatement(const Ast::EventDecl& event, const Ast::Expr& source, Ast::FunctionTypeInstanceExpr& functor) : _event(event), _source(source), _functor(functor) {}
    public:
        inline const Ast::EventDecl& event() const {return _event;}
        inline const Ast::Expr& source() const {return _source;}
        inline Ast::FunctionTypeInstanceExpr& functor() const {return _functor;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::EventDecl& _event;
        const Ast::Expr& _source;
        Ast::FunctionTypeInstanceExpr& _functor;
    };

    class ReturnStatement : public Statement {
    protected:
        inline ReturnStatement(const ExprList& exprList) : _exprList(exprList) {}
    public:
        inline const ExprList& exprList() const {return _exprList;}
    private:
        const ExprList& _exprList;
    };

    class RoutineReturnStatement : public ReturnStatement {
    public:
        inline RoutineReturnStatement(const ExprList& exprList) : ReturnStatement(exprList) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class FunctionReturnStatement : public ReturnStatement {
    public:
        inline FunctionReturnStatement(const ExprList& exprList) : ReturnStatement(exprList) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class CompoundStatement : public Statement {
    public:
        typedef std::list<const Statement*> List;
    public:
        inline CompoundStatement() {}
        inline CompoundStatement& addStatement(const Statement& statement) {_list.push_back(ptr(statement)); return ref(this);}
        inline const List& list() const {return _list;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        List _list;
    };

    struct Statement::Visitor {
        inline void visitNode(const Statement& node) {
            node.visit(ref(this));
        }

        virtual void visit(const ImportStatement& node) = 0;
        virtual void visit(const NamespaceStatement& node) = 0;
        virtual void visit(const LeaveNamespaceStatement& node) = 0;
        virtual void visit(const UserDefinedTypeSpecStatement& node) = 0;
        virtual void visit(const StructMemberVariableStatement& node) = 0;
        virtual void visit(const StructInitStatement& node) = 0;
        virtual void visit(const AutoStatement& node) = 0;
        virtual void visit(const ExprStatement& node) = 0;
        virtual void visit(const PrintStatement& node) = 0;
        virtual void visit(const IfStatement& node) = 0;
        virtual void visit(const IfElseStatement& node) = 0;
        virtual void visit(const WhileStatement& node) = 0;
        virtual void visit(const DoWhileStatement& node) = 0;
        virtual void visit(const ForExprStatement& node) = 0;
        virtual void visit(const ForInitStatement& node) = 0;
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

    inline void ImportStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void NamespaceStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void LeaveNamespaceStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void UserDefinedTypeSpecStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void StructMemberVariableStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void StructInitStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void AutoStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ExprStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void PrintStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void IfStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void IfElseStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void WhileStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void DoWhileStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ForExprStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ForInitStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ForeachListStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ForeachDictStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void CaseExprStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void CaseDefaultStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void SwitchValueStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void SwitchExprStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void BreakStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ContinueStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void AddEventHandlerStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void RoutineReturnStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void FunctionReturnStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void CompoundStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}

    //////////////////////////////////////////////////////////////////
    class Body : public Node {
    public:
        struct Visitor;
    protected:
        inline Body(const CompoundStatement& block) : _block(block) {}
    public:
        inline const CompoundStatement& block() const {return _block;}
    public:
        virtual void visit(Visitor& visitor) const = 0;
    private:
        const CompoundStatement& _block;
    };

    class RoutineBody : public Body {
    public:
        inline RoutineBody(const Routine& routine, const CompoundStatement& block) : Body(block), _routine(routine) {}
        inline const Routine& routine() const {return _routine;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Routine& _routine;
    };

    class FunctionBody : public Body {
    public:
        inline FunctionBody(const Function& function, const CompoundStatement& block) : Body(block), _function(function) {}
        inline const Function& function() const {return _function;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Function& _function;
    };

    struct Body::Visitor {
        inline void visitNode(const Body& node) {
            node.visit(ref(this));
        }

        virtual void visit(const RoutineBody& node) = 0;
        virtual void visit(const FunctionBody& node) = 0;
    };

    inline void RoutineBody::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void FunctionBody::visit(Visitor& visitor) const {visitor.visit(ref(this));}

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    /*! \brief AST Node for a compilation unit
      The Unit AST node is the owner for all AST nodes in the unit.
      This node maintains two namespace hierarchies
      - the root namespace is the namespace for all types defined in this unit
      - the import namespace is the namespace for all types imported into the unit from other modules.
    */
    class Unit {
    public:
        typedef std::list<const Body*> BodyList;
        typedef std::list<const Statement*> StatementList;
        typedef std::list<const Ast::CoerceList*> CoerceListList;
        typedef std::map<const Ast::TypeSpec*, const Ast::Expr*> DefaultValueList;
        typedef std::list<Token> NsPartList;
        typedef std::map<std::string, int> HeaderFileList;

    public:
        inline Unit(const std::string& filename) : _filename(filename), _importNS("*import*"), _rootNS("*root*") {}
    private:
        inline Unit(const Unit& src) : _filename(src._filename), _importNS("*import*"), _rootNS("*root*") {}

    public:
        /// \brief Return the filename
        /// \return The filename
        inline const std::string& filename() const {return _filename;}

    public:
        /// \brief Return the header file list
        /// \return The header file list
        inline const HeaderFileList& headerFileList() const {return _headerFileList;}

        /// \brief Add a header file to the unit
        /// \param list the header file to add
        inline void addheaderFile(const std::string& filename) {_headerFileList[filename]++;}

    public:
        /// \brief Return the statement list
        /// \return The statement list
        inline const StatementList& statementList() const {return _statementList;}

        /// \brief Return the function implementation list
        /// \return The function implementation list
        inline const BodyList& bodyList() const {return _bodyList;}

        /// \brief Return the coercion list
        /// \return The coercion list
        inline const CoerceListList& coercionList() const {return _coerceListList;}

        /// \brief Return the default value list
        /// \return The default value  list
        inline const DefaultValueList& defaultValueList() const {return _defaultValueList;}

    public:
        /// \brief Return the root namespace
        /// \return The root namespace
        inline Root& rootNS() {return _rootNS;}
        inline const Root& rootNS() const {return _rootNS;}

    public:
        /// \brief Return the import namespace
        /// \return The import namespace
        inline Root& importNS() {return _importNS;}

    public:
        /// \brief Add a namespace part to the unit
        /// \param part NS part to add
        inline void addNamespacePart(const Token& part) {_nsPartList.push_back(part);}

        /// \brief Return the namespace part list
        /// \return The namespace part list
        inline const NsPartList& nsPartList() const {return _nsPartList;}

    public:
        /// \brief Add a import statement to the unit
        /// \param statement A pointer to the node to add
        inline void addStatement(const Statement& statement) {
            _statementList.push_back(ptr(statement));
        }

        /// \brief Add a function implementation to the unit
        /// \param functionDefnBase the function implementation to add
        inline void addBody(const Body& body) {_bodyList.push_back(ptr(body));}

        /// \brief Add a coercion list to the unit
        /// \param list the coercion list to add
        inline void addCoercionList(const CoerceList& list) {_coerceListList.push_back(ptr(list));}

        /// \brief Add a default value to the unit
        /// \param typeSpec the typeSpec to add
        /// \param expr the expr to add
        inline void addDefaultValue(const TypeSpec& typeSpec, const Expr& expr) {_defaultValueList[ptr(typeSpec)] = ptr(expr);}

    public:
        /// \brief Add an AST node to the unit
        /// \param node A pointer to the node to add
        /// \return A reference to the newly added node
        template<typename T>
        inline T& addNode(T* node) {_nodeList.push_back(node); return ref(node);}

    private:
        /// \brief Unit Filename
        const std::string _filename;

        /// \brief Unit Unit namespace
        NsPartList _nsPartList;

    private:
        /// \brief This NS contains all imported typespec's.
        /// It is not used for source file generation, only for reference.
        Ast::Root _importNS;

        /// \brief This NS contains all types defined in the current compilation unit.
        Ast::Root _rootNS;

    private:
        /// \brief The list of all import statements in this unit
        StatementList _statementList;

        /// \brief The list of all function implementations in this unit
        BodyList _bodyList;

    private:
        /// \brief The owner list of all nodes in this unit
        std::list<const Node*> _nodeList;

        /// \brief The coercion list for all types in this unit
        CoerceListList _coerceListList;

        /// \brief The list of idefault values for types in this unit
        DefaultValueList _defaultValueList;

        /// \brief The list of header files imported into this unit
        HeaderFileList _headerFileList;
    };

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
        typedef std::list<std::string> PathList;
    public:
        inline Config(const std::string& name) : _name(name), _mode(Mode::Executable), _gui(false), _debug(true), _test(true), _zlibPath("../../zenlang/lib") {}
    public:
        inline const std::string& name() const {return _name;}
    public:
        inline Config& mode(const Mode::T& val) { _mode = val; return ref(this);}
        inline const Mode::T& mode() const {return _mode;}
    public:
        inline Config& gui(const bool& val) { _gui = val; return ref(this);}
        inline const bool& gui() const {return _gui;}
        inline Config& debug(const bool& val) { _debug = val; return ref(this);}
        inline const bool& debug() const {return _debug;}
        inline Config& test(const bool& val) { _test = val; return ref(this);}
        inline const bool& test() const {return _test;}
    public:
        inline Config& zexePath(const std::string& val) { _zexePath = val; return ref(this);}
        inline const std::string& zexePath() const {return _zexePath;}
    public:
        inline Config& zlibPath(const std::string& val) { _zlibPath = val; return ref(this);}
        inline const std::string& zlibPath() const {return _zlibPath;}
    public:
        inline Config& addIncludePath(const std::string& dir) { _includePathList.push_back(dir); return ref(this);}
        inline const PathList& includePathList() const {return _includePathList;}
    public:
        inline Config& addIncludeFile(const std::string& file) { _includeFileList.push_back(file); return ref(this);}
        inline const PathList& includeFileList() const {return _includeFileList;}
    public:
        inline Config& addSourceFile(const std::string& file) { _sourceFileList.push_back(file); return ref(this);}
        inline const PathList& sourceFileList() const {return _sourceFileList;}
    private:
        const std::string _name;
        Mode::T _mode;
        bool _gui;
        bool _debug;
        bool _test;
        std::string _zexePath;
        std::string _zlibPath;
        PathList _includePathList;
        PathList _includeFileList;
        PathList _sourceFileList;
    };

    class Project {
    public:
        typedef std::map<std::string, Config*> ConfigList;

    public:
        inline Project() : _name("main"), _hppExt(".h;.hpp;"), _cppExt(".c;.cpp;"), _zppExt(".zpp;"), _verbose(0) {}
    public:
        inline Project& name(const std::string& val) { _name = val; return ref(this);}
        inline const std::string& name() const {return _name;}
    public:
        inline Project& verbose(const int& val) { _verbose = val; return ref(this);}
        inline const int& verbose() const {return _verbose;}
    public:
        inline Config& config(const std::string& name) {
            ConfigList::iterator it = _configList.find(name);
            if(it == _configList.end()) {
                throw Exception("Config does not exist");
            }
            return ref(it->second);
        }

        inline Config& addConfig(const std::string& name) {
            ConfigList::iterator it = _configList.find(name);
            if(it != _configList.end()) {
                throw Exception("Config already exists");
            }
            _configList[name] = new Config(name);
            return ref(_configList[name]);
        }

    public:
        inline const ConfigList& configList() const {return _configList;}
    public:
        inline const std::string& hppExt() const {return _hppExt;}
        inline const std::string& cppExt() const {return _cppExt;}
        inline const std::string& zppExt() const {return _zppExt;}
    private:
        std::string _name;
        ConfigList _configList;
    private:
        std::string _hppExt;
        std::string _cppExt;
        std::string _zppExt;
        int _verbose;
    };
}
