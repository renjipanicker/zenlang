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
    }
    throw Exception("Internal error: Unknown Definition Type '%d'\n", defType);
}

inline std::string getAccessType(const Ast::AccessType::T& accessType) {
    switch(accessType) {
        case Ast::AccessType::Public:
            return "public";
        case Ast::AccessType::Protected:
            return "protected";
        case Ast::AccessType::Private:
            return "private";
        case Ast::AccessType::Parent:
            return "parent";
        case Ast::AccessType::Internal:
            return "internal";
        case Ast::AccessType::Export:
            return "export";
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
        fprintf(_fp, "%s::_In(", getTypeSpecName(node.expr().qTypeSpec().typeSpec(), _genMode).c_str());
        ExprGenerator(_fp, _genMode, ", ").visitList(node.exprList());
        fprintf(_fp, "))");
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
        visitNode(node.expr());
        fprintf(_fp, "[");
        visitNode(node.index());
        fprintf(_fp, "]");
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
        const std::string bname = getTypeSpecName(node.templateDefn().at(0).typeSpec(), _genMode);
        const std::string dname = getTypeSpecName(expr.qTypeSpec().typeSpec(), _genMode);
        fprintf(_fp, "Creator<%s, %s>::get(", bname.c_str(), dname.c_str());
        fprintf(_fp, "type(\"%s\"), ", dname.c_str());
        ExprGenerator(_fp, _genMode).visitNode(expr);
        fprintf(_fp, ")");
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
                fprintf(_fp, "_in.%s", node.vref().name().text());
                break;
            case Ast::RefType::Local:
                fprintf(_fp, "%s", node.vref().name().text());
                break;
        }
    }

    virtual void visit(const Ast::VariableMemberExpr& node) {
        visitNode(node.expr());
        fprintf(_fp, ".%s", node.vref().name().text());
    }

    virtual void visit(const Ast::StructMemberExpr& node) {
        fprintf(_fp, "%s", getTypeSpecName(node.typeSpec(), GenMode::TypeSpecMemberRef).c_str());
        fprintf(_fp, "::%s", node.vref().name().text());
    }

    virtual void visit(const Ast::EnumMemberExpr& node) {
        fprintf(_fp, "%s", getTypeSpecName(node.typeSpec(), GenMode::TypeSpecMemberRef).c_str());
        if(_genMode == GenMode::Import) {
            fprintf(_fp, ".%s", node.vref().name().text());
        } else {
            fprintf(_fp, "::%s", node.vref().name().text());
        }
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

    virtual void visit(const Ast::FunctionInstanceExpr& node) {
        std::string fname = getTypeSpecName(node.function(), _genMode);
        fprintf(_fp, "%s(", fname.c_str());
        std::string sep;
        for(Ast::Scope::List::const_iterator it = node.function().xref().begin(); it != node.function().xref().end(); ++it) {
            const Ast::VariableDefn& vref = ref(*it);
            fprintf(_fp, "%s%s", sep.c_str(), vref.name().text());
            sep = ", ";
        }
        fprintf(_fp, ")");
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
    inline void visitChildrenIndent(const Ast::TypeSpec& node) {
        visitChildren(node);
    }

    void visit(const Ast::TypedefDecl& node) {
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fpImp, "public typedef %s%s;\n", node.name().text(), getDefinitionType(node.defType()).c_str());
        }
        visitChildrenIndent(node);
    }

    void visit(const Ast::TypedefDefn& node) {
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fpImp, "public typedef %s%s %s;\n", node.name().text(), getDefinitionType(node.defType()).c_str(), getQualifiedTypeSpecName(node.qTypeSpec(), GenMode::Import).c_str());
        }
        visitChildrenIndent(node);
    }

    void visit(const Ast::TemplateDecl& node) {
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fpImp, "public template <");
            std::string sep;
            for(Ast::TemplatePartList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Token& token = *it;
                fprintf(_fpImp, "%s%s\n", sep.c_str(), token.text());
                sep = ", ";
            }
            fprintf(_fpImp, "> %s%s;\n", node.name().text(), getDefinitionType(node.defType()).c_str());
        }
        visitChildrenIndent(node);
    }

    void visit(const Ast::TemplateDefn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::EnumDefn& node) {
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fpImp, "public enum %s%s {\n", node.name().text(), getDefinitionType(node.defType()).c_str());
            for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::VariableDefn& def = ref(*it);
                fprintf(_fpImp, "    %s", def.name().text());
                const Ast::ConstantExpr* cexpr = dynamic_cast<const Ast::ConstantExpr*>(ptr(def.initExpr()));
                if((cexpr == 0) || (ref(cexpr).value().string() != "#")) {
                    fprintf(_fpImp, " = ");
                    ExprGenerator(_fpImp, GenMode::Import).visitNode(def.initExpr());
                }
                fprintf(_fpImp, ";\n");
            }
            fprintf(_fpImp, "};\n");
            fprintf(_fpImp, "\n");
        }
        visitChildrenIndent(node);
    }

    inline void visitStructDefn(const Ast::StructDefn& node, const Ast::StructDefn* base) {
        if(node.accessType() != Ast::AccessType::Private) {
            fprintf(_fpImp, "%s struct %s", getAccessType(node.accessType()).c_str(), node.name().text());
            if(base) {
                fprintf(_fpImp, " : %s", getTypeSpecName(ref(base), GenMode::Import).c_str());
            }
            fprintf(_fpImp, "%s {\n", getDefinitionType(node.defType()).c_str());
            if(node.hasList()) {
                for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const Ast::VariableDefn& vdef = ref(*it);
                    fprintf(_fpImp, "    %s %s", getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GenMode::Import).c_str(), vdef.name().text());
                    fprintf(_fpImp, " = ");
                    ExprGenerator(_fpImp, GenMode::Import).visitNode(vdef.initExpr());
                    fprintf(_fpImp, ";\n");
                }
            }
            fprintf(_fpImp, "};\n");
        }
    }

    void visit(const Ast::RootStructDecl& node) {
        fprintf(_fpImp, "%s struct %s%s;\n", getAccessType(node.accessType()).c_str(), node.name().text(), getDefinitionType(node.defType()).c_str());
    }

    void visit(const Ast::RootStructDefn& node) {
        visitStructDefn(node, 0);
        visitChildrenIndent(node);
    }

    void visit(const Ast::ChildStructDefn& node) {
        visitStructDefn(node, ptr(node.base()));
        visitChildrenIndent(node);
    }

    inline void visitRoutineImp(const Ast::Routine& node) {
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fpImp, "public routine %s ", getQualifiedTypeSpecName(node.outType(), GenMode::Import).c_str());
            fprintf(_fpImp, "%s", node.name().text());
            fprintf(_fpImp, "(");
            std::string sep;
            for(Ast::Scope::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fpImp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GenMode::Normal).c_str(), vdef.name().text());
                sep = ", ";
            }
            fprintf(_fpImp, ")%s;\n", getDefinitionType(node.defType()).c_str());
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
        if((name.size() > 0) || (node.accessType() == Ast::AccessType::Public)) {
            if(!isEvent) {
                fprintf(_fpImp, "public ");
            }
            fprintf(_fpImp, "function (");

            std::string sep;
            for(Ast::Scope::List::const_iterator it = node.sig().out().begin(); it != node.sig().out().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fpImp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GenMode::Import).c_str(), vdef.name().text());
                sep = ", ";
            }
            if(name.size() > 0) {
                fprintf(_fpImp, ")%s(", name.c_str());
            } else {
                fprintf(_fpImp, ")%s(", node.name().text());
            }
            sep = "";
            for(Ast::Scope::List::const_iterator it = node.sig().in().begin(); it != node.sig().in().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fpImp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GenMode::Import).c_str(), vdef.name().text());
                sep = ", ";
            }
            fprintf(_fpImp, ")%s;\n", getDefinitionType(node.defType()).c_str());
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
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fpImp, "public event(%s %s) => ", getQualifiedTypeSpecName(node.in().qualifiedTypeSpec(), GenMode::Import).c_str(), node.in().name().text());
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
    inline ImportGenerator(FILE* fpImp) : _fpImp(fpImp) {}
private:
    FILE* _fpImp;
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

    inline void visitChildrenIndent(const Ast::TypeSpec& node) {
        INDENT;
        visitChildren(node);
    }

    void visit(const Ast::TypedefDecl& node) {
        if(node.defType() == Ast::DefinitionType::Native) {
            fprintf(fpDecl(node), "%s// typedef %s native;\n", Indent::get(), node.name().text());
        } else {
            throw Exception("Internal error '%s'\n", node.name().text());
        }
    }

    void visit(const Ast::TypedefDefn& node) {
        if(node.defType() != Ast::DefinitionType::Native) {
            fprintf(fpDecl(node), "%stypedef %s %s;\n", Indent::get(), getQualifiedTypeSpecName(node.qTypeSpec(), GenMode::Normal).c_str(), node.name().text());
        } else {
            throw Exception("Internal error '%s'\n", node.name().text());
        }
    }

    void visit(const Ast::TemplateDecl& node) {
        if(node.defType() == Ast::DefinitionType::Native) {
            fprintf(fpDecl(node), "%s// template %s native;\n", Indent::get(), node.name().text());
        } else {
            throw Exception("Internal error '%s'\n", node.name().text());
        }
    }

    void visit(const Ast::TemplateDefn& node) {
        throw Exception("Internal error '%s'\n", node.name().text());
    }

    void visit(const Ast::EnumDefn& node) {
        if(node.defType() != Ast::DefinitionType::Native) {
            fprintf(fpDecl(node), "%sstruct %s {\n", Indent::get(), node.name().text());
            fprintf(fpDecl(node), "%s  enum T {\n", Indent::get());
            std::string sep = " ";
            for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& def = ref(*it);
                fprintf(fpDecl(node), "%s%s %s", Indent::get(), sep.c_str(), def.name().text());
                const Ast::ConstantExpr* cexpr = dynamic_cast<const Ast::ConstantExpr*>(ptr(def.initExpr()));
                if((cexpr == 0) || (ref(cexpr).value().string() != "#")) {
                    fprintf(fpDecl(node), " = ");
                    ExprGenerator(fpDecl(node), GenMode::Normal).visitNode(def.initExpr());
                }
                fprintf(fpDecl(node), "\n");
                sep = ",";
            }

            fprintf(fpDecl(node), "%s  };\n", Indent::get());
            fprintf(fpDecl(node), "%s};\n", Indent::get());
            fprintf(fpDecl(node), "\n");
        }
    }

    inline void visitStructDefn(const Ast::StructDefn& node, const Ast::StructDefn* base) {
        FILE* fp = fpDecl(node);
        if(node.defType() == Ast::DefinitionType::Native) {
            fprintf(fp, "%sstruct %s;\n", Indent::get(), node.name().text());
            return;
        }

        if(node.accessType() == Ast::AccessType::Protected) {
            fp = _fpHdr;
        }

        fprintf(fp, "%sstruct %s", Indent::get(), node.name().text());
        if(base) {
            fprintf(fp, " : public %s", getTypeSpecName(ref(base), GenMode::Normal).c_str());
        }
        fprintf(fp, " {\n");
        visitChildrenIndent(node);
        if(node.accessType() == Ast::AccessType::Protected) {
            fprintf(fp, "%s    struct Impl;\n", Indent::get());
            fprintf(fp, "%s    Impl* _impl;\n", Indent::get());
            fprintf(fp, "%s    inline void impl(Impl& val) {_impl = ptr(val);}\n", Indent::get());
        }

        // default-ctor
        fprintf(fp, "%s    explicit inline %s()", Indent::get(), node.name().text());
        std::string sep = " : ";
        if(node.accessType() == Ast::AccessType::Protected) {
            fprintf(fp, "%s_impl(0)", sep.c_str());
            sep = ", ";
        }
        if(node.hasList()) {
            for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(fp, "%s%s(", sep.c_str(), vdef.name().text());
                ExprGenerator(fp, GenMode::Normal).visitNode(vdef.initExpr());
                fprintf(fp, ")");
                sep = ", ";
            }
        }
        fprintf(fp, " {}\n");

        // member-list
        if(node.hasList()) {
            for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& vdef = ref(*it);
                const std::string tname = getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GenMode::Normal);
                fprintf(fp, "%s%s %s; /**/ ", Indent::get(), tname.c_str(), vdef.name().text());
                fprintf(fp, "template <typename T> inline T& _%s(const %s& val) {%s = val; return ref(static_cast<T*>(this));}\n",
                        vdef.name().text(), tname.c_str(), vdef.name().text());
            }
        }
        fprintf(fp, "%s};\n", Indent::get());
        fprintf(fp, "\n");
    }

    void visit(const Ast::RootStructDecl& node) {
        fprintf(fpDecl(node), "%sstruct %s;\n", Indent::get(), node.name().text());
    }

    void visit(const Ast::RootStructDefn& node) {
        visitStructDefn(node, 0);
    }

    void visit(const Ast::ChildStructDefn& node) {
        visitStructDefn(node, ptr(node.base()));
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
            fprintf(fp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GenMode::Normal).c_str(), vdef.name().text());
            sep = ", ";
        }
        fprintf(fp, ")");
    }

    inline void writeScopeMemberList(FILE* fp, const Ast::Scope& scope) {
        for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
            INDENT;
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fp, "%sconst %s %s;\n", Indent::get(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
        }
        const Ast::Scope* posParam = scope.posParam();
        if(posParam) {
            for(Ast::Scope::List::const_iterator it = ref(posParam).list().begin(); it != ref(posParam).list().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(fp, "%sconst %s %s;\n", Indent::get(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
            }
        }
    }

    inline void writeCtor(FILE* fp, const std::string& cname, const Ast::Scope& scope) {
        fprintf(fp, "%s    inline %s(", Indent::get(), cname.c_str());

        std::string sep = "";
        for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fp, "%sconst %s& p%s", sep.c_str(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
            sep = ", ";
        }

        fprintf(fp, ")");

        sep = " : ";
        for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fp, "%s%s(p%s)", sep.c_str(), vdef.name().text(), vdef.name().text());
            sep = ", ";
        }

        const Ast::Scope* posParam = scope.posParam();
        if(posParam) {
        }

        fprintf(fp, " {}\n");
    }

    inline void visitFunctionSig(const Ast::Function& node, const bool& isRoot, const bool& isDecl, const bool& isTest) {
        // child-typespecs
        if(node.childCount() > 0) {
            fprintf(fpDecl(node), "%spublic:\n", Indent::get());
            visitChildrenIndent(node);
        }

        // xref-list
        if(node.xref().size() > 0) {
            fprintf(fpDecl(node), "%spublic:\n", Indent::get());
            writeScopeMemberList(fpDecl(node), node.xrefScope());
            writeCtor(fpDecl(node), node.name().text(), node.xrefScope());
        }

        // in-param-list
        if(isRoot) {
            fprintf(fpDecl(node), "%spublic:\n", Indent::get());
            INDENT;
            fprintf(fpDecl(node), "%sstruct _In {\n", Indent::get());
            writeScopeMemberList(fpDecl(node), node.sig().inScope());
            writeCtor(fpDecl(node), "_In", node.sig().inScope());
            fprintf(fpDecl(node), "%s};\n", Indent::get());
        }

        // run-function-type
        fprintf(fpDecl(node), "%spublic:\n", Indent::get());
        if(isTest) {
            fprintf(fpDecl(node), "%s    const _Out& test(const _In& _in);\n", Indent::get());
        } else if((isDecl) && (node.defType() != Ast::DefinitionType::Native)) {
            fprintf(fpDecl(node), "%s    virtual const _Out& run(const _In& _in) = 0;\n", Indent::get());
        } else {
            fprintf(fpDecl(node), "%s    virtual const _Out& run(const _In& _in);\n", Indent::get());
        }

        if(!isTest) {
            // param-instance
            fprintf(fpDecl(node), "%s    Pointer<_Out> _out;\n", Indent::get());
            fprintf(fpDecl(node), "%s    inline const _Out& out(const _Out& val) {_out = Creator<_Out, _Out>::get(val); return _out.get();}\n", Indent::get());
        }
    }

    inline void visitFunction(const Ast::Function& node, const bool isDecl) {
        fprintf(fpDecl(node), "%sclass %s {\n", Indent::get(), node.name().text());
        visitFunctionSig(node, true, isDecl, false);
        fprintf(fpDecl(node), "%s};\n", Indent::get());
    }

    void visit(const Ast::RoutineDecl& node) {
        visitRoutine(fpDecl(node), node, true);
        fprintf(fpDecl(node), ";\n");
        fprintf(fpDecl(node), "\n");
    }

    void visit(const Ast::RoutineDefn& node) {
        visitRoutine(fpDecl(node), node, true);
        fprintf(fpDecl(node), ";\n");
        fprintf(fpDecl(node), "\n");
    }

    void visit(const Ast::FunctionRetn& node) {
        fprintf(fpDecl(node), "%sstruct _Out {\n", Indent::get());
        // generate out parameters
        for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
            INDENT;
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fpDecl(node), "%s%s %s;\n", Indent::get(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
        }

        // generate out setter
        fprintf(fpDecl(node), "%s    inline %s(", Indent::get(), node.name().text());
        std::string sep = "";
        for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fpDecl(node), "%sconst %s& p%s", sep.c_str(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
            sep = ", ";
        }
        fprintf(fpDecl(node), ")");
        sep = " : ";
        for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fpDecl(node), "%s%s(p%s)", sep.c_str(), vdef.name().text(), vdef.name().text());
            sep = ", ";
        }
        fprintf(fpDecl(node), "{}\n");

        // end return struct
        fprintf(fpDecl(node), "%s};\n", Indent::get());
        fprintf(fpDecl(node), "\n");
    }

    void visit(const Ast::FunctionDecl& node) {
        visitFunction(node, true);
        fprintf(fpDecl(node), "\n");
    }

    void visit(const Ast::RootFunctionDefn& node) {
        visitFunction(node, false);
        fprintf(fpDecl(node), "\n");
    }

    void visit(const Ast::ChildFunctionDefn& node) {
        bool isTest = (getTypeSpecName(node.base(), GenMode::Normal) == "test");
        if((isTest) && (!_config.test())) {
            return;
        }

        if(isTest) {
            fprintf(fpDecl(node), "%sclass %s : public test_< %s > {\n", Indent::get(), node.name().text(), node.name().text());
            fprintf(fpDecl(node), "%spublic:\n", Indent::get());
            fprintf(fpDecl(node), "%s    inline const char* const name() const {return \"%s\";}\n", Indent::get(), getTypeSpecName(node, GenMode::Normal).c_str());
        } else if(getTypeSpecName(node.base(), GenMode::Normal) == "main") {
            fprintf(fpDecl(node), "%sclass %s : public main_< %s > {\n", Indent::get(), node.name().text(), node.name().text());
        } else {
            fprintf(fpDecl(node), "%sclass %s : public %s {\n", Indent::get(), node.name().text(), getTypeSpecName(node.base(), GenMode::Normal).c_str());
        }
        visitFunctionSig(node, false, false, isTest);

        if(getTypeSpecName(node.base(), GenMode::Normal) == "test") {
            fprintf(fpDecl(node), "%s    static TestInstanceT<%s> s_test;\n", Indent::get(), node.name().text());
        } else if(getTypeSpecName(node.base(), GenMode::Normal) == "main") {
            fprintf(fpDecl(node), "%s    static MainInstanceT<%s> s_main;\n", Indent::get(), node.name().text());
        }

        fprintf(fpDecl(node), "%s};\n", Indent::get());
        fprintf(fpDecl(node), "\n");
    }

    void visit(const Ast::EventDecl& node) {
        fprintf(fpDecl(node), "%sstruct %s {\n", Indent::get(), node.name().text());
        // child-typespecs
        if(node.childCount() > 0) {
            fprintf(fpDecl(node), "%spublic:\n", Indent::get());
            visitChildrenIndent(node);
        }
        fprintf(fpDecl(node), "%s    FunctorList<Handler> _list;\n", Indent::get());
        fprintf(fpDecl(node), "%s    static %s instance;\n", Indent::get(), node.name().text());
        fprintf(fpDecl(node), "%s    static inline Handler& add(Handler* h) {return instance._list.add(h);}\n", Indent::get());
        fprintf(fpDecl(node), "%s    static void addHandler(%s %s, Handler* h);\n", Indent::get(), getQualifiedTypeSpecName(node.in().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.in().name().text());
        fprintf(fpDecl(node), "%s    template<typename T>static inline void addHandlerT(%s %s, T h) {return addHandler(%s, new T(h));}\n", Indent::get(), getQualifiedTypeSpecName(node.in().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.in().name().text(), node.in().name().text());
        fprintf(fpDecl(node), "%s};\n", Indent::get());
        fprintf(fpDecl(node), "\n");
        return;
    }

    void visit(const Ast::Namespace& node) {
        fprintf(_fpHdr, "namespace %s {\n", node.name().text());
        fprintf(_fpSrc, "namespace %s {\n", node.name().text());
        visitChildrenIndent(node);
        fprintf(_fpHdr, "} /* %s */", node.name().text());
        fprintf(_fpSrc, "} /* %s */", node.name().text());
    }

    void visit(const Ast::Root& node) {
        printf("root\n");
        visitChildrenIndent(node);
    }

public:
    inline TypeDeclarationGenerator(const Ast::Config& config, FILE* fpHdr, FILE* fpSrc) : _config(config), _fpHdr(fpHdr), _fpSrc(fpSrc) {}

private:
    const Ast::Config& _config;
    FILE* _fpHdr;
    FILE* _fpSrc;
};

struct StatementGenerator : public Ast::Statement::Visitor {
private:
    virtual void visit(const Ast::ImportStatement& node) {
        if(node.headerType() == Ast::HeaderType::Import) {
            fprintf(_fpImp, "import ");
        } else {
            fprintf(_fpImp, "include ");
        }
        std::string sep = "";
        for(Ast::ImportStatement::Part::const_iterator it = node.part().begin(); it != node.part().end(); ++it) {
            const Ast::Token& name = *it;
            fprintf(_fpImp, "%s%s", sep.c_str(), name.text());
            sep = "::";
        }
        if(node.defType() == Ast::DefinitionType::Native) {
            fprintf(_fpImp, " native");
        }
        fprintf(_fpImp, ";\n");

        FILE* fp = (node.accessType() == Ast::AccessType::Public)?_fpHdr:_fpSrc;
        std::string qt = (node.headerType() == Ast::HeaderType::Import)?"<>":"\"\"";
        fprintf(fp, "#include %c", qt.at(0));
        sep = "";
        for(Ast::ImportStatement::Part::const_iterator it = node.part().begin(); it != node.part().end(); ++it) {
            const Ast::Token& name = *it;
            fprintf(fp, "%s%s", sep.c_str(), name.text());
            sep = "/";
        }
        fprintf(fp, ".hpp%c\n", qt.at(1));
    }

    virtual void visit(const Ast::UserDefinedTypeSpecStatement& node) {
        //TypeDeclarationGenerator(_fpHdr, _fpSrc).visitNode(node.typeSpec());
    }

    virtual void visit(const Ast::AutoStatement& node) {
        fprintf(_fpSrc, "%s%s %s = ", Indent::get(), getQualifiedTypeSpecName(node.defn().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.defn().name().text());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.defn().initExpr());
        fprintf(_fpSrc, ";\n");
    }

    virtual void visit(const Ast::ExprStatement& node) {
        fprintf(_fpSrc, "%s", Indent::get());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.expr());
        fprintf(_fpSrc, ";\n");
    }

    virtual void visit(const Ast::PrintStatement& node) {
        fprintf(_fpSrc, "%sLog::get() << ", Indent::get());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.expr());
        fprintf(_fpSrc, " << Log::Out();\n");
    }

    virtual void visit(const Ast::IfStatement& node) {
        fprintf(_fpSrc, "%sif(", Indent::get());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.expr());
        fprintf(_fpSrc, ")\n");
        visitNode(node.tblock());
    }

    virtual void visit(const Ast::IfElseStatement& node) {
        fprintf(_fpSrc, "%sif(", Indent::get());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.expr());
        fprintf(_fpSrc, ")\n");
        visitNode(node.tblock());
        fprintf(_fpSrc, "%selse\n", Indent::get());
        visitNode(node.fblock());
    }

    virtual void visit(const Ast::WhileStatement& node) {
        fprintf(_fpSrc, "%swhile(", Indent::get());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.expr());
        fprintf(_fpSrc, ")\n");
        visitNode(node.block());
    }

    virtual void visit(const Ast::DoWhileStatement& node) {
        fprintf(_fpSrc, "%sdo\n", Indent::get());
        visitNode(node.block());
        fprintf(_fpSrc, "%swhile(", Indent::get());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.expr());
        fprintf(_fpSrc, ");\n");
    }

    virtual void visit(const Ast::ForExprStatement& node) {
        fprintf(_fpSrc, "%sfor(", Indent::get());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.init());
        fprintf(_fpSrc, "; ");
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.expr());
        fprintf(_fpSrc, "; ");
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.incr());
        fprintf(_fpSrc, ")\n");
        visitNode(node.block());
    }

    virtual void visit(const Ast::ForInitStatement& node) {
        fprintf(_fpSrc, "%sfor(%s %s = ", Indent::get(), getQualifiedTypeSpecName(node.init().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.init().name().text());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.init().initExpr());
        fprintf(_fpSrc, "; ");
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.expr());
        fprintf(_fpSrc, "; ");
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.incr());
        fprintf(_fpSrc, ")\n");
        visitNode(node.block());
    }

    virtual void visit(const Ast::ForeachListStatement& node) {
        const std::string etype = getQualifiedTypeSpecName(node.expr().qTypeSpec(), GenMode::Normal);
        const std::string vtype = getQualifiedTypeSpecName(node.valDef().qualifiedTypeSpec(), GenMode::Normal);

        std::string constit = "";
        if(node.expr().qTypeSpec().isConst()) {
            constit = "const_";
        }

        fprintf(_fpSrc, "%s{\n", Indent::get());
        fprintf(_fpSrc, "%s%s& _list = ", Indent::get(), etype.c_str());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.expr());
        fprintf(_fpSrc, ";\n");
        fprintf(_fpSrc, "%sfor(%s::%siterator _it = _list.begin(); _it != _list.end(); ++_it) {\n", Indent::get(), getTypeSpecName(node.expr().qTypeSpec().typeSpec(), GenMode::Normal).c_str(), constit.c_str());
        fprintf(_fpSrc, "%s%s& %s = *_it;\n", Indent::get(), vtype.c_str(), node.valDef().name().text());
        visitNode(node.block());
        fprintf(_fpSrc, "%s}\n", Indent::get());
        fprintf(_fpSrc, "%s}\n", Indent::get());
    }

    virtual void visit(const Ast::ForeachDictStatement& node) {
        std::string constit = "";
        if(node.expr().qTypeSpec().isConst())
            constit = "const_";
        fprintf(_fpSrc, "%s{\n", Indent::get());
        fprintf(_fpSrc, "%s%s& _list = ", Indent::get(), getQualifiedTypeSpecName(node.expr().qTypeSpec(), GenMode::Normal).c_str());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.expr());
        fprintf(_fpSrc, ";\n");
        fprintf(_fpSrc, "%sfor(%s::%siterator _it = _list.begin(); _it != _list.end(); ++_it) {\n", Indent::get(), getTypeSpecName(node.expr().qTypeSpec().typeSpec(), GenMode::Normal).c_str(), constit.c_str());
        fprintf(_fpSrc, "%sconst %s& %s = _it->first;\n", Indent::get(), getQualifiedTypeSpecName(node.keyDef().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.keyDef().name().text());
        fprintf(_fpSrc, "%s%s& %s = _it->second;\n", Indent::get(), getQualifiedTypeSpecName(node.valDef().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.valDef().name().text());
        visitNode(node.block());
        fprintf(_fpSrc, "%s}\n", Indent::get());
        fprintf(_fpSrc, "%s}\n", Indent::get());
    }

    virtual void visit(const Ast::CaseExprStatement& node) {
        fprintf(_fpSrc, "%scase (", Indent::get());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.expr());
        fprintf(_fpSrc, ") : \n");
        visitNode(node.block());
    }

    virtual void visit(const Ast::CaseDefaultStatement& node) {
        fprintf(_fpSrc, "%sdefault : \n", Indent::get());
        visitNode(node.block());
    }

    virtual void visit(const Ast::SwitchValueStatement& node) {
        fprintf(_fpSrc, "%sswitch(", Indent::get());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.expr());
        fprintf(_fpSrc, ")\n");
        visitNode(node.block());
    }

    virtual void visit(const Ast::SwitchExprStatement& node) {
        std::string ifstr = "if";
        for(Ast::CompoundStatement::List::const_iterator sit = node.block().list().begin(); sit != node.block().list().end(); ++sit) {
            const Ast::Statement* s = *sit;
            const Ast::CaseExprStatement* ce = dynamic_cast<const Ast::CaseExprStatement*>(s);
            const Ast::CaseDefaultStatement* cd = dynamic_cast<const Ast::CaseDefaultStatement*>(s);
            if(ce) {
                fprintf(_fpSrc, "%s%s(", Indent::get(), ifstr.c_str());
                ExprGenerator(_fpSrc, GenMode::Normal).visitNode(ref(ce).expr());
                fprintf(_fpSrc, ")\n");
                visitNode(ref(ce).block());
                ifstr = "else if";
            } else if(cd) {
                fprintf(_fpSrc, "%selse\n", Indent::get());
                visitNode(ref(cd).block());
                break;
            } else {
                throw Exception("Internal error: not a case statement inside switch\n");
            }
        }
    }

    virtual void visit(const Ast::BreakStatement& node) {
        fprintf(_fpSrc, "%sbreak;\n", Indent::get());
    }

    virtual void visit(const Ast::ContinueStatement& node) {
        fprintf(_fpSrc, "%scontinue;\n", Indent::get());
    }

    virtual void visit(const Ast::AddEventHandlerStatement& node) {
        std::string ename = getTypeSpecName(node.event(), GenMode::Normal);
        fprintf(_fpSrc, "%s%s::addHandlerT(", Indent::get(), ename.c_str());
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.source());
        fprintf(_fpSrc, ", ");
        ExprGenerator(_fpSrc, GenMode::Normal).visitNode(node.functor());
        fprintf(_fpSrc, ");\n");
    }

    virtual void visit(const Ast::RoutineReturnStatement& node) {
        fprintf(_fpSrc, "%sreturn", Indent::get());
        if(node.exprList().list().size() > 0) {
            fprintf(_fpSrc, " (");
            ExprGenerator(_fpSrc, GenMode::Normal, ", ").visitList(node.exprList());
            fprintf(_fpSrc, ")");
        }
        fprintf(_fpSrc, ";\n");
    }

    virtual void visit(const Ast::FunctionReturnStatement& node) {
        fprintf(_fpSrc, "%sreturn out(_Out(", Indent::get());
        ExprGenerator(_fpSrc, GenMode::Normal, ", ").visitList(node.exprList());
        fprintf(_fpSrc, "));\n");
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

public:
    inline StatementGenerator(FILE* fpHdr, FILE* fpSrc, FILE* fpImp) : _fpHdr(fpHdr), _fpSrc(fpSrc), _fpImp(fpImp) {}
private:
    FILE* _fpHdr;
    FILE* _fpSrc;
    FILE* _fpImp;
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

    void visit(const Ast::RootStructDecl& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::RootStructDefn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::ChildStructDefn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::RoutineDecl& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::RoutineDefn& node) {
        visitChildrenIndent(node);
        TypeDeclarationGenerator::visitRoutine(_fpSrc, node, false);
        fprintf(_fpSrc, "\n");
        StatementGenerator gen(_fpHdr, _fpSrc, _fpImp);
        gen.visitNode(node.block());
        fprintf(_fpSrc, "\n");
    }

    void visit(const Ast::FunctionRetn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::FunctionDecl& node) {
        visitChildrenIndent(node);
    }

    inline void visitFunction(const Ast::FunctionDefn& node, const bool& isTest) {
        if(isTest) {
            fprintf(_fpSrc, "const %s::_Out& %s::test(const _In& _in)\n", getTypeSpecName(node, GenMode::Normal).c_str(), getTypeSpecName(node, GenMode::Normal).c_str());
        } else {
            fprintf(_fpSrc, "const %s::_Out& %s::run(const _In& _in)\n", getTypeSpecName(node, GenMode::Normal).c_str(), getTypeSpecName(node, GenMode::Normal).c_str());
        }
        StatementGenerator gen(_fpHdr, _fpSrc, _fpImp);
        gen.visitNode(node.block());
        fprintf(_fpSrc, "\n");
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
            fprintf(_fpSrc, "TestInstanceT<%s> %s::s_test = TestInstanceT<%s>();\n", fname.c_str(), fname.c_str(), fname.c_str());
        } else if(getTypeSpecName(node.base(), GenMode::Normal) == "main") {
            fprintf(_fpSrc, "MainInstanceT<%s> %s::s_main = MainInstanceT<%s>();\n", fname.c_str(), fname.c_str(), fname.c_str());
        }
    }

    void visit(const Ast::EventDecl& node) {
        visitChildrenIndent(node);
        fprintf(_fpSrc, "%s %s::instance;\n", getTypeSpecName(node, GenMode::Normal).c_str(), getTypeSpecName(node, GenMode::Normal).c_str());
        if(node.defType() == Ast::DefinitionType::Direct) {
            fprintf(_fpSrc, "void %s::addHandler(%s %s, Handler* h) {\n", getTypeSpecName(node, GenMode::Normal).c_str(), getQualifiedTypeSpecName(node.in().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.in().name().text());
            fprintf(_fpSrc, "}\n");
        }
        fprintf(_fpSrc, "const %s::Add::_Out& %s::Add::run(const _In& in) {\n", getTypeSpecName(node, GenMode::Normal).c_str(), getTypeSpecName(node, GenMode::Normal).c_str());
        fprintf(_fpSrc, "    assert(false); //addHandler(in.%s, in.handler);\n", node.in().name().text());
        fprintf(_fpSrc, "    return out(_Out());\n");
        fprintf(_fpSrc, "}\n");
    }

    void visit(const Ast::Namespace& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::Root& node) {
        visitChildrenIndent(node);
    }

public:
    inline TypeDefinitionGenerator(const Ast::Config& config, FILE* fpHdr, FILE* fpSrc, FILE* fpImp) : _config(config), _fpHdr(fpHdr), _fpSrc(fpSrc), _fpImp(fpImp) {}
private:
    const Ast::Config& _config;
    FILE* _fpHdr;
    FILE* _fpSrc;
    FILE* _fpImp;
};

struct Generator::Impl {
    inline Impl(const Ast::Project& project, const Ast::Config& config, const Ast::Unit& unit) : _project(project), _config(config), _unit(unit), _fpHdr(0), _fpSrc(0), _fpImp(0) {}
    inline void generateHeaderIncludes(const std::string& basename);
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

inline void Generator::Impl::generateHeaderIncludes(const std::string& basename) {
    fprintf(_fpHdr, "#pragma once\n\n");
    for(Ast::Config::PathList::const_iterator it = _config.includeFileList().begin(); it != _config.includeFileList().end(); ++it) {
        const std::string& filename = *it;
        fprintf(_fpSrc, "#include \"%s\"\n", filename.c_str());
    }

    StatementGenerator gen(_fpHdr, _fpSrc, _fpImp);
    for(Ast::Unit::ImportStatementList::const_iterator sit = _unit.importStatementList().begin(); sit != _unit.importStatementList().end(); ++sit) {
        const Ast::ImportStatement& s = ref(*sit);
        gen.visitNode(s);
    }
    fprintf(_fpSrc, "#include \"%s.hpp\"\n", basename.c_str());
    fprintf(_fpHdr, "\n");

    std::string ns;
    std::string sep;
    for(Ast::Unit::NsPartList::const_iterator it = _unit.nsPartList().begin(); it != _unit.nsPartList().end(); ++it) {
        const Ast::Token& token = *it;
        ns += sep;
        ns += token.string();
    }

    if(ns.size() > 0) {
        fprintf(_fpImp, "namespace %s;\n\n", ns.c_str());
    }
}

inline void Generator::Impl::run() {
    Indent::init();
    std::string basename = getBaseName(_unit.filename());
    OutputFile ofImp(_fpImp, basename + ".ipp");unused(ofImp);
    OutputFile ofHdr(_fpHdr, basename + ".hpp");unused(ofHdr);
    OutputFile ofSrc(_fpSrc, basename + ".cpp");unused(ofSrc);

    generateHeaderIncludes(basename);

    ImportGenerator(_fpImp).visitChildren(_unit.rootNS());
    TypeDeclarationGenerator(_config, _fpHdr, _fpSrc).visitChildren(_unit.rootNS());
    fprintf(_fpHdr, "\n");
    fprintf(_fpSrc, "\n");

    TypeDefinitionGenerator(_config, _fpHdr, _fpSrc, _fpImp).visitChildren(_unit.rootNS());
}

//////////////////////////////////////////////
Generator::Generator(const Ast::Project& project, const Ast::Config& config, const Ast::Unit& unit) : _impl(0) {_impl = new Impl(project, config, unit);}
Generator::~Generator() {delete _impl;}
void Generator::run() {return ref(_impl).run();}
