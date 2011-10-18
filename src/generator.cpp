#include "pch.hpp"
#include "common.hpp"
#include "exception.hpp"
#include "generator.hpp"

struct Indent {
    inline Indent(int& indent) : _indent(indent) {
        ++_indent;
    }
    inline ~Indent() {
        --_indent;
    }
    inline void generate(FILE* fp) {
        for(int i = 0; i < (_indent - 1); ++i) {
            fputc(' ', fp);
        }
    }

private:
    int& _indent;
};

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
    inline Impl(const Project& project, const Ast::Unit& unit) : _project(project), _unit(unit), _fpHdr(0), _fpSrc(0), _fpImp(0), _indent(0) {}
    inline void generateTypeSpec(const Ast::TypeSpec* typeSpec);
    inline void generateGlobalStatement(const Ast::Statement* statement);
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
    int _indent;
};

inline void Generator::Impl::generateTypeSpec(const Ast::TypeSpec* typeSpec) {
    for(const Ast::TypeDef* t = dynamic_cast<const Ast::TypeDef*>(typeSpec); t != 0; ) {
        if(t->defType() == Ast::DefinitionType::Native) {
            fprintf(_fpHdr, "// typedef %s native;\n", t->name().text());
        }
        return;
    }

    for(const Ast::StructDef* t = dynamic_cast<const Ast::StructDef*>(typeSpec); t != 0; ) {
        fprintf(_fpHdr, "// struct %s ;\n", t->name().text());
        if(t->defType() == Ast::DefinitionType::Native) {
        }
        return;
    }

    for(const Ast::FunctionDef* t = dynamic_cast<const Ast::FunctionDef*>(typeSpec); t != 0; ) {
        fprintf(_fpHdr, "// function %s;\n", t->name().text());
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

inline void Generator::Impl::enterNamespace() {
    for(Ast::Unit::UnitNS::const_iterator it = _unit.unitNS().begin(); it != _unit.unitNS().end(); ++it) {
        const Ast::Token& name = *it;
        fprintf(_fpHdr, "namespace %s {", name.text());
    }
    fprintf(_fpHdr, "\n");
}

inline void Generator::Impl::leaveNamespace() {
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

inline void Generator::Impl::run() {
    std::string basename = getBaseName(_unit.name().string());
    OutputFile ofImp(_fpImp, basename + "ipp");
    OutputFile ofHdr(_fpHdr, basename + "hpp");
    OutputFile ofSrc(_fpSrc, basename + "cpp");
    fprintf(_fpHdr, "#pragma once\n\n");
    fprintf(_fpHdr, "#include \"function.hpp\"\n");
    generateImportStatement();

    enterNamespace();
    for(Ast::Unit::StatementList::const_iterator sit = _unit.globalStatementList().begin(); sit != _unit.globalStatementList().end(); ++sit) {
        const Ast::Statement* statement = *sit;
        generateGlobalStatement(statement);
    }
    leaveNamespace();
}

//////////////////////////////////////////////
Generator::Generator(const Project& project, const Ast::Unit& unit) : _impl(0) {_impl = new Impl(project, unit);}
Generator::~Generator() {delete _impl;}
void Generator::run() {return ref(_impl).run();}
