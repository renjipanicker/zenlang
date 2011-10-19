#include "base/pch.hpp"
#include "base/common.hpp"
#include "base/exception.hpp"
#include "generator.hpp"

inline bool getName(const Ast::TypeSpec& typeSpec, const std::string& sep, std::string& name) {
    const Ast::ChildTypeSpec* ctypeSpec = dynamic_cast<const Ast::ChildTypeSpec*>(ptr(typeSpec));
    if(!ctypeSpec)
        return false;

    if(getName(ref(ctypeSpec).parent(), sep, name))
        name += sep;
    name += typeSpec.name().string();
    if(dynamic_cast<const Ast::EnumDef*>(ptr(typeSpec)) != 0) {
        name += sep;
        name += "T";
    }
    return true;
}

inline std::string getName(const Ast::TypeSpec& typeSpec, const std::string& sep = "::") {
    std::string name;
    getName(typeSpec, sep, name);
    return name;
}

inline std::string getName(const Ast::QualifiedTypeSpec& qtypeSpec, const std::string& sep = "::") {
    std::string name;
    if(qtypeSpec.isConst())
        name += "const ";
    getName(qtypeSpec.typeSpec(), sep, name);
    if(qtypeSpec.isRef())
        name += "&";
    return name;
}

inline std::string getBaseName(const std::string& filename) {
    std::string basename = filename;
    size_t idx = -1;

    // strip last extension, if any
    idx = basename.rfind('.');
    if(idx >= 0)
        basename = basename.substr(0, idx + 1);

    // strip path, if any
    idx = basename.rfind('/');
    if(idx >= 0)
        basename = basename.substr(idx + 1);

    return basename;
}

struct Indent {
    inline Indent() {
        ind[_indent] = 32;
        _indent += 4;
        ind[_indent] = 0;
    }
    inline ~Indent() {
        ind[_indent] = 32;
        _indent -= 4;
        ind[_indent] = 0;
    }
    inline static const char* get() {return ind;}
    inline static void init() {
        if(_indent < 0) {
            memset(ind, 32, Size);
            _indent = 0;
            ind[_indent] = 0;
        }
    }
private:
    static const int Size = 1024;
    static char ind[Size];
    static int _indent;
};
char Indent::ind[Size] = {32};
int Indent::_indent = -1;
#define INDENT Indent _ind_

struct OutputFile {
    inline OutputFile(FILE*& fp, const std::string& filename) : _fp(fp) {
        _fp = fopen(filename.c_str(), "w");
        if(_fp == 0) {
            throw Exception("Unable to open output file %s\n", filename.c_str());
        }
    }

    inline ~OutputFile() {
        fclose(_fp);
    }
    FILE*& _fp;
};

struct Generator::Impl {
    inline Impl(const Project& project, const Ast::Unit& unit) : _project(project), _unit(unit), _fpHdr(0), _fpSrc(0), _fpImp(0) {}
    inline void generateTypeSpec(const Ast::TypeSpec* typeSpec);
    inline void generateGlobalStatement(const Ast::Statement* statement);
    inline void generateGlobalStatementList();
    inline void enterNamespace();
    inline void leaveNamespace();
    inline void generateImportStatement();
    inline void run();
private:
    const Project& _project;
    const Ast::Unit& _unit;
private:
    FILE* _fpHdr;
    FILE* _fpSrc;
    FILE* _fpImp;
};

inline void Generator::Impl::generateTypeSpec(const Ast::TypeSpec* typeSpec) {
    for(const Ast::TypeDef* t = dynamic_cast<const Ast::TypeDef*>(typeSpec); t != 0; ) {
        if(ref(t).defType() == Ast::DefinitionType::Native) {
            fprintf(_fpHdr, "%s// typedef %s native;\n", Indent::get(), ref(t).name().text());
        }
        return;
    }

    for(const Ast::EnumDef* t = dynamic_cast<const Ast::EnumDef*>(typeSpec); t != 0; ) {
        if(ref(t).defType() != Ast::DefinitionType::Native) {
            fprintf(_fpHdr, "%sstruct %s {\n", Indent::get(), ref(t).name().text());
            fprintf(_fpHdr, "%s  enum T {\n", Indent::get(), ref(t).name().text());
            std::string sep = " ";
            for(Ast::EnumMemberDefList::List::const_iterator it = ref(t).list().begin(); it != ref(t).list().end(); ++it) {
                INDENT;
                const Ast::EnumMemberDef& def = ref(*it);
                fprintf(_fpHdr, "%s%s %s\n", Indent::get(), sep.c_str(), def.name().text());
                sep = ",";
            }
            fprintf(_fpHdr, "%s  };\n", Indent::get());
            fprintf(_fpHdr, "%s};\n", Indent::get());
            fprintf(_fpHdr, "\n");
        }
        return;
    }

    for(const Ast::StructDef* t = dynamic_cast<const Ast::StructDef*>(typeSpec); t != 0; ) {
        FILE* fp = 0;
        std::string impl;
        if(ref(t).accessType() == Ast::AccessType::Protected) {
            fprintf(_fpHdr, "%sstruct %s {\n", Indent::get(), ref(t).name().text());
            {
                INDENT;
                fprintf(_fpHdr, "%sstruct Impl;\n", Indent::get());
                fprintf(_fpHdr, "%sImpl* _impl;\n", Indent::get());
            }
            fprintf(_fpHdr, "%s};\n", Indent::get());
            fprintf(_fpHdr, "\n");
            fp = _fpSrc;
            impl = "::Impl";
        } else {
            fp = (ref(t).accessType() == Ast::AccessType::Private)?_fpSrc:_fpHdr;
        }

        if(ref(t).defType() != Ast::DefinitionType::Native) {
            fprintf(fp, "%sstruct %s%s {\n", Indent::get(), ref(t).name().text(), impl.c_str());
            fprintf(fp, "%sprivate:\n", Indent::get());
            for(Ast::VariableDefList::List::const_iterator it = ref(t).list().begin(); it != ref(t).list().end(); ++it) {
                INDENT;
                const Ast::VariableDef& vdef = ref(*it);
                fprintf(fp, "%s%s _%s;\n", Indent::get(), getName(vdef.qualifiedTypeSpec()).c_str(), vdef.name().text());
            }
            fprintf(fp, "%spublic:\n", Indent::get());
            for(Ast::VariableDefList::List::const_iterator it = ref(t).list().begin(); it != ref(t).list().end(); ++it) {
                INDENT;
                const Ast::VariableDef& vdef = ref(*it);
                fprintf(fp, "%sinline %s& %s(const %s& val) {_%s = val; return ref(this);}\n",
                        Indent::get(), ref(t).name().text(), vdef.name().text(), getName(vdef.qualifiedTypeSpec()).c_str(), vdef.name().text());
                fprintf(fp, "%sinline const %s& %s() const {return _%s;}\n",
                        Indent::get(), getName(vdef.qualifiedTypeSpec()).c_str(), vdef.name().text(), vdef.name().text());
            }
            fprintf(fp, "%s};\n", Indent::get());
            fprintf(_fpHdr, "\n");
        }
        return;
    }

    for(const Ast::FunctionDef* t = dynamic_cast<const Ast::FunctionDef*>(typeSpec); t != 0; ) {
        fprintf(_fpHdr, "%sclass %s : public Function<%s> {\n", Indent::get(), ref(t).name().text(), ref(t).name().text());
        fprintf(_fpHdr, "%s    static void impl(%s& This);\n", Indent::get(), ref(t).name().text());
        for(Ast::VariableDefList::List::const_iterator it = ref(t).in().begin(); it != ref(t).in().end(); ++it) {
            INDENT;
            const Ast::VariableDef& vdef = ref(*it);
            fprintf(_fpHdr, "%sconst %s _%s;\n", Indent::get(), getName(vdef.qualifiedTypeSpec().typeSpec()).c_str(), vdef.name().text());
        }
        fprintf(_fpHdr, "%spublic:\n", Indent::get());
        fprintf(_fpHdr, "%s    inline %s(", Indent::get(), ref(t).name().text());
        std::string sep = "";
        for(Ast::VariableDefList::List::const_iterator it = ref(t).in().begin(); it != ref(t).in().end(); ++it) {
            const Ast::VariableDef& vdef = ref(*it);
            fprintf(_fpHdr, "%sconst %s& %s", sep.c_str(), getName(vdef.qualifiedTypeSpec().typeSpec()).c_str(), vdef.name().text());
            sep = ", ";
        }
        fprintf(_fpHdr, ") : Function(&impl)");
        for(Ast::VariableDefList::List::const_iterator it = ref(t).in().begin(); it != ref(t).in().end(); ++it) {
            const Ast::VariableDef& vdef = ref(*it);
            fprintf(_fpHdr, ", _%s(%s)", vdef.name().text(), vdef.name().text());
        }
        fprintf(_fpHdr, " {}\n");

        fprintf(_fpHdr, "%s};\n", Indent::get());
        fprintf(_fpHdr, "\n");
        return;
    }

    for(const Ast::EventDef* t = dynamic_cast<const Ast::EventDef*>(typeSpec); t != 0; ) {
        fprintf(_fpHdr, "%sstruct %s : public Event<%s> {\n", Indent::get(), ref(t).name().text(), ref(t).name().text());
        {
            INDENT;
            fprintf(_fpHdr, "%sclass Add : public AddHandler<Add> {\n", Indent::get());
            fprintf(_fpHdr, "%s    static void impl(Add& This);\n", Indent::get());
            fprintf(_fpHdr, "%s    const %s _%s;\n", Indent::get(), getName(ref(t).in().qualifiedTypeSpec().typeSpec()).c_str(), ref(t).in().name().text());
            fprintf(_fpHdr, "%spublic:\n", Indent::get());
            fprintf(_fpHdr, "%s    inline Add(", Indent::get(), ref(t).name().text());
            fprintf(_fpHdr, "const %s& %s, Handler* handler", getName(ref(t).in().qualifiedTypeSpec().typeSpec()).c_str(), ref(t).in().name().text());
            fprintf(_fpHdr, ") : AddHandler(&impl, add(handler)),  _%s(%s) {}\n", ref(t).in().name().text(), ref(t).in().name().text());
            fprintf(_fpHdr, "%s};\n", Indent::get());
        }
        fprintf(_fpHdr, "%s};\n", Indent::get());
        fprintf(_fpHdr, "\n");
        return;
    }

    throw Exception("Generator: Unknown typespec: %s\n", ref(typeSpec).name().text());
}

inline void Generator::Impl::generateGlobalStatement(const Ast::Statement* statement) {
    for(const Ast::UserDefinedTypeSpecStatement* s = dynamic_cast<const Ast::UserDefinedTypeSpecStatement*>(statement); s != 0; ) {
        return generateTypeSpec(ptr(ref(s).typeSpec()));
    }
    throw Exception("Unknown statement\n");
}

inline void Generator::Impl::generateGlobalStatementList() {
    INDENT;
    for(Ast::Unit::StatementList::const_iterator sit = _unit.globalStatementList().begin(); sit != _unit.globalStatementList().end(); ++sit) {
        const Ast::Statement* statement = *sit;
        generateGlobalStatement(statement);
    }
}

inline void Generator::Impl::enterNamespace() {
    fprintf(_fpHdr, "%s", Indent::get());
    for(Ast::Unit::UnitNS::const_iterator it = _unit.unitNS().begin(); it != _unit.unitNS().end(); ++it) {
        const Ast::Token& name = *it;
        fprintf(_fpHdr, "namespace %s {", name.text());
    }
    fprintf(_fpHdr, "\n");
}

inline void Generator::Impl::leaveNamespace() {
    fprintf(_fpHdr, "%s", Indent::get());
    for(Ast::Unit::UnitNS::const_iterator it = _unit.unitNS().begin(); it != _unit.unitNS().end(); ++it) {
        const Ast::Token& name = *it;
        fprintf(_fpHdr, "} /* %s */ ", name.text());
    }
    fprintf(_fpHdr, "\n");
}

inline void Generator::Impl::generateImportStatement() {
    for(Ast::Unit::ImportStatementList::const_iterator sit = _unit.importStatementList().begin(); sit != _unit.importStatementList().end(); ++sit) {
        const Ast::ImportStatement* s = *sit;
        std::string qt = (ref(s).headerType() == Ast::HeaderType::Import)?"<>":"\"\"";
        fprintf(_fpHdr, "#include %c", qt.at(0));
        std::string sep = "";
        for(Ast::ImportStatement::Part::const_iterator it = ref(s).part().begin(); it != ref(s).part().end(); ++it) {
            const Ast::Token& name = *it;
            fprintf(_fpHdr, "%s%s", sep.c_str(), name.text());
            sep = "/";
        }
        fprintf(_fpHdr, ".hpp%c\n", qt.at(1));
    }
    fprintf(_fpHdr, "\n");
}

inline void Generator::Impl::run() {
    Indent::init();
    std::string basename = getBaseName(_unit.name().string());
    OutputFile ofImp(_fpImp, basename + "ipp");
    OutputFile ofHdr(_fpHdr, basename + "hpp");
    OutputFile ofSrc(_fpSrc, basename + "cpp");

    fprintf(_fpHdr, "#pragma once\n\n");
    fprintf(_fpHdr, "#include \"base/function.hpp\"\n");

    fprintf(_fpSrc, "#include \"base/pch.hpp\"\n");
    fprintf(_fpSrc, "#include \"base/common.hpp\"\n");
    fprintf(_fpSrc, "#include \"base/exception.hpp\"\n");
    fprintf(_fpSrc, "#include \"%shpp\"\n", basename.c_str());

    generateImportStatement();

    enterNamespace();
    generateGlobalStatementList();
    leaveNamespace();
}

//////////////////////////////////////////////
Generator::Generator(const Project& project, const Ast::Unit& unit) : _impl(0) {_impl = new Impl(project, unit);}
Generator::~Generator() {delete _impl;}
void Generator::run() {return ref(_impl).run();}
