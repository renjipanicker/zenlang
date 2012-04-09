#pragma once
#include "base/unit.hpp"

namespace z {
struct TypespecNameGenerator {
    z::string tn(const z::Ast::TypeSpec& typeSpec);
    z::string qtn(const z::Ast::QualifiedTypeSpec& qtypeSpec);
private:
    bool getName(const z::Ast::TypeSpec& typeSpec, z::string& name);
    virtual void getTypeName(const z::Ast::TypeSpec& typeSpec, z::string& name) = 0;
protected:
    inline TypespecNameGenerator(const z::string& sep) : _sep(sep) {}
    const z::string _sep;
};

struct ZenlangNameGenerator : public TypespecNameGenerator {
    virtual void getTypeName(const z::Ast::TypeSpec& typeSpec, z::string& name);
public:
    inline ZenlangNameGenerator(const z::string& sep = "::") : TypespecNameGenerator(sep) {}
};

inline const z::Ast::TypeSpec* resolveTypedef(const z::Ast::TypeSpec& typeSpec) {
    const z::Ast::TypeSpec* subType = z::ptr(typeSpec);
    for(const z::Ast::TypedefDefn* td = dynamic_cast<const z::Ast::TypedefDefn*>(subType);td != 0; td = dynamic_cast<const z::Ast::TypedefDefn*>(subType)) {
        const z::Ast::QualifiedTypeSpec& qTypeSpec = z::ref(td).qTypeSpec();
        subType = z::ptr(qTypeSpec.typeSpec());
    }
    return subType;
}

inline const z::Ast::TypeSpec& resolveTypedefR(const z::Ast::TypeSpec& typeSpec) {
    const z::Ast::TypeSpec* ts = resolveTypedef(typeSpec);
    return z::ref(ts);
}

template <typename T>
inline const T* resolveTypedefT(const z::Ast::TypeSpec& typeSpec) {
    const z::Ast::TypeSpec* ts = resolveTypedef(typeSpec);
    const T* td = dynamic_cast<const T*>(ts);
    return td;
}

template <typename DefnT, typename ChildT>
struct BaseIterator {
    inline BaseIterator(const DefnT* defn) : _defn(defn) {}
    inline bool hasNext() const {return (_defn != 0);}
    inline const DefnT& get() const {return z::ref(_defn);}
    inline void next() {
        const ChildT* csd = dynamic_cast<const ChildT*>(_defn);
        if(csd) {
            _defn = z::ptr(z::ref(csd).base());
        } else {
            _defn = 0;
        }
    }

private:
    const DefnT* _defn;
};

struct StructBaseIterator : public BaseIterator<z::Ast::StructDefn, z::Ast::ChildStructDefn> {
    inline StructBaseIterator(const z::Ast::StructDefn* defn) : BaseIterator(defn) {}
};

struct FunctionBaseIterator : public BaseIterator<z::Ast::Function, z::Ast::ChildFunctionDefn> {
    inline FunctionBaseIterator(const z::Ast::FunctionDefn* defn) : BaseIterator(defn) {}
};
}
