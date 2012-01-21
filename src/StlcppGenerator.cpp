#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "StlcppGenerator.hpp"
#include "typename.hpp"

struct StlcppNameGenerator : public TypespecNameGenerator {
    virtual void getTypeName(const Ast::TypeSpec& typeSpec, std::string& name);
public:
    inline StlcppNameGenerator(const std::string& sep = "::") : TypespecNameGenerator(sep) {}
};

void StlcppNameGenerator::getTypeName(const Ast::TypeSpec& typeSpec, std::string& name) {
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(typeSpec));
    if(templateDefn) {
        if(z::ref(templateDefn).name().string() == "ptr") {
            const Ast::QualifiedTypeSpec& qTypeSpec = z::ref(templateDefn).list().front();
            name += tn(qTypeSpec.typeSpec());
            name += "*";
            return;
        }

        if(z::ref(templateDefn).name().string() == "pointer") {
            name += "z::pointer";
        } else if(z::ref(templateDefn).name().string() == "list") {
            name += "z::list";
        } else if(z::ref(templateDefn).name().string() == "dict") {
            name += "z::dict";
        } else if(z::ref(templateDefn).name().string() == "future") {
            name += "z::FutureT";
        } else if(z::ref(templateDefn).name().string() == "functor") {
            name += "z::FunctorT";
        } else {
            name += z::ref(templateDefn).name().string();
        }

        name += "<";
        std::string sep;
        for(Ast::TemplateTypePartList::List::const_iterator it = z::ref(templateDefn).list().begin(); it != z::ref(templateDefn).list().end(); ++it) {
            const Ast::QualifiedTypeSpec& qTypeSpec = it->get();
            name += sep;
            if(z::ref(templateDefn).name().string() == "pointer") {
                name += tn(qTypeSpec.typeSpec());
            } else {
                name += qtn(qTypeSpec);
            }
            sep = ", ";
        }
        name += "> ";
        return;
    }

    if(typeSpec.name().string() == "string") {
        name += "std::string";
        return;
    }

    if(typeSpec.name().string() == "type") {
        name += "z::type";
        return;
    }

    if(typeSpec.name().string() == "ArgList") {
        name += "z::ArgList";
        return;
    }
    name += typeSpec.name().string();

    const Ast::EnumDefn* enumDefn = dynamic_cast<const Ast::EnumDefn*>(z::ptr(typeSpec));
    if(enumDefn != 0) {
        name += _sep;
        name += "T";
    }
}

namespace {
    struct FileSet {
        inline FileSet(FILE* fpHdr, FILE* fpSrc) : _fpHdr(fpHdr), _fpSrc(fpSrc) {}
        FILE* _fpHdr;
        FILE* _fpSrc;
    };

    struct GeneratorContext {
        struct TargetMode {
            enum T {
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
        inline ExprGenerator(FILE* fp, const std::string& sep2 = "", const std::string& sep1 = "") : _fp(fp), _sep2(sep2), _sep1(sep1), _sep0(sep1) {}
    private:
        inline void visitTernary(const Ast::TernaryOpExpr& node) {
            fprintf(_fp, "(");
            visitNode(node.lhs());
            fprintf(_fp, "%s", node.op1().text());
            visitNode(node.rhs1());
            fprintf(_fp, "%s", node.op2().text());
            visitNode(node.rhs2());
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::ConditionalExpr& node) {
            return visitTernary(node);
        }

        virtual void visitBinary(const Ast::BinaryExpr& node) {
            visitNode(node.lhs());
            fprintf(_fp, "%s", node.op().text());
            visitNode(node.rhs());
        }

        virtual void visit(const Ast::BooleanAndExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BooleanOrExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BooleanEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BooleanNotEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BooleanLessThanExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BooleanGreaterThanExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BooleanLessThanOrEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BooleanGreaterThanOrEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BooleanHasExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryAssignEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryPlusEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryMinusEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryTimesEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryDivideEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryModEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryBitwiseAndEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryBitwiseOrEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryBitwiseXorEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryShiftLeftEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryShiftRightEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryPlusExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryMinusExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryTimesExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryDivideExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryModExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryBitwiseAndExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryBitwiseOrExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryBitwiseXorExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryShiftLeftExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const Ast::BinaryShiftRightExpr& node) {
            return visitBinary(node);
        }

        inline void visitPostfix(const Ast::PostfixExpr& node) {
            visitNode(node.lhs());
            fprintf(_fp, "%s", node.op().text());
        }

        virtual void visit(const Ast::PostfixIncExpr& node) {
            return visitPostfix(node);
        }

        virtual void visit(const Ast::PostfixDecExpr& node) {
            return visitPostfix(node);
        }

        inline void visitPrefix(const Ast::PrefixExpr& node) {
            fprintf(_fp, "%s", node.op().text());
            visitNode(node.rhs());
        }

        virtual void visit(const Ast::PrefixNotExpr& node) {
            return visitPrefix(node);
        }

        virtual void visit(const Ast::PrefixPlusExpr& node) {
            return visitPrefix(node);
        }

        virtual void visit(const Ast::PrefixMinusExpr& node) {
            return visitPrefix(node);
        }

        virtual void visit(const Ast::PrefixIncExpr& node) {
            return visitPrefix(node);
        }

        virtual void visit(const Ast::PrefixDecExpr& node) {
            return visitPrefix(node);
        }

        virtual void visit(const Ast::PrefixBitwiseNotExpr& node) {
            return visitPrefix(node);
        }

        virtual void visit(const Ast::SetIndexExpr& node) {
            visitNode(node.lhs().expr());
            fprintf(_fp, ".set(");
            visitNode(node.lhs().index());
            fprintf(_fp, ", ");
            visitNode(node.rhs());
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::ListExpr& node) {
            fprintf(_fp, "z::list<%s>::creator()", StlcppNameGenerator().qtn(node.list().valueType()).c_str());
            for(Ast::ListList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const Ast::ListItem& item = it->get();
                fprintf(_fp, ".add(");
                visitNode(item.valueExpr());
                fprintf(_fp, ")");
            }
            fprintf(_fp, ".get()");
        }

        virtual void visit(const Ast::DictExpr& node) {
            fprintf(_fp, "z::dict<%s, %s>::creator()", StlcppNameGenerator().qtn(node.list().keyType()).c_str(), StlcppNameGenerator().qtn(node.list().valueType()).c_str());
            for(Ast::DictList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const Ast::DictItem& item = it->get();
                fprintf(_fp, ".add(");
                visitNode(item.keyExpr());
                fprintf(_fp, ", ");
                visitNode(item.valueExpr());
                fprintf(_fp, ")");
            }
            fprintf(_fp, ".get()");
        }

        virtual void visit(const Ast::FormatExpr& node) {
            fprintf(_fp, "String::Formatter(");
            visitNode(node.stringExpr());
            fprintf(_fp, ")");
            for(Ast::DictList::List::const_iterator it = node.dictExpr().list().list().begin(); it != node.dictExpr().list().list().end(); ++it) {
                const Ast::DictItem& item = it->get();
                fprintf(_fp, ".add(");
                visitNode(item.keyExpr());
                fprintf(_fp, ", ");
                visitNode(item.valueExpr());
                fprintf(_fp, ")");
            }
            fprintf(_fp, ".get()");
        }

        virtual void visit(const Ast::RoutineCallExpr& node) {
            const std::string name = StlcppNameGenerator().tn(node.routine());
            if((name == "assert") || (name == "unused") || (name == "verify")) {
                std::string sep;
                for(Ast::ExprList::List::const_iterator it = node.exprList().list().begin(); it != node.exprList().list().end(); ++it) {
                    const Ast::Expr& expr = it->get();
                    fprintf(_fp, "%s", sep.c_str());
                    if(name == "verify")
                        fprintf(_fp, "if(!");
                    fprintf(_fp, "%s(", name.c_str());
                    ExprGenerator(_fp).visitNode(expr);
                    fprintf(_fp, ")");
                    if(name == "verify")
                        fprintf(_fp, ")return out(_Out(false))");
                    sep = ";";
                }
                return;
            }

            fprintf(_fp, "%s(", name.c_str());
            ExprGenerator(_fp, ", ").visitList(node.exprList());
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::FunctorCallExpr& node) {
            ExprGenerator(_fp).visitNode(node.expr());
            fprintf(_fp, ".run(");
            ExprGenerator(_fp, ", ").visitList(node.exprList());
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::RunExpr& node) {
            fprintf(_fp, "z::CallContext::get().add(");
            ExprGenerator(_fp).visitNode(node.callExpr().expr());
            fprintf(_fp, ", ");
            fprintf(_fp, "%s::_In(", StlcppNameGenerator().tn(node.callExpr().expr().qTypeSpec().typeSpec()).c_str());
            ExprGenerator(_fp, ", ").visitList(node.callExpr().exprList());
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
                at = z::ref(ts).name().string();
            }

            visitNode(node.expr());
            fprintf(_fp, ".%s(", at.c_str());
            visitNode(node.index());
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::SpliceExpr& node) {
            fprintf(_fp, "splice(");
            visitNode(node.expr());
            fprintf(_fp, ", ");
            visitNode(node.from());
            fprintf(_fp, ", ");
            visitNode(node.to());
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::TypeofTypeExpr& node) {
            fprintf(_fp, "z::type(\"%s\")", StlcppNameGenerator().qtn(node.typeSpec()).c_str());
        }

        virtual void visit(const Ast::TypeofExprExpr& node) {
            const Ast::TypeSpec* typeSpec = z::ptr(node.expr().qTypeSpec().typeSpec());
            const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(typeSpec);
            if(td) {
                visitNode(node.expr());
                fprintf(_fp, ".tname()");
            } else {
                fprintf(_fp, "z::type(\"%s\")", StlcppNameGenerator().qtn(node.expr().qTypeSpec()).c_str());
            }
        }

        virtual void visit(const Ast::StaticTypecastExpr& node) {
            fprintf(_fp, "static_cast<%s>(", StlcppNameGenerator().qtn(node.qTypeSpec()).c_str());
            ExprGenerator(_fp).visitNode(node.expr());
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::DynamicTypecastExpr& node) {
            ExprGenerator(_fp).visitNode(node.expr());
            fprintf(_fp, ".getT<%s>()", StlcppNameGenerator().tn(node.qTypeSpec().typeSpec()).c_str());
        }

        virtual void visit(const Ast::PointerInstanceExpr& node) {
            const Ast::Expr& expr = node.exprList().at(0);
            const std::string dname = StlcppNameGenerator().tn(expr.qTypeSpec().typeSpec());

            const std::string bname = StlcppNameGenerator().tn(node.templateDefn().at(0).typeSpec());
            fprintf(_fp, "z::PointerCreator<%s, %s>::get(", bname.c_str(), dname.c_str());
            fprintf(_fp, "z::type(\"%s\"), ", dname.c_str());
            ExprGenerator(_fp).visitNode(expr);
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::ValueInstanceExpr& node) {
            const Ast::Expr& expr = node.exprList().at(0);
            ExprGenerator(_fp).visitNode(expr);
            fprintf(_fp, ".getT<%s>()", StlcppNameGenerator().tn(node.qTypeSpec().typeSpec()).c_str());
        }

        virtual void visit(const Ast::VariableRefExpr& node) {
            switch(node.refType()) {
                case Ast::RefType::Global:
                    break;
                case Ast::RefType::XRef:
                    fprintf(_fp, "z::ref(this).%s", node.vref().name().text());
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
            fprintf(_fp, "%s", ZenlangNameGenerator().tn(node.typeSpec()).c_str());
            fprintf(_fp, "::%s", node.vref().name().text());
        }

        virtual void visit(const Ast::StructMemberExpr& node) {
            fprintf(_fp, "%s", ZenlangNameGenerator().tn(node.typeSpec()).c_str());
            fprintf(_fp, "::%s", node.vref().name().text());
        }

        virtual void visit(const Ast::StructInstanceExpr& node) {
            fprintf(_fp, "%s()", StlcppNameGenerator().tn(node.structDefn()).c_str());
            for(Ast::StructInitPartList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const Ast::StructInitPart& part = it->get();
                fprintf(_fp, "._%s<%s>(", part.vdef().name().text(), StlcppNameGenerator().tn(node.structDefn()).c_str());
                visitNode(part.expr());
                fprintf(_fp, ")");
            }
        }

        inline void visitFunctionTypeInstance(const Ast::Function& function) {
            std::string fname = StlcppNameGenerator().tn(function);
            fprintf(_fp, "%s(", fname.c_str());
            std::string sep;
            for(Ast::Scope::List::const_iterator it = function.xref().begin(); it != function.xref().end(); ++it) {
                const Ast::VariableDefn& vref = it->get();
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

        virtual void visit(const Ast::ConstantFloatExpr& node) {
            fprintf(_fp, "%f", node.value());
        }

        virtual void visit(const Ast::ConstantDoubleExpr& node) {
            fprintf(_fp, "%f", node.value());
        }

        virtual void visit(const Ast::ConstantBooleanExpr& node) {
            fprintf(_fp, "%s", node.value()?"true":"false");
        }

        virtual void visit(const Ast::ConstantStringExpr& node) {
            fprintf(_fp, "\"%s\"", node.value().c_str());
        }

        virtual void visit(const Ast::ConstantCharExpr& node) {
            fprintf(_fp, "\'%s\'", node.value().c_str());
        }

        virtual void visit(const Ast::ConstantLongExpr& node) {
            fprintf(_fp, "%ld", node.value());
        }

        virtual void visit(const Ast::ConstantIntExpr& node) {
            fprintf(_fp, "%d", node.value());
        }

        virtual void visit(const Ast::ConstantShortExpr& node) {
            fprintf(_fp, "%d", node.value());
        }

        virtual void sep() {
            fprintf(_fp, "%s", _sep0.c_str());
            _sep0 = _sep2;
        }

    private:
        FILE* _fp;
        const std::string _sep2;
        const std::string _sep1;
        std::string _sep0;
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
                throw err("Stlcpp", node.pos(), "Internal error '%s'\n", node.name().text());
            }
        }

        void visit(const Ast::TypedefDefn& node) {
            if(node.defType() != Ast::DefinitionType::Native) {
                fprintf(_fp, "%stypedef %s %s;\n", Indent::get(), StlcppNameGenerator().qtn(node.qTypeSpec()).c_str(), node.name().text());
            }
        }

        void visit(const Ast::TemplateDecl& node) {
            if(node.defType() != Ast::DefinitionType::Native) {
                throw err("Stlcpp", node.pos(), "Internal error: template declaration cannot be generated '%s'\n", node.name().text());
            }
        }

        void visit(const Ast::TemplateDefn& node) {
            throw err("Stlcpp", node.pos(), "Internal error: template definition cannot be generated '%s'\n", node.name().text());
        }

        void visit(const Ast::EnumDefn& node) {
            if(node.defType() != Ast::DefinitionType::Native) {
                fprintf(_fp, "%sstruct %s {\n", Indent::get(), node.name().text());
                fprintf(_fp, "%s  enum T {\n", Indent::get());
                std::string sep = " ";
                for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    INDENT;
                    const Ast::VariableDefn& def = it->get();
                    fprintf(_fp, "%s%s %s", Indent::get(), sep.c_str(), def.name().text());
                    const Ast::ConstantIntExpr* cexpr = dynamic_cast<const Ast::ConstantIntExpr*>(z::ptr(def.initExpr()));
                    if((cexpr == 0) || (z::ref(cexpr).pos().string() != "#")) { /// \todo hack
                        fprintf(_fp, " = ");
                        ExprGenerator(_fp).visitNode(def.initExpr());
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
                fprintf(_fp, " : public %s", StlcppNameGenerator().tn(z::ref(base)).c_str());
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
            visitStructDefn(node, z::ptr(node.base()));
        }

        inline void visitProperty(const Ast::PropertyDecl& node) {
            std::string cnst;
            if(node.qTypeSpec().isConst())
                cnst = "const ";
            fprintf(_fp, "%s%s%s& _%s() const;\n", Indent::get(), cnst.c_str(), StlcppNameGenerator().qtn(node.qTypeSpec()).c_str(), node.name().text());
        }

        void visit(const Ast::PropertyDeclRW& node) {
            visitProperty(node);
            fprintf(_fp, "%svoid _%s(const %s& val);\n", Indent::get(), node.name().text(), StlcppNameGenerator().tn(node.qTypeSpec().typeSpec()).c_str());
        }

        void visit(const Ast::PropertyDeclRO& node) {
            visitProperty(node);
        }

        inline bool isVoid(const Ast::Scope& out) const {
            if(out.isTuple())
                return false;

            const Ast::VariableDefn& vdef = out.list().front();
            if(vdef.qTypeSpec().typeSpec().name().string() == "void")
                return true;

            return false;
        }

        static inline void visitRoutine(FILE* fp, const Ast::Routine& node, const bool& inNS) {
            fprintf(fp, "%s%s ", Indent::get(), StlcppNameGenerator().qtn(node.outType()).c_str());
            if(inNS) {
                fprintf(fp, "%s", node.name().text());
            } else {
                fprintf(fp, "%s", StlcppNameGenerator().tn(node).c_str());
            }
            fprintf(fp, "(");
            std::string sep;
            for(Ast::Scope::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
                const Ast::VariableDefn& vdef = it->get();
                fprintf(fp, "%s%s %s", sep.c_str(), StlcppNameGenerator().qtn(vdef.qTypeSpec()).c_str(), vdef.name().text());
                sep = ", ";
            }
            fprintf(fp, ")");
        }

        inline void writeScopeMemberList(const Ast::Scope& scope) {
            for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& vdef = it->get();
                fprintf(_fp, "%sconst %s %s;\n", Indent::get(), StlcppNameGenerator().tn(vdef.qTypeSpec().typeSpec()).c_str(), vdef.name().text());
            }
            if(scope.hasPosParam()) {
                const Ast::Scope& posParam = scope.posParam();
                for(Ast::Scope::List::const_iterator it = posParam.list().begin(); it != posParam.list().end(); ++it) {
                    INDENT;
                    const Ast::VariableDefn& vdef = it->get();
                    fprintf(_fp, "%sconst %s %s;\n", Indent::get(), StlcppNameGenerator().tn(vdef.qTypeSpec().typeSpec()).c_str(), vdef.name().text());
                }
            }
        }

        static inline void writeScopeParamList(FILE* fp, const Ast::Scope& scope, const std::string& prefix) {
            std::string sep = "";
            for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                const Ast::VariableDefn& vdef = it->get();
                fprintf(fp, "%sconst %s& %s%s", sep.c_str(), StlcppNameGenerator().tn(vdef.qTypeSpec().typeSpec()).c_str(), prefix.c_str(), vdef.name().text());
                sep = ", ";
            }
        }

        inline void writeScopeInCallList(const Ast::Scope& scope) {
            std::string sep = "";
            for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                const Ast::VariableDefn& vdef = it->get();
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
                const Ast::VariableDefn& vdef = it->get();
                fprintf(_fp, "%s%s(p%s)", sep.c_str(), vdef.name().text(), vdef.name().text());
                sep = ", ";
            }

            if(scope.hasPosParam()) {
                const Ast::Scope& posParam = scope.posParam();
                unused(posParam);
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
                const Ast::VariableDefn& vdef = node.sig().out().front();
                out1 = StlcppNameGenerator().qtn(vdef.qTypeSpec());
                out2 = out1;
            }

            if(!isTest) {
                // param-instance
                fprintf(_fp, "%s    z::Pointer<_Out> _out;\n", Indent::get());
                fprintf(_fp, "%s    inline %s out(const _Out& val) {_out = z::PointerCreator<_Out, _Out>::get(val);", Indent::get(), out2.c_str());
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
            } else if((isDecl) && ((node.defType() == Ast::DefinitionType::Final) || (node.defType() == Ast::DefinitionType::Abstract))) {
                fprintf(_fp, "%s    virtual %s _run(const _In& _in) = 0;\n", Indent::get(), out2.c_str());
            } else {
                fprintf(_fp, "%s    %s run(", Indent::get(), out1.c_str());
                writeScopeParamList(_fp, node.sig().inScope(), "p");
                fprintf(_fp, ");\n");
                fprintf(_fp, "%s    %s _run(const _In& _in) {\n", Indent::get(), out2.c_str());
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
                    const Ast::VariableDefn& vdef = it->get();
                    fprintf(_fp, "%s%s %s;\n", Indent::get(), StlcppNameGenerator().tn(vdef.qTypeSpec().typeSpec()).c_str(), vdef.name().text());
                }

                // generate out setter
                fprintf(_fp, "%s    inline %s(", Indent::get(), node.name().text());
                std::string sep = "";
                for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
                    const Ast::VariableDefn& vdef = it->get();
                    fprintf(_fp, "%sconst %s& p%s", sep.c_str(), StlcppNameGenerator().tn(vdef.qTypeSpec().typeSpec()).c_str(), vdef.name().text());
                    sep = ", ";
                }
                fprintf(_fp, ")");
                sep = " : ";
                for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
                    const Ast::VariableDefn& vdef = it->get();
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
            bool isTest = (StlcppNameGenerator().tn(node.base()) == "test");
            if((isTest) && (!_config.test())) {
                return;
            }

            if(isTest) {
                fprintf(_fp, "%sclass %s : public z::test_< %s > {\n", Indent::get(), node.name().text(), node.name().text());
                fprintf(_fp, "%spublic:\n", Indent::get());
                fprintf(_fp, "%s    inline const char* const name() const {return \"%s\";}\n", Indent::get(), StlcppNameGenerator().tn(node).c_str());
            } else if(StlcppNameGenerator().tn(node.base()) == "main") {
                fprintf(_fp, "%sclass %s : public z::main_< %s > {\n", Indent::get(), node.name().text(), node.name().text());
            } else {
                fprintf(_fp, "%sclass %s : public %s {\n", Indent::get(), node.name().text(), StlcppNameGenerator().tn(node.base()).c_str());
            }
            visitFunctionSig(node, false, false, isTest);

            if(StlcppNameGenerator().tn(node.base()) == "test") {
                fprintf(_fp, "%s    static z::TestInstanceT<%s> s_test;\n", Indent::get(), node.name().text());
            } else if(StlcppNameGenerator().tn(node.base()) == "main") {
                fprintf(_fp, "%s    static z::MainInstanceT<%s> s_main;\n", Indent::get(), node.name().text());
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
            fprintf(_fp, "%s    z::FunctorList<Handler> _list;\n", Indent::get());
            fprintf(_fp, "%s    static %s instance;\n", Indent::get(), node.name().text());
            fprintf(_fp, "%s    static inline Handler& add(Handler* h) {return instance._list.add(h);}\n", Indent::get());
            fprintf(_fp, "%s    static void addHandler(%s %s, Handler* h);\n", Indent::get(), StlcppNameGenerator().qtn(node.in().qTypeSpec()).c_str(), node.in().name().text());
            fprintf(_fp, "%s    template<typename T>static inline void addHandlerT(%s %s, T h) {return addHandler(%s, new T(h));}\n", Indent::get(), StlcppNameGenerator().qtn(node.in().qTypeSpec()).c_str(), node.in().name().text(), node.in().name().text());
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
            const std::string tname = StlcppNameGenerator().tn(node);
            std::string out;
            if(node.sig().outScope().isTuple()) {
                out = "const " + tname + "::_Out&";
            } else {
                const Ast::VariableDefn& vdef = node.sig().out().front();
                out = StlcppNameGenerator().qtn(vdef.qTypeSpec());
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
            bool isTest = (StlcppNameGenerator().tn(node.base()) == "test");
            if((isTest) && (!_config.test())) {
                return;
            }

            visitChildrenIndent(node);
            visitFunction(node, isTest);
            std::string fname = StlcppNameGenerator().tn(node);

            if(isTest) {
                fprintf(_fp, "z::TestInstanceT<%s> %s::s_test = z::TestInstanceT<%s>();\n\n", fname.c_str(), fname.c_str(), fname.c_str());
            } else if(StlcppNameGenerator().tn(node.base()) == "main") {
                fprintf(_fp, "z::MainInstanceT<%s> %s::s_main = z::MainInstanceT<%s>();\n\n", fname.c_str(), fname.c_str(), fname.c_str());
            }
        }

        void visit(const Ast::EventDecl& node) {
            visitChildrenIndent(node);
            fprintf(_fp, "%s %s::instance;\n", StlcppNameGenerator().tn(node).c_str(), StlcppNameGenerator().tn(node).c_str());
            if(node.defType() == Ast::DefinitionType::Final) {
                fprintf(_fp, "void %s::addHandler(%s %s, Handler* h) {\n", StlcppNameGenerator().tn(node).c_str(), StlcppNameGenerator().qtn(node.in().qTypeSpec()).c_str(), node.in().name().text());
                fprintf(_fp, "}\n");
                fprintf(_fp, "const %s::Add::_Out& %s::Add::run(", StlcppNameGenerator().tn(node).c_str(), StlcppNameGenerator().tn(node).c_str());
                TypeDeclarationGenerator::writeScopeParamList(_fp, node.addFunction().sig().inScope(), "");
                fprintf(_fp, ") {\n");
                fprintf(_fp, "    assert(false); //addHandler(in.%s, in.handler);\n", node.in().name().text());
                fprintf(_fp, "    return out(_Out());\n");
                fprintf(_fp, "}\n");
            }
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

        inline FILE* fpDecl(const Ast::TypeSpec& node) const {
            if(node.accessType() == Ast::AccessType::Parent) {
                const Ast::ChildTypeSpec* child = dynamic_cast<const Ast::ChildTypeSpec*>(z::ptr(node));
                if(!child) {
                    throw err("Stlcpp", node.pos(), "Internal error: Invalid access type in typespec\n");
                }
                return fpDecl(z::ref(child).parent());
            }
            return (node.accessType() == Ast::AccessType::Private)?_fs._fpSrc:_fs._fpHdr;
        }

        inline FILE* fpDefn() const {
            return _fs._fpSrc;
        }
    private:
        virtual void visit(const Ast::ImportStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                FILE* fp = fpDecl(node.accessType());
                std::string qt = (node.headerType() == Ast::HeaderType::Import)?"<>":"\"\"";
                fprintf(fp, "#include %c", qt.at(0));
                std::string sep = "";
                for(Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const Ast::Token& name = it->get().name();
                    fprintf(fp, "%s%s", sep.c_str(), name.text());
                    sep = "/";
                }
                fprintf(fp, ".hpp%c\n", qt.at(1));
            }
        }

        virtual void visit(const Ast::EnterNamespaceStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                assert(_basename.size() > 0);
                fprintf(_fs._fpSrc, "#include \"%s.hpp\"\n", _basename.c_str());

                for(Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const Ast::Namespace& ns = it->get();
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
                    const Ast::Namespace& ns = it->get();
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
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                TypeDeclarationGenerator(_config, _fs, fpDecl(node.typeSpec())).visitNode(node.typeSpec());
            }

            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDefn) {
                TypeDefinitionGenerator(_config, _fs, _fs._fpSrc).visitNode(node.typeSpec());
            }
        }

        inline bool isPtr(const Ast::TypeSpec& typeSpec) const {
            const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(typeSpec));
            if(templateDefn) {
                if(typeSpec.name().string() == "ptr") {
                    return true;
                }
            }
            return false;
        }

        virtual void visit(const Ast::StructMemberVariableStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                std::string tname = StlcppNameGenerator().qtn(node.defn().qTypeSpec());
                std::string pname = "const " + tname + "&";
                if(isPtr(node.defn().qTypeSpec().typeSpec())) {
                    pname = tname;
                }
                fprintf(fpDecl(node.structDefn()), "%s%s %s; /**/ ", Indent::get(), tname.c_str(), node.defn().name().text());
                fprintf(fpDecl(node.structDefn()), "template <typename T> inline T& _%s(%s val) {%s = val; return z::ref(static_cast<T*>(this));}\n",
                        node.defn().name().text(), pname.c_str(), node.defn().name().text());
            }
        }

        virtual void visit(const Ast::StructInitStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                // default-ctor
                fprintf(fpDecl(node.structDefn()), "%sexplicit inline %s()", Indent::get(), node.structDefn().name().text());
                std::string sep = " : ";
                for(Ast::Scope::List::const_iterator it = node.structDefn().list().begin(); it != node.structDefn().list().end(); ++it) {
                    const Ast::VariableDefn& vdef = it->get();
                    fprintf(fpDecl(node.structDefn()), "%s%s(", sep.c_str(), vdef.name().text());
                    ExprGenerator(fpDecl(node.structDefn())).visitNode(vdef.initExpr());
                    fprintf(fpDecl(node.structDefn()), ")");
                    sep = ", ";
                }
                fprintf(fpDecl(node.structDefn()), " {}\n");
            }
        }

        virtual void visit(const Ast::EmptyStatement& node) {
            unused(node);
            fprintf(fpDefn(), ";\n");
        }

        virtual void visit(const Ast::AutoStatement& node) {
            fprintf(fpDefn(), "%s%s %s = ", Indent::get(), StlcppNameGenerator().qtn(node.defn().qTypeSpec()).c_str(), node.defn().name().text());
            ExprGenerator(fpDefn()).visitNode(node.defn().initExpr());
            fprintf(fpDefn(), ";\n");
        }

        virtual void visit(const Ast::ExprStatement& node) {
            fprintf(fpDefn(), "%s", Indent::get());
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), ";\n");
        }

        virtual void visit(const Ast::PrintStatement& node) {
            fprintf(fpDefn(), "%sz::Log::get() << ", Indent::get());
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), " << z::Log::Out();\n");
        }

        virtual void visit(const Ast::IfStatement& node) {
            fprintf(fpDefn(), "%sif(", Indent::get());
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), ") ");
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.tblock());
        }

        virtual void visit(const Ast::IfElseStatement& node) {
            fprintf(fpDefn(), "%sif(", Indent::get());
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), ")");
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.tblock());
            fprintf(fpDefn(), "%selse", Indent::get());
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.fblock());
        }

        virtual void visit(const Ast::WhileStatement& node) {
            fprintf(fpDefn(), "%swhile(", Indent::get());
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), ")");
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
        }

        virtual void visit(const Ast::DoWhileStatement& node) {
            fprintf(fpDefn(), "%sdo", Indent::get());
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
            fprintf(fpDefn(), "%swhile(", Indent::get());
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), ");\n");
        }

        virtual void visit(const Ast::ForExprStatement& node) {
            fprintf(fpDefn(), "%sfor(", Indent::get());
            ExprGenerator(fpDefn()).visitNode(node.init());
            fprintf(fpDefn(), "; ");
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), "; ");
            ExprGenerator(fpDefn()).visitNode(node.incr());
            fprintf(fpDefn(), ")");
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
        }

        virtual void visit(const Ast::ForInitStatement& node) {
            fprintf(fpDefn(), "%sfor(%s %s = ", Indent::get(), StlcppNameGenerator().qtn(node.init().qTypeSpec()).c_str(), node.init().name().text());
            ExprGenerator(fpDefn()).visitNode(node.init().initExpr());
            fprintf(fpDefn(), "; ");
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), "; ");
            ExprGenerator(fpDefn()).visitNode(node.incr());
            fprintf(fpDefn(), ")");
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
        }

        virtual void visit(const Ast::ForeachStringStatement& node) {
            const std::string etype = StlcppNameGenerator().qtn(node.expr().qTypeSpec());

            std::string constit = "";
            if(node.expr().qTypeSpec().isConst()) {
                constit = "const_";
            }

            fprintf(fpDefn(), "%s{\n", Indent::get());
            fprintf(fpDefn(), "%s  %s _str = ", Indent::get(), etype.c_str());
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), ";\n");
            fprintf(fpDefn(), "%s  for(%s::%siterator _it = _str.begin(); _it != _str.end(); ++_it) {\n", Indent::get(), StlcppNameGenerator().tn(node.expr().qTypeSpec().typeSpec()).c_str(), constit.c_str());
            fprintf(fpDefn(), "%s  %s %s = *_it;\n", Indent::get(), StlcppNameGenerator().qtn(node.valDef().qTypeSpec()).c_str(), node.valDef().name().text());

            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::IndentedBrace).run(_config, _fs, "", node.block());

            fprintf(fpDefn(), "%s}\n", Indent::get());
            fprintf(fpDefn(), "%s}\n", Indent::get());
        }

        virtual void visit(const Ast::ForeachListStatement& node) {
            const std::string etype = StlcppNameGenerator().qtn(node.expr().qTypeSpec());

            std::string constit = "";
            if(node.expr().qTypeSpec().isConst()) {
                constit = "const_";
            }

            fprintf(fpDefn(), "%s{\n", Indent::get());
            fprintf(fpDefn(), "%s  %s _list = ", Indent::get(), etype.c_str());
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), ";\n");
            fprintf(fpDefn(), "%s  for(%s::%siterator _it = _list.begin(); _it != _list.end(); ++_it) {\n", Indent::get(), StlcppNameGenerator().tn(node.expr().qTypeSpec().typeSpec()).c_str(), constit.c_str());
            fprintf(fpDefn(), "%s  %s %s = *_it;\n", Indent::get(), StlcppNameGenerator().qtn(node.valDef().qTypeSpec()).c_str(), node.valDef().name().text());

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
            fprintf(fpDefn(), "%s  %s _list = ", Indent::get(), StlcppNameGenerator().qtn(node.expr().qTypeSpec()).c_str());
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), ";\n");
            fprintf(fpDefn(), "%s  for(%s::%siterator _it = _list.begin(); _it != _list.end(); ++_it) {\n", Indent::get(), StlcppNameGenerator().tn(node.expr().qTypeSpec().typeSpec()).c_str(), constit.c_str());
            fprintf(fpDefn(), "%s  %s %s = _it->first;\n", Indent::get(), StlcppNameGenerator().qtn(node.keyDef().qTypeSpec()).c_str(), node.keyDef().name().text());
            fprintf(fpDefn(), "%s  %s %s = _it->second;\n", Indent::get(), StlcppNameGenerator().qtn(node.valDef().qTypeSpec()).c_str(), node.valDef().name().text());

            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::IndentedBrace).run(_config, _fs, "", node.block());

            fprintf(fpDefn(), "%s}\n", Indent::get());
            fprintf(fpDefn(), "%s}\n", Indent::get());
        }

        virtual void visit(const Ast::CaseExprStatement& node) {
            fprintf(fpDefn(), "%scase (", Indent::get());
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), ") :");
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
        }

        virtual void visit(const Ast::CaseDefaultStatement& node) {
            fprintf(fpDefn(), "%sdefault :", Indent::get());
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
        }

        virtual void visit(const Ast::SwitchValueStatement& node) {
            fprintf(fpDefn(), "%sswitch(", Indent::get());
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fprintf(fpDefn(), ")");
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, "", node.block());
        }

        virtual void visit(const Ast::SwitchExprStatement& node) {
            std::string ifstr = "if";
            for(Ast::CompoundStatement::List::const_iterator it = node.block().list().begin(); it != node.block().list().end(); ++it) {
                const Ast::Statement& s = it->get();
                const Ast::CaseExprStatement* ce = dynamic_cast<const Ast::CaseExprStatement*>(z::ptr(s));
                const Ast::CaseDefaultStatement* cd = dynamic_cast<const Ast::CaseDefaultStatement*>(z::ptr(s));
                if(ce) {
                    fprintf(fpDefn(), "%s%s(", Indent::get(), ifstr.c_str());
                    ExprGenerator(fpDefn()).visitNode(z::ref(ce).expr());
                    fprintf(fpDefn(), ")\n");
                    visitNode(z::ref(ce).block());
                    ifstr = "else if";
                } else if(cd) {
                    fprintf(fpDefn(), "%selse\n", Indent::get());
                    visitNode(z::ref(cd).block());
                    break;
                } else {
                    throw err("Stlcpp", node.pos(), "Internal error: not a case statement inside switch\n");
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
            std::string ename = StlcppNameGenerator().tn(node.event());
            fprintf(fpDefn(), "%s%s::addHandlerT(", Indent::get(), ename.c_str());
            ExprGenerator(fpDefn()).visitNode(node.source());
            fprintf(fpDefn(), ", ");
            ExprGenerator(fpDefn()).visitNode(node.functor());
            fprintf(fpDefn(), ");\n");
        }

        virtual void visit(const Ast::RoutineReturnStatement& node) {
            fprintf(fpDefn(), "%sreturn", Indent::get());
            if(node.exprList().list().size() > 0) {
                fprintf(fpDefn(), " (");
                ExprGenerator(fpDefn(), ", ").visitList(node.exprList());
                fprintf(fpDefn(), ")");
            }
            fprintf(fpDefn(), ";\n");
        }

        virtual void visit(const Ast::FunctionReturnStatement& node) {
            fprintf(fpDefn(), "%sreturn out(_Out(", Indent::get());
            ExprGenerator(fpDefn(), ", ").visitList(node.exprList());
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
                for(Ast::CompoundStatement::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const Ast::Statement& s = it->get();
                    z::ref(this).visitNode(s);
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
        StatementGenerator gen(config, fs, z::ref(this), basename);
        gen.visitNode(block);
    }
}

struct StlcppGenerator::Impl {
    inline Impl(const Ast::Project& project, const Ast::Config& config, const Ast::Module& module) : _project(project), _config(config), _module(module), _fpHdr(0), _fpSrc(0) {}
    inline void run();
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
    const Ast::Module& _module;
private:
    FILE* _fpHdr;
    FILE* _fpSrc;
};

inline void StlcppGenerator::Impl::run() {
    Indent::init();
    std::string basename = getBaseName(_module.filename());
    OutputFile ofHdr(_fpHdr, basename + ".hpp");unused(ofHdr);
    OutputFile ofSrc(_fpSrc, basename + ".cpp");unused(ofSrc);
    FileSet fs(_fpHdr, _fpSrc);

    fprintf(_fpHdr, "#pragma once\n\n");
    for(Ast::Config::PathList::const_iterator it = _config.includeFileList().begin(); it != _config.includeFileList().end(); ++it) {
        const std::string& filename = *it;
        fprintf(_fpSrc, "#include \"%s\"\n", filename.c_str());
    }

    for(Ast::CompoundStatement::List::const_iterator it = _module.globalStatementList().list().begin(); it != _module.globalStatementList().list().end(); ++it) {
        const Ast::Statement& s = it->get();
        GeneratorContext(GeneratorContext::TargetMode::TypeDecl, GeneratorContext::IndentMode::WithBrace).run(_config, fs, basename, s);
    }

    for(Ast::CompoundStatement::List::const_iterator it = _module.globalStatementList().list().begin(); it != _module.globalStatementList().list().end(); ++it) {
        const Ast::Statement& s = it->get();
        GeneratorContext(GeneratorContext::TargetMode::TypeDefn, GeneratorContext::IndentMode::WithBrace).run(_config, fs, basename, s);
    }
}

//////////////////////////////////////////////
StlcppGenerator::StlcppGenerator(const Ast::Project& project, const Ast::Config& config, const Ast::Module& module) : _impl(0) {_impl = new Impl(project, config, module);}
StlcppGenerator::~StlcppGenerator() {delete _impl;}
void StlcppGenerator::run() {return z::ref(_impl).run();}
