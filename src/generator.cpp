#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "generator.hpp"
#include "outfile.hpp"

struct GetNameMode {
    enum T {
        Normal,
        Import,
        TypeSpecMemberRef
    };
};

static bool getRootName(const Ast::TypeSpec& typeSpec, const std::string& sep, std::string& name, const GetNameMode::T& mode);

inline std::string getTypeSpecName(const Ast::TypeSpec& typeSpec, const GetNameMode::T& mode, const std::string& sep = "::") {
    std::string name;
    getRootName(typeSpec, sep, name, mode);
    return name;
}

inline std::string getQualifiedTypeSpecName(const Ast::QualifiedTypeSpec& qtypeSpec, const GetNameMode::T& mode, const std::string& sep = "::") {
    std::string name;
    if(qtypeSpec.isConst())
        name += "const ";
    getRootName(qtypeSpec.typeSpec(), sep, name, mode);
    if(qtypeSpec.isRef())
        name += "&";
    return name;
}

static bool getName(const Ast::TypeSpec& typeSpec, const std::string& sep, std::string& name, const GetNameMode::T& mode) {
    const Ast::ChildTypeSpec* ctypeSpec = dynamic_cast<const Ast::ChildTypeSpec*>(ptr(typeSpec));
    if(!ctypeSpec)
        return false;

    if(getName(ref(ctypeSpec).parent(), sep, name, mode))
        name += sep;

    name += typeSpec.name().string();

    if(dynamic_cast<const Ast::EnumDefn*>(ptr(typeSpec)) != 0) {
        if(mode == GetNameMode::Normal) {
            name += sep;
            name += "T";
        } else if(mode == GetNameMode::Import) {
        }
    }

    return true;
}

static bool getRootName(const Ast::TypeSpec& typeSpec, const std::string& sep, std::string& name, const GetNameMode::T& mode) {
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(ptr(typeSpec));

    if(mode == GetNameMode::Import) {
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

inline std::string getDefinitionType(const Ast::DefinitionType::T& defType) {
    switch(defType) {
        case Ast::DefinitionType::Direct:
            return "";
        case Ast::DefinitionType::Native:
            return " native";
    }
    throw Exception("Internal error: Unknown Definition Type '%d'\n", defType);
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
    inline ExprGenerator(FILE* fp, const bool& isImp, const std::string& sep2 = "", const std::string& sep1 = "") : _fp(fp), _isImp(isImp), _sep2(sep2), _sep1(sep1), _sep0(sep1) {}
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
        if(_isImp) {
            fprintf(_fp, "[");
            if(node.list().list().size() == 0) {
                fprintf(_fp, "%s", getQualifiedTypeSpecName(node.list().valueType(), GetNameMode::Import).c_str());
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
            fprintf(_fp, "list<%s>::creator()", getQualifiedTypeSpecName(node.list().valueType(), GetNameMode::Normal).c_str());
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
        fprintf(_fp, "dict<%s, %s>::creator()", getQualifiedTypeSpecName(node.list().keyType(), GetNameMode::Normal).c_str(), getQualifiedTypeSpecName(node.list().valueType(), GetNameMode::Normal).c_str());
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
        const std::string name = getTypeSpecName(node.routine(), GetNameMode::Normal);
        if((name == "assert") || (name == "unused")) {
            std::string sep;
            for(Ast::ExprList::List::const_iterator it = node.exprList().list().begin(); it != node.exprList().list().end(); ++it) {
                const Ast::Expr& expr = ref(*it);
                fprintf(_fp, "%s%s(", sep.c_str(), name.c_str());
                ExprGenerator(_fp, _isImp).visitNode(expr);
                fprintf(_fp, ")");
                sep = ";";
            }
            return;
        }

        if(name == "check") {
        }

        fprintf(_fp, "%s(", name.c_str());
        ExprGenerator(_fp, ", ").visitList(node.exprList());
        fprintf(_fp, ")");
    }

    virtual void visit(const Ast::FunctorCallExpr& node) {
        ExprGenerator(_fp, _isImp).visitNode(node.expr());
        fprintf(_fp, ".run(");
        fprintf(_fp, "%s::_In(", getTypeSpecName(node.expr().qTypeSpec().typeSpec(), GetNameMode::Normal).c_str());
        ExprGenerator(_fp, _isImp, ", ").visitList(node.exprList());
        fprintf(_fp, "))");
    }

    virtual void visit(const Ast::RunExpr& node) {
        fprintf(_fp, "CallContext::get().add(");
        ExprGenerator(_fp, _isImp).visitNode(node.callExpr().expr());
        fprintf(_fp, ", ");
        fprintf(_fp, "%s::_In(", getTypeSpecName(node.callExpr().expr().qTypeSpec().typeSpec(), GetNameMode::Normal).c_str());
        ExprGenerator(_fp, _isImp, ", ").visitList(node.callExpr().exprList());
        fprintf(_fp, "))");
    }

    virtual void visit(const Ast::OrderedExpr& node) {
        fprintf(_fp, "(");
        visitNode(node.expr());
        fprintf(_fp, ")");
    }

    virtual void visit(const Ast::TypeofExpr& node) {
        fprintf(_fp, "\"%s\"", getQualifiedTypeSpecName(node.expr().qTypeSpec(), GetNameMode::Import).c_str());
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
        fprintf(_fp, "%s", getTypeSpecName(node.typeSpec(), GetNameMode::TypeSpecMemberRef).c_str());
        fprintf(_fp, "::%s", node.vref().name().text());
    }

    virtual void visit(const Ast::EnumMemberExpr& node) {
        fprintf(_fp, "%s", getTypeSpecName(node.typeSpec(), GetNameMode::TypeSpecMemberRef).c_str());
        if(_isImp) {
            fprintf(_fp, ".%s", node.vref().name().text());
        } else {
            fprintf(_fp, "::%s", node.vref().name().text());
        }
    }

    virtual void visit(const Ast::StructInstanceExpr& node) {
        fprintf(_fp, "%s()", getTypeSpecName(node.structDefn(), GetNameMode::Normal).c_str());
        for(Ast::StructInitPartList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
            const Ast::StructInitPart& part = ref(*it);
            fprintf(_fp, "._%s<%s>(", part.vdef().name().text(), getTypeSpecName(node.structDefn(), GetNameMode::Normal).c_str());
            visitNode(part.expr());
            fprintf(_fp, ")");
        }
    }

    virtual void visit(const Ast::FunctionInstanceExpr& node) {
        std::string fname = getTypeSpecName(node.function(), GetNameMode::Normal);
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
        if(getQualifiedTypeSpecName(node.qTypeSpec(), GetNameMode::Normal) == "char") {
            fprintf(_fp, "\'%s\'", node.value().text());
            return;
        }
        if(getQualifiedTypeSpecName(node.qTypeSpec(), GetNameMode::Normal) == "std::string") {
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
    const bool _isImp;
    const std::string _sep2;
    const std::string _sep1;
    std::string _sep0;
};

struct ImportGenerator : public Ast::TypeSpec::Visitor {
    inline void visitChildrenIndent(const Ast::TypeSpec& node) {
        visitChildren(node);
    }

    void visit(const Ast::TypedefDefn& node) {
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fpImp, "public typedef %s%s;\n", node.name().text(), getDefinitionType(node.defType()).c_str());
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
                    ExprGenerator(_fpImp, true).visitNode(def.initExpr());
                }
                fprintf(_fpImp, ";\n");
            }
            fprintf(_fpImp, "};\n");
            fprintf(_fpImp, "\n");
        }
        visitChildrenIndent(node);
    }

    inline void visitStructDefn(const Ast::StructDefn& node, const Ast::StructDefn* base) {
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fpImp, "public struct %s", node.name().text());
            if(base) {
                fprintf(_fpImp, " : %s", getTypeSpecName(ref(base), GetNameMode::Import).c_str());
            }
            fprintf(_fpImp, "%s {\n", getDefinitionType(node.defType()).c_str());
            for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fpImp, "    %s %s", getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GetNameMode::Import).c_str(), vdef.name().text());
                fprintf(_fpImp, " = ");
                ExprGenerator(_fpImp, true).visitNode(vdef.initExpr());
                fprintf(_fpImp, ";\n");
            }
            fprintf(_fpImp, "};\n");
        } else if(node.accessType() == Ast::AccessType::Protected) {
            fprintf(_fpImp, "protected struct %s%s;\n", node.name().text(), getDefinitionType(node.defType()).c_str());
        }
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
            fprintf(_fpImp, "public routine %s ", getQualifiedTypeSpecName(node.outType(), GetNameMode::Import).c_str());
            fprintf(_fpImp, "%s", node.name().text());
            fprintf(_fpImp, "(");
            std::string sep;
            for(Ast::Scope::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fpImp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GetNameMode::Normal).c_str(), vdef.name().text());
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
                fprintf(_fpImp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GetNameMode::Import).c_str(), vdef.name().text());
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
                fprintf(_fpImp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GetNameMode::Import).c_str(), vdef.name().text());
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
            fprintf(_fpImp, "public event(%s %s) => ", getQualifiedTypeSpecName(node.in().qualifiedTypeSpec(), GetNameMode::Import).c_str(), node.in().name().text());
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

    void visit(const Ast::TypedefDefn& node) {
        if(node.defType() == Ast::DefinitionType::Native) {
            fprintf(fpDecl(node), "%s// typedef %s native;\n", Indent::get(), node.name().text());
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
                    ExprGenerator(fpDecl(node), false).visitNode(def.initExpr());
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
        FILE* fp = 0;
        std::string impl;
        if(node.accessType() == Ast::AccessType::Protected) {
            fprintf(_fpHdr, "%sstruct %s {\n", Indent::get(), node.name().text());
            {
                INDENT;
                fprintf(_fpHdr, "%sstruct Impl;\n", Indent::get());
                fprintf(_fpHdr, "%sImpl* _impl;\n", Indent::get());
            }
            fprintf(_fpHdr, "%s    %s();\n", Indent::get(), node.name().text());
            fprintf(_fpHdr, "%s    %s(const Impl& impl);\n", Indent::get(), node.name().text());
            fprintf(_fpHdr, "%s};\n", Indent::get());
            fprintf(_fpHdr, "\n");
            impl = "::Impl";
            fp = _fpSrc;
        } else {
            fp = fpDecl(node);
        }

        if(node.defType() != Ast::DefinitionType::Native) {
            fprintf(fp, "%sstruct %s%s ", Indent::get(), node.name().text(), impl.c_str());
            if(base) {
                fprintf(fp, " : public %s ", getTypeSpecName(ref(base), GetNameMode::Normal).c_str());
            }
            fprintf(fp, "{\n");
            for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(fp, "%s%s %s;\n", Indent::get(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GetNameMode::Normal).c_str(), vdef.name().text());
                fprintf(fp, "%stemplate <typename T> inline T& _%s(const %s& val) {%s = val; return ref(static_cast<T*>(this));}\n",
                        Indent::get(), vdef.name().text(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GetNameMode::Normal).c_str(), vdef.name().text());
            }
            visitChildrenIndent(node);
            fprintf(fp, "%s};\n", Indent::get());
            fprintf(fp, "\n");
        }

        if(node.accessType() == Ast::AccessType::Protected) {
            fprintf(fp, "%s::%s() : _impl(0) {_impl = new Impl();} \n", node.name().text(), node.name().text());
            fprintf(fp, "%s::%s(const Impl& impl) : _impl(0) {_impl = new Impl(impl);} \n", node.name().text(), node.name().text());
        }
        return;
    }

    void visit(const Ast::RootStructDefn& node) {
        visitStructDefn(node, 0);
    }

    void visit(const Ast::ChildStructDefn& node) {
        visitStructDefn(node, ptr(node.base()));
    }

    static inline void visitRoutine(FILE* fp, const Ast::Routine& node, const bool& inNS) {
        fprintf(fp, "%s%s ", Indent::get(), getQualifiedTypeSpecName(node.outType(), GetNameMode::Normal).c_str());
        if(inNS) {
            fprintf(fp, "%s", node.name().text());
        } else {
            fprintf(fp, "%s", getTypeSpecName(node, GetNameMode::Normal).c_str());
        }
        fprintf(fp, "(");
        std::string sep;
        for(Ast::Scope::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GetNameMode::Normal).c_str(), vdef.name().text());
            sep = ", ";
        }
        fprintf(fp, ")");
    }

    inline void writeScopeMemberList(FILE* fp, const Ast::Scope& scope) {
        for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
            INDENT;
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fp, "%sconst %s %s;\n", Indent::get(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GetNameMode::Normal).c_str(), vdef.name().text());
        }
        const Ast::Scope* posParam = scope.posParam();
        if(posParam) {
            for(Ast::Scope::List::const_iterator it = ref(posParam).list().begin(); it != ref(posParam).list().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(fp, "%sconst %s %s;\n", Indent::get(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GetNameMode::Normal).c_str(), vdef.name().text());
            }
        }
    }

    inline void writeCtor(FILE* fp, const std::string& cname, const Ast::Scope& scope) {
        fprintf(fp, "%s    inline %s(", Indent::get(), cname.c_str());

        std::string sep = "";
        for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fp, "%sconst %s& p%s", sep.c_str(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GetNameMode::Normal).c_str(), vdef.name().text());
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
            fprintf(fpDecl(node), "%s    inline const _Out& out(_Out* val) {_out = val; return *_out;}\n", Indent::get());
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
            fprintf(fpDecl(node), "%s%s %s;\n", Indent::get(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GetNameMode::Normal).c_str(), vdef.name().text());
        }

        // generate out setter
        fprintf(fpDecl(node), "%s    inline %s(", Indent::get(), node.name().text());
        std::string sep = "";
        for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fpDecl(node), "%sconst %s& p%s", sep.c_str(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GetNameMode::Normal).c_str(), vdef.name().text());
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
        bool isTest = (getTypeSpecName(node.base(), GetNameMode::Normal) == "test");
        if((isTest) && (!_config.test())) {
            return;
        }

        if(isTest) {
            fprintf(fpDecl(node), "%sclass %s : public test_< %s > {\n", Indent::get(), node.name().text(), node.name().text());
            fprintf(fpDecl(node), "%spublic:\n", Indent::get());
            fprintf(fpDecl(node), "%s    inline const char* const name() const {return \"%s\";}\n", Indent::get(), getTypeSpecName(node, GetNameMode::Normal).c_str());
        } else if(getTypeSpecName(node.base(), GetNameMode::Normal) == "main") {
            fprintf(fpDecl(node), "%sclass %s : public main_< %s > {\n", Indent::get(), node.name().text(), node.name().text());
        } else {
            fprintf(fpDecl(node), "%sclass %s : public %s {\n", Indent::get(), node.name().text(), getTypeSpecName(node.base(), GetNameMode::Normal).c_str());
        }
        visitFunctionSig(node, false, false, isTest);

        if(getTypeSpecName(node.base(), GetNameMode::Normal) == "test") {
            fprintf(fpDecl(node), "%s    static TestInstanceT<%s> s_test;\n", Indent::get(), node.name().text());
        } else if(getTypeSpecName(node.base(), GetNameMode::Normal) == "main") {
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
        fprintf(fpDecl(node), "%s    static void addHandler(%s %s, Handler* h);\n", Indent::get(), getQualifiedTypeSpecName(node.in().qualifiedTypeSpec(), GetNameMode::Normal).c_str(), node.in().name().text());
        fprintf(fpDecl(node), "%s    template<typename T>static inline void addHandlerT(%s %s, T h) {return addHandler(%s, new T(h));}\n", Indent::get(), getQualifiedTypeSpecName(node.in().qualifiedTypeSpec(), GetNameMode::Normal).c_str(), node.in().name().text(), node.in().name().text());
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

        FILE* fp = (node.defType() == Ast::DefinitionType::Native)?_fpSrc:_fpHdr;
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
        fprintf(_fpSrc, "%s%s %s = ", Indent::get(), getQualifiedTypeSpecName(node.defn().qualifiedTypeSpec(), GetNameMode::Normal).c_str(), node.defn().name().text());
        ExprGenerator(_fpSrc, false).visitNode(node.defn().initExpr());
        fprintf(_fpSrc, ";\n");
    }

    virtual void visit(const Ast::ExprStatement& node) {
        fprintf(_fpSrc, "%s", Indent::get());
        ExprGenerator(_fpSrc, false).visitNode(node.expr());
        fprintf(_fpSrc, ";\n");
    }

    virtual void visit(const Ast::PrintStatement& node) {
        fprintf(_fpSrc, "%sLog::get() << ", Indent::get());
        ExprGenerator(_fpSrc, false).visitNode(node.expr());
        fprintf(_fpSrc, " << Log::Out();\n");
    }

    virtual void visit(const Ast::IfStatement& node) {
        fprintf(_fpSrc, "%sif(", Indent::get());
        ExprGenerator(_fpSrc, false).visitNode(node.expr());
        fprintf(_fpSrc, ")\n");
        visitNode(node.tblock());
    }

    virtual void visit(const Ast::IfElseStatement& node) {
        fprintf(_fpSrc, "%sif(", Indent::get());
        ExprGenerator(_fpSrc, false).visitNode(node.expr());
        fprintf(_fpSrc, ")\n");
        visitNode(node.tblock());
        fprintf(_fpSrc, "%selse\n", Indent::get());
        visitNode(node.fblock());
    }

    virtual void visit(const Ast::WhileStatement& node) {
        fprintf(_fpSrc, "%swhile(", Indent::get());
        ExprGenerator(_fpSrc, false).visitNode(node.expr());
        fprintf(_fpSrc, ")\n");
        visitNode(node.block());
    }

    virtual void visit(const Ast::DoWhileStatement& node) {
        fprintf(_fpSrc, "%sdo\n", Indent::get());
        visitNode(node.block());
        fprintf(_fpSrc, "%swhile(", Indent::get());
        ExprGenerator(_fpSrc, false).visitNode(node.expr());
        fprintf(_fpSrc, ");\n");
    }

    virtual void visit(const Ast::ForExprStatement& node) {
        fprintf(_fpSrc, "%sfor(", Indent::get());
        ExprGenerator(_fpSrc, false).visitNode(node.init());
        fprintf(_fpSrc, "; ");
        ExprGenerator(_fpSrc, false).visitNode(node.expr());
        fprintf(_fpSrc, "; ");
        ExprGenerator(_fpSrc, false).visitNode(node.incr());
        fprintf(_fpSrc, ")\n");
        visitNode(node.block());
    }

    virtual void visit(const Ast::ForInitStatement& node) {
        fprintf(_fpSrc, "%sfor(%s %s = ", Indent::get(), getQualifiedTypeSpecName(node.init().qualifiedTypeSpec(), GetNameMode::Normal).c_str(), node.init().name().text());
        ExprGenerator(_fpSrc, false).visitNode(node.init().initExpr());
        fprintf(_fpSrc, "; ");
        ExprGenerator(_fpSrc, false).visitNode(node.expr());
        fprintf(_fpSrc, "; ");
        ExprGenerator(_fpSrc, false).visitNode(node.incr());
        fprintf(_fpSrc, ")\n");
        visitNode(node.block());
    }

    virtual void visit(const Ast::ForeachListStatement& node) {
        const std::string etype = getQualifiedTypeSpecName(node.expr().qTypeSpec(), GetNameMode::Normal);
        const std::string vtype = getQualifiedTypeSpecName(node.valDef().qualifiedTypeSpec(), GetNameMode::Normal);

        std::string constit = "";
        if(node.expr().qTypeSpec().isConst()) {
            constit = "const_";
        }

        fprintf(_fpSrc, "%s{\n", Indent::get());
        fprintf(_fpSrc, "%s%s& _list = ", Indent::get(), etype.c_str());
        ExprGenerator(_fpSrc, false).visitNode(node.expr());
        fprintf(_fpSrc, ";\n");
        fprintf(_fpSrc, "%sfor(%s::%siterator _it = _list.begin(); _it != _list.end(); ++_it) {\n", Indent::get(), getTypeSpecName(node.expr().qTypeSpec().typeSpec(), GetNameMode::Normal).c_str(), constit.c_str());
        fprintf(_fpSrc, "%s%s& %s = _list.val(*_it);\n", Indent::get(), vtype.c_str(), node.valDef().name().text());
        visitNode(node.block());
        fprintf(_fpSrc, "%s}\n", Indent::get());
        fprintf(_fpSrc, "%s}\n", Indent::get());
    }

    virtual void visit(const Ast::ForeachDictStatement& node) {
        std::string constit = "";
        if(node.expr().qTypeSpec().isConst())
            constit = "const_";
        fprintf(_fpSrc, "%s{\n", Indent::get());
        fprintf(_fpSrc, "%s%s& _list = ", Indent::get(), getQualifiedTypeSpecName(node.expr().qTypeSpec(), GetNameMode::Normal).c_str());
        ExprGenerator(_fpSrc, false).visitNode(node.expr());
        fprintf(_fpSrc, ";\n");
        fprintf(_fpSrc, "%sfor(%s::%siterator _it = _list.begin(); _it != _list.end(); ++_it) {\n", Indent::get(), getTypeSpecName(node.expr().qTypeSpec().typeSpec(), GetNameMode::Normal).c_str(), constit.c_str());
        fprintf(_fpSrc, "%sconst %s& %s = _it->first;\n", Indent::get(), getQualifiedTypeSpecName(node.keyDef().qualifiedTypeSpec(), GetNameMode::Normal).c_str(), node.keyDef().name().text());
        fprintf(_fpSrc, "%s%s& %s = _list.val(_it->second);\n", Indent::get(), getQualifiedTypeSpecName(node.valDef().qualifiedTypeSpec(), GetNameMode::Normal).c_str(), node.valDef().name().text());
        visitNode(node.block());
        fprintf(_fpSrc, "%s}\n", Indent::get());
        fprintf(_fpSrc, "%s}\n", Indent::get());
    }

    virtual void visit(const Ast::CaseExprStatement& node) {
        fprintf(_fpSrc, "%scase (", Indent::get());
        ExprGenerator(_fpSrc, false).visitNode(node.expr());
        fprintf(_fpSrc, ") : \n");
        visitNode(node.block());
    }

    virtual void visit(const Ast::CaseDefaultStatement& node) {
        fprintf(_fpSrc, "%sdefault : \n", Indent::get());
        visitNode(node.block());
    }

    virtual void visit(const Ast::SwitchValueStatement& node) {
        fprintf(_fpSrc, "%sswitch(", Indent::get());
        ExprGenerator(_fpSrc, false).visitNode(node.expr());
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
                ExprGenerator(_fpSrc, false).visitNode(ref(ce).expr());
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
        std::string ename = getTypeSpecName(node.event(), GetNameMode::Normal);
        fprintf(_fpSrc, "%s%s::addHandlerT(", Indent::get(), ename.c_str());
        ExprGenerator(_fpSrc, false).visitNode(node.source());
        fprintf(_fpSrc, ", ");
        ExprGenerator(_fpSrc, false).visitNode(node.functor());
        fprintf(_fpSrc, ");\n");
    }

    virtual void visit(const Ast::RoutineReturnStatement& node) {
        fprintf(_fpSrc, "%sreturn", Indent::get());
        if(node.exprList().list().size() > 0) {
            fprintf(_fpSrc, " (");
            ExprGenerator(_fpSrc, false, ", ").visitList(node.exprList());
            fprintf(_fpSrc, ")");
        }
        fprintf(_fpSrc, ";\n");
    }

    virtual void visit(const Ast::FunctionReturnStatement& node) {
        fprintf(_fpSrc, "%sreturn out(new _Out(", Indent::get());
        ExprGenerator(_fpSrc, false, ", ").visitList(node.exprList());
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
            fprintf(_fpSrc, "const %s::_Out& %s::test(const _In& _in)\n", getTypeSpecName(node, GetNameMode::Normal).c_str(), getTypeSpecName(node, GetNameMode::Normal).c_str());
        } else {
            fprintf(_fpSrc, "const %s::_Out& %s::run(const _In& _in)\n", getTypeSpecName(node, GetNameMode::Normal).c_str(), getTypeSpecName(node, GetNameMode::Normal).c_str());
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
        bool isTest = (getTypeSpecName(node.base(), GetNameMode::Normal) == "test");
        if((isTest) && (!_config.test())) {
            return;
        }

        visitChildrenIndent(node);
        visitFunction(node, isTest);
        std::string fname = getTypeSpecName(node, GetNameMode::Normal);

        if(isTest) {
            fprintf(_fpSrc, "TestInstanceT<%s> %s::s_test = TestInstanceT<%s>();\n", fname.c_str(), fname.c_str(), fname.c_str());
        } else if(getTypeSpecName(node.base(), GetNameMode::Normal) == "main") {
            fprintf(_fpSrc, "MainInstanceT<%s> %s::s_main = MainInstanceT<%s>();\n", fname.c_str(), fname.c_str(), fname.c_str());
        }
    }

    void visit(const Ast::EventDecl& node) {
        visitChildrenIndent(node);
        fprintf(_fpSrc, "%s %s::instance;\n", getTypeSpecName(node, GetNameMode::Normal).c_str(), getTypeSpecName(node, GetNameMode::Normal).c_str());
        if(node.defType() == Ast::DefinitionType::Direct) {
            fprintf(_fpSrc, "void %s::addHandler(%s %s, Handler* h) {\n", getTypeSpecName(node, GetNameMode::Normal).c_str(), getQualifiedTypeSpecName(node.in().qualifiedTypeSpec(), GetNameMode::Normal).c_str(), node.in().name().text());
            fprintf(_fpSrc, "}\n");
        }
        fprintf(_fpSrc, "const %s::Add::_Out& %s::Add::run(const _In& in) {\n", getTypeSpecName(node, GetNameMode::Normal).c_str(), getTypeSpecName(node, GetNameMode::Normal).c_str());
        fprintf(_fpSrc, "    assert(false); //addHandler(in.%s, in.handler);\n", node.in().name().text());
        fprintf(_fpSrc, "    return out(new _Out());\n");
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
    fprintf(_fpSrc, "#include \"%s.hpp\"\n", basename.c_str());

    StatementGenerator gen(_fpHdr, _fpSrc, _fpImp);
    for(Ast::Unit::ImportStatementList::const_iterator sit = _unit.importStatementList().begin(); sit != _unit.importStatementList().end(); ++sit) {
        const Ast::ImportStatement& s = ref(*sit);
        gen.visitNode(s);
    }
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
