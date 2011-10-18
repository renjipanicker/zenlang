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

    class TypeSpec : public Node {
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
    };

    class StructDef : public UserDefinedTypeSpec {
    public:
        inline StructDef(const TypeSpec& parent, const Token& name, const DefinitionType::T& defType, const Ast::VariableDefList& list) : UserDefinedTypeSpec(parent, name, defType), _list(list) {}
        inline const Ast::VariableDefList::List& list() const {return _list.list();}
    private:
        const Ast::VariableDefList& _list;
    };

    class FunctionDef : public UserDefinedTypeSpec {
    public:
        inline FunctionDef(const TypeSpec& parent, const Ast::VariableDefList& out, const Ast::Token& name, const Ast::VariableDefList& in, const DefinitionType::T& defType) : UserDefinedTypeSpec(parent, name, defType), _out(out), _in(in) {}
    private:
        const Ast::VariableDefList& _out;
        const Ast::VariableDefList& _in;
    };

    class Namespace : public ChildTypeSpec {
    public:
        inline Namespace(const TypeSpec& parent, const Token& name) : ChildTypeSpec(parent, name) {}
    };

    class Root : public RootTypeSpec {
    public:
        inline Root(const std::string& name) : RootTypeSpec(Token(0, 0, name)) {}
    };

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

    /*! \brief AST Node for a compilation unit
      The Unit AST node is the owner for all AST nodes in the unit.
      This node maintains two namespace hierarchies
      - the root namespace is the namespace for all types defined in this unit
      - the import namespace is the namespace for all types imported into the unit from other modules.
    */
    class Unit : public RootTypeSpec {
    public:
        typedef std::list<const ImportStatement*> ImportStatementList;
        typedef std::list<const Statement*> StatementList;
        typedef std::list<Token> UnitNS;
    public:
        inline Unit(const Token& name) : RootTypeSpec(name), _importNS("*import*"), _rootNS("*root*") {}
    private:
        inline Unit(const Unit& src) : RootTypeSpec(src.name()), _importNS("*import*"), _rootNS("*root*") {}

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
