#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "generator.hpp"
#include "outfile.hpp"
#include "typename.hpp"

inline std::string getDefinitionType(const Ast::DefinitionType::T& defType) {
    switch(defType) {
        case Ast::DefinitionType::Direct:
            return "";
        case Ast::DefinitionType::Native:
            return " native";
        case Ast::DefinitionType::Abstract:
            return " abstract";
    }
    throw Exception("Internal error: Unknown Definition Type '%d'\n", defType);
}

inline std::string getAccessType(const Ast::AccessType::T& accessType) {
    switch(accessType) {
        case Ast::AccessType::Private:
            return "";
        case Ast::AccessType::Public:
            return "public ";
        case Ast::AccessType::Internal:
            return "internal ";
        case Ast::AccessType::External:
            return "external ";
        case Ast::AccessType::Parent:
            return "";
    }
    throw Exception("Internal error: Unknown Access Type '%d'\n", accessType);
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

struct FileSet {
    inline FileSet(FILE* fpHdr, FILE* fpSrc, FILE* fpImp) : _fpHdr(fpHdr), _fpSrc(fpSrc), _fpImp(fpImp) {}
    FILE* _fpHdr;
    FILE* _fpSrc;
    FILE* _fpImp;
};

struct GeneratorContext {
    struct TargetMode {
        enum T {
            Import,
            TypeDecl,
            TypeDefn,
            Local
        };
    };
    struct IndentMode {
        enum T {
            WithBrace,     /// display brace
            IndentedBrace, /// indent and display brace
            NoBrace        /// no braces
        };
    };

    TargetMode::T _targetMode;
    IndentMode::T _indentMode;
    inline GeneratorContext(const TargetMode::T& targetMode, const IndentMode::T& indentMode) : _targetMode(targetMode), _indentMode(indentMode) {}
    void run(const Ast::Config& config, FileSet& fs, const std::string& basename, const Ast::Statement& block);
};

struct ExprGenerator : public Ast::Expr::Visitor {
public:
    inline ExprGenerator(FILE* fp, const GenMode::T& genMode, const std::string& sep2 = "", const std::string& sep1 = "") : _fp(fp), _genMode(genMode), _sep2(sep2), _sep1(sep1), _sep0(sep1) {}
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
        visitNode(node.lhs());
        fprintf(_fp, "%s", node.op().text());
        visitNode(node.rhs());
    }

    virtual void visit(const Ast::SetIndexExpr& node) {
        visitNode(node.lhs().expr());
        fprintf(_fp, ".set(");
        visitNode(node.lhs().index());
        fprintf(_fp, ", ");
        visitNode(node.rhs());
        fprintf(_fp, ")");
    }

    virtual void visit(const Ast::PostfixOpExpr& node) {
        visitNode(node.lhs());
        fprintf(_fp, "%s", node.op().text());
    }

    virtual void visit(const Ast::PrefixOpExpr& node) {
        fprintf(_fp, "%s", node.op().text());
        visitNode(node.rhs());
    }

    virtual void visit(const Ast::ListExpr& node) {
        if(_genMode == GenMode::Import) {
            fprintf(_fp, "[");
            if(node.list().list().size() == 0) {
                fprintf(_fp, "%s", getQualifiedTypeSpecName(node.list().valueType(), _genMode).c_str());
            } else {
                std::string sep;
                for(Ast::ListList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                    const Ast::ListItem& item = ref(*it);
                    fprintf(_fp, "%s", sep.c_str());
                    visitNode(item.valueExpr());
                    sep = ", ";
                }
            }
            fprintf(_fp, "]");
        } else {
            fprintf(_fp, "list<%s>::creator()", getQualifiedTypeSpecName(node.list().valueType(), _genMode).c_str());
            for(Ast::ListList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const Ast::ListItem& item = ref(*it);
                fprintf(_fp, ".add(");
                visitNode(item.valueExpr());
                fprintf(_fp, ")");
            }
            fprintf(_fp, ".get()");
        }
    }

    virtual void visit(const Ast::DictExpr& node) {
        if(_genMode == GenMode::Import) {
            fprintf(_fp, "[");
            if(node.list().list().size() == 0) {
                fprintf(_fp, "%s:%s", getQualifiedTypeSpecName(node.list().keyType(), _genMode).c_str(), getQualifiedTypeSpecName(node.list().valueType(), _genMode).c_str());
            } else {
                std::string sep;
                for(Ast::DictList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                    const Ast::DictItem& item = ref(*it);
                    fprintf(_fp, "%s", sep.c_str());
                    visitNode(item.valueExpr());
                    fprintf(_fp, ":");
                    visitNode(item.valueExpr());
                    sep = ", ";
                }
            }
            fprintf(_fp, "]");
        } else {
            fprintf(_fp, "dict<%s, %s>::creator()", getQualifiedTypeSpecName(node.list().keyType(), _genMode).c_str(), getQualifiedTypeSpecName(node.list().valueType(), _genMode).c_str());
            for(Ast::DictList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const Ast::DictItem& item = ref(*it);
                fprintf(_fp, ".add(");
                visitNode(item.keyExpr());
                fprintf(_fp, ", ");
                visitNode(item.valueExpr());
                fprintf(_fp, ")");
            }
            fprintf(_fp, ".get()");
        }
    }

    virtual void visit(const Ast::FormatExpr& node) {
        fprintf(_fp, "Formatter(");
        visitNode(node.stringExpr());
        fprintf(_fp, ")");
        for(Ast::DictList::List::const_iterator it = node.dictExpr().list().list().begin(); it != node.dictExpr().list().list().end(); ++it) {
            const Ast::DictItem& item = ref(*it);
            fprintf(_fp, ".add(");
            visitNode(item.keyExpr());
            fprintf(_fp, ", ");
            visitNode(item.valueExpr());
            fprintf(_fp, ")");
        }
        fprintf(_fp, ".get()");
    }

    virtual void visit(const Ast::RoutineCallExpr& node) {
        const std::string name = getTypeSpecName(node.routine(), _genMode);
        if((name == "assert") || (name == "unused")) {
            std::string sep;
            for(Ast::ExprList::List::const_iterator it = node.exprList().list().begin(); it != node.exprList().list().end(); ++it) {
                const Ast::Expr& expr = ref(*it);
                fprintf(_fp, "%s%s(", sep.c_str(), name.c_str());
                ExprGenerator(_fp, _genMode).visitNode(expr);
                fprintf(_fp, ")");
                sep = ";";
            }
            return;
        }

        if(name == "check") {
        }

        fprintf(_fp, "%s(", name.c_str());
        ExprGenerator(_fp, _genMode, ", ").visitList(node.exprList());
        fprintf(_fp, ")");
    }

    virtual void visit(const Ast::FunctorCallExpr& node) {
        ExprGenerator(_fp, _genMode).visitNode(node.expr());
        fprintf(_fp, ".run(");
        ExprGenerator(_fp, _genMode, ", ").visitList(node.exprList());
        fprintf(_fp, ")");
    }

    virtual void visit(const Ast::RunExpr& node) {
        fprintf(_fp, "CallContext::get().add(");
        ExprGenerator(_fp, _genMode).visitNode(node.callExpr().expr());
        fprintf(_fp, ", ");
        fprintf(_fp, "%s::_In(", getTypeSpecName(node.callExpr().expr().qTypeSpec().typeSpec(), _genMode).c_str());
        ExprGenerator(_fp, _genMode, ", ").visitList(node.callExpr().exprList());
        fprintf(_fp, "))");
    }

    virtual void visit(const Ast::OrderedExpr& node) {
        fprintf(_fp, "(");
        visitNode(node.expr());
        fprintf(_fp, ")");
    }

    virtual void visit(const Ast::IndexExpr& node) {
        std::string at = "at";

        const Ast::PropertyDecl* ts = resolveTypedefT<Ast::PropertyDecl>(node.expr().qTypeSpec().typeSpec());
        if(ts != 0) {
            at = ref(ts).name().string();
        }

        visitNode(node.expr());
        fprintf(_fp, ".%s(", at.c_str());
        visitNode(node.index());
        fprintf(_fp, ")");
    }

    virtual void visit(const Ast::TypeofTypeExpr& node) {
        fprintf(_fp, "type(\"%s\")", getQualifiedTypeSpecName(node.typeSpec(), _genMode).c_str());
    }

    virtual void visit(const Ast::TypeofExprExpr& node) {
        const Ast::TypeSpec* typeSpec = ptr(node.expr().qTypeSpec().typeSpec());
        const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(typeSpec);
        if(td) {
            visitNode(node.expr());
            fprintf(_fp, ".tname()");
        } else {
            fprintf(_fp, "type(\"%s\")", getQualifiedTypeSpecName(node.expr().qTypeSpec(), _genMode).c_str());
        }
    }

    virtual void visit(const Ast::StaticTypecastExpr& node) {
        fprintf(_fp, "static_cast<%s>(", getQualifiedTypeSpecName(node.qTypeSpec(), _genMode).c_str());
        ExprGenerator(_fp, _genMode).visitNode(node.expr());
        fprintf(_fp, ")");
    }

    virtual void visit(const Ast::DynamicTypecastExpr& node) {
        ExprGenerator(_fp, _genMode).visitNode(node.expr());
        fprintf(_fp, ".getT<%s>()", getTypeSpecName(node.qTypeSpec().typeSpec(), _genMode).c_str());
    }

    virtual void visit(const Ast::PointerInstanceExpr& node) {
        const Ast::Expr& expr = node.exprList().at(0);
        const std::string dname = getTypeSpecName(expr.qTypeSpec().typeSpec(), _genMode);

        if(_genMode == GenMode::Import) {
            fprintf(_fp, "&(%s())", dname.c_str());
        } else {
            const std::string bname = getTypeSpecName(node.templateDefn().at(0).typeSpec(), _genMode);
            fprintf(_fp, "Creator<%s, %s>::get(", bname.c_str(), dname.c_str());
            fprintf(_fp, "type(\"%s\"), ", dname.c_str());
            ExprGenerator(_fp, _genMode).visitNode(expr);
            fprintf(_fp, ")");
        }
    }

    virtual void visit(const Ast::ValueInstanceExpr& node) {
        const Ast::Expr& expr = node.exprList().at(0);
        ExprGenerator(_fp, _genMode).visitNode(expr);
        fprintf(_fp, ".getT<%s>()", getTypeSpecName(node.qTypeSpec().typeSpec(), _genMode).c_str());
    }

    virtual void visit(const Ast::VariableRefExpr& node) {
        switch(node.refType()) {
            case Ast::RefType::Global:
                break;
            case Ast::RefType::XRef:
                fprintf(_fp, "ref(this).%s", node.vref().name().text());
                break;
            case Ast::RefType::Param:
                fprintf(_fp, "%s", node.vref().name().text());
                break;
            case Ast::RefType::Local:
                fprintf(_fp, "%s", node.vref().name().text());
                break;
        }
    }

    virtual void visit(const Ast::MemberVariableExpr& node) {
        visitNode(node.expr());
        fprintf(_fp, ".%s", node.vref().name().text());
    }

    virtual void visit(const Ast::MemberPropertyExpr& node) {
        visitNode(node.expr());
        fprintf(_fp, "._%s()", node.pref().name().text());
    }

    virtual void visit(const Ast::EnumMemberExpr& node) {
        fprintf(_fp, "%s", getTypeSpecName(node.typeSpec(), GenMode::TypeSpecMemberRef).c_str());
        if(_genMode == GenMode::Import) {
            fprintf(_fp, ".%s", node.vref().name().text());
        } else {
            fprintf(_fp, "::%s", node.vref().name().text());
        }
    }

    virtual void visit(const Ast::StructMemberExpr& node) {
        fprintf(_fp, "%s", getTypeSpecName(node.typeSpec(), GenMode::TypeSpecMemberRef).c_str());
        fprintf(_fp, "::%s", node.vref().name().text());
    }

    virtual void visit(const Ast::StructInstanceExpr& node) {
        fprintf(_fp, "%s()", getTypeSpecName(node.structDefn(), _genMode).c_str());
        for(Ast::StructInitPartList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
            const Ast::StructInitPart& part = ref(*it);
            fprintf(_fp, "._%s<%s>(", part.vdef().name().text(), getTypeSpecName(node.structDefn(), _genMode).c_str());
            visitNode(part.expr());
            fprintf(_fp, ")");
        }
    }

    inline void visitFunctionTypeInstance(const Ast::Function& function) {
        std::string fname = getTypeSpecName(function, _genMode);
        fprintf(_fp, "%s(", fname.c_str());
        std::string sep;
        for(Ast::Scope::List::const_iterator it = function.xref().begin(); it != function.xref().end(); ++it) {
            const Ast::VariableDefn& vref = ref(*it);
            fprintf(_fp, "%s%s", sep.c_str(), vref.name().text());
            sep = ", ";
        }
        fprintf(_fp, ")");
    }

    virtual void visit(const Ast::FunctionInstanceExpr& node) {
        visitFunctionTypeInstance(node.function());
    }

    virtual void visit(const Ast::AnonymousFunctionExpr& node) {
        visitFunctionTypeInstance(node.function());
    }

    virtual void visit(const Ast::ConstantExpr& node) {
        if(getQualifiedTypeSpecName(node.qTypeSpec(), GenMode::Import) == "char") {
            fprintf(_fp, "\'%s\'", node.value().text());
            return;
        }
        if(getQualifiedTypeSpecName(node.qTypeSpec(), GenMode::Import) == "string") {
            fprintf(_fp, "\"%s\"", node.value().text());
            return;
        }
        fprintf(_fp, "%s", node.value().text());
    }

    virtual void sep() {
        fprintf(_fp, "%s", _sep0.c_str());
        _sep0 = _sep2;
    }

private:
    FILE* _fp;
    const GenMode::T& _genMode;
    const std::string _sep2;
    const std::string _sep1;
    std::string _sep0;
};

struct ImportGenerator : public Ast::TypeSpec::Visitor {
    inline bool canWrite(const Ast::AccessType::T& accessType) const {
        return ((accessType == Ast::AccessType::Public) || (accessType == Ast::AccessType::Parent));
    }

    inline void visitChildrenIndent(const Ast::TypeSpec& node) {
        visitChildren(node);
    }

    void visit(const Ast::TypedefDecl& node) {
        if(canWrite(node.accessType())) {
            fprintf(_fp, "%stypedef %s%s;\n", getAccessType(node.accessType()).c_str(), node.name().text(), getDefinitionType(node.defType()).c_str());
        }
        visitChildrenIndent(node);
    }

    void visit(const Ast::TypedefDefn& node) {
        if(canWrite(node.accessType())) {
            fprintf(_fp, "%stypedef %s%s %s;\n", getAccessType(node.accessType()).c_str(), node.name().text(), getDefinitionType(node.defType()).c_str(), getQualifiedTypeSpecName(node.qTypeSpec(), GenMode::Import).c_str());
        }
        visitChildrenIndent(node);
    }

    void visit(const Ast::TemplateDecl& node) {
        if(canWrite(node.accessType())) {
            fprintf(_fp, "%stemplate <", getAccessType(node.accessType()).c_str());
            std::string sep;
            for(Ast::TemplatePartList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Token& token = *it;
                fprintf(_fp, "%s%s\n", sep.c_str(), token.text());
                sep = ", ";
            }
            fprintf(_fp, "> %s%s;\n", node.name().text(), getDefinitionType(node.defType()).c_str());
        }
        visitChildrenIndent(node);
    }

    void visit(const Ast::TemplateDefn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::EnumDefn& node) {
        if(canWrite(node.accessType())) {
            fprintf(_fp, "%senum %s%s {\n", getAccessType(node.accessType()).c_str(), node.name().text(), getDefinitionType(node.defType()).c_str());
            for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::VariableDefn& def = ref(*it);
                fprintf(_fp, "    %s", def.name().text());
                const Ast::ConstantExpr* cexpr = dynamic_cast<const Ast::ConstantExpr*>(ptr(def.initExpr()));
                if((cexpr == 0) || (ref(cexpr).value().string() != "#")) {
                    fprintf(_fp, " = ");
                    ExprGenerator(_fp, GenMode::Import).visitNode(def.initExpr());
                }
                fprintf(_fp, ";\n");
            }
            fprintf(_fp, "};\n");
            fprintf(_fp, "\n");
        }
        visitChildrenIndent(node);
    }

    inline void visitStructDefn(const Ast::StructDefn& node, const Ast::StructDefn* base) {
        if(canWrite(node.accessType())) {
            fprintf(_fp, "%sstruct %s", getAccessType(node.accessType()).c_str(), node.name().text());
            if(base) {
                fprintf(_fp, " : %s", getTypeSpecName(ref(base), GenMode::Import).c_str());
            }
            fprintf(_fp, "%s {\n", getDefinitionType(node.defType()).c_str());
            GeneratorContext(GeneratorContext::TargetMode::Import, GeneratorContext::IndentMode::NoBrace).run(_config, _fs, "", node.block());
            fprintf(_fp, "};\n");
        }
    }

    void visit(const Ast::StructDecl& node) {
        if(canWrite(node.accessType())) {
            fprintf(_fp, "%sstruct %s%s;\n", getAccessType(node.accessType()).c_str(), node.name().text(), getDefinitionType(node.defType()).c_str());
        }
    }

    void visit(const Ast::RootStructDefn& node) {
        visitStructDefn(node, 0);
    }

    void visit(const Ast::ChildStructDefn& node) {
        visitStructDefn(node, ptr(node.base()));
    }

    void visit(const Ast::PropertyDeclRW& node) {
        if(canWrite(node.accessType())) {
            fprintf(_fp, "%sproperty %s %s %s get set;\n",
                    getAccessType(node.accessType()).c_str(),
                    getQualifiedTypeSpecName(node.qTypeSpec(), GenMode::Import).c_str(),
                    node.name().text(),
                    getDefinitionType(node.defType()).c_str());
        }
    }

    void visit(const Ast::PropertyDeclRO& node) {
        if(canWrite(node.accessType())) {
            fprintf(_fp, "%sproperty %s %s %s get;\n",
                    getAccessType(node.accessType()).c_str(),
                    getQualifiedTypeSpecName(node.qTypeSpec(), GenMode::Import).c_str(),
                    node.name().text(),
                    getDefinitionType(node.defType()).c_str());
        }
    }

    inline void visitRoutineImp(const Ast::Routine& node) {
        if(canWrite(node.accessType())) {
            fprintf(_fp, "%sroutine %s ", getAccessType(node.accessType()).c_str(), getQualifiedTypeSpecName(node.outType(), GenMode::Import).c_str());
            fprintf(_fp, "%s", node.name().text());
            fprintf(_fp, "(");
            std::string sep;
            for(Ast::Scope::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qTypeSpec(), GenMode::Import).c_str(), vdef.name().text());
                sep = ", ";
            }
            fprintf(_fp, ")%s;\n", getDefinitionType(node.defType()).c_str());
        }
    }

    void visit(const Ast::RoutineDecl& node) {
        visitRoutineImp(node);
        visitChildrenIndent(node);
    }

    void visit(const Ast::RoutineDefn& node) {
        visitRoutineImp(node);
        visitChildrenIndent(node);
    }

    void visit(const Ast::FunctionRetn& node) {
        visitChildrenIndent(node);
    }

    inline void visitFunctionImp(const Ast::Function& node, const std::string& name, const bool& isEvent) {
        if((name.size() > 0) || (canWrite(node.accessType()))) {
            if(!isEvent) {
                fprintf(_fp, "%s", getAccessType(node.accessType()).c_str());
            }

            fprintf(_fp, "function ");
            if(node.sig().outScope().isTuple()) {
                fprintf(_fp, "(");
                std::string sep;
                for(Ast::Scope::List::const_iterator it = node.sig().out().begin(); it != node.sig().out().end(); ++it) {
                    const Ast::VariableDefn& vdef = ref(*it);
                    fprintf(_fp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qTypeSpec(), GenMode::Import).c_str(), vdef.name().text());
                    sep = ", ";
                }
                fprintf(_fp, ")");
            } else {
                const Ast::VariableDefn& vdef = ref(node.sig().out().front());
                fprintf(_fp, "%s ", getQualifiedTypeSpecName(vdef.qTypeSpec(), GenMode::Import).c_str());
            }

            if(name.size() > 0) {
                fprintf(_fp, "%s(", name.c_str());
            } else {
                fprintf(_fp, "%s(", node.name().text());
            }
            std::string sep = "";
            for(Ast::Scope::List::const_iterator it = node.sig().in().begin(); it != node.sig().in().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qTypeSpec(), GenMode::Import).c_str(), vdef.name().text());
                sep = ", ";
            }
            fprintf(_fp, ")%s;\n", getDefinitionType(node.defType()).c_str());
        }
    }

    void visit(const Ast::FunctionDecl& node) {
        visitFunctionImp(node, "", false);
        visitChildrenIndent(node);
    }

    void visit(const Ast::RootFunctionDefn& node) {
        visitFunctionImp(node, "", false);
        visitChildrenIndent(node);
    }

    void visit(const Ast::ChildFunctionDefn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::EventDecl& node) {
        if(canWrite(node.accessType())) {
            fprintf(_fp, "%sevent(%s %s) => ", getAccessType(node.accessType()).c_str(), getQualifiedTypeSpecName(node.in().qTypeSpec(), GenMode::Import).c_str(), node.in().name().text());
            visitFunctionImp(node.handler(), node.name().string(), true);
        }
        visitChildrenIndent(node);
    }

    void visit(const Ast::Namespace& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::Root& node) {
        visitChildrenIndent(node);
    }

public:
    inline ImportGenerator(const Ast::Config& config, FileSet& fs, FILE* fp) : _config(config), _fs(fs), _fp(fp) {}

private:
    const Ast::Config& _config;
    FileSet& _fs;
    FILE* _fp;
};

struct TypeDeclarationGenerator : public Ast::TypeSpec::Visitor {
    inline void visitChildrenIndent(const Ast::TypeSpec& node) {
        INDENT;
        visitChildren(node);
    }

    void visit(const Ast::TypedefDecl& node) {
        if(node.defType() == Ast::DefinitionType::Native) {
            fprintf(_fp, "%s// typedef %s native;\n", Indent::get(), node.name().text());
        } else {
            throw Exception("Internal error '%s'\n", node.name().text());
        }
    }

    void visit(const Ast::TypedefDefn& node) {
        if(node.defType() != Ast::DefinitionType::Native) {
            fprintf(_fp, "%stypedef %s %s;\n", Indent::get(), getQualifiedTypeSpecName(node.qTypeSpec(), GenMode::Normal).c_str(), node.name().text());
        }
    }

    void visit(const Ast::TemplateDecl& node) {
        if(node.defType() != Ast::DefinitionType::Native) {
            throw Exception("Internal error: template declaration cannot be generated '%s'\n", node.name().text());
        }
    }

    void visit(const Ast::TemplateDefn& node) {
        throw Exception("Internal error: template definition cannot be generated '%s'\n", node.name().text());
    }

    void visit(const Ast::EnumDefn& node) {
        if(node.defType() != Ast::DefinitionType::Native) {
            fprintf(_fp, "%sstruct %s {\n", Indent::get(), node.name().text());
            fprintf(_fp, "%s  enum T {\n", Indent::get());
            std::string sep = " ";
            for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& def = ref(*it);
                fprintf(_fp, "%s%s %s", Indent::get(), sep.c_str(), def.name().text());
                const Ast::ConstantExpr* cexpr = dynamic_cast<const Ast::ConstantExpr*>(ptr(def.initExpr()));
                if((cexpr == 0) || (ref(cexpr).value().string() != "#")) {
                    fprintf(_fp, " = ");
                    ExprGenerator(_fp, GenMode::Normal).visitNode(def.initExpr());
                }
                fprintf(_fp, "\n");
                sep = ",";
            }

            fprintf(_fp, "%s  };\n", Indent::get());
            fprintf(_fp, "%s};\n", Indent::get());
            fprintf(_fp, "\n");
        }
    }

    inline void visitStructDefn(const Ast::StructDefn& node, const Ast::StructDefn* base) {
        if(node.defType() == Ast::DefinitionType::Native) {
            fprintf(_fp, "%sstruct %s;\n", Indent::get(), node.name().text());
            return;
        }

        fprintf(_fp, "%sstruct %s", Indent::get(), node.name().text());
        if(base) {
            fprintf(_fp, " : public %s", getTypeSpecName(ref(base), GenMode::Normal).c_str());
        }

        fprintf(_fp, " {\n");

        // if abstract type, generate virtual dtor
        if(node.defType() == Ast::DefinitionType::Abstract) {
            fprintf(_fp, "%s    virtual ~%s() {}\n", Indent::get(), node.name().text());
        }

        GeneratorContext(GeneratorContext::TargetMode::TypeDecl, GeneratorContext::IndentMode::NoBrace).run(_config, _fs, "", node.block());

        fprintf(_fp, "%s};\n", Indent::get());
        fprintf(_fp, "\n");
    }

    void visit(const Ast::StructDecl& node) {
        fprintf(_fp, "%sstruct %s;\n", Indent::get(), node.name().text());
    }

    void visit(const Ast::RootStructDefn& node) {
        visitStructDefn(node, 0);
    }

    void visit(const Ast::ChildStructDefn& node) {
        visitStructDefn(node, ptr(node.base()));
    }

    inline void visitProperty(const Ast::PropertyDecl& node) {
        std::string cnst;
        if(node.qTypeSpec().isConst())
            cnst = "const ";
        fprintf(_fp, "%s%s%s& _%s() const;\n", Indent::get(), cnst.c_str(), getQualifiedTypeSpecName(node.qTypeSpec(), GenMode::Normal).c_str(), node.name().text());
    }

    void visit(const Ast::PropertyDeclRW& node) {
        visitProperty(node);
        fprintf(_fp, "%svoid _%s(const %s& val);\n", Indent::get(), node.name().text(), getTypeSpecName(node.qTypeSpec().typeSpec(), GenMode::Normal).c_str());
    }

    void visit(const Ast::PropertyDeclRO& node) {
        visitProperty(node);
    }

    inline bool isVoid(const Ast::Scope& out) const {
        if(out.isTuple())
            return false;

        const Ast::VariableDefn& vdef = ref(out.list().front());
        if(vdef.qTypeSpec().typeSpec().name().string() == "void")
            return true;

        return false;
    }

    static inline void visitRoutine(FILE* fp, const Ast::Routine& node, const bool& inNS) {
        fprintf(fp, "%s%s ", Indent::get(), getQualifiedTypeSpecName(node.outType(), GenMode::Normal).c_str());
        if(inNS) {
            fprintf(fp, "%s", node.name().text());
        } else {
            fprintf(fp, "%s", getTypeSpecName(node, GenMode::Normal).c_str());
        }
        fprintf(fp, "(");
        std::string sep;
        for(Ast::Scope::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qTypeSpec(), GenMode::Normal).c_str(), vdef.name().text());
            sep = ", ";
        }
        fprintf(fp, ")");
    }

    inline void writeScopeMemberList(const Ast::Scope& scope) {
        for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
            INDENT;
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(_fp, "%sconst %s %s;\n", Indent::get(), getTypeSpecName(vdef.qTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
        }
        const Ast::Scope* posParam = scope.posParam();
        if(posParam) {
            for(Ast::Scope::List::const_iterator it = ref(posParam).list().begin(); it != ref(posParam).list().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fp, "%sconst %s %s;\n", Indent::get(), getTypeSpecName(vdef.qTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
            }
        }
    }

    static inline void writeScopeParamList(FILE* fp, const Ast::Scope& scope, const std::string& prefix) {
        std::string sep = "";
        for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fp, "%sconst %s& %s%s", sep.c_str(), getTypeSpecName(vdef.qTypeSpec().typeSpec(), GenMode::Normal).c_str(), prefix.c_str(), vdef.name().text());
            sep = ", ";
        }
    }

    inline void writeScopeInCallList(const Ast::Scope& scope) {
        std::string sep = "";
        for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(_fp, "%s_in.%s", sep.c_str(), vdef.name().text());
            sep = ", ";
        }
    }

    inline void writeCtor(const std::string& cname, const Ast::Scope& scope) {
        fprintf(_fp, "%s    inline %s(", Indent::get(), cname.c_str());
        writeScopeParamList(_fp, scope, "p");
        fprintf(_fp, ")");

        std::string sep = " : ";
        for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(_fp, "%s%s(p%s)", sep.c_str(), vdef.name().text(), vdef.name().text());
            sep = ", ";
        }

        const Ast::Scope* posParam = scope.posParam();
        if(posParam) {
            /// \todo implement positional-param
        }

        fprintf(_fp, " {}\n");
    }

    inline void visitFunctionSig(const Ast::Function& node, const bool& isRoot, const bool& isDecl, const bool& isTest) {
        // child-typespecs
        if(node.childCount() > 0) {
            fprintf(_fp, "%spublic:\n", Indent::get());
            visitChildrenIndent(node);
        }

        // xref-list
        if(node.xref().size() > 0) {
            fprintf(_fp, "%spublic:\n", Indent::get());
            writeScopeMemberList(node.xrefScope());
            writeCtor(node.name().text(), node.xrefScope());
        }

        // in-param-list
        if(isRoot) {
            fprintf(_fp, "%spublic:\n", Indent::get());
            INDENT;
            fprintf(_fp, "%sstruct _In {\n", Indent::get());
            writeScopeMemberList(node.sig().inScope());
            writeCtor("_In", node.sig().inScope());
            fprintf(_fp, "%s};\n", Indent::get());
        }

        std::string out1;
        std::string out2;
        if(isVoid(node.sig().outScope())) {
            out1 = "void";
            out2 = "const _Out &";
        } else if(node.sig().outScope().isTuple()) {
            out1 = "const _Out&";
            out2 = out1;
        } else {
            const Ast::VariableDefn& vdef = ref(node.sig().out().front());
            out1 = getQualifiedTypeSpecName(vdef.qTypeSpec(), GenMode::Normal);
            out2 = out1;
        }

        if(!isTest) {
            // param-instance
            fprintf(_fp, "%s    Pointer<_Out> _out;\n", Indent::get());
            fprintf(_fp, "%s    inline %s out(const _Out& val) {_out = Creator<_Out, _Out>::get(val);", Indent::get(), out2.c_str());
            fprintf(_fp, "return _out.get()");
            if((!isVoid(node.sig().outScope())) && (!node.sig().outScope().isTuple())) {
                fprintf(_fp, "._out");
            }
            fprintf(_fp, ";}\n");
        }

        // run-function-type
        fprintf(_fp, "%spublic:\n", Indent::get());
        if(isTest) {
            fprintf(_fp, "%s    %s test();\n", Indent::get(), out2.c_str());
        } else if((isDecl) && (node.defType() != Ast::DefinitionType::Native)) {
            fprintf(_fp, "%s    virtual %s _run(const _In& _in) = 0;\n", Indent::get(), out2.c_str());
        } else {
            fprintf(_fp, "%s    %s run(", Indent::get(), out1.c_str());
            writeScopeParamList(_fp, node.sig().inScope(), "p");
            fprintf(_fp, ");\n");
            fprintf(_fp, "%s    virtual %s _run(const _In& _in) {\n", Indent::get(), out2.c_str());
            if(isVoid(node.sig().outScope())) {
                fprintf(_fp, "%s        run(", Indent::get());
                writeScopeInCallList(node.sig().inScope());
                fprintf(_fp, ");\n");
                fprintf(_fp, "%s        return out(_Out());\n", Indent::get());
            } else {
                fprintf(_fp, "%s        return run(", Indent::get());
                writeScopeInCallList(node.sig().inScope());
                fprintf(_fp, ");\n");
            }
            fprintf(_fp, "%s    }\n", Indent::get());
        }
    }

    inline void visitFunction(const Ast::Function& node, const bool isDecl) {
        fprintf(_fp, "%sclass %s {\n", Indent::get(), node.name().text());
        visitFunctionSig(node, true, isDecl, false);
        fprintf(_fp, "%s};\n", Indent::get());
    }

    void visit(const Ast::RoutineDecl& node) {
        visitRoutine(_fp, node, true);
        fprintf(_fp, ";\n");
        fprintf(_fp, "\n");
    }

    void visit(const Ast::RoutineDefn& node) {
        visitRoutine(_fp, node, true);
        fprintf(_fp, ";\n");
        fprintf(_fp, "\n");
    }

    void visit(const Ast::FunctionRetn& node) {
        fprintf(_fp, "%sstruct _Out {\n", Indent::get());

        if(!isVoid(node.outScope())) {
            // generate out parameters
            for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fp, "%s%s %s;\n", Indent::get(), getTypeSpecName(vdef.qTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
            }

            // generate out setter
            fprintf(_fp, "%s    inline %s(", Indent::get(), node.name().text());
            std::string sep = "";
            for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fp, "%sconst %s& p%s", sep.c_str(), getTypeSpecName(vdef.qTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
                sep = ", ";
            }
            fprintf(_fp, ")");
            sep = " : ";
            for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fp, "%s%s(p%s)", sep.c_str(), vdef.name().text(), vdef.name().text());
                sep = ", ";
            }
            fprintf(_fp, "{}\n");

        }

        // end return struct
        fprintf(_fp, "%s};\n", Indent::get());
        fprintf(_fp, "\n");
    }

    void visit(const Ast::FunctionDecl& node) {
        visitFunction(node, true);
        fprintf(_fp, "\n");
    }

    void visit(const Ast::RootFunctionDefn& node) {
        visitFunction(node, false);
        fprintf(_fp, "\n");
    }

    void visit(const Ast::ChildFunctionDefn& node) {
        bool isTest = (getTypeSpecName(node.base(), GenMode::Normal) == "test");
        if((isTest) && (!_config.test())) {
            return;
        }

        if(isTest) {
            fprintf(_fp, "%sclass %s : public test_< %s > {\n", Indent::get(), node.name().text(), node.name().text());
            fprintf(_fp, "%spublic:\n", Indent::get());
            fprintf(_fp, "%s    inline const char* const name() const {return \"%s\";}\n", Indent::get(), getTypeSpecName(node, GenMode::Normal).c_str());
        } else if(getTypeSpecName(node.base(), GenMode::Normal) == "main") {
            fprintf(_fp, "%sclass %s : public main_< %s > {\n", Indent::get(), node.name().text(), node.name().text());
        } else {
            fprintf(_fp, "%sclass %s : public %s {\n", Indent::get(), node.name().text(), getTypeSpecName(node.base(), GenMode::Normal).c_str());
        }
        visitFunctionSig(node, false, false, isTest);

        if(getTypeSpecName(node.base(), GenMode::Normal) == "test") {
            fprintf(_fp, "%s    static TestInstanceT<%s> s_test;\n", Indent::get(), node.name().text());
        } else if(getTypeSpecName(node.base(), GenMode::Normal) == "main") {
            fprintf(_fp, "%s    static MainInstanceT<%s> s_main;\n", Indent::get(), node.name().text());
        }

        fprintf(_fp, "%s};\n", Indent::get());
        fprintf(_fp, "\n");
    }

    void visit(const Ast::EventDecl& node) {
        fprintf(_fp, "%sstruct %s {\n", Indent::get(), node.name().text());
        // child-typespecs
        if(node.childCount() > 0) {
            fprintf(_fp, "%spublic:\n", Indent::get());
            visitChildrenIndent(node);
        }
        fprintf(_fp, "%s    FunctorList<Handler> _list;\n", Indent::get());
        fprintf(_fp, "%s    static %s instance;\n", Indent::get(), node.name().text());
        fprintf(_fp, "%s    static inline Handler& add(Handler* h) {return instance._list.add(h);}\n", Indent::get());
        fprintf(_fp, "%s    static void addHandler(%s %s, Handler* h);\n", Indent::get(), getQualifiedTypeSpecName(node.in().qTypeSpec(), GenMode::Normal).c_str(), node.in().name().text());
        fprintf(_fp, "%s    template<typename T>static inline void addHandlerT(%s %s, T h) {return addHandler(%s, new T(h));}\n", Indent::get(), getQualifiedTypeSpecName(node.in().qTypeSpec(), GenMode::Normal).c_str(), node.in().name().text(), node.in().name().text());
        fprintf(_fp, "%s};\n", Indent::get());
        fprintf(_fp, "\n");
        return;
    }

    void visit(const Ast::Namespace& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::Root& node) {
        visitChildrenIndent(node);
    }

public:
    inline TypeDeclarationGenerator(const Ast::Config& config, FileSet& fs, FILE* fp) : _config(config), _fs(fs), _fp(fp) {}

private:
    const Ast::Config& _config;
    FileSet& _fs;
    FILE* _fp;
};

struct TypeDefinitionGenerator : public Ast::TypeSpec::Visitor {
    inline void visitChildrenIndent(const Ast::TypeSpec& node) {
        INDENT;
        visitChildren(node);
    }

    void visit(const Ast::TypedefDecl& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::TypedefDefn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::TemplateDecl& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::TemplateDefn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::EnumDefn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::StructDecl& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::RootStructDefn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::ChildStructDefn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::PropertyDeclRW& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::PropertyDeclRO& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::RoutineDecl& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::RoutineDefn& node) {
        visitChildrenIndent(node);
        TypeDeclarationGenerator::visitRoutine(_fp, node, false);
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
        fprintf(_fp, "\n");
    }

    void visit(const Ast::FunctionRetn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::FunctionDecl& node) {
        visitChildrenIndent(node);
    }

    inline void visitFunction(const Ast::FunctionDefn& node, const bool& isTest) {
        const std::string tname = getTypeSpecName(node, GenMode::Normal);
        std::string out;
        if(node.sig().outScope().isTuple()) {
            out = "const " + tname + "::_Out&";
        } else {
            const Ast::VariableDefn& vdef = ref(node.sig().out().front());
            out = getQualifiedTypeSpecName(vdef.qTypeSpec(), GenMode::Normal);
        }

        if(isTest) {
            fprintf(_fp, "%s %s::test() ", out.c_str(), tname.c_str());
        } else {
            fprintf(_fp, "%s %s::run(", out.c_str(), tname.c_str());
            TypeDeclarationGenerator::writeScopeParamList(_fp, node.sig().inScope(), "");
            fprintf(_fp, ") ");
        }
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
        fprintf(_fp, "\n");
    }

    void visit(const Ast::RootFunctionDefn& node) {
        visitChildrenIndent(node);
        visitFunction(node, false);
    }

    void visit(const Ast::ChildFunctionDefn& node) {
        bool isTest = (getTypeSpecName(node.base(), GenMode::Normal) == "test");
        if((isTest) && (!_config.test())) {
            return;
        }

        visitChildrenIndent(node);
        visitFunction(node, isTest);
        std::string fname = getTypeSpecName(node, GenMode::Normal);

        if(isTest) {
            fprintf(_fp, "TestInstanceT<%s> %s::s_test = TestInstanceT<%s>();\n\n", fname.c_str(), fname.c_str(), fname.c_str());
        } else if(getTypeSpecName(node.base(), GenMode::Normal) == "main") {
            fprintf(_fp, "MainInstanceT<%s> %s::s_main = MainInstanceT<%s>();\n\n", fname.c_str(), fname.c_str(), fname.c_str());
        }
    }

    void visit(const Ast::EventDecl& node) {
        visitChildrenIndent(node);
        fprintf(_fp, "%s %s::instance;\n", getTypeSpecName(node, GenMode::Normal).c_str(), getTypeSpecName(node, GenMode::Normal).c_str());
        if(node.defType() == Ast::DefinitionType::Direct) {
            fprintf(_fp, "void %s::addHandler(%s %s, Handler* h) {\n", getTypeSpecName(node, GenMode::Normal).c_str(), getQualifiedTypeSpecName(node.in().qTypeSpec(), GenMode::Normal).c_str(), node.in().name().text());
            fprintf(_fp, "}\n");
        }
        fprintf(_fp, "const %s::Add::_Out& %s::Add::run(", getTypeSpecName(node, GenMode::Normal).c_str(), getTypeSpecName(node, GenMode::Normal).c_str());
        TypeDeclarationGenerator::writeScopeParamList(_fp, node.addFunction().sig().inScope(), "");
        fprintf(_fp, ") {\n");
        fprintf(_fp, "    assert(false); //addHandler(in.%s, in.handler);\n", node.in().name().text());
        fprintf(_fp, "    return out(_Out());\n");
        fprintf(_fp, "}\n");
    }

    void visit(const Ast::Namespace& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::Root& node) {
        visitChildrenIndent(node);
    }

public:
    inline TypeDefinitionGenerator(const Ast::Config& config, FileSet& fs, FILE* fp) : _config(config), _fs(fs), _fp(fp) {}
private:
    const Ast::Config& _config;
    FileSet& _fs;
    FILE* _fp;
};

struct StatementGenerator : public Ast::Statement::Visitor {
private:
    inline FILE* fpDecl(const Ast::AccessType::T& accessType) const {
        return (accessType == Ast::AccessType::Private)?_fs._fpSrc:_fs._fpHdr;
    }

    inline FILE* fpDefn() const {
        return _fs._fpSrc;
    }
private:
    virtual void visit(const Ast::ImportStatement& node) {
        fprintf(_fs._fpImp, "// import\n");
        if(_ctx._targetMode == GeneratorContext::TargetMode::Import) {
            if(node.headerType() == Ast::HeaderType::Import) {
                fprintf(_fs._fpImp, "import ");
            } else {
                fprintf(_fs._fpImp, "include ");
            }

            std::string sep = "";
            for(Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Token& name = ref(*it).name();
                fprintf(_fs._fpImp, "%s%s", sep.c_str(), name.text());
                sep = "::";
            }

            if(node.defType() == Ast::DefinitionType::Native) {
                fprintf(_fs._fpImp, " native");
            }

            fprintf(_fs._fpImp, ";\n");
        }

        if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
            FILE* fp = fpDecl(node.accessType());
            std::string qt = (node.headerType() == Ast::HeaderType::Import)?"<>":"\"\"";
            fprintf(fp, "#include %c", qt.at(0));
            std::string sep = "";
            for(Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Token& name = ref(*it).name();
                fprintf(fp, "%s%s", sep.c_str(), name.text());
                sep = "/";
            }
            fprintf(fp, ".hpp%c\n", qt.at(1));
        }
    }

    virtual void visit(const Ast::EnterNamespaceStatement& node) {
        if(_ctx._targetMode == GeneratorContext::TargetMode::Import) {
            std::string fqn;
            std::string sep;
            for(Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Namespace& ns = ref(*it);
                fqn += sep;
                fqn += ns.name().string();
            }
            if(fqn.size() > 0) {
                fprintf(_fs._fpImp, "namespace %s;\n\n", fqn.c_str());
            }
        }

        if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
            assert(_basename.size() > 0);
            fprintf(_fs._fpSrc, "#include \"%s.hpp\"\n", _basename.c_str());

            for(Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Namespace& ns = ref(*it);
                fprintf(_fs._fpHdr, "namespace %s {", ns.name().text());
                fprintf(_fs._fpSrc, "namespace %s {", ns.name().text());
            }

            if(node.list().size() > 0) {
                fprintf(_fs._fpHdr, "\n");
                fprintf(_fs._fpSrc, "\n");
            }
        }
    }

    virtual void visit(const Ast::LeaveNamespaceStatement& node) {
        if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
            for(Ast::NamespaceList::List::const_reverse_iterator it = node.statement().list().rbegin(); it != node.statement().list().rend(); ++it) {
                const Ast::Namespace& ns = ref(*it);
                fprintf(_fs._fpHdr, "} /* %s */ ", ns.name().text());
                fprintf(_fs._fpSrc, "} /* %s */ ", ns.name().text());
            }
            if(node.statement().list().size() > 0) {
                fprintf(_fs._fpHdr, "\n");
                fprintf(_fs._fpSrc, "\n");
            }
        }
    }

    virtual void visit(const Ast::UserDefinedTypeSpecStatement& node) {
        if(_ctx._targetMode == GeneratorContext::TargetMode::Import) {
            ImportGenerator(_config, _fs, _fs._fpImp).visitNode(node.typeSpec());
        }

        if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
            TypeDeclarationGenerator(_config, _fs, fpDecl(node.typeSpec().accessType())).visitNode(node.typeSpec());
        }

        if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDefn) {
            TypeDefinitionGenerator(_config, _fs, _fs._fpSrc).visitNode(node.typeSpec());
        }
    }

    inline bool isPtr(const Ast::TypeSpec& typeSpec) const {
        const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(ptr(typeSpec));
        if(templateDefn) {
            if(typeSpec.name().string() == "ptr") {
                return true;
            }
        }
        return false;
    }

    virtual void visit(const Ast::StructMemberVariableStatement& node) {
        if(_ctx._targetMode == GeneratorContext::TargetMode::Import) {
            fprintf(_fs._fpImp, "%s%s %s = ", Indent::get(), getQualifiedTypeSpecName(node.defn().qTypeSpec(), GenMode::Import).c_str(), node.defn().name().text());
            ExprGenerator(_fs._fpImp, GenMode::Import).visitNode(node.defn().initExpr());
            fprintf(_fs._fpImp, ";\n");
        }

        if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
            std::string tname = getQualifiedTypeSpecName(node.defn().qTypeSpec(), GenMode::Normal);
            std::string pname = "const " + tname + "&";
            if(isPtr(node.defn().qTypeSpec().typeSpec())) {
                pname = tname;
            }
            fprintf(fpDecl(node.structDefn().accessType()), "%s%s %s; /**/ ", Indent::get(), tname.c_str(), node.defn().name().text());
            fprintf(fpDecl(node.structDefn().accessType()), "template <typename T> inline T& _%s(%s val) {%s = val; return ref(static_cast<T*>(this));}\n",
                    node.defn().name().text(), pname.c_str(), node.defn().name().text());
        }
    }

    virtual void visit(const Ast::StructInitStatement& node) {
        if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
            // default-ctor
            fprintf(fpDecl(node.structDefn().accessType()), "%sexplicit inline %s()", Indent::get(), node.structDefn().name().text());
            std::string sep = " : ";
            for(Ast::Scope::List::const_iterator it = node.structDefn().list().begin(); it != node.structDefn().list().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(fpDecl(node.structDefn().accessType()), "%s%s(", sep.c_str(), vdef.name().text());
                ExprGenerator(fpDecl(node.structDefn().accessType()), GenMode::Normal).visitNode(vdef.initExpr());
                fprintf(fpDecl(node.structDefn().accessType()), ")");
                sep = ", ";
            }
            fprintf(fpDecl(node.structDefn().accessType()), " {}\n");
        }
    }

    virtual void visit(const Ast::AutoStatement& node) {
        fprintf(fpDefn(), "%s%s %s = ", Indent::get(), getQualifiedTypeSpecName(node.defn().qTypeSpec(), GenMode::Normal).c_str(), node.defn().name().text());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.defn().initExpr());
        fprintf(fpDefn(), ";\n");
    }

    virtual void visit(const Ast::ExprStatement& node) {
        fprintf(fpDefn(), "%s", Indent::get());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.expr());
        fprintf(fpDefn(), ";\n");
    }

    virtual void visit(const Ast::PrintStatement& node) {
        fprintf(fpDefn(), "%sLog::get() << ", Indent::get());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.expr());
        fprintf(fpDefn(), " << Log::Out();\n");
    }

    virtual void visit(const Ast::IfStatement& node) {
        fprintf(fpDefn(), "%sif(", Indent::get());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.expr());
        fprintf(fpDefn(), ") ");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.tblock());
    }

    virtual void visit(const Ast::IfElseStatement& node) {
        fprintf(fpDefn(), "%sif(", Indent::get());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.expr());
        fprintf(fpDefn(), ")");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.tblock());
        fprintf(fpDefn(), "%selse", Indent::get());
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.fblock());
    }

    virtual void visit(const Ast::WhileStatement& node) {
        fprintf(fpDefn(), "%swhile(", Indent::get());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.expr());
        fprintf(fpDefn(), ")");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
    }

    virtual void visit(const Ast::DoWhileStatement& node) {
        fprintf(fpDefn(), "%sdo", Indent::get());
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
        fprintf(fpDefn(), "%swhile(", Indent::get());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.expr());
        fprintf(fpDefn(), ");\n");
    }

    virtual void visit(const Ast::ForExprStatement& node) {
        fprintf(fpDefn(), "%sfor(", Indent::get());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.init());
        fprintf(fpDefn(), "; ");
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.expr());
        fprintf(fpDefn(), "; ");
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.incr());
        fprintf(fpDefn(), ")");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
    }

    virtual void visit(const Ast::ForInitStatement& node) {
        fprintf(fpDefn(), "%sfor(%s %s = ", Indent::get(), getQualifiedTypeSpecName(node.init().qTypeSpec(), GenMode::Normal).c_str(), node.init().name().text());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.init().initExpr());
        fprintf(fpDefn(), "; ");
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.expr());
        fprintf(fpDefn(), "; ");
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.incr());
        fprintf(fpDefn(), ")");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
    }

    virtual void visit(const Ast::ForeachListStatement& node) {
        const std::string etype = getQualifiedTypeSpecName(node.expr().qTypeSpec(), GenMode::Normal);

        std::string constit = "";
        if(node.expr().qTypeSpec().isConst()) {
            constit = "const_";
        }

        fprintf(fpDefn(), "%s{\n", Indent::get());
        fprintf(fpDefn(), "%s  %s _list = ", Indent::get(), etype.c_str());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.expr());
        fprintf(fpDefn(), ";\n");
        fprintf(fpDefn(), "%s  for(%s::%siterator _it = _list.begin(); _it != _list.end(); ++_it) {\n", Indent::get(), getTypeSpecName(node.expr().qTypeSpec().typeSpec(), GenMode::Normal).c_str(), constit.c_str());
        fprintf(fpDefn(), "%s  %s %s = _it->second;\n", Indent::get(), getQualifiedTypeSpecName(node.valDef().qTypeSpec(), GenMode::Normal).c_str(), node.valDef().name().text());

        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::IndentedBrace).run(_config, _fs, "", node.block());

        fprintf(fpDefn(), "%s}\n", Indent::get());
        fprintf(fpDefn(), "%s}\n", Indent::get());
    }

    virtual void visit(const Ast::ForeachDictStatement& node) {
        std::string constit = "";
        fprintf(fpDefn(), "%s{\n", Indent::get());
        if(node.expr().qTypeSpec().isConst()) {
            constit = "const_";
        }
        fprintf(fpDefn(), "%s  %s _list = ", Indent::get(), getQualifiedTypeSpecName(node.expr().qTypeSpec(), GenMode::Normal).c_str());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.expr());
        fprintf(fpDefn(), ";\n");
        fprintf(fpDefn(), "%s  for(%s::%siterator _it = _list.begin(); _it != _list.end(); ++_it) {\n", Indent::get(), getTypeSpecName(node.expr().qTypeSpec().typeSpec(), GenMode::Normal).c_str(), constit.c_str());
        fprintf(fpDefn(), "%s  %s %s = _it->first;\n", Indent::get(), getQualifiedTypeSpecName(node.keyDef().qTypeSpec(), GenMode::Normal).c_str(), node.keyDef().name().text());
        fprintf(fpDefn(), "%s  %s %s = _it->second;\n", Indent::get(), getQualifiedTypeSpecName(node.valDef().qTypeSpec(), GenMode::Normal).c_str(), node.valDef().name().text());

        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::IndentedBrace).run(_config, _fs, "", node.block());

        fprintf(fpDefn(), "%s}\n", Indent::get());
        fprintf(fpDefn(), "%s}\n", Indent::get());
    }

    virtual void visit(const Ast::CaseExprStatement& node) {
        fprintf(fpDefn(), "%scase (", Indent::get());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.expr());
        fprintf(fpDefn(), ") :");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
    }

    virtual void visit(const Ast::CaseDefaultStatement& node) {
        fprintf(fpDefn(), "%sdefault :", Indent::get());
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
    }

    virtual void visit(const Ast::SwitchValueStatement& node) {
        fprintf(fpDefn(), "%sswitch(", Indent::get());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.expr());
        fprintf(fpDefn(), ")");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
    }

    virtual void visit(const Ast::SwitchExprStatement& node) {
        std::string ifstr = "if";
        for(Ast::CompoundStatement::List::const_iterator sit = node.block().list().begin(); sit != node.block().list().end(); ++sit) {
            const Ast::Statement* s = *sit;
            const Ast::CaseExprStatement* ce = dynamic_cast<const Ast::CaseExprStatement*>(s);
            const Ast::CaseDefaultStatement* cd = dynamic_cast<const Ast::CaseDefaultStatement*>(s);
            if(ce) {
                fprintf(fpDefn(), "%s%s(", Indent::get(), ifstr.c_str());
                ExprGenerator(fpDefn(), GenMode::Normal).visitNode(ref(ce).expr());
                fprintf(fpDefn(), ")\n");
                visitNode(ref(ce).block());
                ifstr = "else if";
            } else if(cd) {
                fprintf(fpDefn(), "%selse\n", Indent::get());
                visitNode(ref(cd).block());
                break;
            } else {
                throw Exception("Internal error: not a case statement inside switch\n");
            }
        }
    }

    virtual void visit(const Ast::BreakStatement& node) {
        fprintf(fpDefn(), "%sbreak;\n", Indent::get());
    }

    virtual void visit(const Ast::ContinueStatement& node) {
        fprintf(fpDefn(), "%scontinue;\n", Indent::get());
    }

    virtual void visit(const Ast::AddEventHandlerStatement& node) {
        std::string ename = getTypeSpecName(node.event(), GenMode::Normal);
        fprintf(fpDefn(), "%s%s::addHandlerT(", Indent::get(), ename.c_str());
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.source());
        fprintf(fpDefn(), ", ");
        ExprGenerator(fpDefn(), GenMode::Normal).visitNode(node.functor());
        fprintf(fpDefn(), ");\n");
    }

    virtual void visit(const Ast::RoutineReturnStatement& node) {
        fprintf(fpDefn(), "%sreturn", Indent::get());
        if(node.exprList().list().size() > 0) {
            fprintf(fpDefn(), " (");
            ExprGenerator(fpDefn(), GenMode::Normal, ", ").visitList(node.exprList());
            fprintf(fpDefn(), ")");
        }
        fprintf(fpDefn(), ";\n");
    }

    virtual void visit(const Ast::FunctionReturnStatement& node) {
        fprintf(fpDefn(), "%sreturn out(_Out(", Indent::get());
        ExprGenerator(fpDefn(), GenMode::Normal, ", ").visitList(node.exprList());
        fprintf(fpDefn(), "));\n");
    }

    virtual void visit(const Ast::CompoundStatement& node) {
        if(_ctx._indentMode == GeneratorContext::IndentMode::IndentedBrace) {
            fprintf(fpDefn(), "%s", Indent::get());
        }

        if((_ctx._indentMode == GeneratorContext::IndentMode::IndentedBrace) || (_ctx._indentMode == GeneratorContext::IndentMode::WithBrace)) {
            fprintf(fpDefn(), "{\n");
        }

        {
            INDENT;
            for(Ast::CompoundStatement::List::const_iterator sit = node.list().begin(); sit != node.list().end(); ++sit) {
                const Ast::Statement& s = ref(*sit);
                ref(this).visitNode(s);
            }
        }

        if((_ctx._indentMode == GeneratorContext::IndentMode::IndentedBrace) || (_ctx._indentMode == GeneratorContext::IndentMode::WithBrace)) {
            fprintf(fpDefn(), "%s}\n", Indent::get());
        }
    }

public:
    inline StatementGenerator(const Ast::Config& config, FileSet& fs, GeneratorContext& ctx, const std::string& basename) : _config(config), _fs(fs), _ctx(ctx), _basename(basename) {}
private:
    const Ast::Config& _config;
    FileSet& _fs;
    GeneratorContext& _ctx;
    const std::string& _basename;
};

void GeneratorContext::run(const Ast::Config& config, FileSet& fs, const std::string& basename, const Ast::Statement& block) {
    StatementGenerator gen(config, fs, ref(this), basename);
    gen.visitNode(block);
}

struct Generator::Impl {
    inline Impl(const Ast::Project& project, const Ast::Config& config, const Ast::Unit& unit) : _project(project), _config(config), _unit(unit), _fpHdr(0), _fpSrc(0), _fpImp(0) {}
    inline void run();
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
    const Ast::Unit& _unit;
private:
    FILE* _fpHdr;
    FILE* _fpSrc;
    FILE* _fpImp;
};

inline void Generator::Impl::run() {
    Indent::init();
    std::string basename = getBaseName(_unit.filename());
    OutputFile ofImp(_fpImp, basename + ".ipp");unused(ofImp);
    OutputFile ofHdr(_fpHdr, basename + ".hpp");unused(ofHdr);
    OutputFile ofSrc(_fpSrc, basename + ".cpp");unused(ofSrc);
    FileSet fs(_fpHdr, _fpSrc, _fpImp);

    fprintf(_fpHdr, "#pragma once\n\n");
    for(Ast::Config::PathList::const_iterator it = _config.includeFileList().begin(); it != _config.includeFileList().end(); ++it) {
        const std::string& filename = *it;
        fprintf(_fpSrc, "#include \"%s\"\n", filename.c_str());
    }

    for(Ast::Unit::StatementList::const_iterator sit = _unit.statementList().begin(); sit != _unit.statementList().end(); ++sit) {
        const Ast::Statement& s = ref(*sit);
        GeneratorContext(GeneratorContext::TargetMode::Import, GeneratorContext::IndentMode::WithBrace).run(_config, fs, basename, s);
    }

    for(Ast::Unit::StatementList::const_iterator sit = _unit.statementList().begin(); sit != _unit.statementList().end(); ++sit) {
        const Ast::Statement& s = ref(*sit);
        GeneratorContext(GeneratorContext::TargetMode::TypeDecl, GeneratorContext::IndentMode::WithBrace).run(_config, fs, basename, s);
    }

    for(Ast::Unit::StatementList::const_iterator sit = _unit.statementList().begin(); sit != _unit.statementList().end(); ++sit) {
        const Ast::Statement& s = ref(*sit);
        GeneratorContext(GeneratorContext::TargetMode::TypeDefn, GeneratorContext::IndentMode::WithBrace).run(_config, fs, basename, s);
    }
}

//////////////////////////////////////////////
Generator::Generator(const Ast::Project& project, const Ast::Config& config, const Ast::Unit& unit) : _impl(0) {_impl = new Impl(project, config, unit);}
Generator::~Generator() {delete _impl;}
void Generator::run() {return ref(_impl).run();}
