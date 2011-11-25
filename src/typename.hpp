#pragma once

struct GenMode {
    enum T {
        Normal,
        Import,
        TypeSpecMemberRef
    };
};

static bool getRootName(const Ast::TypeSpec& typeSpec, const std::string& sep, std::string& name, const GenMode::T& mode);

inline std::string getTypeSpecName(const Ast::TypeSpec& typeSpec, const GenMode::T& mode, const std::string& sep = "::") {
    std::string name;
    getRootName(typeSpec, sep, name, mode);
    return name;
}

inline std::string getQualifiedTypeSpecName(const Ast::QualifiedTypeSpec& qtypeSpec, const GenMode::T& mode, const std::string& sep = "::") {
    std::string name;
    if(qtypeSpec.isConst())
        name += "const ";
    getRootName(qtypeSpec.typeSpec(), sep, name, mode);
    if(qtypeSpec.isRef())
        name += "&";
    return name;
}

static bool getName(const Ast::TypeSpec& typeSpec, const std::string& sep, std::string& name, const GenMode::T& mode) {
    const Ast::ChildTypeSpec* ctypeSpec = dynamic_cast<const Ast::ChildTypeSpec*>(ptr(typeSpec));
    if(!ctypeSpec)
        return false;

    if(getName(ref(ctypeSpec).parent(), sep, name, mode))
        name += sep;

    name += typeSpec.name().string();

    if(dynamic_cast<const Ast::EnumDefn*>(ptr(typeSpec)) != 0) {
        if(mode == GenMode::Normal) {
            name += sep;
            name += "T";
        } else if(mode == GenMode::Import) {
        }
    }

    return true;
}

static bool getRootName(const Ast::TypeSpec& typeSpec, const std::string& sep, std::string& name, const GenMode::T& mode) {
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(ptr(typeSpec));

    if(mode == GenMode::Import) {
        if(templateDefn) {
            name += ref(templateDefn).name().string();
            name += "<";
            std::string sep;
            for(Ast::TemplateDefn::List::const_iterator it = ref(templateDefn).list().begin(); it != ref(templateDefn).list().end(); ++it) {
                const Ast::QualifiedTypeSpec& qTypeSpec = ref(*it);
                name += sep;
                name += getQualifiedTypeSpecName(qTypeSpec, mode);
                sep = ", ";
            }
            name += "> ";
            return true;
        }
        return getName(typeSpec, sep, name, mode);
    }

    if(typeSpec.name().string() == "string") {
        name += "std::string";
        return true;
    }

    if(templateDefn) {
        if(typeSpec.name().string() == "pointer") {
            name += "pointer";
        } else if(typeSpec.name().string() == "list") {
            name += "list";
        } else if(typeSpec.name().string() == "dict") {
            name += "dict";
        } else if(typeSpec.name().string() == "future") {
            name += "FutureT";
        } else if(typeSpec.name().string() == "functor") {
            name += "FunctorT";
        } else {
            throw Exception("Unknown template type '%s'\n", typeSpec.name().text());
        }
        name += "<";
        std::string sep;
        for(Ast::TemplateDefn::List::const_iterator it = ref(templateDefn).list().begin(); it != ref(templateDefn).list().end(); ++it) {
            const Ast::QualifiedTypeSpec& qTypeSpec = ref(*it);
            name += sep;
            name += getQualifiedTypeSpecName(qTypeSpec, mode);
            sep = ", ";
        }
        name += "> ";
        return true;
    }

    return getName(typeSpec, sep, name, mode);
}
