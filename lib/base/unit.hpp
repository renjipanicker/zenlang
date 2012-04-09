#pragma once
#include "base/ast.hpp"
#include "base/error.hpp"

namespace z {
namespace Ast {
    /*! \brief A compilation unit
      The Unit AST node is the owner for all AST nodes in the unit.
      This node maintains two namespace hierarchies
      - the root namespace is the namespace for all types defined in this unit
      - the import namespace is the namespace for all types imported into the unit from other modules.
    */
    class Unit {
    public:
        typedef SLst<z::Ast::Namespace> NamespaceStack;
        typedef SLst<z::Ast::Scope> ScopeStack;
        typedef SLst<z::Ast::TypeSpec> TypeSpecStack;
        typedef std::map<const z::Ast::TypeSpec*, Ptr<const z::Ast::Expr> > DefaultValueList;
        typedef SLst<const Body> BodyList;
        typedef SLst<const z::Ast::CoerceList> CoerceListList;
        typedef std::list<Token> NsPartList;
        typedef std::map<z::string, int> HeaderFileList;
        typedef size_t UniqueId_t;

    public:
        Unit();
        ~Unit();

    public: // everything related to imported header files
        /// \brief Return the header file list
        /// \return The header file list
        inline const HeaderFileList& headerFileList() const {return _headerFileList;}

        /// \brief Add a header file to the unit
        /// \param list the header file to add
        inline void addheaderFile(const z::string& filename) {_headerFileList[filename]++;}

    private:
        /// \brief The list of header files imported into this unit
        HeaderFileList _headerFileList;

    public: // everything related to namespace stack
        inline NamespaceStack& namespaceStack() {return _namespaceStack;}
        inline void addNamespace(z::Ast::Namespace& ns) {_namespaceStack.push(ns);}
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
        inline Root& rootNS() {return _rootNS.get();}
        inline const Root& rootNS() const {return _rootNS.get();}

    private:
        /// \brief This NS contains all types defined in the current compilation unit.
        Ptr<z::Ast::Root> _rootNS;

    public: // everything related to import namespace
        /// \brief Return the import namespace
        /// \return The import namespace
        inline Root& importNS() {return _importNS.get();}
        inline const Root& importNS() const {return _importNS.get();}

    private:
        /// \brief This NS contains all imported typespec's.
        /// It is not used for source file generation, only for reference.
        Ptr<z::Ast::Root> _importNS;

    public: // everything related to anonymous namespace
        /// \brief add to the anonymous namespace
        inline void addAnonymous(z::Ast::ChildTypeSpec& ts) {_anonymousNS.get().addChild(ts);}
        inline const Root& anonymousNS() const {return _anonymousNS.get();}

    private:
        /// \brief This NS contains all imported typespec's.
        /// It is not used for source file generation, only for reference.
        Ptr<z::Ast::Root> _anonymousNS;

    public: // everything related to default values
        /// \brief Return the default value list
        /// \return The default value  list
        inline const DefaultValueList& defaultValueList() const {return _defaultValueList;}

        /// \brief Add a default value to the unit
        /// \param typeSpec the typeSpec to add
        /// \param expr the expr to add
        inline void addDefaultValue(const TypeSpec& typeSpec, const Expr& expr) {_defaultValueList[z::ptr(typeSpec)].reset(expr);}

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
        const z::Ast::QualifiedTypeSpec* canCoerceX(const z::Ast::QualifiedTypeSpec& lhs, const z::Ast::QualifiedTypeSpec& rhs, CoercionResult::T& mode) const;
        inline const z::Ast::QualifiedTypeSpec* canCoerce(const z::Ast::QualifiedTypeSpec& lhs, const z::Ast::QualifiedTypeSpec& rhs) const;
        const z::Ast::QualifiedTypeSpec& coerce(const z::Ast::Token& pos, const z::Ast::QualifiedTypeSpec& lhs, const z::Ast::QualifiedTypeSpec& rhs);

    public:
        /// \brief Return the coercion list
        /// \return The coercion list
        inline const CoerceListList& coercionList() const {return _coerceListList;}

        /// \brief Add a coercion list to the unit
        /// \param list the coercion list to add
        inline void addCoercionList(const CoerceList& list) {_coerceListList.add(list);}

    private:
        /// \brief The coercion list for all types in this unit
        CoerceListList _coerceListList;

    public: // everything related to scope stack
        struct ScopeCallback {
            virtual void enteringScope(z::Ast::Scope& scope) = 0;
            virtual void leavingScope(z::Ast::Scope& scope) = 0;
        protected:
            inline ScopeCallback() {}
        };

    public:
        z::Ast::Scope& enterScope(const z::Ast::Token& pos);
        z::Ast::Scope& enterScope(z::Ast::Scope& scope);
        void        leaveScope();
        void        leaveScope(z::Ast::Scope& scope);
        z::Ast::Scope& currentScope();
        const z::Ast::VariableDefn* hasMember(const z::Ast::Scope& scope, const z::Ast::Token& name) const;
        const z::Ast::VariableDefn* getVariableDef(const z::Ast::Token& name, z::Ast::RefType::T& refType) const;
        inline void setScopeCallback(ScopeCallback* val) {_scopeCallback = val;}
    private:
        ScopeStack _scopeStack;
        ScopeCallback* _scopeCallback;

    public: // everything related to struct-init stack
        inline void pushStructInit(const z::Ast::StructDefn& structDefn) {_structInitStack.push_back(z::ptr(structDefn));}
        inline void popStructInit() {_structInitStack.pop_back();}
        inline const z::Ast::StructDefn* structInit() {if(_structInitStack.size() == 0) return 0; return _structInitStack.back();}
    private:
        typedef std::list<const z::Ast::StructDefn*> StructInitStack;
        StructInitStack _structInitStack;

    public: // everything related to current typespec
        template <typename T> inline const T* setCurrentRootTypeRef(const int& level, const z::Ast::Token& name) {
            const T& td = getRootTypeSpec<T>(level, name);
            _currentTypeRef = z::ptr(td);
            _currentImportedTypeRef = hasImportRootTypeSpec(level, name);
            return z::ptr(td);
        }

        template <typename T> inline const T* setCurrentChildTypeRef(const z::Ast::TypeSpec& parent, const z::Ast::Token& name, const z::string& extype) {
            if(z::ptr(parent) != _currentTypeRef) {
                throw z::Exception("Unit", zfmt(name, "Internal error: %{s} parent mismatch '%{t}'")
                                   .arg("s", extype)
                                   .arg("t", name)
                                   );
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

            throw z::Exception("Unit", zfmt(name, "%{s} type expected '%{t}'").arg("s", extype).arg("t", name));
        }

        template <typename T> inline const T* resetCurrentTypeRef(const T& typeSpec) {
            _currentTypeRef = 0;
            return z::ptr(typeSpec);
        }

        const z::Ast::TypeSpec* currentTypeRefHasChild(const z::Ast::Token& name) const;
    private:
        const z::Ast::TypeSpec* _currentTypeRef;
        const z::Ast::TypeSpec* _currentImportedTypeRef;

    public: // everything related to typespec-stack
        z::Ast::Root& getRootNamespace(const int& level);
        const z::Ast::TypeSpec* hasRootTypeSpec(const int& level, const z::Ast::Token& name) const;
        inline TypeSpecStack& typeSpecStack() {return _typeSpecStack;}
        inline const TypeSpecStack& typeSpecStack() const {return _typeSpecStack;}

    public:
        template <typename T> const T& getRootTypeSpec(const int& level, const z::Ast::Token& name) const {
            const z::Ast::TypeSpec* typeSpec = hasRootTypeSpec(level, name);
            if(!typeSpec) {
                throw z::Exception("Unit", zfmt(name, "Unknown root type '%{s}'").arg("s", name ));
            }
            const T* tTypeSpec = dynamic_cast<const T*>(typeSpec);
            if(!tTypeSpec) {
                throw z::Exception("Unit", zfmt(name, "Type mismatch '%{s}'").arg("s", name ));
            }
            return z::ref(tTypeSpec);
        }

    private:
        inline const z::Ast::TypeSpec* findTypeSpec(const z::Ast::TypeSpec& parent, const z::Ast::Token& name) const;

    public:
        z::Ast::TypeSpec& currentTypeSpec() const;
        z::Ast::TypeSpec& enterTypeSpec(z::Ast::TypeSpec& typeSpec);
        z::Ast::TypeSpec& leaveTypeSpec(z::Ast::TypeSpec& typeSpec);
        z::Ast::StructDefn& getCurrentStructDefn(const z::Ast::Token& pos);

    private:
        const z::Ast::TypeSpec* hasImportRootTypeSpec(const int& level, const z::Ast::Token& name) const;
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

            typedef std::vector<const z::Ast::QualifiedTypeSpec*> List;

            inline ExpectedTypeSpec(const Type& type, const z::Ast::QualifiedTypeSpec& typeSpec) : _type(type), _typeSpec(typeSpec) {}
            inline ExpectedTypeSpec(const Type& type) : _type(type) {}
            inline const Type& type() const {return _type;}
            inline bool hasTypeSpec() const {return (!_typeSpec.empty());}
            inline const z::Ast::QualifiedTypeSpec& typeSpec() const {return _typeSpec.get();}
        private:
            Type _type;
            const z::Ast::Ptr<const z::Ast::QualifiedTypeSpec> _typeSpec;
        };
        typedef std::list<ExpectedTypeSpec> ExpectedTypeSpecStack;

    public:
        const z::Ast::StructDefn* isStructExpected() const;
        const z::Ast::Function* isFunctionExpected() const;
        const z::Ast::TemplateDefn* isPointerExpected() const;
        const z::Ast::TemplateDefn* isPointerToExprExpected(const z::Ast::Expr& expr) const;
        const z::Ast::StructDefn* isPointerToStructExpected() const;
        const z::Ast::StructDefn* isListOfStructExpected() const;
        const z::Ast::StructDefn* isListOfPointerToStructExpected() const;

    public:
        void pushExpectedTypeSpec(const ExpectedTypeSpec::Type& type, const z::Ast::QualifiedTypeSpec& qTypeSpec);
        void pushExpectedTypeSpec(const ExpectedTypeSpec::Type& type);
        void popExpectedTypeSpec(const z::Ast::Token& pos, const ExpectedTypeSpec::Type& type);
        bool popExpectedTypeSpecOrAuto(const z::Ast::Token& pos, const ExpectedTypeSpec::Type& type);
        const z::Ast::QualifiedTypeSpec* getExpectedTypeSpecIfAny() const;
        const z::Ast::QualifiedTypeSpec& getExpectedTypeSpec(const z::Ast::Token& pos, const z::Ast::QualifiedTypeSpec* qTypeSpec) const;

    private:
        inline z::string getExpectedTypeName(const z::Ast::Token& pos, const ExpectedTypeSpec::Type& exType);
        inline ExpectedTypeSpec::Type getExpectedType(const z::Ast::Token& pos) const;
        inline const ExpectedTypeSpec& getExpectedTypeList(const z::Ast::Token& pos) const;
        inline const z::Ast::QualifiedTypeSpec& getExpectedTypeSpecEx(const z::Ast::Token& pos) const;

    public:
        const z::Ast::TemplateDefn* isEnteringList() const;
    private:
        inline const z::Ast::TypeSpec* isListOfPointerExpected() const;
        inline const z::Ast::TemplateDefn* isEnteringTemplate() const;

    public:
        void pushCallArgList(const z::Ast::Scope& in);
        void popCallArgList(const z::Ast::Token& pos, const z::Ast::Scope& in);
        void popCallArg(const z::Ast::Token& pos);

    private:
        ExpectedTypeSpecStack _expectedTypeSpecStack;

    public: // everything related to function body lists
        /// \brief Return the function implementation list
        /// \return The function implementation list
        inline const BodyList& bodyList() const {return _bodyList;}

        /// \brief Add a function implementation to the unit
        /// \param functionDefnBase the function implementation to add
        inline void addBody(const Body& body) {_bodyList.add(body);}

    private:
        /// \brief The list of all function implementations in this unit
        BodyList _bodyList;

    public: // owning-list of all nodes
        template<typename T> inline T& addNode(T* node) {/*_nodeList.push_back(node); */return z::ref(node);}

//    private:
//        /// \brief The owner list of all nodes in this unit
//        std::list<const Node*> _nodeList;

    public: // A unique numeric id for anonymous functions
        /// \brief Return unique id
        /// \return unique id
        inline UniqueId_t uniqueIdx() {return ++_uniqueIdx;}

    private:
        /// \brief unique id value
        UniqueId_t _uniqueIdx;
    };

    //////////////////////////////////////////////////////////////////
    /*! \brief A module
      The Module stores all the global statements in the unit.
    */
    class Module {
    public:
        typedef size_t Level_t;
    public:
        inline Module(Unit& unit, const z::string& filename, const Level_t& level) : _unit(unit), _filename(filename), _level(level) {
            z::Ast::CompoundStatement& gs = _unit.addNode(new z::Ast::CompoundStatement(Token(_filename, 0, 0, "")));
            _globalStatementList.reset(gs);
        }
    private:
        inline Module(const Module& src) : _unit(src._unit), _filename(src._filename), _level(src._level) {}

    public:
        /// \brief Return the unit
        /// \return The unit
        inline z::Ast::Unit& unit() const {return _unit;}

    private:
        /// \brief Unit
        z::Ast::Unit& _unit;

    public:
        inline const z::string& filename() const {return _filename;}

    private:
        const z::string _filename;

    public:
        inline const Level_t& level() const {return _level;}

    private:
        const Level_t _level;

    public:
        /// \brief Return the statement list
        /// \return The statement list
        inline const CompoundStatement& globalStatementList() const {return _globalStatementList.get();}

        /// \brief Add a statement to the module
        /// \param statement the statement to add
        inline void addGlobalStatement(const Statement& statement) {_globalStatementList.get().addStatement(statement);}

        /// \brief Clear statement list
        inline void clearGlobalStatementList() {
        }

    private:
        /// \brief The list of all import statements in this module
        Ptr<CompoundStatement> _globalStatementList;
    };
}
}
