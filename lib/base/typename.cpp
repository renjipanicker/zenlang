#include "zenlang.hpp"
#if defined(UN_AMALGAMATED)
#include "base/base.hpp"
#include "base/typename.hpp"
#endif

struct NameType {
    enum T {
        None,
        Normal
    };
};

z::string TypespecNameGenerator::tn(const Ast::TypeSpec& typeSpec) {
    z::string name;
    getName(typeSpec, name);
    return name;
}

z::string TypespecNameGenerator::qtn(const Ast::QualifiedTypeSpec& qtypeSpec) {
    z::string name;
    if(qtypeSpec.isConst())
        name += "const ";
    name += tn(qtypeSpec.typeSpec());
    if(qtypeSpec.isRef())
        name += "&";
    return name;
}

bool TypespecNameGenerator::getName(const Ast::TypeSpec& typeSpec, z::string& name) {
    const Ast::ChildTypeSpec* ctypeSpec = dynamic_cast<const Ast::ChildTypeSpec*>(z::ptr(typeSpec));
    if(!ctypeSpec)
        return false;

    if(getName(z::ref(ctypeSpec).parent(), name))
        name += _sep;

    getTypeName(typeSpec, name);

    return true;
}

void ZenlangNameGenerator::getTypeName(const Ast::TypeSpec& typeSpec, z::string& name) {
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(typeSpec));
    if(templateDefn) {
        name += z::ref(templateDefn).name().string();
        name += "<";
        z::string sep;
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
