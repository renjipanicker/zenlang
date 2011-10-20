#pragma once
#include "token.hpp"

namespace Ast {
    /// \brief Outer struct for AccessType enumeration.
    struct AccessType {
        /// \brief The access type for any user-defined TypeSpec.
        enum T {
            Private,     /// TypeSpec is visible only within current compilation unit
            Internal,    /// TypeSpec is externally visible only as a reference
            Protected,   /// TypeSpec is exposed as a pimpl type.
            Public,      /// TypeSpec is fully exposed externally
            Export,      /// unused for now, to be used for dllexport later.
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

    /// \brief Outer struct for ImportType enumeration.
    struct HeaderType {
        /// \brief The import type for any header file.
        enum T {
            Import,     /// import system file
            Include     /// include application file
        };
    };

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
    class TypeSpec : public Node {
    public:
        struct Visitor;
    private:
        typedef std::map<std::string, TypeSpec*> ChildTypeSpecList;
    public:
        inline TypeSpec(const Token& name) : _accessType(AccessType::Parent), _name(name) {}
    public:
        inline TypeSpec& accessType(const AccessType::T& val) {_accessType = val; return ref(this);}
        inline const Token& name() const {return _name;}
        inline const AccessType::T& accessType() const {return _accessType;}
    public:
        inline void addChild(TypeSpec& typeSpec) {_childTypeSpecList[typeSpec.name().text()] = ptr(typeSpec);}
        inline const TypeSpec* hasChild(const std::string& name) const {
            ChildTypeSpecList::const_iterator it = _childTypeSpecList.find(name);
            if(it == _childTypeSpecList.end())
                return 0;
            return it->second;
        }

    public:
        virtual void visit(Visitor& visitor) = 0;
    private:
        AccessType::T _accessType;
        const Token _name;
    private:
        ChildTypeSpecList _childTypeSpecList;
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

    class VariableDef : public Node {
    public:
        inline VariableDef(const QualifiedTypeSpec& qualifiedTypeSpec, const Token& name) : _qualifiedTypeSpec(qualifiedTypeSpec), _name(name) {}
        inline const QualifiedTypeSpec& qualifiedTypeSpec() const {return _qualifiedTypeSpec;}
        inline const Token& name() const {return _name;}
    private:
        const QualifiedTypeSpec& _qualifiedTypeSpec;
        const Token _name;
    };

    class VariableDefList : public Node {
    public:
        typedef std::list<const VariableDef*> List;
    public:
        inline VariableDefList() {}
        inline VariableDefList& addVariableDef(const VariableDef& variableDef) {_list.push_back(ptr(variableDef)); return ref(this);}
        inline const List& list() const {return _list;}
    private:
        List _list;
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

    class TypeDef : public UserDefinedTypeSpec {
    public:
        inline TypeDef(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType) {}
    private:
        virtual void visit(Visitor& visitor);
    };

    class EnumMemberDef : public Node {
    public:
        inline EnumMemberDef(const Token& name) : _name(name) {}
        inline const Token& name() const {return _name;}
    private:
        const Token _name;
    };

    class EnumMemberDefList : public Node {
    public:
        typedef std::list<const EnumMemberDef*> List;
    public:
        inline EnumMemberDefList() {}
        inline EnumMemberDefList& addEnumMemberDef(const EnumMemberDef& enumMemberDef) {_list.push_back(ptr(enumMemberDef)); return ref(this);}
        inline const List& list() const {return _list;}
    private:
        List _list;
    };

    class EnumDef : public UserDefinedTypeSpec {
    public:
        inline EnumDef(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const EnumMemberDefList& list) : UserDefinedTypeSpec(parent, name, defType), _list(list) {}
        inline const EnumMemberDefList::List& list() const {return _list.list();}
    private:
        virtual void visit(Visitor& visitor);
    private:
        const EnumMemberDefList& _list;
    };

    class StructDef : public UserDefinedTypeSpec {
    public:
        inline StructDef(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const Ast::VariableDefList& list) : UserDefinedTypeSpec(parent, name, defType), _list(list) {}
        inline const Ast::VariableDefList::List& list() const {return _list.list();}
    private:
        virtual void visit(Visitor& visitor);
    private:
        const Ast::VariableDefList& _list;
    };

    class RoutineDef : public UserDefinedTypeSpec {
    public:
        inline RoutineDef(const TypeSpec& parent, const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::VariableDefList& in, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType), _outType(outType), _in(in) {}
        inline const Ast::QualifiedTypeSpec& outType() const {return _outType;}
        inline const Ast::VariableDefList::List& in()  const {return _in.list();}
    private:
        virtual void visit(Visitor& visitor);
    private:
        const Ast::QualifiedTypeSpec& _outType;
        const Ast::VariableDefList& _in;
    };

    class FunctionDef : public UserDefinedTypeSpec {
    public:
        inline FunctionDef(const TypeSpec& parent, const Ast::VariableDefList& out, const Ast::Token& name, const Ast::VariableDefList& in, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType), _out(out), _in(in) {}
        inline const Ast::VariableDefList::List& out() const {return _out.list();}
        inline const Ast::VariableDefList::List& in()  const {return _in.list();}
    private:
        virtual void visit(Visitor& visitor);
    private:
        const Ast::VariableDefList& _out;
        const Ast::VariableDefList& _in;
    };

    class EventDef : public UserDefinedTypeSpec {
    public:
        inline EventDef(const TypeSpec& parent, const Ast::Token& name, const Ast::VariableDef& in, const FunctionDef& functionDef, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType), _in(in), _functionDef(functionDef) {}
        inline const Ast::VariableDef& in()  const {return _in;}
        inline const Ast::FunctionDef& functionDef() const {return _functionDef;}
    private:
        virtual void visit(Visitor& visitor);
    private:
        const Ast::VariableDef& _in;
        const FunctionDef& _functionDef;
    };

    class Namespace : public ChildTypeSpec {
    public:
        inline Namespace(const TypeSpec& parent, const Token& name) : ChildTypeSpec(parent, name) {}
    private:
        virtual void visit(Visitor& visitor);
    };

    class Root : public RootTypeSpec {
    public:
        inline Root(const std::string& name) : RootTypeSpec(Token(0, 0, name)) {}
    private:
        virtual void visit(Visitor& visitor);
    };

    struct TypeSpec::Visitor {
        virtual void visit(const TypeDef& node) = 0;
        virtual void visit(const EnumDef& node) = 0;
        virtual void visit(const StructDef& node) = 0;
        virtual void visit(const RoutineDef& node) = 0;
        virtual void visit(const FunctionDef& node) = 0;
        virtual void visit(const EventDef& node) = 0;
        virtual void visit(const Namespace& node) = 0;
        virtual void visit(const Root& node) = 0;
    };

    inline void TypeDef::visit(Visitor& visitor) {visitor.visit(ref(this));}
    inline void EnumDef::visit(Visitor& visitor) {visitor.visit(ref(this));}
    inline void StructDef::visit(Visitor& visitor) {visitor.visit(ref(this));}
    inline void RoutineDef::visit(Visitor& visitor) {visitor.visit(ref(this));}
    inline void FunctionDef::visit(Visitor& visitor) {visitor.visit(ref(this));}
    inline void EventDef::visit(Visitor& visitor) {visitor.visit(ref(this));}
    inline void Namespace::visit(Visitor& visitor) {visitor.visit(ref(this));}
    inline void Root::visit(Visitor& visitor) {visitor.visit(ref(this));}

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    class Expr : public Node {
    protected:
        inline Expr(const TypeSpec& typeSpec) : _typeSpec(typeSpec) {}
    public:
        inline const TypeSpec& typeSpec() const {return _typeSpec;}
    private:
        const TypeSpec& _typeSpec;
    };

    class TernaryOpExpr : public Expr {
    public:
        inline TernaryOpExpr(const TypeSpec& typeSpec, const Token& op, const Expr& lhs, const Expr& rhs1, const Expr& rhs2) : Expr(typeSpec), _op(op), _lhs(lhs), _rhs1(rhs1), _rhs2(rhs2) {}
    private:
        const Token _op;
        const Expr& _lhs;
        const Expr& _rhs1;
        const Expr& _rhs2;
    };

    class BinaryOpExpr : public Expr {
    public:
        inline BinaryOpExpr(const TypeSpec& typeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : Expr(typeSpec), _op(op), _lhs(lhs), _rhs(rhs) {}
    private:
        const Token _op;
        const Expr& _lhs;
        const Expr& _rhs;
    };

    class PostfixOpExpr : public Expr {
    public:
        inline PostfixOpExpr(const TypeSpec& typeSpec, const Token& op, const Expr& lhs) : Expr(typeSpec), _op(op), _lhs(lhs) {}
    private:
        const Token _op;
        const Expr& _lhs;
    };

    class PrefixOpExpr : public Expr {
    public:
        inline PrefixOpExpr(const TypeSpec& typeSpec, const Token& op, const Expr& rhs) : Expr(typeSpec), _op(op), _rhs(rhs) {}
    private:
        const Token _op;
        const Expr& _rhs;
    };

    class StructMemberRefExpr : public Expr {
    public:
        inline StructMemberRefExpr(const TypeSpec& typeSpec, const StructDef& structDef, const Token& name) : Expr(typeSpec), _structDef(structDef), _name(name) {}
    private:
        const StructDef& _structDef;
        const Token _name;
    };

    class EnumMemberRefExpr : public Expr {
    public:
        inline EnumMemberRefExpr(const TypeSpec& typeSpec, const EnumDef& enumDef, const Token& name) : Expr(typeSpec), _enumDef(enumDef), _name(name) {}
    private:
        const EnumDef& _enumDef;
        const Token _name;
    };

    class ConstantExpr : public Expr {
    public:
        inline ConstantExpr(const TypeSpec& typeSpec, const Token& value) : Expr(typeSpec), _value(value) {}
    private:
        const Token _value;
    };

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    class Statement : public Node {
    public:
        inline Statement() {}
    };

    class ImportStatement : public Statement {
    public:
        typedef std::list<Token> Part;
    public:
        inline ImportStatement() : _headerType(HeaderType::Include), _defType(DefinitionType::Direct) {}
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
        Part _part;
        HeaderType::T _headerType;
        DefinitionType::T _defType;
    };

    class UserDefinedTypeSpecStatement : public Statement {
    public:
        inline UserDefinedTypeSpecStatement(const UserDefinedTypeSpec& typeSpec) : _typeSpec(typeSpec) {}
        inline const UserDefinedTypeSpec& typeSpec() const {return _typeSpec;}
    private:
        const UserDefinedTypeSpec& _typeSpec;
    };

    class ExprStatement : public Statement {
    public:
        inline ExprStatement(const Expr& expr) : _expr(expr) {}
        inline const Expr& expr() const {return _expr;}
    private:
        const Expr& _expr;
    };

    class CompoundStatement : public Statement {
    public:
        typedef std::list<const Statement*> List;
    public:
        inline CompoundStatement() {}
        inline CompoundStatement& addStatement(const Statement& statement) {_list.push_back(ptr(statement)); return ref(this);}
        inline const List& list() const {return _list;}
    private:
        List _list;
    };

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
        typedef std::list<const ImportStatement*> ImportStatementList;
        typedef std::list<const Statement*> StatementList;
        typedef std::list<Token> UnitNS;
    public:
        inline Unit(const std::string& filename) : _filename(filename), _importNS("*import*"), _rootNS("*root*") {}
    private:
        inline Unit(const Unit& src) : _filename(src._filename), _importNS("*import*"), _rootNS("*root*") {}

    public:
        /// \brief Return the filename
        /// \return The filename
        inline const std::string& filename() const {return _filename;}

    public:
        /// \brief Return the import statement list
        /// \return The import statement list
        inline const ImportStatementList& importStatementList() const {return _importStatementList;}

    public:
        /// \brief Return the global statement list
        /// \return The global statement list
        inline const StatementList& globalStatementList() const {return _globalStatementList;}

    public:
        /// \brief Return the root namespace
        /// \return The root namespace
        inline Root& rootNS() {return _rootNS;}

    public:
        /// \brief Return the import namespace
        /// \return The import namespace
        inline Root& importNS() {return _importNS;}

    public:
        /// \brief Return the namespace list
        /// \return The namespace list
        inline const UnitNS& unitNS() const {return _unitNS;}

    public:
        /// \brief Add a namespace to the unit
        /// \param name Namespace name
        inline void addNamespace(const Token& name) {_unitNS.push_back(name);}

    public:
        /// \brief Add a global statement to the unit
        /// \param statement A pointer to the node to add
        inline void addGlobalStatement(const Statement& statement) {_globalStatementList.push_back(ptr(statement));}

    public:
        /// \brief Add a import statement to the unit
        /// \param statement A pointer to the node to add
        inline void addImportStatement(const ImportStatement& statement) {_importStatementList.push_back(ptr(statement));}

    public:
        /// \brief Add an AST node to the unit
        /// \param node A pointer to the node to add
        /// \return A reference to the newly added node
        template<typename T>
        inline T& addNode(T* node) {_nodeList.push_back(node); return ref(node);}

    private:
        /// \brief Unit Filename
        const std::string& _filename;

    private:
        /// \brief This NS contains all imported typespec's.
        /// It is not used for source file generation, only for reference.
        Ast::Root _importNS;

        /// \brief This NS contains all types defined in the current compilation unit.
        Ast::Root _rootNS;

        /// \brief The namespace of the current unit.
        UnitNS _unitNS;

    private:
        /// \brief The list of all import statements in this unit
        ImportStatementList _importStatementList;

    private:
        /// \brief The list of all global statements in this unit
        StatementList _globalStatementList;

    private:
        /// \brief The owner list of all nodes in this unit
        std::list<const Node*> _nodeList;
    };
}
