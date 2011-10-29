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
        typedef std::list<TypeSpec*> ChildTypeSpecList;
        typedef std::map<std::string, TypeSpec*> ChildTypeSpecMap;
    public:
        inline TypeSpec(const Token& name) : _accessType(AccessType::Parent), _name(name) {}
    public:
        inline TypeSpec& accessType(const AccessType::T& val) {_accessType = val; return ref(this);}
        inline const Token& name() const {return _name;}
        inline const AccessType::T& accessType() const {return _accessType;}
        inline const ChildTypeSpecList& childTypeSpecList() const {return _childTypeSpecList;}
    public:
        inline void addChild(TypeSpec& typeSpec) {
            _childTypeSpecList.push_back(ptr(typeSpec));
            _childTypeSpecMap[typeSpec.name().text()] = ptr(typeSpec);
        }

        inline const TypeSpec* hasChild(const std::string& name) const {
            ChildTypeSpecMap::const_iterator it = _childTypeSpecMap.find(name);
            if(it == _childTypeSpecMap.end())
                return 0;
            return it->second;
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

    class VariableDefn : public Node {
    public:
        inline VariableDefn(const QualifiedTypeSpec& qualifiedTypeSpec, const Token& name) : _qualifiedTypeSpec(qualifiedTypeSpec), _name(name) {}
        inline const QualifiedTypeSpec& qualifiedTypeSpec() const {return _qualifiedTypeSpec;}
        inline const Token& name() const {return _name;}
    private:
        const QualifiedTypeSpec& _qualifiedTypeSpec;
        const Token _name;
    };

    class Scope : public Node {
    public:
        typedef std::list<const VariableDefn*> List;
    public:
        inline Scope() {}
        inline Scope& addVariableDef(const VariableDefn& variableDef) {_list.push_back(ptr(variableDef)); return ref(this);}
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

    class TypedefDefn : public UserDefinedTypeSpec {
    public:
        inline TypedefDefn(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class EnumMemberDefn : public Node {
    public:
        inline EnumMemberDefn(const Token& name) : _name(name) {}
        inline const Token& name() const {return _name;}
    private:
        const Token _name;
    };

    class EnumMemberDefnList : public Node {
    public:
        typedef std::list<const EnumMemberDefn*> List;
    public:
        inline EnumMemberDefnList() {}
        inline EnumMemberDefnList& addEnumMemberDef(const EnumMemberDefn& enumMemberDef) {_list.push_back(ptr(enumMemberDef)); return ref(this);}
        inline const List& list() const {return _list;}
    private:
        List _list;
    };

    class EnumDefn : public UserDefinedTypeSpec {
    public:
        inline EnumDefn(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const EnumMemberDefnList& list) : UserDefinedTypeSpec(parent, name, defType), _list(list) {}
        inline const EnumMemberDefnList::List& list() const {return _list.list();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const EnumMemberDefnList& _list;
    };

    class StructDefn : public UserDefinedTypeSpec {
    public:
        inline StructDefn(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const Ast::Scope& list) : UserDefinedTypeSpec(parent, name, defType), _list(list) {}
        inline const Ast::Scope::List& list() const {return _list.list();}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::Scope& _list;
    };

    class Routine : public UserDefinedTypeSpec {
    protected:
        inline Routine(const TypeSpec& parent, const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::Scope& in, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType), _outType(outType), _in(in) {}
    public:
        inline const Ast::QualifiedTypeSpec& outType() const {return _outType;}
        inline const Ast::Scope::List& in()  const {return _in.list();}
    private:
        const Ast::QualifiedTypeSpec& _outType;
        const Ast::Scope& _in;
    };

    class RoutineDecl : public Routine {
    public:
        inline RoutineDecl(const TypeSpec& parent, const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::Scope& in, const DefinitionType::T& defType) : Routine(parent, outType, name, in, defType) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class CompoundStatement;
    class RoutineDefn : public Routine {
    public:
        inline RoutineDefn(const TypeSpec& parent, const Ast::QualifiedTypeSpec& outType, const Ast::Token& name, const Ast::Scope& in, const DefinitionType::T& defType)
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
        inline const Ast::Scope::List& out() const {return _out.list();}
        inline const Token& name() const {return _name;}
        inline const Ast::Scope::List& in() const {return _in.list();}
        inline Ast::Scope& inScope()  const {return _in;}
    private:
        const Ast::Scope& _out;
        const Token _name;
        Ast::Scope& _in;
    };

    class Function : public UserDefinedTypeSpec {
    public:
        inline Function(const TypeSpec& parent, const Ast::Token& name, const DefinitionType::T& defType, const Ast::FunctionSig& sig) : UserDefinedTypeSpec(parent, name, defType), _sig(sig) {}
        inline const Ast::FunctionSig& sig() const {return _sig;}
    private:
        const Ast::FunctionSig& _sig;
    };

    class FunctionDecl : public Function {
    public:
        inline FunctionDecl(const TypeSpec& parent, const Ast::Token& name, const DefinitionType::T& defType, const Ast::FunctionSig& sig)
            : Function(parent, name, defType, sig) {}
    private:
        virtual void visit(Visitor& visitor) const;
    };

    class FunctionDefn : public Function {
    public:
        inline FunctionDefn(const TypeSpec& parent, const Ast::Token& name, const DefinitionType::T& defType, const Ast::FunctionSig& sig)
            : Function(parent, name, defType, sig), _block(0) {}
        inline const Ast::CompoundStatement& block() const {return ref(_block);}
        inline void setBlock(const Ast::CompoundStatement& block) {_block = ptr(block);}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::CompoundStatement* _block;
    };

    class EventDecl : public UserDefinedTypeSpec {
    public:
        inline EventDecl(const TypeSpec& parent, const Ast::Token& name, const Ast::VariableDefn& in, const FunctionSig& functionSig, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType), _in(in), _functionSig(functionSig) {}
    public:
        inline const Ast::VariableDefn& in()  const {return _in;}
        inline const Ast::FunctionSig& functionSig() const {return _functionSig;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Ast::VariableDefn& _in;
        const FunctionSig& _functionSig;
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

        virtual void visit(const TypedefDefn& node) = 0;
        virtual void visit(const EnumDefn& node) = 0;
        virtual void visit(const StructDefn& node) = 0;
        virtual void visit(const RoutineDecl& node) = 0;
        virtual void visit(const RoutineDefn& node) = 0;
        virtual void visit(const FunctionDecl& node) = 0;
        virtual void visit(const FunctionDefn& node) = 0;
        virtual void visit(const EventDecl& node) = 0;
        virtual void visit(const Namespace& node) = 0;
        virtual void visit(const Root& node) = 0;
    };

    inline void TypedefDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void EnumDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void StructDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void RoutineDecl::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void RoutineDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void FunctionDecl::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void FunctionDefn::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void EventDecl::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void Namespace::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void Root::visit(Visitor& visitor) const {visitor.visit(ref(this));}

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    class Expr : public Node {
    public:
        struct Visitor;
    protected:
        inline Expr(const TypeSpec& typeSpec) : _typeSpec(typeSpec) {}
    public:
        inline const TypeSpec& typeSpec() const {return _typeSpec;}
    public:
        virtual void visit(Visitor& visitor) const = 0;
    private:
        const TypeSpec& _typeSpec;
    };

    class ExprList : public Node {
    public:
        typedef std::list<const Expr*> List;
    public:
        inline ExprList() {}
        inline ExprList& addExpr(const Expr& expr) {_list.push_back(ptr(expr)); return ref(this);}
        inline const List& list() const {return _list;}
    private:
        List _list;
    };

    class TernaryOpExpr : public Expr {
    public:
        inline TernaryOpExpr(const TypeSpec& typeSpec, const Token& op1, const Token& op2, const Expr& lhs, const Expr& rhs1, const Expr& rhs2)
            : Expr(typeSpec), _op1(op1), _op2(op2), _lhs(lhs), _rhs1(rhs1), _rhs2(rhs2) {}
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
        inline BinaryOpExpr(const TypeSpec& typeSpec, const Token& op, const Expr& lhs, const Expr& rhs) : Expr(typeSpec), _op(op), _lhs(lhs), _rhs(rhs) {}
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
        inline PostfixOpExpr(const TypeSpec& typeSpec, const Token& op, const Expr& lhs) : Expr(typeSpec), _op(op), _lhs(lhs) {}
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
        inline PrefixOpExpr(const TypeSpec& typeSpec, const Token& op, const Expr& rhs) : Expr(typeSpec), _op(op), _rhs(rhs) {}
        inline const Token& op() const {return _op;}
        inline const Expr& rhs() const {return _rhs;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Token _op;
        const Expr& _rhs;
    };

    class StructMemberRefExpr : public Expr {
    public:
        inline StructMemberRefExpr(const TypeSpec& typeSpec, const StructDefn& structDef, const Token& name) : Expr(typeSpec), _structDef(structDef), _name(name) {}
        inline const StructDefn& structDef() const {return _structDef;}
        inline const Token& name() const {return _name;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const StructDefn& _structDef;
        const Token _name;
    };

    class EnumMemberRefExpr : public Expr {
    public:
        inline EnumMemberRefExpr(const TypeSpec& typeSpec, const EnumDefn& enumDef, const Token& name) : Expr(typeSpec), _enumDef(enumDef), _name(name) {}
        inline const EnumDefn& enumDef() const {return _enumDef;}
        inline const Token& name() const {return _name;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const EnumDefn& _enumDef;
        const Token _name;
    };

    class OrderedExpr : public Expr {
    public:
        inline OrderedExpr(const TypeSpec& typeSpec, const Expr& expr) : Expr(typeSpec), _expr(expr) {}
        inline const Expr& expr() const {return _expr;}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Expr& _expr;
    };

    class ConstantExpr : public Expr {
    public:
        inline ConstantExpr(const TypeSpec& typeSpec, const Token& value) : Expr(typeSpec), _value(value) {}
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
        virtual void visit(const PostfixOpExpr& node) = 0;
        virtual void visit(const PrefixOpExpr& node) = 0;
        virtual void visit(const StructMemberRefExpr& node) = 0;
        virtual void visit(const EnumMemberRefExpr& node) = 0;
        virtual void visit(const OrderedExpr& node) = 0;
        virtual void visit(const ConstantExpr& node) = 0;
        virtual void sep() = 0;
    };

    inline void TernaryOpExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void BinaryOpExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void PostfixOpExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void PrefixOpExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void StructMemberRefExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void EnumMemberRefExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void OrderedExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ConstantExpr::visit(Visitor& visitor) const {visitor.visit(ref(this));}

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    class Statement : public Node {
    public:
        struct Visitor;
    public:
        inline Statement() {}
    public:
        virtual void visit(Visitor& visitor) const = 0;
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
        virtual void visit(Visitor& visitor) const;
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
        virtual void visit(Visitor& visitor) const;
    private:
        const UserDefinedTypeSpec& _typeSpec;
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
        inline FunctionReturnStatement(const Function& function, const ExprList& exprList) : ReturnStatement(exprList), _function(function) {}
    private:
        virtual void visit(Visitor& visitor) const;
    private:
        const Function& _function;
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
        virtual void visit(const UserDefinedTypeSpecStatement& node) = 0;
        virtual void visit(const ExprStatement& node) = 0;
        virtual void visit(const RoutineReturnStatement& node) = 0;
        virtual void visit(const FunctionReturnStatement& node) = 0;
        virtual void visit(const CompoundStatement& node) = 0;
    };

    inline void ImportStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void UserDefinedTypeSpecStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void ExprStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void RoutineReturnStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void FunctionReturnStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}
    inline void CompoundStatement::visit(Visitor& visitor) const {visitor.visit(ref(this));}

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
        /// \brief Return the root namespace
        /// \return The root namespace
        inline Root& rootNS() {return _rootNS;}
        inline const Root& rootNS() const {return _rootNS;}

    public:
        /// \brief Return the import namespace
        /// \return The import namespace
        inline Root& importNS() {return _importNS;}

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

    private:
        /// \brief The list of all import statements in this unit
        ImportStatementList _importStatementList;

    private:
        /// \brief The owner list of all nodes in this unit
        std::list<const Node*> _nodeList;
    };

    class Config {
    public:
        typedef std::list<std::string> PathList;
    public:
        inline Config(const std::string& name) : _name(name) {}
    public:
        inline const std::string& name() const {return _name;}
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
        PathList _includePathList;
        PathList _includeFileList;
        PathList _sourceFileList;
    };

    class Project {
    public:
        struct Mode {
            enum T {
                Compile,
                Executable,
                Shared,
                Static
            };
        };
        typedef std::list<Config*> ConfigList;

    public:
        inline Project() : _name("main"), _mode(Mode::Executable), _global("global"), _hppExt(".h;.hpp;"), _cppExt(".c;.cpp;"), _zppExt(".zpp;") {}
    public:
        inline Project& name(const std::string& val) { _name = val; return ref(this);}
        inline const std::string& name() const {return _name;}
    public:
        inline Project& mode(const Mode::T& val) { _mode = val; return ref(this);}
        inline const Mode::T& mode() const {return _mode;}
    public:
        inline Config& global() {return _global;}
        inline const Config& global() const {return _global;}
    public:
        inline const ConfigList& configList() const {return _configList;}
    public:
        inline const std::string& hppExt() const {return _hppExt;}
        inline const std::string& cppExt() const {return _cppExt;}
        inline const std::string& zppExt() const {return _zppExt;}
    private:
        std::string _name;
        Mode::T _mode;
        Config _global;
        ConfigList _configList;
    private:
        std::string _hppExt;
        std::string _cppExt;
        std::string _zppExt;
    };
}
