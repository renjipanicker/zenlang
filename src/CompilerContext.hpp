#pragma once

#include "ast.hpp"
#include "error.hpp"

namespace Ast {
    class Context {
    public:
        struct CoercionResult {
            enum T {
                None,
                Lhs,
                Rhs
            };
        };
    public:
        typedef std::list<Ast::Namespace*> NamespaceStack;
        typedef std::list<Ast::Scope*> ScopeStack;
        typedef std::list<Ast::TypeSpec*> TypeSpecStack;

    public:
        inline Context(Ast::Unit& unit, const std::string& filename, const int& level)
            : _unit(unit), _filename(filename), _level(level), _currentTypeRef(0), _currentImportedTypeRef(0) {
            Ast::Root& rootTypeSpec = getRootNamespace();
            enterTypeSpec(rootTypeSpec);
        }
        inline ~Context() {
            assert(_expectedTypeSpecStack.size() == 0);
            assert(_typeSpecStack.size() == 1);
            Ast::Root& rootTypeSpec = getRootNamespace();
            leaveTypeSpec(rootTypeSpec);
        }

        inline const std::string& filename() const {return _filename;}
        inline const int& level() const {return _level;}
    private:
        Ast::Unit& _unit;
        const std::string _filename;
        const int _level;

    // Various helpers
    public:
        const Ast::QualifiedTypeSpec* canCoerceX(const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs, CoercionResult::T& mode) const;
        inline const Ast::QualifiedTypeSpec* canCoerce(const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs) const;
        const Ast::QualifiedTypeSpec& coerce(const Ast::Token& pos, const Ast::QualifiedTypeSpec& lhs, const Ast::QualifiedTypeSpec& rhs);

    // everything related to scope stack
    public:
        Ast::Scope& enterScope(Ast::Scope& scope);
        Ast::Scope& leaveScope();
        Ast::Scope& leaveScope(Ast::Scope& scope);
        Ast::Scope& currentScope();
        const Ast::VariableDefn* hasMember(const Ast::Scope& scope, const Ast::Token& name) const;
        const Ast::VariableDefn* getVariableDef(const std::string& filename, const Ast::Token& name, Ast::RefType::T& refType) const;
    private:
        ScopeStack _scopeStack;

    // everything related to namespace stack
    public:
        inline NamespaceStack& namespaceStack() {return _namespaceStack;}
        inline void addNamespace(Ast::Namespace& ns) {_namespaceStack.push_back(z::ptr(ns));}
        void leaveNamespace();
    private:
        NamespaceStack _namespaceStack;

    // everything related to struct-init stack
    public:
        inline void pushStructInit(const Ast::StructDefn& structDefn) {_structInitStack.push_back(z::ptr(structDefn));}
        inline void popStructInit() {_structInitStack.pop_back();}
        inline const Ast::StructDefn* structInit() {if(_structInitStack.size() == 0) return 0; return _structInitStack.back();}
    private:
        typedef std::list<const Ast::StructDefn*> StructInitStack;
        StructInitStack _structInitStack;

    // everything related to current typespec
    public:
        template <typename T> inline const T* setCurrentRootTypeRef(const Ast::Token& name) {
            const T& td = getRootTypeSpec<T>(name);
            _currentTypeRef = z::ptr(td);
            _currentImportedTypeRef = hasImportRootTypeSpec(name);
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

    // everything related to typespec-stack
    public:
        Ast::Root& getRootNamespace() const;
        const Ast::TypeSpec* hasRootTypeSpec(const Ast::Token& name) const;
        inline TypeSpecStack& typeSpecStack() {return _typeSpecStack;}
        inline const TypeSpecStack& typeSpecStack() const {return _typeSpecStack;}

    public:
        const Ast::TypeSpec* hasImportRootTypeSpec(const Ast::Token& name) const;
        template <typename T> const T& getRootTypeSpec(const Ast::Token &name) const {
            const Ast::TypeSpec* typeSpec = hasRootTypeSpec(name);
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
        TypeSpecStack _typeSpecStack;

    // everything related to expected typespec
    public:
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
    };
}
