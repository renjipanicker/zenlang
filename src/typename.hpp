#pragma once

struct GenMode {
    enum T {
        Normal,
        Import,
        TypeSpecMemberRef
    };
};

std::string getTypeSpecName(const Ast::TypeSpec& typeSpec, const GenMode::T& mode, const std::string& sep = "::");
std::string getQualifiedTypeSpecName(const Ast::QualifiedTypeSpec& qtypeSpec, const GenMode::T& mode, const std::string& sep = "::");

inline const Ast::TypeSpec* resolveTypedef(const Ast::TypeSpec& typeSpec) {
    const Ast::TypeSpec* subType = z::ptr(typeSpec);
    for(const Ast::TypedefDefn* td = dynamic_cast<const Ast::TypedefDefn*>(subType);td != 0; td = dynamic_cast<const Ast::TypedefDefn*>(subType)) {
        const Ast::QualifiedTypeSpec& qTypeSpec = z::ref(td).qTypeSpec();
        subType = z::ptr(qTypeSpec.typeSpec());
    }
    return subType;
}

inline const Ast::TypeSpec& resolveTypedefR(const Ast::TypeSpec& typeSpec) {
    const Ast::TypeSpec* ts = resolveTypedef(typeSpec);
    return z::ref(ts);
}

template <typename T>
inline const T* resolveTypedefT(const Ast::TypeSpec& typeSpec) {
    const Ast::TypeSpec* ts = resolveTypedef(typeSpec);
    const T* td = dynamic_cast<const T*>(ts);
    return td;
}
