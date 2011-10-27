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
    if(dynamic_cast<const Ast::EnumDefn*>(ptr(typeSpec)) != 0) {
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
        if(node.accessType() == Ast::AccessType::Parent) {
            const Ast::ChildTypeSpec* childTypeSpec = dynamic_cast<const Ast::ChildTypeSpec*>(ptr(node));
            if(childTypeSpec) {
                return fpDecl(ref(childTypeSpec).parent());
            }
        }
        return _fpSrc;
    }

    void visit(const Ast::TypedefDefn& node) {
        if(node.defType() == Ast::DefinitionType::Native) {
            fprintf(fpDecl(node), "%s// typedef %s native;\n", Indent::get(), node.name().text());
        }
    }

    void visit(const Ast::EnumDefn& node) {
        if(node.defType() != Ast::DefinitionType::Native) {
            fprintf(fpDecl(node), "%sstruct %s {\n", Indent::get(), node.name().text());
            fprintf(fpDecl(node), "%s  enum T {\n", Indent::get(), node.name().text());
            std::string sep = " ";
            for(Ast::EnumMemberDefnList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                INDENT;
                const Ast::EnumMemberDefn& def = ref(*it);
                fprintf(fpDecl(node), "%s%s %s\n", Indent::get(), sep.c_str(), def.name().text());
                sep = ",";
            }
            fprintf(fpDecl(node), "%s  };\n", Indent::get());
            fprintf(fpDecl(node), "%s};\n", Indent::get());
            fprintf(fpDecl(node), "\n");
        }
    }

    void visit(const Ast::StructDefn& node) {
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
            for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(fp, "%s%s _%s;\n", Indent::get(), getName(vdef.qualifiedTypeSpec()).c_str(), vdef.name().text());
            }
            fprintf(fp, "%spublic:\n", Indent::get());
            for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& vdef = ref(*it);
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

    void visitBlock(const Ast::CompoundStatement& block);

    void visitRoutine(FILE* fp, const Ast::Routine& node) {
        fprintf(fp, "%s%s %s(", Indent::get(), getName(node.outType()).c_str(), node.name().text());
        std::string sep;
        for(Ast::Scope::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fp, "%s%s %s", sep.c_str(), getName(vdef.qualifiedTypeSpec()).c_str(), vdef.name().text());
            sep = ", ";
        }
        fprintf(fp, ")");
    }

    void visit(const Ast::RoutineDecl& node) {
        visitRoutine(fpDecl(node), node);
        fprintf(fpDecl(node), ";\n");
        fprintf(fpDecl(node), "\n");
    }

    void visit(const Ast::RoutineDefn& node) {
        visitRoutine(fpDecl(node), node);
        fprintf(fpDecl(node), ";\n");
        fprintf(fpDecl(node), "\n");

        visitRoutine(_fpSrc, node);
        fprintf(_fpSrc, "\n");
        visitBlock(node.block());
    }

    void visitFunction(const Ast::Function& node) {
        fprintf(fpDecl(node), "%sclass %s : public Function<%s> {\n", Indent::get(), node.name().text(), node.name().text());
        fprintf(fpDecl(node), "%s    static %s& impl(%s& This);\n", Indent::get(), node.name().text(), node.name().text());
        for(Ast::Scope::List::const_iterator it = node.sig().in().begin(); it != node.sig().in().end(); ++it) {
            INDENT;
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fpDecl(node), "%sconst %s _%s;\n", Indent::get(), getName(vdef.qualifiedTypeSpec().typeSpec()).c_str(), vdef.name().text());
        }
        fprintf(fpDecl(node), "%spublic:\n", Indent::get());
        fprintf(fpDecl(node), "%s    inline %s(", Indent::get(), node.name().text());
        std::string sep = "";
        for(Ast::Scope::List::const_iterator it = node.sig().in().begin(); it != node.sig().in().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fpDecl(node), "%sconst %s& %s", sep.c_str(), getName(vdef.qualifiedTypeSpec().typeSpec()).c_str(), vdef.name().text());
            sep = ", ";
        }
        fprintf(fpDecl(node), ") : Function(&impl)");
        for(Ast::Scope::List::const_iterator it = node.sig().in().begin(); it != node.sig().in().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fpDecl(node), ", _%s(%s)", vdef.name().text(), vdef.name().text());
        }
        fprintf(fpDecl(node), " {}\n");

        fprintf(fpDecl(node), "%s};\n", Indent::get());
    }

    void visit(const Ast::FunctionDecl& node) {
        visitFunction(node);
        fprintf(fpDecl(node), "\n");
    }

    void visit(const Ast::FunctionDefn& node) {
        visitFunction(node);

        fprintf(_fpSrc, "%s%s& %s::impl(%s& This)\n", Indent::get(), node.name().text(), node.name().text(), node.name().text());
        visitBlock(node.block());
        fprintf(fpDecl(node), "\n");
    }

    void visit(const Ast::EventDecl& node) {
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

struct ExprGenerator : public Ast::Expr::Visitor {
public:
    inline ExprGenerator(FILE* fp) : _fp(fp) {}
private:
    virtual void visit(const Ast::TernaryOpExpr& node) {
        fprintf(_fp, "(");
        visitNode(node.lhs());
        fprintf(_fp, "%s", node.op1().text());
        visitNode(node.rhs1());
        fprintf(_fp, "%s", node.op2().text());
        visitNode(node.rhs2());
        fprintf(_fp, ")");
    }

    virtual void visit(const Ast::BinaryOpExpr& node) {
    }

    virtual void visit(const Ast::PostfixOpExpr& node) {
    }

    virtual void visit(const Ast::PrefixOpExpr& node) {
    }

    virtual void visit(const Ast::StructMemberRefExpr& node) {
    }

    virtual void visit(const Ast::EnumMemberRefExpr& node) {
    }

    virtual void visit(const Ast::ConstantExpr& node) {
    }

private:
    FILE* _fp;
};

struct StatementGenerator : public Ast::Statement::Visitor {
public:
    inline StatementGenerator(FILE* fpHdr, FILE* fpSrc, FILE* fpImp) : _fpHdr(fpHdr), _fpSrc(fpSrc), _fpImp(fpImp) {}
private:
    virtual void visit(const Ast::ImportStatement& node) {
        std::string qt = (node.headerType() == Ast::HeaderType::Import)?"<>":"\"\"";
        fprintf(_fpHdr, "#include %c", qt.at(0));
        std::string sep = "";
        for(Ast::ImportStatement::Part::const_iterator it = node.part().begin(); it != node.part().end(); ++it) {
            const Ast::Token& name = *it;
            fprintf(_fpHdr, "%s%s", sep.c_str(), name.text());
            sep = "/";
        }
        fprintf(_fpHdr, ".hpp%c\n", qt.at(1));
    }

    virtual void visit(const Ast::UserDefinedTypeSpecStatement& node) {
        TypeDeclarationGenerator(_fpHdr, _fpSrc, _fpImp).visitNode(node.typeSpec());
    }

    virtual void visit(const Ast::ExprStatement& node) {
    }

    void visitReturnStatement(const Ast::ReturnStatement& node) {
        fprintf(_fpSrc, "%sreturn", Indent::get());
        if(node.exprList().list().size() > 0) {
            fprintf(_fpSrc, " (");
            ExprGenerator(_fpSrc).visitList(node.exprList());
            fprintf(_fpSrc, ")");
        }
        fprintf(_fpSrc, ";\n");
    }

    virtual void visit(const Ast::RoutineReturnStatement& node) {
        visitReturnStatement(node);
    }

    virtual void visit(const Ast::FunctionReturnStatement& node) {
        visitReturnStatement(node);
    }

    virtual void visit(const Ast::CompoundStatement& node) {
        fprintf(_fpSrc, "%s{\n", Indent::get());
        {
            INDENT;
            for(Ast::CompoundStatement::List::const_iterator sit = node.list().begin(); sit != node.list().end(); ++sit) {
                const Ast::Statement& s = ref(*sit);
                ref(this).visitNode(s);
            }
        }
        fprintf(_fpSrc, "%s}\n", Indent::get());
    }

private:
    FILE* _fpHdr;
    FILE* _fpSrc;
    FILE* _fpImp;
};

void TypeDeclarationGenerator::visitBlock(const Ast::CompoundStatement& block) {
    StatementGenerator gen(_fpHdr, _fpSrc, _fpImp);
    gen.visitNode(block);
}

struct Generator::Impl {
    inline Impl(const Project& project, const Ast::Unit& unit) : _project(project), _unit(unit), _fpHdr(0), _fpSrc(0), _fpImp(0) {}
    inline void generateHeaderIncludes(const std::string& basename);
    inline void run();
private:
    const Project& _project;
    const Ast::Unit& _unit;
private:
    FILE* _fpHdr;
    FILE* _fpSrc;
    FILE* _fpImp;
};

inline void Generator::Impl::generateHeaderIncludes(const std::string& basename) {
    fprintf(_fpHdr, "#pragma once\n\n");
    for(Project::PathList::const_iterator it = _project.includeFileList().begin(); it != _project.includeFileList().end(); ++it) {
        const std::string& filename = *it;
        fprintf(_fpSrc, "#include \"%s\"\n", filename.c_str());
    }
    fprintf(_fpSrc, "#include \"%shpp\"\n", basename.c_str());

    StatementGenerator gen(_fpHdr, _fpSrc, _fpImp);
    for(Ast::Unit::ImportStatementList::const_iterator sit = _unit.importStatementList().begin(); sit != _unit.importStatementList().end(); ++sit) {
        const Ast::ImportStatement& s = ref(*sit);
        gen.visitNode(s);
    }
    fprintf(_fpHdr, "\n");
}

inline void Generator::Impl::run() {
    Indent::init();
    std::string basename = getBaseName(_unit.filename());
    OutputFile ofImp(_fpImp, basename + "ipp");unused(ofImp);
    OutputFile ofHdr(_fpHdr, basename + "hpp");unused(ofHdr);
    OutputFile ofSrc(_fpSrc, basename + "cpp");unused(ofSrc);

    generateHeaderIncludes(basename);

    TypeDeclarationGenerator(_fpHdr, _fpSrc, _fpImp).visitChildren(_unit.rootNS());
    fprintf(_fpHdr, "\n");
    fprintf(_fpSrc, "\n\n");
}

//////////////////////////////////////////////
Generator::Generator(const Project& project, const Ast::Unit& unit) : _impl(0) {_impl = new Impl(project, unit);}
Generator::~Generator() {delete _impl;}
void Generator::run() {return ref(_impl).run();}
