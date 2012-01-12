#pragma once

#include "ast.hpp"
#include "error.hpp"

namespace Ast {
    class Context {
    public:
        typedef std::list<Ast::Namespace*> NamespaceStack;
        typedef std::list<Ast::Scope*> ScopeStack;
        typedef std::list<Ast::TypeSpec*> TypeSpecStack;
        typedef std::map<const Ast::TypeSpec*, const Ast::Expr*> DefaultValueList;
        typedef std::list<const Body*> BodyList;
        typedef std::list<const Ast::CoerceList*> CoerceListList;
        typedef std::list<Token> NsPartList;
        typedef std::map<std::string, int> HeaderFileList;

    public:
        Context(Ast::Unit& unit, const std::string& filename);
        ~Context();

    public:
        inline const std::string& filename() const {return _filename;}

    private:
        Ast::Unit& _unit;
        const std::string _filename;

    public: // everything related to statement-callback
        inline void setStatementVisitor(Ast::Statement::Visitor& val) { _statementVisitor = z::ptr(val);}
        inline bool hasStatementVisitor() {return (0 != _statementVisitor);}
        inline Ast::Statement::Visitor& statementVisitor() {return z::ref(_statementVisitor);}
    private:
        Ast::Statement::Visitor* _statementVisitor;

    public: // everything related to imported header files
        /// \brief Return the header file list
        /// \return The header file list
        inline const HeaderFileList& headerFileList() const {return _headerFileList;}

        /// \brief Add a header file to the unit
        /// \param list the header file to add
        inline void addheaderFile(const std::string& filename) {_headerFileList[filename]++;}

    private:
        /// \brief The list of header files imported into this unit
        HeaderFileList _headerFileList;

    public: // everything related to namespace stack
        inline NamespaceStack& namespaceStack() {return _namespaceStack;}
        inline void addNamespace(Ast::Namespace& ns) {_namespaceStack.push_back(z::ptr(ns));}
        void leaveNamespace();
    private:
        NamespaceStack _namespaceStack;

    public: // everything related to namesace of current unit
        /// \brief Add a namespace part to the unit
        /// \param part NS part to add
        inline void addNamespacePart(const Token& part) {_nsPartList.push_back(part);}

        /// \brief Return the namespace part list
        /// \return The namespace part list
        inline const NsPartList& nsPartList() const {return _nsPartList;}

    private:
        /// \brief Unit Unit namespace
        NsPartList _nsPartList;

    public: // everything related to root namespace
        /// \brief Return the root namespace
        /// \return The root namespace
        inline Root& rootNS() {return _rootNS;}
        inline const Root& rootNS() const {return _rootNS;}

    private:
        /// \brief This NS contains all types defined in the current compilation unit.
        Ast::Root _rootNS;

    public: // everything related to import namespace
        /// \brief Return the import namespace
        /// \return The import namespace
        inline Root& importNS() {return _importNS;}

    private:
        /// \brief This NS contains all imported typespec's.
        /// It is not used for source file generation, only for reference.
        Ast::Root _importNS;

    public: // everything related to default values
        /// \brief Return the default value list
        /// \return The default value  list
        inline const DefaultValueList& defaultValueList() const {return _defaultValueList;}

        /// \brief Add a default value to the unit
        /// \param typeSpec the typeSpec to add
        /// \param expr the expr to add
        inline void addDefaultValue(const TypeSpec& typeSpec, const Expr& expr) {_defaultValueList[z::ptr(typeSpec)] = z::ptr(expr);}

    private:
        /// \brief The list of default values for types in this unit
        DefaultValueList _defaultValueList;

    public: // everything related to type coercion
        struct CoercionResult {
            enum T {
                None,
                Lhs,
                Rhs
            };
        };
    public:
        const Ast::QualifiedTypeSpec* canCoerceX(const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs, CoercionResult::T& mode) const;
        inline const Ast::QualifiedTypeSpec* canCoerce(const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs) const;
        const Ast::QualifiedTypeSpec& coerce(const Ast::Token& pos, const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs);

    public:
        /// \brief Return the coercion list
        /// \return The coercion list
        inline const CoerceListList& coercionList() const {return _coerceListList;}

        /// \brief Add a coercion list to the unit
        /// \param list the coercion list to add
        inline void addCoercionList(const CoerceList& list) {_coerceListList.push_back(z::ptr(list));}

    private:
        /// \brief The coercion list for all types in this unit
        CoerceListList _coerceListList;

    public: // everything related to scope stack
        Ast::Scope& enterScope(Ast::Scope& scope);
        Ast::Scope& leaveScope();
        Ast::Scope& leaveScope(Ast::Scope& scope);
        Ast::Scope& currentScope();
        const Ast::VariableDefn* hasMember(const Ast::Scope& scope, const Ast::Token& name) const;
        const Ast::VariableDefn* getVariableDef(const std::string& filename, const Ast::Token& name, Ast::RefType::T& refType) const;
    private:
        ScopeStack _scopeStack;

    public: // everything related to struct-init stack
        inline void pushStructInit(const Ast::StructDefn& structDefn) {_structInitStack.push_back(z::ptr(structDefn));}
        inline void popStructInit() {_structInitStack.pop_back();}
        inline const Ast::StructDefn* structInit() {if(_structInitStack.size() == 0) return 0; return _structInitStack.back();}
    private:
        typedef std::list<const Ast::StructDefn*> StructInitStack;
        StructInitStack _structInitStack;

    public: // everything related to current typespec
        template <typename T> inline const T* setCurrentRootTypeRef(const int& level, const Ast::Token& name) {
            const T& td = getRootTypeSpec<T>(level, name);
            _currentTypeRef = z::ptr(td);
            _currentImportedTypeRef = hasImportRootTypeSpec(level, name);
            return z::ptr(td);
        }

        template <typename T> inline const T* setCurrentChildTypeRef(const Ast::TypeSpec& parent, const Ast::Token& name, const std::string& extype) {
            if(z::ptr(parent) != _currentTypeRef) {
                throw z::Exception("%s Internal error: %s parent mismatch '%s'\n", err(_filename, name).c_str(), extype.c_str(), name.text());
            }
            const T* td = z::ref(_currentTypeRef).hasChild<const T>(name.string());
            if(td) {
                _currentTypeRef = td;
                if(_currentImportedTypeRef) {
                    const T* itd = z::ref(_currentImportedTypeRef).hasChild<const T>(name.string());
                    if(itd) {
                        _currentImportedTypeRef = itd;
                    } else {
                        _currentImportedTypeRef = 0;
                    }
                }
                return td;
            }

            if(_currentImportedTypeRef) {
                const T* itd = z::ref(_currentImportedTypeRef).hasChild<const T>(name.string());
                if(itd) {
                    _currentImportedTypeRef = 0;
                    _currentTypeRef = itd;
                    return itd;
                } else {
                    _currentImportedTypeRef = 0;
                }
            }

            throw z::Exception("%s %s type expected '%s'\n", err(_filename, name).c_str(), extype.c_str(), name.text());
        }

        template <typename T> inline const T* resetCurrentTypeRef(const T& typeSpec) {
            _currentTypeRef = 0;
            return z::ptr(typeSpec);
        }

        const Ast::TypeSpec* currentTypeRefHasChild(const Ast::Token& name) const;
    private:
        const Ast::TypeSpec* _currentTypeRef;
        const Ast::TypeSpec* _currentImportedTypeRef;

    public: // everything related to typespec-stack
        Ast::Root& getRootNamespace(const int& level);
        const Ast::TypeSpec* hasRootTypeSpec(const int& level, const Ast::Token& name) const;
        inline TypeSpecStack& typeSpecStack() {return _typeSpecStack;}
        inline const TypeSpecStack& typeSpecStack() const {return _typeSpecStack;}

    public:
        template <typename T> const T& getRootTypeSpec(const int& level, const Ast::Token &name) const {
            const Ast::TypeSpec* typeSpec = hasRootTypeSpec(level, name);
            if(!typeSpec) {
                throw z::Exception("%s Unknown root type '%s'\n", err(_filename, name).c_str(), name.text());
            }
            const T* tTypeSpec = dynamic_cast<const T*>(typeSpec);
            if(!tTypeSpec) {
                throw z::Exception("%s Type mismatch '%s'\n", err(_filename, name).c_str(), name.text());
            }
            return z::ref(tTypeSpec);
        }
        inline const Ast::TypeSpec* findTypeSpec(const Ast::TypeSpec& parent, const Ast::Token& name) const;

    public:
        Ast::TypeSpec& currentTypeSpec() const;
        Ast::TypeSpec& enterTypeSpec(Ast::TypeSpec& typeSpec);
        Ast::TypeSpec& leaveTypeSpec(Ast::TypeSpec& typeSpec);
        Ast::StructDefn& getCurrentStructDefn(const Ast::Token& pos);

    private:
        const Ast::TypeSpec* hasImportRootTypeSpec(const int& level, const Ast::Token& name) const;
    private:
        TypeSpecStack _typeSpecStack;

    public: // everything related to expected typespec
        struct ExpectedTypeSpec {
            enum Type {
                etAuto,
                etVarArg,
                etCallArg,
                etListVal,
                etDictKey,
                etDictVal,
                etAssignment,
                etEventHandler,
                etStructInit
            };

            typedef std::vector<const Ast::QualifiedTypeSpec*> List;

            inline ExpectedTypeSpec(const Type& type, const Ast::QualifiedTypeSpec* typeSpec) : _type(type), _typeSpec(typeSpec) {}
            inline ExpectedTypeSpec(const Type& type) : _type(type), _typeSpec(0) {}
            inline const Type& type() const {return _type;}
            inline bool hasTypeSpec() const {return (_typeSpec != 0);}
            inline const Ast::QualifiedTypeSpec& typeSpec() const {return z::ref(_typeSpec);}
        private:
            Type _type;
            const Ast::QualifiedTypeSpec* _typeSpec;
        };
        typedef std::vector<ExpectedTypeSpec> ExpectedTypeSpecStack;
    public:
        const Ast::StructDefn* isStructExpected() const;
        const Ast::Function* isFunctionExpected() const;
        const Ast::TemplateDefn* isPointerExpected() const;
        const Ast::TemplateDefn* isPointerToExprExpected(const Ast::Expr& expr) const;
        const Ast::StructDefn* isPointerToStructExpected() const;
        const Ast::StructDefn* isListOfStructExpected() const;
        const Ast::StructDefn* isListOfPointerToStructExpected() const;

    public:
        void pushExpectedTypeSpec(const ExpectedTypeSpec::Type& type, const Ast::QualifiedTypeSpec& qTypeSpec);
        void pushExpectedTypeSpec(const ExpectedTypeSpec::Type& type);
        void popExpectedTypeSpec(const Ast::Token& pos, const ExpectedTypeSpec::Type& type);
        bool popExpectedTypeSpecOrAuto(const Ast::Token& pos, const ExpectedTypeSpec::Type& type);
        const Ast::QualifiedTypeSpec* getExpectedTypeSpecIfAny() const;
        const Ast::QualifiedTypeSpec& getExpectedTypeSpec(const Ast::QualifiedTypeSpec* qTypeSpec) const;

    private:
        inline std::string getExpectedTypeName(const ExpectedTypeSpec::Type& exType);
        inline ExpectedTypeSpec::Type getExpectedType(const Ast::Token& pos) const;
        inline const ExpectedTypeSpec& getExpectedTypeList(const Ast::Token& pos) const;
        inline const Ast::QualifiedTypeSpec& getExpectedTypeSpecEx(const Ast::Token& pos) const;

    public:
        const Ast::TemplateDefn* isEnteringList() const;
    private:
        inline const Ast::TypeSpec* isListOfPointerExpected() const;
        inline const Ast::TemplateDefn* isEnteringTemplate() const;

    public:
        void pushCallArgList(const Ast::Scope& in);
        void popCallArgList(const Ast::Token& pos, const Ast::Scope& in);
        void popCallArg(const Ast::Token& pos);

    private:
        ExpectedTypeSpecStack _expectedTypeSpecStack;

    public: // everything related to function body lists
        /// \brief Return the function implementation list
        /// \return The function implementation list
        inline const BodyList& bodyList() const {return _bodyList;}

        /// \brief Add a function implementation to the unit
        /// \param functionDefnBase the function implementation to add
        inline void addBody(const Body& body) {_bodyList.push_back(z::ptr(body));}

    private:
        /// \brief The list of all function implementations in this unit
        BodyList _bodyList;

    public: // owning-list of all nodes
        /// \brief Return the node list
        /// \return The node list
        inline NodeList& nodeList() {return _nodeList;}

    private:
        /// \brief The list of nodes in this unit
        NodeList _nodeList;
    };
}
