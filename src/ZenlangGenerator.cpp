#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "ZenlangGenerator.hpp"
#include "typename.hpp"

namespace {
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
        inline ExprGenerator(std::ostream& os, const std::string& sep2 = "", const std::string& sep1 = "") : _os(os), _sep2(sep2), _sep1(sep1), _sep0(sep1) {}
    private:
        inline void visitTernary(const Ast::TernaryOpExpr& node) {
            _os << "(";
            visitNode(node.lhs());
            _os << node.op1().string();
            visitNode(node.rhs1());
            _os << node.op2().string();
            visitNode(node.rhs2());
            _os << ")";
        }

        virtual void visit(const Ast::ConditionalExpr& node) {
            return visitTernary(node);
        }

        virtual void visitBinary(const Ast::BinaryExpr& node) {
            visitNode(node.lhs());
            _os << node.op().string();
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
            _os << node.op().string();
        }

        virtual void visit(const Ast::PostfixIncExpr& node) {
            return visitPostfix(node);
        }

        virtual void visit(const Ast::PostfixDecExpr& node) {
            return visitPostfix(node);
        }

        inline void visitPrefix(const Ast::PrefixExpr& node) {
            _os << node.op().string();
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
            _os << "[";
            visitNode(node.lhs().index());
            _os << "] = ";
            visitNode(node.rhs());
        }

        virtual void visit(const Ast::ListExpr& node) {
            _os << "[";
            if(node.list().list().size() == 0) {
                _os << getQualifiedTypeSpecName(node.list().valueType(), GenMode::Import);
            } else {
                std::string sep;
                for(Ast::ListList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                    const Ast::ListItem& item = it->get();
                    _os << sep;
                    visitNode(item.valueExpr());
                    sep = ", ";
                }
            }
            _os << "]";
        }

        virtual void visit(const Ast::DictExpr& node) {
            _os << "[";
            if(node.list().list().size() == 0) {
                _os << getQualifiedTypeSpecName(node.list().keyType(), GenMode::Import) << ":" << getQualifiedTypeSpecName(node.list().valueType(), GenMode::Import);
            } else {
                std::string sep;
                for(Ast::DictList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                    const Ast::DictItem& item = it->get();
                    _os << sep;
                    visitNode(item.valueExpr());
                    _os << ":";
                    visitNode(item.valueExpr());
                    sep = ", ";
                }
            }
            _os << "]";
        }

        virtual void visit(const Ast::FormatExpr& node) {
            visitNode(node.stringExpr());
            if(node.dictExpr().list().list().size() > 0) {
                std::string sep;
                _os << " @ {";
                for(Ast::DictList::List::const_iterator it = node.dictExpr().list().list().begin(); it != node.dictExpr().list().list().end(); ++it) {
                    const Ast::DictItem& item = it->get();
                    _os << sep;
                    visitNode(item.keyExpr());
                    _os << ":";
                    visitNode(item.valueExpr());
                    sep = ", ";
                }
                _os << "}";
            }
        }

        virtual void visit(const Ast::RoutineCallExpr& node) {
            _os << getTypeSpecName(node.routine(), GenMode::Import) << "(";
            ExprGenerator(_os, ", ").visitList(node.exprList());
            _os << ")";
        }

        virtual void visit(const Ast::FunctorCallExpr& node) {
            ExprGenerator(_os).visitNode(node.expr());
            _os << "(";
            ExprGenerator(_os, ", ").visitList(node.exprList());
            _os << ")";
        }

        virtual void visit(const Ast::RunExpr& node) {
            _os << "run ";
            ExprGenerator(_os).visitNode(node.callExpr().expr());
            _os << "(";
            ExprGenerator(_os, ", ").visitList(node.callExpr().exprList());
            _os << ")";
        }

        virtual void visit(const Ast::OrderedExpr& node) {
            _os << "(";
            visitNode(node.expr());
            _os << ")";
        }

        virtual void visit(const Ast::IndexExpr& node) {
            visitNode(node.expr());
            _os << "[";
            visitNode(node.index());
            _os << "]";
        }

        virtual void visit(const Ast::SpliceExpr& node) {
            visitNode(node.expr());
            _os << "[";
            visitNode(node.from());
            _os << ":";
            visitNode(node.to());
            _os << "]";
        }

        virtual void visit(const Ast::TypeofTypeExpr& node) {
            _os << "typeof(" << getQualifiedTypeSpecName(node.typeSpec(), GenMode::Import) << ")";
        }

        virtual void visit(const Ast::TypeofExprExpr& node) {
            _os << "typeof(";
            ExprGenerator(_os).visitNode(node.expr());
            _os << ")";
        }

        virtual void visit(const Ast::StaticTypecastExpr& node) {
            _os << "<" << getQualifiedTypeSpecName(node.qTypeSpec(), GenMode::Import) << ">(";
            ExprGenerator(_os).visitNode(node.expr());
            _os << ")";
        }

        virtual void visit(const Ast::DynamicTypecastExpr& node) {
            _os << "<" << getTypeSpecName(node.qTypeSpec().typeSpec(), GenMode::Import) << ">(";
            ExprGenerator(_os).visitNode(node.expr());
            _os << ")";
        }

        virtual void visit(const Ast::PointerInstanceExpr& node) {
            const Ast::Expr& expr = node.exprList().at(0);
            const std::string dname = getTypeSpecName(expr.qTypeSpec().typeSpec(), GenMode::Import);
            _os << "&(" <<dname << "())";
        }

        virtual void visit(const Ast::ValueInstanceExpr& node) {
            _os << "<" << getTypeSpecName(node.qTypeSpec().typeSpec(), GenMode::Import) << ">(";
            ExprGenerator(_os).visitList(node.exprList());
            _os << ")";
        }

        virtual void visit(const Ast::VariableRefExpr& node) {
            _os << node.vref().name().string();
        }

        virtual void visit(const Ast::MemberVariableExpr& node) {
            visitNode(node.expr());
            _os << "." << node.vref().name().string();
        }

        virtual void visit(const Ast::MemberPropertyExpr& node) {
            visitNode(node.expr());
            _os << "." << node.pref().name().string();
        }

        virtual void visit(const Ast::EnumMemberExpr& node) {
            _os << getTypeSpecName(node.typeSpec(), GenMode::TypeSpecMemberRef);
            _os << "." << node.vref().name().string();
        }

        virtual void visit(const Ast::StructMemberExpr& node) {
            _os << getTypeSpecName(node.typeSpec(), GenMode::TypeSpecMemberRef);
            _os << "::" << node.vref().name().string();
        }

        virtual void visit(const Ast::StructInstanceExpr& node) {
            _os << getTypeSpecName(node.structDefn(), GenMode::Import) << "(";
            for(Ast::StructInitPartList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const Ast::StructInitPart& part = it->get();
                _os << part.vdef().name().string() << ":";
                visitNode(part.expr());
                _os << ";";
            }
            _os << ")";
        }

        inline void visitFunctionTypeInstance(const Ast::Function& function) {
            std::string fname = getTypeSpecName(function, GenMode::Import);
            _os << fname << "(";
            std::string sep;
            for(Ast::Scope::List::const_iterator it = function.xref().begin(); it != function.xref().end(); ++it) {
                const Ast::VariableDefn& vref = it->get();
                _os << sep << vref.name().string();
                sep = ", ";
            }
            _os << ")";
        }

        virtual void visit(const Ast::FunctionInstanceExpr& node) {
            visitFunctionTypeInstance(node.function());
        }

        virtual void visit(const Ast::AnonymousFunctionExpr& node) {
            visitFunctionTypeInstance(node.function());
        }

        virtual void visit(const Ast::ConstantFloatExpr& node) {
            _os << node.value();
        }

        virtual void visit(const Ast::ConstantDoubleExpr& node) {
            _os << node.value();
        }

        virtual void visit(const Ast::ConstantBooleanExpr& node) {
            _os << ((node.value() == true)?"true":"false");
        }

        virtual void visit(const Ast::ConstantStringExpr& node) {
            _os << "\"" << node.value() << "\"";
        }

        virtual void visit(const Ast::ConstantCharExpr& node) {
            _os << "\'" << node.value() << "\'";
        }

        virtual void visit(const Ast::ConstantLongExpr& node) {
            _os << node.value();
        }

        virtual void visit(const Ast::ConstantIntExpr& node) {
            _os << node.value();
        }

        virtual void visit(const Ast::ConstantShortExpr& node) {
            _os << node.value();
        }

        virtual void sep() {
            _os << _sep0;
            _sep0 = _sep2;
        }

    private:
        std::ostream& _os;
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
                    const Ast::VariableDefn& def = it->get();
                    fprintf(_fp, "    %s", def.name().text());
                    const Ast::ConstantIntExpr* cexpr = dynamic_cast<const Ast::ConstantIntExpr*>(z::ptr(def.initExpr()));
                    if((cexpr == 0) || (z::ref(cexpr).pos().string() != "#")) { /// \todo workaround, until enum-init implemented
                        fprintf(_fp, " = ");
                        std::string estr = ZenlangGenerator::convertExprToString(def.initExpr());
                        fprintf(_fp, "%s", estr.c_str());
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
                    const Ast::VariableDefn& vdef = it->get();
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
                        const Ast::VariableDefn& vdef = it->get();
                        fprintf(_fp, "%s%s %s", sep.c_str(), getQualifiedTypeSpecName(vdef.qTypeSpec(), GenMode::Import).c_str(), vdef.name().text());
                        sep = ", ";
                    }
                    fprintf(_fp, ")");
                } else {
                    const Ast::VariableDefn& vdef = node.sig().out().front();
                    fprintf(_fp, "%s ", getQualifiedTypeSpecName(vdef.qTypeSpec(), GenMode::Import).c_str());
                }

                if(name.size() > 0) {
                    fprintf(_fp, "%s(", name.c_str());
                } else {
                    fprintf(_fp, "%s(", node.name().text());
                }
                std::string sep = "";
                for(Ast::Scope::List::const_iterator it = node.sig().in().begin(); it != node.sig().in().end(); ++it) {
                    const Ast::VariableDefn& vdef = it->get();
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
                const Ast::Token& name = it->get().name();
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
                const Ast::Namespace& ns = it->get();
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
            std::string estr = ZenlangGenerator::convertExprToString(node.defn().initExpr());
            fprintf(_fp, "%s", estr.c_str());
            fprintf(_fp, ";\n");
        }

        virtual void visit(const Ast::StructInitStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::EmptyStatement& node) {
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
            for(Ast::CompoundStatement::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Statement& s = it->get();
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

struct ZenlangGenerator::Impl {
    inline Impl(const Ast::Project& project, const Ast::Config& config, const Ast::Module& module) : _project(project), _config(config), _module(module), _fpImp(0) {}
    inline void run();
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
    const Ast::Module& _module;
private:
    FILE* _fpImp;
};

inline void ZenlangGenerator::Impl::run() {
    Indent::init();
    std::string basename = getBaseName(_module.unit().filename());
    OutputFile ofImp(_fpImp, basename + ".ipp");unused(ofImp);

    for(Ast::CompoundStatement::List::const_iterator it = _module.globalStatementList().list().begin(); it != _module.globalStatementList().list().end(); ++it) {
        const Ast::Statement& s = it->get();
        runStatementGenerator(_config, _fpImp, s);
    }
}

//////////////////////////////////////////////
ZenlangGenerator::ZenlangGenerator(const Ast::Project& project, const Ast::Config& config, const Ast::Module& module) : _impl(0) {_impl = new Impl(project, config, module);}
ZenlangGenerator::~ZenlangGenerator() {delete _impl;}
void ZenlangGenerator::run() {return z::ref(_impl).run();}

std::string ZenlangGenerator::convertExprToString(const Ast::Expr& expr) {
    std::stringstream os;
    ExprGenerator(os).visitNode(expr);
    return os.str();
}
