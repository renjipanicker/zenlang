#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "Interpreter.hpp"
#include "typename.hpp"
#include "compiler.hpp"
#include "Context.hpp"

namespace {
    class InterpreterContext : public Ast::Context {
    public:
        inline InterpreterContext() {}
    };

    inline std::string getDefinitionType(const Ast::DefinitionType::T& defType) {
        switch(defType) {
            case Ast::DefinitionType::Final:
                return "";
            case Ast::DefinitionType::Native:
                return " native";
            case Ast::DefinitionType::Abstract:
                return " abstract";
        }
        throw z::Exception("Internal error: Unknown Definition Type '%d'\n", defType);
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
        throw z::Exception("Internal error: Unknown Access Type '%d'\n", accessType);
    }

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
            fprintf(_fp, "[");
            visitNode(node.lhs().index());
            fprintf(_fp, "] = ");
            visitNode(node.rhs());
        }

        virtual void visit(const Ast::ListExpr& node) {
            fprintf(_fp, "[");
            if(node.list().list().size() == 0) {
                fprintf(_fp, "%s", getQualifiedTypeSpecName(node.list().valueType(), GenMode::Import).c_str());
            } else {
                std::string sep;
                for(Ast::ListList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                    const Ast::ListItem& item = z::ref(*it);
                    fprintf(_fp, "%s", sep.c_str());
                    visitNode(item.valueExpr());
                    sep = ", ";
                }
            }
            fprintf(_fp, "]");
        }

        virtual void visit(const Ast::DictExpr& node) {
            fprintf(_fp, "[");
            if(node.list().list().size() == 0) {
                fprintf(_fp, "%s:%s", getQualifiedTypeSpecName(node.list().keyType(), GenMode::Import).c_str(), getQualifiedTypeSpecName(node.list().valueType(), GenMode::Import).c_str());
            } else {
                std::string sep;
                for(Ast::DictList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                    const Ast::DictItem& item = z::ref(*it);
                    fprintf(_fp, "%s", sep.c_str());
                    visitNode(item.valueExpr());
                    fprintf(_fp, ":");
                    visitNode(item.valueExpr());
                    sep = ", ";
                }
            }
            fprintf(_fp, "]");
        }

        virtual void visit(const Ast::FormatExpr& node) {
            visitNode(node.stringExpr());
            if(node.dictExpr().list().list().size() > 0) {
                std::string sep;
                fprintf(_fp, " @ {");
                for(Ast::DictList::List::const_iterator it = node.dictExpr().list().list().begin(); it != node.dictExpr().list().list().end(); ++it) {
                    const Ast::DictItem& item = z::ref(*it);
                    fprintf(_fp, "%s", sep.c_str());
                    visitNode(item.keyExpr());
                    fprintf(_fp, ":");
                    visitNode(item.valueExpr());
                    sep = ", ";
                }
                fprintf(_fp, "}");
            }
        }

        virtual void visit(const Ast::RoutineCallExpr& node) {
            fprintf(_fp, "%s(", getTypeSpecName(node.routine(), GenMode::Import).c_str());
            ExprGenerator(_fp, ", ").visitList(node.exprList());
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::FunctorCallExpr& node) {
            ExprGenerator(_fp).visitNode(node.expr());
            fprintf(_fp, "(");
            ExprGenerator(_fp, ", ").visitList(node.exprList());
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::RunExpr& node) {
            fprintf(_fp, "run ");
            ExprGenerator(_fp).visitNode(node.callExpr().expr());
            fprintf(_fp, "(");
            ExprGenerator(_fp, ", ").visitList(node.callExpr().exprList());
            fprintf(_fp, ")");
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

        virtual void visit(const Ast::SpliceExpr& node) {
            visitNode(node.expr());
            fprintf(_fp, "[");
            visitNode(node.from());
            fprintf(_fp, ":");
            visitNode(node.to());
            fprintf(_fp, "]");
        }

        virtual void visit(const Ast::TypeofTypeExpr& node) {
            fprintf(_fp, "typeof(%s)", getQualifiedTypeSpecName(node.typeSpec(), GenMode::Import).c_str());
        }

        virtual void visit(const Ast::TypeofExprExpr& node) {
//            fprintf(_fp, "typeof(%s)", getQualifiedTypeSpecName(node.typeSpec(), GenMode::Import).c_str());
        }

        virtual void visit(const Ast::StaticTypecastExpr& node) {
            fprintf(_fp, "<%s>(", getQualifiedTypeSpecName(node.qTypeSpec(), GenMode::Import).c_str());
            ExprGenerator(_fp).visitNode(node.expr());
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::DynamicTypecastExpr& node) {
            fprintf(_fp, "<%s>(", getTypeSpecName(node.qTypeSpec().typeSpec(), GenMode::Import).c_str());
            ExprGenerator(_fp).visitNode(node.expr());
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::PointerInstanceExpr& node) {
            const Ast::Expr& expr = node.exprList().at(0);
            const std::string dname = getTypeSpecName(expr.qTypeSpec().typeSpec(), GenMode::Import);

            fprintf(_fp, "&(%s())", dname.c_str());
        }

        virtual void visit(const Ast::ValueInstanceExpr& node) {
            fprintf(_fp, "<%s>(", getTypeSpecName(node.qTypeSpec().typeSpec(), GenMode::Import).c_str());
//            ExprGenerator(_fp).visitNode(node.expr());
            fprintf(_fp, ")");
        }

        virtual void visit(const Ast::VariableRefExpr& node) {
            fprintf(_fp, "%s", node.vref().name().text());
        }

        virtual void visit(const Ast::MemberVariableExpr& node) {
            visitNode(node.expr());
            fprintf(_fp, ".%s", node.vref().name().text());
        }

        virtual void visit(const Ast::MemberPropertyExpr& node) {
            visitNode(node.expr());
            fprintf(_fp, ".%s", node.pref().name().text());
        }

        virtual void visit(const Ast::EnumMemberExpr& node) {
            fprintf(_fp, "%s", getTypeSpecName(node.typeSpec(), GenMode::TypeSpecMemberRef).c_str());
            fprintf(_fp, ".%s", node.vref().name().text());
        }

        virtual void visit(const Ast::StructMemberExpr& node) {
            fprintf(_fp, "%s", getTypeSpecName(node.typeSpec(), GenMode::TypeSpecMemberRef).c_str());
            fprintf(_fp, "::%s", node.vref().name().text());
        }

        virtual void visit(const Ast::StructInstanceExpr& node) {
            fprintf(_fp, "%s(", getTypeSpecName(node.structDefn(), GenMode::Import).c_str());
            for(Ast::StructInitPartList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const Ast::StructInitPart& part = z::ref(*it);
                fprintf(_fp, "%s:", part.vdef().name().text());
                visitNode(part.expr());
                fprintf(_fp, ";");
            }
            fprintf(_fp, ")");
        }

        inline void visitFunctionTypeInstance(const Ast::Function& function) {
            std::string fname = getTypeSpecName(function, GenMode::Import);
            fprintf(_fp, "%s(", fname.c_str());
            std::string sep;
            for(Ast::Scope::List::const_iterator it = function.xref().begin(); it != function.xref().end(); ++it) {
                const Ast::VariableDefn& vref = z::ref(*it);
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

        inline void visitConstant(const Ast::ConstantExpr& node) {
            fprintf(_fp, "%s", node.token().text());
        }

        virtual void visit(const Ast::ConstantFloatExpr& node) {
            visitConstant(node);
        }

        virtual void visit(const Ast::ConstantDoubleExpr& node) {
            visitConstant(node);
        }

        virtual void visit(const Ast::ConstantBooleanExpr& node) {
            visitConstant(node);
        }

        virtual void visit(const Ast::ConstantStringExpr& node) {
            fprintf(_fp, "\"%s\"", node.token().text());
        }

        virtual void visit(const Ast::ConstantCharExpr& node) {
            fprintf(_fp, "\'%s\'", node.token().text());
        }

        virtual void visit(const Ast::ConstantLongExpr& node) {
            visitConstant(node);
        }

        virtual void visit(const Ast::ConstantIntExpr& node) {
            visitConstant(node);
        }

        virtual void visit(const Ast::ConstantShortExpr& node) {
            visitConstant(node);
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

    void runStatementGenerator(const Ast::Config& config, FILE* fp, const Ast::Statement& block);

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
                    const Ast::VariableDefn& def = z::ref(*it);
                    fprintf(_fp, "    %s", def.name().text());
                    const Ast::ConstantIntExpr* cexpr = dynamic_cast<const Ast::ConstantIntExpr*>(z::ptr(def.initExpr()));
                    if((cexpr == 0) || (z::ref(cexpr).token().string() != "#")) {
                        fprintf(_fp, " = ");
                        ExprGenerator(_fp).visitNode(def.initExpr());
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
                    fprintf(_fp, " : %s", getTypeSpecName(z::ref(base), GenMode::Import).c_str());
                }
                fprintf(_fp, "%s {\n", getDefinitionType(node.defType()).c_str());
                runStatementGenerator(_config, _fp, node.block());
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
            visitStructDefn(node, z::ptr(node.base()));
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
                    const Ast::VariableDefn& vdef = z::ref(*it);
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
                        const Ast::VariableDefn& vdef = z::ref(*it);
                        fprintf(_fp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qTypeSpec(), GenMode::Import).c_str(), vdef.name().text());
                        sep = ", ";
                    }
                    fprintf(_fp, ")");
                } else {
                    const Ast::VariableDefn& vdef = z::ref(node.sig().out().front());
                    fprintf(_fp, "%s ", getQualifiedTypeSpecName(vdef.qTypeSpec(), GenMode::Import).c_str());
                }

                if(name.size() > 0) {
                    fprintf(_fp, "%s(", name.c_str());
                } else {
                    fprintf(_fp, "%s(", node.name().text());
                }
                std::string sep = "";
                for(Ast::Scope::List::const_iterator it = node.sig().in().begin(); it != node.sig().in().end(); ++it) {
                    const Ast::VariableDefn& vdef = z::ref(*it);
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
                fprintf(_fp, "%sevent(%s %s)%s => ", getAccessType(node.accessType()).c_str(), getQualifiedTypeSpecName(node.in().qTypeSpec(), GenMode::Import).c_str(), node.in().name().text(), getDefinitionType(node.defType()).c_str());
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
        inline ImportGenerator(const Ast::Config& config, FILE* fp) : _config(config), _fp(fp) {}

    private:
        const Ast::Config& _config;
        FILE* _fp;
    };

    struct StatementGenerator : public Ast::Statement::Visitor {
    private:
        virtual void visit(const Ast::ImportStatement& node) {
            if(node.headerType() == Ast::HeaderType::Import) {
                fprintf(_fp, "import ");
            } else {
                fprintf(_fp, "include ");
            }

            std::string sep = "";
            for(Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Token& name = z::ref(*it).name();
                fprintf(_fp, "%s%s", sep.c_str(), name.text());
                sep = "::";
            }

            if(node.defType() == Ast::DefinitionType::Native) {
                fprintf(_fp, " native");
            }

            fprintf(_fp, ";\n");
        }

        virtual void visit(const Ast::EnterNamespaceStatement& node) {
            std::string fqn;
            std::string sep;
            for(Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Namespace& ns = z::ref(*it);
                fqn += sep;
                fqn += ns.name().string();
            }
            if(fqn.size() > 0) {
                fprintf(_fp, "namespace %s;\n\n", fqn.c_str());
            }
        }

        virtual void visit(const Ast::LeaveNamespaceStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::UserDefinedTypeSpecStatement& node) {
            ImportGenerator(_config, _fp).visitNode(node.typeSpec());
        }

        virtual void visit(const Ast::StructMemberVariableStatement& node) {
            fprintf(_fp, "%s%s %s = ", Indent::get(), getQualifiedTypeSpecName(node.defn().qTypeSpec(), GenMode::Import).c_str(), node.defn().name().text());
            ExprGenerator(_fp).visitNode(node.defn().initExpr());
            fprintf(_fp, ";\n");
        }

        virtual void visit(const Ast::StructInitStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::AutoStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::ExprStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::PrintStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::IfStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::IfElseStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::WhileStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::DoWhileStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::ForExprStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::ForInitStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::ForeachStringStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::ForeachListStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::ForeachDictStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::CaseExprStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::CaseDefaultStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::SwitchValueStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::SwitchExprStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::BreakStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::ContinueStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::AddEventHandlerStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::RoutineReturnStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::FunctionReturnStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::CompoundStatement& node) {
            INDENT;
            for(Ast::CompoundStatement::List::const_iterator sit = node.list().begin(); sit != node.list().end(); ++sit) {
                const Ast::Statement& s = z::ref(*sit);
                z::ref(this).visitNode(s);
            }
        }

    public:
        inline StatementGenerator(const Ast::Config& config, FILE* fp) : _config(config), _fp(fp) {}
    private:
        const Ast::Config& _config;
        FILE* _fp;
    };

    void runStatementGenerator(const Ast::Config& config, FILE* fp, const Ast::Statement& block) {
        StatementGenerator gen(config, fp);
        gen.visitNode(block);
    }
}

struct Interpreter::Impl {
    inline Impl(const Ast::Project& project, const Ast::Config& config) : _project(project), _config(config), _fpImp(0) {}
    inline void run();
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
private:
    FILE* _fpImp;
};

inline void Interpreter::Impl::run() {
    printf("Entering interpretor mode\n");
    InterpreterContext ctx;
    Ast::Unit unit("");
    Compiler c(_project, _config);
    c.initContext(ctx, unit);
    Ast::Token pos(0, 0, "");
    Ast::Scope global(pos, Ast::ScopeType::Local);
    ctx.enterScope(global);
    bool quit = false;
    while (quit == false) {
        std::cout << ">";
        std::string cmd;
        std::getline(std::cin, cmd);
        std::cout << cmd << std::endl;
        if(cmd == ".q")
            break;
        try {
            c.parseString(ctx, unit, cmd, 0);
            for(Ast::Unit::StatementList::const_iterator sit = unit.globalStatementList().begin(); sit != unit.globalStatementList().end(); ++sit) {
                const Ast::Statement& s = z::ref(*sit);
                runStatementGenerator(_config, _fpImp, s);
            }
        } catch (...) {
        }
    }
    ctx.leaveScope(global);
}

//////////////////////////////////////////////
Interpreter::Interpreter(const Ast::Project& project, const Ast::Config& config) : _impl(0) {_impl = new Impl(project, config);}
Interpreter::~Interpreter() {delete _impl;}
void Interpreter::run() {return z::ref(_impl).run();}
