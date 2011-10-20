#include "base/pch.hpp"
#include "base/zenlang.hpp"
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

struct TypeDeclarationGenerator : public Ast::TypeSpec::Visitor {
    inline FILE* fpDecl(const Ast::TypeSpec& node) {
        if(node.accessType() == Ast::AccessType::Public) {
            return _fpHdr;
        }
        if(node.accessType() == Ast::AccessType::Private) {
            return _fpSrc;
        }
        if(node.accessType() == Ast::AccessType::Parent) {
            const Ast::ChildTypeSpec* childTypeSpec = dynamic_cast<const Ast::ChildTypeSpec*>(ptr(node));
            if(childTypeSpec) {
                return fpDecl(ref(childTypeSpec).parent());
            }
        }
        throw Exception("Invalid access state: %d for %s\n", node.accessType(), node.name().text());
    }

    void visit(const Ast::TypeDef& node) {
        if(node.defType() == Ast::DefinitionType::Native) {
            fprintf(fpDecl(node), "%s// typedef %s native;\n", Indent::get(), node.name().text());
        }
    }

    void visit(const Ast::EnumDef& node) {
        if(node.defType() != Ast::DefinitionType::Native) {
            fprintf(fpDecl(node), "%sstruct %s {\n", Indent::get(), node.name().text());
            fprintf(fpDecl(node), "%s  enum T {\n", Indent::get(), node.name().text());
            std::string sep = " ";
            for(Ast::EnumMemberDefList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                INDENT;
                const Ast::EnumMemberDef& def = ref(*it);
                fprintf(fpDecl(node), "%s%s %s\n", Indent::get(), sep.c_str(), def.name().text());
                sep = ",";
            }
            fprintf(fpDecl(node), "%s  };\n", Indent::get());
            fprintf(fpDecl(node), "%s};\n", Indent::get());
            fprintf(fpDecl(node), "\n");
        }
    }

    void visit(const Ast::StructDef& node) {
        FILE* fp = 0;
        std::string impl;
        if(node.accessType() == Ast::AccessType::Protected) {
            fprintf(_fpHdr, "%sstruct %s {\n", Indent::get(), node.name().text());
            {
                INDENT;
                fprintf(_fpHdr, "%sstruct Impl;\n", Indent::get());
                fprintf(_fpHdr, "%sImpl* _impl;\n", Indent::get());
            }
            fprintf(_fpHdr, "%s};\n", Indent::get());
            fprintf(_fpHdr, "\n");
            impl = "::Impl";
            fp = _fpSrc;
        } else {
            fp = fpDecl(node);
        }

        if(node.defType() != Ast::DefinitionType::Native) {
            fprintf(fp, "%sstruct %s%s {\n", Indent::get(), node.name().text(), impl.c_str());
            fprintf(fp, "%sprivate:\n", Indent::get());
            for(Ast::VariableDefList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                INDENT;
                const Ast::VariableDef& vdef = ref(*it);
                fprintf(fp, "%s%s _%s;\n", Indent::get(), getName(vdef.qualifiedTypeSpec()).c_str(), vdef.name().text());
            }
            fprintf(fp, "%spublic:\n", Indent::get());
            for(Ast::VariableDefList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                INDENT;
                const Ast::VariableDef& vdef = ref(*it);
                fprintf(fp, "%sinline %s& %s(const %s& val) {_%s = val; return ref(this);}\n",
                        Indent::get(), node.name().text(), vdef.name().text(), getName(vdef.qualifiedTypeSpec()).c_str(), vdef.name().text());
                fprintf(fp, "%sinline const %s& %s() const {return _%s;}\n",
                        Indent::get(), getName(vdef.qualifiedTypeSpec()).c_str(), vdef.name().text(), vdef.name().text());
            }
            fprintf(fp, "%s};\n", Indent::get());
            fprintf(fp, "\n");
        }
        return;
    }

    void visit(const Ast::RoutineDef& node) {
        fprintf(fpDecl(node), "%s%s %s(", Indent::get(), getName(node.outType()).c_str(), node.name().text());
        std::string sep;
        for(Ast::VariableDefList::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
            const Ast::VariableDef& vdef = ref(*it);
            fprintf(fpDecl(node), "%s%s %s", sep.c_str(), getName(vdef.qualifiedTypeSpec()).c_str(), vdef.name().text());
            sep = ", ";
        }
        fprintf(fpDecl(node), ");\n");
        fprintf(fpDecl(node), "\n");
        return;
    }

    void visit(const Ast::FunctionDef& node) {
        fprintf(fpDecl(node), "%sclass %s : public Function<%s> {\n", Indent::get(), node.name().text(), node.name().text());
        fprintf(fpDecl(node), "%s    static void impl(%s& This);\n", Indent::get(), node.name().text());
        for(Ast::VariableDefList::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
            INDENT;
            const Ast::VariableDef& vdef = ref(*it);
            fprintf(fpDecl(node), "%sconst %s _%s;\n", Indent::get(), getName(vdef.qualifiedTypeSpec().typeSpec()).c_str(), vdef.name().text());
        }
        fprintf(fpDecl(node), "%spublic:\n", Indent::get());
        fprintf(fpDecl(node), "%s    inline %s(", Indent::get(), node.name().text());
        std::string sep = "";
        for(Ast::VariableDefList::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
            const Ast::VariableDef& vdef = ref(*it);
            fprintf(fpDecl(node), "%sconst %s& %s", sep.c_str(), getName(vdef.qualifiedTypeSpec().typeSpec()).c_str(), vdef.name().text());
            sep = ", ";
        }
        fprintf(fpDecl(node), ") : Function(&impl)");
        for(Ast::VariableDefList::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
            const Ast::VariableDef& vdef = ref(*it);
            fprintf(fpDecl(node), ", _%s(%s)", vdef.name().text(), vdef.name().text());
        }
        fprintf(fpDecl(node), " {}\n");

        fprintf(fpDecl(node), "%s};\n", Indent::get());
        fprintf(fpDecl(node), "\n");
        return;
    }

    void visit(const Ast::EventDef& node) {
        fprintf(fpDecl(node), "%sstruct %s : public Event<%s> {\n", Indent::get(), node.name().text(), node.name().text());
        {
            INDENT;
            fprintf(fpDecl(node), "%sclass Add : public AddHandler<Add> {\n", Indent::get());
            fprintf(fpDecl(node), "%s    static void impl(Add& This);\n", Indent::get());
            fprintf(fpDecl(node), "%s    const %s _%s;\n", Indent::get(), getName(node.in().qualifiedTypeSpec().typeSpec()).c_str(), node.in().name().text());
            fprintf(fpDecl(node), "%spublic:\n", Indent::get());
            fprintf(fpDecl(node), "%s    inline Add(", Indent::get(), node.name().text());
            fprintf(fpDecl(node), "const %s& %s, Handler* handler", getName(node.in().qualifiedTypeSpec().typeSpec()).c_str(), node.in().name().text());
            fprintf(fpDecl(node), ") : AddHandler(&impl, add(handler)), _%s(%s) {}\n", node.in().name().text(), node.in().name().text());
            fprintf(fpDecl(node), "%s};\n", Indent::get());
        }
        fprintf(fpDecl(node), "%s};\n", Indent::get());
        fprintf(fpDecl(node), "\n");
        return;
    }

    void visit(const Ast::Namespace& node) {
        fprintf(_fpHdr, "namespace %s {\n", node.name().text());
        fprintf(_fpSrc, "namespace %s {\n", node.name().text());
        {
            INDENT;
            visitChildren(node);
        }
        fprintf(_fpHdr, "} /* %s */", node.name().text());
        fprintf(_fpSrc, "} /* %s */", node.name().text());
    }

    void visit(const Ast::Root& node) {
        printf("root\n");
        visitChildren(node);
    }

public:
    inline TypeDeclarationGenerator(FILE* fpHdr, FILE* fpSrc, FILE* fpImp) : _fpHdr(fpHdr), _fpSrc(fpSrc), _fpImp(fpImp) {}
private:
    FILE* _fpHdr;
    FILE* _fpSrc;
    FILE* _fpImp;
};

struct Generator::Impl {
    inline Impl(const Project& project, const Ast::Unit& unit) : _project(project), _unit(unit), _fpHdr(0), _fpSrc(0), _fpImp(0) {}
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

inline void Generator::Impl::generateGlobalStatement(const Ast::Statement* statement) {
    for(const Ast::UserDefinedTypeSpecStatement* s = dynamic_cast<const Ast::UserDefinedTypeSpecStatement*>(statement); s != 0; ) {
        return ref(s).typeSpec().visit(TypeDeclarationGenerator(_fpHdr, _fpSrc, _fpImp));
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
    fprintf(_fpSrc, "%s", Indent::get());
    for(Ast::Unit::UnitNS::const_iterator it = _unit.unitNS().begin(); it != _unit.unitNS().end(); ++it) {
        const Ast::Token& name = *it;
        fprintf(_fpHdr, "namespace %s {", name.text());
        fprintf(_fpSrc, "namespace %s {", name.text());
    }
    fprintf(_fpHdr, "\n");
    fprintf(_fpSrc, "\n");
}

inline void Generator::Impl::leaveNamespace() {
    fprintf(_fpHdr, "%s", Indent::get());
    fprintf(_fpSrc, "%s", Indent::get());
    for(Ast::Unit::UnitNS::const_iterator it = _unit.unitNS().begin(); it != _unit.unitNS().end(); ++it) {
        const Ast::Token& name = *it;
        fprintf(_fpHdr, "} /* %s */ ", name.text());
        fprintf(_fpSrc, "} /* %s */ ", name.text());
    }
    fprintf(_fpHdr, "\n");
    fprintf(_fpSrc, "\n");
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
    std::string basename = getBaseName(_unit.filename());
    OutputFile ofImp(_fpImp, basename + "ipp");
    OutputFile ofHdr(_fpHdr, basename + "hpp");
    OutputFile ofSrc(_fpSrc, basename + "cpp");

    fprintf(_fpHdr, "#pragma once\n\n");
    for(Project::PathList::const_iterator it = _project.sourceList().begin(); it != _project.sourceList().end(); ++it) {
        const std::string& filename = *it;
        fprintf(_fpSrc, "#include \"%s\"\n", filename.c_str());
    }

    fprintf(_fpSrc, "#include \"%shpp\"\n", basename.c_str());

    generateImportStatement();
    TypeDeclarationGenerator(_fpHdr, _fpSrc, _fpImp).visitChildren(_unit.rootNS());
}

//////////////////////////////////////////////
Generator::Generator(const Project& project, const Ast::Unit& unit) : _impl(0) {_impl = new Impl(project, unit);}
Generator::~Generator() {delete _impl;}
void Generator::run() {return ref(_impl).run();}
