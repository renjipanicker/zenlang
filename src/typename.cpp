#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "generator.hpp"
#include "typename.hpp"

struct NameType {
    enum T {
        None,
        Normal
    };
};

std::string TypespecNameGenerator::tn(const Ast::TypeSpec& typeSpec) {
    std::string name;
    getName(typeSpec, name);
//    if(mode == GenMode::Stlcpp) {
//        StlcppTypespecNameGenerator gen(sep);
//        gen.getName(typeSpec, sep, name, mode);
//    } else {
//        ZenlangTypespecNameGenerator gen;
//        gen.getName(typeSpec, sep, name, mode);
//    }
    return name;
}

std::string TypespecNameGenerator::qtn(const Ast::QualifiedTypeSpec& qtypeSpec) {
    std::string name;
    if(qtypeSpec.isConst())
        name += "const ";
    name += tn(qtypeSpec.typeSpec());
    if(qtypeSpec.isRef())
        name += "&";
    return name;
}

bool TypespecNameGenerator::getName(const Ast::TypeSpec& typeSpec, std::string& name) {
    const Ast::ChildTypeSpec* ctypeSpec = dynamic_cast<const Ast::ChildTypeSpec*>(z::ptr(typeSpec));
    if(!ctypeSpec)
        return false;

    if(getName(z::ref(ctypeSpec).parent(), name))
        name += _sep;

    getTypeName(typeSpec, name);

    return true;
}

void ZenlangNameGenerator::getTypeName(const Ast::TypeSpec& typeSpec, std::string& name) {
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(typeSpec));
    if(templateDefn) {
        name += z::ref(templateDefn).name().string();
        name += "<";
        std::string sep;
        for(Ast::TemplateTypePartList::List::const_iterator it = z::ref(templateDefn).list().begin(); it != z::ref(templateDefn).list().end(); ++it) {
            const Ast::QualifiedTypeSpec& qTypeSpec = it->get();
            name += sep;
            name += qtn(qTypeSpec);
            sep = ", ";
        }
        name += "> ";
        return;
    }
    name += typeSpec.name().string();
}
