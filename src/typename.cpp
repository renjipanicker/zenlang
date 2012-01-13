#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "generator.hpp"
#include "typename.hpp"

struct NameType {
    enum T {
        None,
        Normal,
        Ptr
    };
};

static NameType::T getName(const Ast::TypeSpec& typeSpec, const std::string& sep, std::string& name, const GenMode::T& mode) {
    const Ast::ChildTypeSpec* ctypeSpec = dynamic_cast<const Ast::ChildTypeSpec*>(z::ptr(typeSpec));
    if(!ctypeSpec)
        return NameType::None;

    if(getName(z::ref(ctypeSpec).parent(), sep, name, mode))
        name += sep;

    name += typeSpec.name().string();

    if(dynamic_cast<const Ast::EnumDefn*>(z::ptr(typeSpec)) != 0) {
        if(mode == GenMode::Normal) {
            name += sep;
            name += "T";
        } else if(mode == GenMode::Import) {
        }
    }

    return NameType::Normal;
}

static NameType::T getRootName(const Ast::TypeSpec& typeSpec, const std::string& sep, std::string& name, const GenMode::T& mode) {
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(typeSpec));

    if(mode == GenMode::Import) {
        if(templateDefn) {
            name += z::ref(templateDefn).name().string();
            name += "<";
            std::string sep;
            for(Ast::TemplateDefn::List::const_iterator it = z::ref(templateDefn).list().begin(); it != z::ref(templateDefn).list().end(); ++it) {
                const Ast::QualifiedTypeSpec& qTypeSpec = it->get();
                name += sep;
                name += getQualifiedTypeSpecName(qTypeSpec, mode);
                sep = ", ";
            }
            name += "> ";
            return NameType::Normal;
        }
        return getName(typeSpec, sep, name, mode);
    }

    if(typeSpec.name().string() == "string") {
        name += "std::string";
        return NameType::Normal;
    }

    if(typeSpec.name().string() == "type") {
        name += "z::type";
        return NameType::Normal;
    }

    if(typeSpec.name().string() == "ArgList") {
        name += "z::ArgList";
        return NameType::Normal;
    }

    if(templateDefn) {
        if(typeSpec.name().string() == "ptr") {
            const Ast::QualifiedTypeSpec& qTypeSpec = z::ref(templateDefn).list().front();
            name += getTypeSpecName(qTypeSpec.typeSpec(), mode);
            name += "*";
            return NameType::Ptr;
        }

        if(typeSpec.name().string() == "pointer") {
            name += "z::pointer";
        } else if(typeSpec.name().string() == "list") {
            name += "z::list";
        } else if(typeSpec.name().string() == "dict") {
            name += "z::dict";
        } else if(typeSpec.name().string() == "future") {
            name += "z::FutureT";
        } else if(typeSpec.name().string() == "functor") {
            name += "z::FunctorT";
        } else {
            name += typeSpec.name().string();
        }
        name += "<";
        std::string sep;
        for(Ast::TemplateDefn::List::const_iterator it = z::ref(templateDefn).list().begin(); it != z::ref(templateDefn).list().end(); ++it) {
            const Ast::QualifiedTypeSpec& qTypeSpec = it->get();
            name += sep;
            if(typeSpec.name().string() == "pointer") {
                name += getTypeSpecName(qTypeSpec.typeSpec(), mode);
            } else {
                name += getQualifiedTypeSpecName(qTypeSpec, mode);
            }
            sep = ", ";
        }
        name += "> ";
        return NameType::Normal;
    }

    return getName(typeSpec, sep, name, mode);
}

std::string getTypeSpecName(const Ast::TypeSpec& typeSpec, const GenMode::T& mode, const std::string& sep) {
    std::string name;
    getRootName(typeSpec, sep, name, mode);
    return name;
}

std::string getQualifiedTypeSpecName(const Ast::QualifiedTypeSpec& qtypeSpec, const GenMode::T& mode, const std::string& sep) {
    std::string tname;
    NameType::T nt = getRootName(qtypeSpec.typeSpec(), sep, tname, mode);
    if(nt == NameType::Ptr) {
//        return tname;
    }

    std::string name;
    if(qtypeSpec.isConst())
        name += "const ";
    name += tname;
    if(qtypeSpec.isRef())
        name += "&";
    return name;
}
