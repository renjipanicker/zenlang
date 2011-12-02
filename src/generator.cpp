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
                fprintf(_fp, "%s", node.vref().name().text());
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

struct GeneratorContext {
    typedef std::list<const Ast::Statement*> List;
    struct TargetMode {
        enum T {
            Import,
            Header,
            Source,
            Body,
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
    List _list;
    inline GeneratorContext(const TargetMode::T& targetMode, const IndentMode::T& indentMode) : _targetMode(targetMode), _indentMode(indentMode) {}
    void run(const Ast::Config& config, FILE* fp, const std::string& basename, const Ast::Statement& block);
};

struct ImportGenerator : public Ast::TypeSpec::Visitor {
    inline void visitChildrenIndent(const Ast::TypeSpec& node) {
        visitChildren(node);
    }

    void visit(const Ast::TypedefDecl& node) {
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fp, "public typedef %s%s;\n", node.name().text(), getDefinitionType(node.defType()).c_str());
        }
        visitChildrenIndent(node);
    }

    void visit(const Ast::TypedefDefn& node) {
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fp, "public typedef %s%s %s;\n", node.name().text(), getDefinitionType(node.defType()).c_str(), getQualifiedTypeSpecName(node.qTypeSpec(), GenMode::Import).c_str());
        }
        visitChildrenIndent(node);
    }

    void visit(const Ast::TemplateDecl& node) {
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fp, "public template <");
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
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fp, "public enum %s%s {\n", node.name().text(), getDefinitionType(node.defType()).c_str());
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
        if(node.accessType() != Ast::AccessType::Private) {
            fprintf(_fp, "%s struct %s", getAccessType(node.accessType()).c_str(), node.name().text());
            if(base) {
                fprintf(_fp, " : %s", getTypeSpecName(ref(base), GenMode::Import).c_str());
            }
            fprintf(_fp, "%s {\n", getDefinitionType(node.defType()).c_str());
            for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fp, "    %s %s", getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GenMode::Import).c_str(), vdef.name().text());
                fprintf(_fp, " = ");
                ExprGenerator(_fp, GenMode::Import).visitNode(vdef.initExpr());
                fprintf(_fp, ";\n");
            }
            fprintf(_fp, "};\n");
        }
    }

    void visit(const Ast::StructDecl& node) {
        fprintf(_fp, "%s struct %s%s;\n", getAccessType(node.accessType()).c_str(), node.name().text(), getDefinitionType(node.defType()).c_str());
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
            fprintf(_fp, "public routine %s ", getQualifiedTypeSpecName(node.outType(), GenMode::Import).c_str());
            fprintf(_fp, "%s", node.name().text());
            fprintf(_fp, "(");
            std::string sep;
            for(Ast::Scope::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GenMode::Normal).c_str(), vdef.name().text());
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
        if((name.size() > 0) || (node.accessType() == Ast::AccessType::Public)) {
            if(!isEvent) {
                fprintf(_fp, "public ");
            }
            fprintf(_fp, "function (");

            std::string sep;
            for(Ast::Scope::List::const_iterator it = node.sig().out().begin(); it != node.sig().out().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GenMode::Import).c_str(), vdef.name().text());
                sep = ", ";
            }
            if(name.size() > 0) {
                fprintf(_fp, ")%s(", name.c_str());
            } else {
                fprintf(_fp, ")%s(", node.name().text());
            }
            sep = "";
            for(Ast::Scope::List::const_iterator it = node.sig().in().begin(); it != node.sig().in().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qualifiedTypeSpec(), GenMode::Import).c_str(), vdef.name().text());
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
        if(node.accessType() == Ast::AccessType::Public) {
            fprintf(_fp, "public event(%s %s) => ", getQualifiedTypeSpecName(node.in().qualifiedTypeSpec(), GenMode::Import).c_str(), node.in().name().text());
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
    inline ImportGenerator(FILE* fp) : _fp(fp) {}
private:
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
            return;
        }

        fprintf(_fp, "%sstruct %s", Indent::get(), node.name().text());
        if(base) {
            fprintf(_fp, " : public %s", getTypeSpecName(ref(base), GenMode::Normal).c_str());
        }

        fprintf(_fp, " {\n");

        visitChildrenIndent(node);

        // member-list
        GeneratorContext(GeneratorContext::TargetMode::Header, GeneratorContext::IndentMode::NoBrace).run(_config, _fp, "", node.block());

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

    inline void writeScopeMemberList(const Ast::Scope& scope) {
        for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
            INDENT;
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(_fp, "%sconst %s %s;\n", Indent::get(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
        }
        const Ast::Scope* posParam = scope.posParam();
        if(posParam) {
            for(Ast::Scope::List::const_iterator it = ref(posParam).list().begin(); it != ref(posParam).list().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fp, "%sconst %s %s;\n", Indent::get(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
            }
        }
    }

    static inline void writeScopeParamList(FILE* fp, const Ast::Scope& scope, const std::string& prefix) {
        std::string sep = "";
        for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(fp, "%sconst %s& %s%s", sep.c_str(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GenMode::Normal).c_str(), prefix.c_str(), vdef.name().text());
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

        // run-function-type
        fprintf(_fp, "%spublic:\n", Indent::get());
        if(isTest) {
            fprintf(_fp, "%s    const _Out& test();\n", Indent::get());
        } else if((isDecl) && (node.defType() != Ast::DefinitionType::Native)) {
            fprintf(_fp, "%s    virtual const _Out& _run(const _In& _in) = 0;\n", Indent::get());
        } else {
            fprintf(_fp, "%s    const _Out& run(", Indent::get());
            writeScopeParamList(_fp, node.sig().inScope(), "p");
            fprintf(_fp, ");\n");
            fprintf(_fp, "%s    virtual const _Out& _run(const _In& _in) {\n", Indent::get());
            fprintf(_fp, "%s        return run(", Indent::get());
            writeScopeInCallList(node.sig().inScope());
            fprintf(_fp, ");\n");
            fprintf(_fp, "%s    }\n", Indent::get());
        }

        if(!isTest) {
            // param-instance
            fprintf(_fp, "%s    Pointer<_Out> _out;\n", Indent::get());
            fprintf(_fp, "%s    inline const _Out& out(const _Out& val) {_out = Creator<_Out, _Out>::get(val); return _out.get();}\n", Indent::get());
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
        // generate out parameters
        for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
            INDENT;
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(_fp, "%s%s %s;\n", Indent::get(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
        }

        // generate out setter
        fprintf(_fp, "%s    inline %s(", Indent::get(), node.name().text());
        std::string sep = "";
        for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
            const Ast::VariableDefn& vdef = ref(*it);
            fprintf(_fp, "%sconst %s& p%s", sep.c_str(), getTypeSpecName(vdef.qualifiedTypeSpec().typeSpec(), GenMode::Normal).c_str(), vdef.name().text());
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
        fprintf(_fp, "%s    static void addHandler(%s %s, Handler* h);\n", Indent::get(), getQualifiedTypeSpecName(node.in().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.in().name().text());
        fprintf(_fp, "%s    template<typename T>static inline void addHandlerT(%s %s, T h) {return addHandler(%s, new T(h));}\n", Indent::get(), getQualifiedTypeSpecName(node.in().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.in().name().text(), node.in().name().text());
        fprintf(_fp, "%s};\n", Indent::get());
        fprintf(_fp, "\n");
        return;
    }

    void visit(const Ast::Namespace& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::Root& node) {
        printf("root\n");
        visitChildrenIndent(node);
    }

public:
    inline TypeDeclarationGenerator(const Ast::Config& config, FILE* fp) : _config(config), _fp(fp) {}

private:
    const Ast::Config& _config;
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

    void visit(const Ast::RoutineDecl& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::RoutineDefn& node) {
        visitChildrenIndent(node);
        TypeDeclarationGenerator::visitRoutine(_fp, node, false);
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fp, "", node.block());
        fprintf(_fp, "\n");
    }

    void visit(const Ast::FunctionRetn& node) {
        visitChildrenIndent(node);
    }

    void visit(const Ast::FunctionDecl& node) {
        visitChildrenIndent(node);
    }

    inline void visitFunction(const Ast::FunctionDefn& node, const bool& isTest) {
        if(isTest) {
            fprintf(_fp, "const %s::_Out& %s::test() ", getTypeSpecName(node, GenMode::Normal).c_str(), getTypeSpecName(node, GenMode::Normal).c_str());
        } else {
            fprintf(_fp, "const %s::_Out& %s::run(", getTypeSpecName(node, GenMode::Normal).c_str(), getTypeSpecName(node, GenMode::Normal).c_str());
            TypeDeclarationGenerator::writeScopeParamList(_fp, node.sig().inScope(), "");
            fprintf(_fp, ") ");
        }
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fp, "", node.block());
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
            fprintf(_fp, "void %s::addHandler(%s %s, Handler* h) {\n", getTypeSpecName(node, GenMode::Normal).c_str(), getQualifiedTypeSpecName(node.in().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.in().name().text());
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
    inline TypeDefinitionGenerator(const Ast::Config& config, FILE* fp) : _config(config), _fp(fp) {}
private:
    const Ast::Config& _config;
    FILE* _fp;
};

struct StatementGenerator : public Ast::Statement::Visitor {
private:
    virtual void visit(const Ast::ImportStatement& node) {
        if(_ctx._targetMode == GeneratorContext::TargetMode::Import) {
            if(node.headerType() == Ast::HeaderType::Import) {
                fprintf(_fp, "import ");
            } else {
                fprintf(_fp, "include ");
            }

            std::string sep = "";
            for(Ast::ImportStatement::Part::const_iterator it = node.part().begin(); it != node.part().end(); ++it) {
                const Ast::Token& name = *it;
                fprintf(_fp, "%s%s", sep.c_str(), name.text());
                sep = "::";
            }

            if(node.defType() == Ast::DefinitionType::Native) {
                fprintf(_fp, " native");
            }

            fprintf(_fp, ";\n");
            return;
        }

        if(((_ctx._targetMode == GeneratorContext::TargetMode::Header) && (node.accessType() == Ast::AccessType::Public)) ||
           ((_ctx._targetMode == GeneratorContext::TargetMode::Source) && (node.accessType() != Ast::AccessType::Public))) {
            std::string qt = (node.headerType() == Ast::HeaderType::Import)?"<>":"\"\"";
            fprintf(_fp, "#include %c", qt.at(0));
            std::string sep = "";
            for(Ast::ImportStatement::Part::const_iterator it = node.part().begin(); it != node.part().end(); ++it) {
                const Ast::Token& name = *it;
                fprintf(_fp, "%s%s", sep.c_str(), name.text());
                sep = "/";
            }
            fprintf(_fp, ".hpp%c\n", qt.at(1));
        } else {
        }
    }

    virtual void visit(const Ast::NamespaceStatement& node) {
        if(_ctx._targetMode == GeneratorContext::TargetMode::Import) {
            std::string fqn;
            std::string sep;
            for(Ast::NamespaceStatement::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Namespace& ns = ref(*it);
                fqn += sep;
                fqn += ns.name().string();
            }
            if(fqn.size() > 0) {
                fprintf(_fp, "namespace %s;\n\n", fqn.c_str());
            }
            return;
        }

        if(_ctx._targetMode == GeneratorContext::TargetMode::Source) {
            assert(_basename.size() > 0);
            fprintf(_fp, "#include \"%s.hpp\"\n", _basename.c_str());
        }

        if((_ctx._targetMode == GeneratorContext::TargetMode::Header) || (_ctx._targetMode == GeneratorContext::TargetMode::Source)) {
            for(Ast::NamespaceStatement::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Namespace& ns = ref(*it);
                fprintf(_fp, "namespace %s {", ns.name().text());
            }

            if(node.list().size() > 0) {
                fprintf(_fp, "\n");
            }
        }
    }

    virtual void visit(const Ast::LeaveNamespaceStatement& node) {
        if(_ctx._targetMode == GeneratorContext::TargetMode::Import) {
            return;
        }

        if((_ctx._targetMode == GeneratorContext::TargetMode::Header) || (_ctx._targetMode == GeneratorContext::TargetMode::Source)) {
            for(Ast::NamespaceStatement::List::const_reverse_iterator it = node.statement().list().rbegin(); it != node.statement().list().rend(); ++it) {
                const Ast::Namespace& ns = ref(*it);
                fprintf(_fp, "} /*** %s */ ", ns.name().text());
            }
            if(node.statement().list().size() > 0) {
                fprintf(_fp, "\n");
            }
        }
    }

    bool isHeader(const Ast::TypeSpec& typeSpec) const {
        if((typeSpec.accessType() == Ast::AccessType::Public) || (typeSpec.accessType() == Ast::AccessType::Protected)) {
            return true;
        }
        if(typeSpec.accessType() == Ast::AccessType::Parent) {
            const Ast::ChildTypeSpec* childTypeSpec = dynamic_cast<const Ast::ChildTypeSpec*>(ptr(typeSpec));
            if(childTypeSpec) {
                return isHeader(ref(childTypeSpec).parent());
            }
        }
        return false;
    }

    virtual void visit(const Ast::UserDefinedTypeSpecStatement& node) {
        if(_ctx._targetMode == GeneratorContext::TargetMode::Import) {
            ImportGenerator(_fp).visitNode(node.typeSpec());
        } else if(_ctx._targetMode == GeneratorContext::TargetMode::Header) {
            if(isHeader(node.typeSpec())) {
                TypeDeclarationGenerator(_config, _fp).visitNode(node.typeSpec());
            }
            _ctx._list.push_back(ptr(node));
        } else if(_ctx._targetMode == GeneratorContext::TargetMode::Source) {
            if(!isHeader(node.typeSpec())) {
                TypeDeclarationGenerator(_config, _fp).visitNode(node.typeSpec());
            }
            _ctx._list.push_back(ptr(node));
        } else if(_ctx._targetMode == GeneratorContext::TargetMode::Body) {
            TypeDefinitionGenerator(_config, _fp).visitNode(node.typeSpec());
        }
    }

    virtual void visit(const Ast::StructMemberStatement& node) {
        if(_ctx._targetMode == GeneratorContext::TargetMode::Import) {
            fprintf(_fp, "%s%s %s = ", Indent::get(), getQualifiedTypeSpecName(node.defn().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.defn().name().text());
            ExprGenerator(_fp, GenMode::Normal).visitNode(node.defn().initExpr());
            fprintf(_fp, ";\n");
        } else if(_ctx._targetMode == GeneratorContext::TargetMode::Header) {
            fprintf(_fp, "%s%s %s; /**/ ", Indent::get(), getQualifiedTypeSpecName(node.defn().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.defn().name().text());
            fprintf(_fp, "template <typename T> inline T& _%s(const %s& val) {%s = val; return ref(static_cast<T*>(this));}\n",
                    node.defn().name().text(), getQualifiedTypeSpecName(node.defn().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.defn().name().text());
        } else if(_ctx._targetMode == GeneratorContext::TargetMode::Source) {
        }
    }

    virtual void visit(const Ast::StructInitStatement& node) {
        if(_ctx._targetMode == GeneratorContext::TargetMode::Header) {
            // internal-impl
            if(node.defn().accessType() == Ast::AccessType::Protected) {
                fprintf(_fp, "%sstruct Impl;\n", Indent::get());
                fprintf(_fp, "%sImpl* _impl;\n", Indent::get());
                fprintf(_fp, "%sinline void impl(Impl& val) {_impl = ptr(val);}\n", Indent::get());
            }

            // default-ctor
            fprintf(_fp, "%sexplicit inline %s()", Indent::get(), node.defn().name().text());
            std::string sep = " : ";
            if(node.defn().accessType() == Ast::AccessType::Protected) {
                fprintf(_fp, "%s_impl(0)", sep.c_str());
                sep = ", ";
            }
            for(Ast::Scope::List::const_iterator it = node.defn().list().begin(); it != node.defn().list().end(); ++it) {
                const Ast::VariableDefn& vdef = ref(*it);
                fprintf(_fp, "%s%s(", sep.c_str(), vdef.name().text());
                ExprGenerator(_fp, GenMode::Normal).visitNode(vdef.initExpr());
                fprintf(_fp, ")");
                sep = ", ";
            }
            fprintf(_fp, " {}\n");
        }
    }

    virtual void visit(const Ast::AutoStatement& node) {
        fprintf(_fp, "%s%s %s = ", Indent::get(), getQualifiedTypeSpecName(node.defn().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.defn().name().text());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.defn().initExpr());
        fprintf(_fp, ";\n");
    }

    virtual void visit(const Ast::ExprStatement& node) {
        fprintf(_fp, "%s", Indent::get());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.expr());
        fprintf(_fp, ";\n");
    }

    virtual void visit(const Ast::PrintStatement& node) {
        fprintf(_fp, "%sLog::get() << ", Indent::get());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.expr());
        fprintf(_fp, " << Log::Out();\n");
    }

    virtual void visit(const Ast::IfStatement& node) {
        fprintf(_fp, "%sif(", Indent::get());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.expr());
        fprintf(_fp, ") ");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fp, "", node.tblock());
    }

    virtual void visit(const Ast::IfElseStatement& node) {
        fprintf(_fp, "%sif(", Indent::get());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.expr());
        fprintf(_fp, ")");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fp, "", node.tblock());
        fprintf(_fp, "%selse", Indent::get());
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fp, "", node.fblock());
    }

    virtual void visit(const Ast::WhileStatement& node) {
        fprintf(_fp, "%swhile(", Indent::get());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.expr());
        fprintf(_fp, ")");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fp, "", node.block());
    }

    virtual void visit(const Ast::DoWhileStatement& node) {
        fprintf(_fp, "%sdo", Indent::get());
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fp, "", node.block());
        fprintf(_fp, "%swhile(", Indent::get());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.expr());
        fprintf(_fp, ");\n");
    }

    virtual void visit(const Ast::ForExprStatement& node) {
        fprintf(_fp, "%sfor(", Indent::get());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.init());
        fprintf(_fp, "; ");
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.expr());
        fprintf(_fp, "; ");
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.incr());
        fprintf(_fp, ")");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fp, "", node.block());
    }

    virtual void visit(const Ast::ForInitStatement& node) {
        fprintf(_fp, "%sfor(%s %s = ", Indent::get(), getQualifiedTypeSpecName(node.init().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.init().name().text());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.init().initExpr());
        fprintf(_fp, "; ");
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.expr());
        fprintf(_fp, "; ");
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.incr());
        fprintf(_fp, ")");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fp, "", node.block());
    }

    virtual void visit(const Ast::ForeachListStatement& node) {
        const std::string etype = getQualifiedTypeSpecName(node.expr().qTypeSpec(), GenMode::Normal);
        const std::string vtype = getQualifiedTypeSpecName(node.valDef().qualifiedTypeSpec(), GenMode::Normal);

        std::string constit = "";
        if(node.expr().qTypeSpec().isConst()) {
            constit = "const_";
        }

        fprintf(_fp, "%s{\n", Indent::get());
        fprintf(_fp, "%s%s& _list = ", Indent::get(), etype.c_str());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.expr());
        fprintf(_fp, ";\n");
        fprintf(_fp, "%sfor(%s::%siterator _it = _list.begin(); _it != _list.end(); ++_it) {\n", Indent::get(), getTypeSpecName(node.expr().qTypeSpec().typeSpec(), GenMode::Normal).c_str(), constit.c_str());
        fprintf(_fp, "%s%s& %s = _it->second;\n", Indent::get(), getQualifiedTypeSpecName(node.valDef().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.valDef().name().text());

        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::IndentedBrace).run(_config, _fp, "", node.block());

        fprintf(_fp, "%s}\n", Indent::get());
        fprintf(_fp, "%s}\n", Indent::get());
    }

    virtual void visit(const Ast::ForeachDictStatement& node) {
        std::string constit = "";
        if(node.expr().qTypeSpec().isConst())
            constit = "const_";
        fprintf(_fp, "%s{\n", Indent::get());
        fprintf(_fp, "%s%s& _list = ", Indent::get(), getQualifiedTypeSpecName(node.expr().qTypeSpec(), GenMode::Normal).c_str());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.expr());
        fprintf(_fp, ";\n");
        fprintf(_fp, "%sfor(%s::%siterator _it = _list.begin(); _it != _list.end(); ++_it) {\n", Indent::get(), getTypeSpecName(node.expr().qTypeSpec().typeSpec(), GenMode::Normal).c_str(), constit.c_str());
        fprintf(_fp, "%sconst %s& %s = _it->first;\n", Indent::get(), getQualifiedTypeSpecName(node.keyDef().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.keyDef().name().text());
        fprintf(_fp, "%s%s& %s = _it->second;\n", Indent::get(), getQualifiedTypeSpecName(node.valDef().qualifiedTypeSpec(), GenMode::Normal).c_str(), node.valDef().name().text());

        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::IndentedBrace).run(_config, _fp, "", node.block());

        fprintf(_fp, "%s}\n", Indent::get());
        fprintf(_fp, "%s}\n", Indent::get());
    }

    virtual void visit(const Ast::CaseExprStatement& node) {
        fprintf(_fp, "%scase (", Indent::get());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.expr());
        fprintf(_fp, ") :");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fp, "", node.block());
    }

    virtual void visit(const Ast::CaseDefaultStatement& node) {
        fprintf(_fp, "%sdefault :", Indent::get());
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fp, "", node.block());
    }

    virtual void visit(const Ast::SwitchValueStatement& node) {
        fprintf(_fp, "%sswitch(", Indent::get());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.expr());
        fprintf(_fp, ")");
        GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fp, "", node.block());
    }

    virtual void visit(const Ast::SwitchExprStatement& node) {
        std::string ifstr = "if";
        for(Ast::CompoundStatement::List::const_iterator sit = node.block().list().begin(); sit != node.block().list().end(); ++sit) {
            const Ast::Statement* s = *sit;
            const Ast::CaseExprStatement* ce = dynamic_cast<const Ast::CaseExprStatement*>(s);
            const Ast::CaseDefaultStatement* cd = dynamic_cast<const Ast::CaseDefaultStatement*>(s);
            if(ce) {
                fprintf(_fp, "%s%s(", Indent::get(), ifstr.c_str());
                ExprGenerator(_fp, GenMode::Normal).visitNode(ref(ce).expr());
                fprintf(_fp, ")\n");
                visitNode(ref(ce).block());
                ifstr = "else if";
            } else if(cd) {
                fprintf(_fp, "%selse\n", Indent::get());
                visitNode(ref(cd).block());
                break;
            } else {
                throw Exception("Internal error: not a case statement inside switch\n");
            }
        }
    }

    virtual void visit(const Ast::BreakStatement& node) {
        fprintf(_fp, "%sbreak;\n", Indent::get());
    }

    virtual void visit(const Ast::ContinueStatement& node) {
        fprintf(_fp, "%scontinue;\n", Indent::get());
    }

    virtual void visit(const Ast::AddEventHandlerStatement& node) {
        std::string ename = getTypeSpecName(node.event(), GenMode::Normal);
        fprintf(_fp, "%s%s::addHandlerT(", Indent::get(), ename.c_str());
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.source());
        fprintf(_fp, ", ");
        ExprGenerator(_fp, GenMode::Normal).visitNode(node.functor());
        fprintf(_fp, ");\n");
    }

    virtual void visit(const Ast::RoutineReturnStatement& node) {
        fprintf(_fp, "%sreturn", Indent::get());
        if(node.exprList().list().size() > 0) {
            fprintf(_fp, " (");
            ExprGenerator(_fp, GenMode::Normal, ", ").visitList(node.exprList());
            fprintf(_fp, ")");
        }
        fprintf(_fp, ";\n");
    }

    virtual void visit(const Ast::FunctionReturnStatement& node) {
        fprintf(_fp, "%sreturn out(_Out(", Indent::get());
        ExprGenerator(_fp, GenMode::Normal, ", ").visitList(node.exprList());
        fprintf(_fp, "));\n");
    }

    virtual void visit(const Ast::CompoundStatement& node) {
        if(_ctx._indentMode == GeneratorContext::IndentMode::IndentedBrace) {
            fprintf(_fp, "%s", Indent::get());
        }

        if((_ctx._indentMode == GeneratorContext::IndentMode::IndentedBrace) || (_ctx._indentMode == GeneratorContext::IndentMode::WithBrace)) {
            fprintf(_fp, "{\n");
        }

        {
            INDENT;
            for(Ast::CompoundStatement::List::const_iterator sit = node.list().begin(); sit != node.list().end(); ++sit) {
                const Ast::Statement& s = ref(*sit);
                ref(this).visitNode(s);
            }
        }

        if((_ctx._indentMode == GeneratorContext::IndentMode::IndentedBrace) || (_ctx._indentMode == GeneratorContext::IndentMode::WithBrace)) {
            fprintf(_fp, "%s}\n", Indent::get());
        }
    }

public:
    inline StatementGenerator(const Ast::Config& config, GeneratorContext& ctx, FILE* fp, const std::string& basename) : _config(config), _ctx(ctx), _fp(fp), _basename(basename) {}
private:
    const Ast::Config& _config;
    GeneratorContext& _ctx;
    FILE* _fp;
    const std::string& _basename;
};

void GeneratorContext::run(const Ast::Config& config, FILE* fp, const std::string& basename, const Ast::Statement& block) {
    StatementGenerator gen(config, ref(this), fp, basename);
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

    fprintf(_fpHdr, "#pragma once\n\n");
    for(Ast::Config::PathList::const_iterator it = _config.includeFileList().begin(); it != _config.includeFileList().end(); ++it) {
        const std::string& filename = *it;
        fprintf(_fpSrc, "#include \"%s\"\n", filename.c_str());
    }

    for(Ast::Unit::StatementList::const_iterator sit = _unit.statementList().begin(); sit != _unit.statementList().end(); ++sit) {
        const Ast::Statement& s = ref(*sit);
        GeneratorContext(GeneratorContext::TargetMode::Import, GeneratorContext::IndentMode::WithBrace).run(_config, _fpImp, basename, s);
    }

    for(Ast::Unit::StatementList::const_iterator sit = _unit.statementList().begin(); sit != _unit.statementList().end(); ++sit) {
        const Ast::Statement& s = ref(*sit);
        GeneratorContext(GeneratorContext::TargetMode::Header, GeneratorContext::IndentMode::WithBrace).run(_config, _fpHdr, basename, s);
    }

    for(Ast::Unit::StatementList::const_iterator sit = _unit.statementList().begin(); sit != _unit.statementList().end(); ++sit) {
        const Ast::Statement& s = ref(*sit);
        GeneratorContext(GeneratorContext::TargetMode::Source, GeneratorContext::IndentMode::WithBrace).run(_config, _fpSrc, basename, s);
    }

    for(Ast::Unit::StatementList::const_iterator sit = _unit.statementList().begin(); sit != _unit.statementList().end(); ++sit) {
        const Ast::Statement& s = ref(*sit);
        GeneratorContext(GeneratorContext::TargetMode::Body, GeneratorContext::IndentMode::WithBrace).run(_config, _fpSrc, basename, s);
    }

//    StatementGeneratorContext ctx3(StatementGeneratorContext::TargetMode::Body, StatementGeneratorContext::IndentMode::WithBrace);
//    StatementGenerator gen3(_config, ctx3, _fpSrc, basename);
//    for(StatementGeneratorContext::List::const_iterator it = ctx1._list.begin(); it != ctx1._list.end(); ++it) {
//        const Ast::Statement& s = ref(*it);
//        gen3.visitNode(s);
//    }
}

//////////////////////////////////////////////
Generator::Generator(const Ast::Project& project, const Ast::Config& config, const Ast::Unit& unit) : _impl(0) {_impl = new Impl(project, config, unit);}
Generator::~Generator() {delete _impl;}
void Generator::run() {return ref(_impl).run();}
