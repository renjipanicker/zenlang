#include "pch.hpp"
#include "common.hpp"
#include "exception.hpp"
#include "generator.hpp"

inline bool getName(const Ast::TypeSpec& typeSpec, const std::string& sep, std::string& name) {
    const Ast::ChildTypeSpec* ctypeSpec = dynamic_cast<const Ast::ChildTypeSpec*>(ptr(typeSpec));
    if(!ctypeSpec)
        return false;

    if(getName(ref(ctypeSpec).parent(), sep, name))
        name += sep;
    name += typeSpec.name().string();
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
    inline static const char* get() {printf("ind: %d\n", strlen(ind)); return ind;}
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
#define INDENT Indent _ind_;

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
        if(t->defType() == Ast::DefinitionType::Native) {
            fprintf(_fpHdr, "%s// typedef %s native;\n", Indent::get(), t->name().text());
        }
        return;
    }

    for(const Ast::StructDef* t = dynamic_cast<const Ast::StructDef*>(typeSpec); t != 0; ) {
        FILE* fp = 0;
        std::string impl;
        if(t->accessType() == Ast::AccessType::Protected) {
            fprintf(_fpHdr, "%sstruct %s {\n", Indent::get(), t->name().text());
            INDENT {
                fprintf(_fpHdr, "%sstruct Impl;\n", Indent::get());
                fprintf(_fpHdr, "%sImpl* _impl;\n", Indent::get());
            }
            fprintf(_fpHdr, "%s};\n", Indent::get());
            fp= _fpSrc;
            impl = "::Impl";
        } else {
            fp = (t->accessType() == Ast::AccessType::Private)?_fpSrc:_fpHdr;
        }

        INDENT {
            fprintf(fp, "%sstruct %s%s {\n", Indent::get(), t->name().text(), impl.c_str());
            for(Ast::VariableDefList::List::const_iterator it = ref(t).list().begin(); it != ref(t).list().end(); ++it) {
                const Ast::VariableDef& vdef = ref(*it);
                fprintf(fp, "%s%s %s;\n", Indent::get(), getName(vdef.qualifiedTypeSpec()).c_str(), vdef.name().text());
            }
        }
        fprintf(fp, "%s};\n", Indent::get());
        return;
    }

    for(const Ast::FunctionDef* t = dynamic_cast<const Ast::FunctionDef*>(typeSpec); t != 0; ) {
        fprintf(_fpHdr, "%s// function %s;\n", Indent::get(), t->name().text());
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
    fprintf(_fpHdr, "#include \"function.hpp\"\n");
    generateImportStatement();

    enterNamespace();
    generateGlobalStatementList();
    leaveNamespace();
}

//////////////////////////////////////////////
Generator::Generator(const Project& project, const Ast::Unit& unit) : _impl(0) {_impl = new Impl(project, unit);}
Generator::~Generator() {delete _impl;}
void Generator::run() {return ref(_impl).run();}
