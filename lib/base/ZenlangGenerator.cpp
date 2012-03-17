#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "base/typename.hpp"
#include "base/ZenlangGenerator.hpp"

namespace {
    inline z::string getDefinitionType(const Ast::Token& pos, const Ast::DefinitionType::T& defType) {
        switch(defType) {
            case Ast::DefinitionType::Final:
                return "";
            case Ast::DefinitionType::Native:
                return " native";
            case Ast::DefinitionType::Abstract:
                return " abstract";
        }
        throw z::Exception("ZenlangGenerator", zfmt(pos, "Internal error: Unknown Definition Type '%{s}'").arg("s", defType ));
    }

    inline z::string getAccessType(const Ast::Token& pos, const Ast::AccessType::T& accessType) {
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
        throw z::Exception("ZenlangGenerator", zfmt(pos, "Internal error: Unknown Access Type '%{s}'").arg("s", accessType ));
    }

    struct ExprGenerator : public Ast::Expr::Visitor {
    public:
        inline ExprGenerator(std::ostream& os, const z::string& sep2 = "", const z::string& sep1 = "") : _os(os), _sep2(sep2), _sep1(sep1), _sep0(sep1) {}
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
                _os << ZenlangNameGenerator().qtn(node.list().valueType());
            } else {
                z::string sep;
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
                _os << ZenlangNameGenerator().qtn(node.list().keyType()) << ":" << ZenlangNameGenerator().qtn(node.list().valueType());
            } else {
                z::string sep;
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
                z::string sep;
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
            _os << ZenlangNameGenerator().tn(node.routine()) << "(";
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
            _os << "typeof(" << ZenlangNameGenerator().qtn(node.typeSpec()) << ")";
        }

        virtual void visit(const Ast::TypeofExprExpr& node) {
            _os << "typeof(";
            ExprGenerator(_os).visitNode(node.expr());
            _os << ")";
        }

        virtual void visit(const Ast::StaticTypecastExpr& node) {
            _os << "<" << ZenlangNameGenerator().qtn(node.qTypeSpec()) << ">(";
            ExprGenerator(_os).visitNode(node.expr());
            _os << ")";
        }

        virtual void visit(const Ast::DynamicTypecastExpr& node) {
            _os << "<" << ZenlangNameGenerator().tn(node.qTypeSpec().typeSpec()) << ">(";
            ExprGenerator(_os).visitNode(node.expr());
            _os << ")";
        }

        virtual void visit(const Ast::PointerInstanceExpr& node) {
            const Ast::Expr& expr = node.exprList().at(0);
            const z::string dname = ZenlangNameGenerator().tn(expr.qTypeSpec().typeSpec());
            _os << "&(" <<dname << "())";
        }

        virtual void visit(const Ast::ValueInstanceExpr& node) {
            _os << "<" << ZenlangNameGenerator().tn(node.qTypeSpec().typeSpec()) << ">(";
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
            _os << ZenlangNameGenerator().tn(node.typeSpec());
            _os << "." << node.vref().name().string();
        }

        virtual void visit(const Ast::StructMemberExpr& node) {
            _os << ZenlangNameGenerator().tn(node.typeSpec());
            _os << "::" << node.vref().name().string();
        }

        virtual void visit(const Ast::StructInstanceExpr& node) {
            _os << ZenlangNameGenerator().tn(node.structDefn()) << "(";
            for(Ast::StructInitPartList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const Ast::StructInitPart& part = it->get();
                _os << part.vdef().name().string() << ":";
                visitNode(part.expr());
                _os << ";";
            }
            _os << ")";
        }

        inline void visitFunctionTypeInstance(const Ast::Function& function) {
            z::string fname = ZenlangNameGenerator().tn(function);
            _os << fname << "(";
            z::string sep;
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

        virtual void visit(const Ast::ConstantNullExpr& node) {
            unused(node);
            _os << "null";
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
        const z::string _sep2;
        const z::string _sep1;
        z::string _sep0;
    };

    void runStatementGenerator(const Ast::Config& config, z::ofile& fp, const Ast::Statement& block);

    struct ImportGenerator : public Ast::TypeSpec::Visitor {
        inline bool canWrite(const Ast::AccessType::T& accessType) const {
            return ((accessType == Ast::AccessType::Public) || (accessType == Ast::AccessType::Parent));
        }

        inline void visitChildrenIndent(const Ast::TypeSpec& node) {
            visitChildren(node);
        }

        void visit(const Ast::TypedefDecl& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "typedef " << node.name() << getDefinitionType(node.pos(), node.defType()) << std::endl;
            }
            visitChildrenIndent(node);
        }

        void visit(const Ast::TypedefDefn& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType())
                      << "typedef " << node.name()
                      << getDefinitionType(node.pos(), node.defType())
                      << " " << ZenlangNameGenerator().qtn(node.qTypeSpec())
                      << ";" << std::endl;
            }
            visitChildrenIndent(node);
        }

        void visit(const Ast::TemplateDecl& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "template <";
                z::string sep;
                for(Ast::TemplatePartList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const Ast::Token& token = *it;
                    _os() << sep << token << std::endl;
                    sep = ", ";
                }
                _os() << "> " << node.name() << getDefinitionType(node.pos(), node.defType()) << ";" << std::endl;
            }
            visitChildrenIndent(node);
        }

        void visit(const Ast::TemplateDefn& node) {
            visitChildrenIndent(node);
        }

        void visit(const Ast::EnumDecl& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "enum " << node.name() << getDefinitionType(node.pos(), node.defType()) << ";" << std::endl;
            }
            visitChildrenIndent(node);
        }

        void visit(const Ast::EnumDefn& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "enum " << node.name() << getDefinitionType(node.pos(), node.defType()) << " {" << std::endl;
                for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const Ast::VariableDefn& def = it->get();
                    _os() << "    " << def.name();
                    const Ast::ConstantIntExpr* cexpr = dynamic_cast<const Ast::ConstantIntExpr*>(z::ptr(def.initExpr()));
                    if((cexpr == 0) || (z::ref(cexpr).pos().string() != "#")) { /// \todo workaround, until enum-init implemented
                        _os() << " = ";
                        ExprGenerator(_os()).visitNode(def.initExpr());
                    }
                    _os() << ";" << std::endl;
                }
                _os() << "};" << std::endl;
                _os() << std::endl;
            }
            visitChildrenIndent(node);
        }

        inline void visitStructDefn(const Ast::StructDefn& node, const Ast::StructDefn* base) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "struct " << node.name();
                if(base) {
                    _os() << " : " << ZenlangNameGenerator().tn(z::ref(base));
                }
                _os() << getDefinitionType(node.pos(), node.defType()) << " {" << std::endl;
                runStatementGenerator(_config, _os, node.block());
                _os() << "};" << std::endl;
            }
        }

        void visit(const Ast::StructDecl& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "struct " << node.name() << getDefinitionType(node.pos(), node.defType()) << ";" << std::endl;
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
                _os() << getAccessType(node.pos(), node.accessType())
                      << "property " << ZenlangNameGenerator().qtn(node.qTypeSpec())
                      << " " << node.name()
                      << " " << getDefinitionType(node.pos(), node.defType())
                      << " get set;" << std::endl;
            }
        }

        void visit(const Ast::PropertyDeclRO& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType())
                      << "property " << ZenlangNameGenerator().qtn(node.qTypeSpec())
                      << " " << node.name()
                      << " " << getDefinitionType(node.pos(), node.defType())
                      << " get;" << std::endl;
            }
        }

        inline void visitRoutineImp(const Ast::Routine& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "routine " << ZenlangNameGenerator().qtn(node.outType()) << " ";
                _os() << node.name();
                _os() << "(";
                z::string sep;
                for(Ast::Scope::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
                    const Ast::VariableDefn& vdef = it->get();
                    _os() << sep << ZenlangNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name();
                    sep = ", ";
                }
                _os() << ")" << getDefinitionType(node.pos(), node.defType()) << ";" << std::endl;
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

        inline void visitFunctionImp(const Ast::Function& node, const z::string& name, const bool& isEvent) {
            if((name.size() > 0) || (canWrite(node.accessType()))) {
                if(!isEvent) {
                    _os() << getAccessType(node.pos(), node.accessType());
                }

                _os() << "function ";
                if(node.sig().outScope().isTuple()) {
                    _os() << "(";
                    z::string sep;
                    for(Ast::Scope::List::const_iterator it = node.sig().out().begin(); it != node.sig().out().end(); ++it) {
                        const Ast::VariableDefn& vdef = it->get();
                        _os() << sep << ZenlangNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name();
                        sep = ", ";
                    }
                    _os() << ")";
                } else {
                    const Ast::VariableDefn& vdef = node.sig().out().front();
                    _os() << ZenlangNameGenerator().qtn(vdef.qTypeSpec()) << " ";
                }

                if(name.size() > 0) {
                    _os() << name << "(";
                } else {
                    _os() << node.name() << "(";
                }
                z::string sep = "";
                for(Ast::Scope::List::const_iterator it = node.sig().in().begin(); it != node.sig().in().end(); ++it) {
                    const Ast::VariableDefn& vdef = it->get();
                    _os() << sep << ZenlangNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name();
                    sep = ", ";
                }
                _os() << ")" << getDefinitionType(node.pos(), node.defType()) << ";" << std::endl;
            }
        }

        void visit(const Ast::RootFunctionDecl& node) {
            visitFunctionImp(node, "", false);
            visitChildrenIndent(node);
        }

        void visit(const Ast::ChildFunctionDecl& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType());
                _os() << "function " << node.name();
                _os() << " : " << ZenlangNameGenerator().tn(node.base());
                _os() << getDefinitionType(node.pos(), node.defType()) << ";" << std::endl;
            }

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
                _os() << getAccessType(node.pos(), node.accessType())
                      << "event(" << ZenlangNameGenerator().qtn(node.in().qTypeSpec())
                      << " " << node.in().name() << ")" << getDefinitionType(node.pos(), node.defType()) << " => ";
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
        inline ImportGenerator(const Ast::Config& config, z::ofile& os) : _config(config), _os(os) {}

    private:
        const Ast::Config& _config;
        z::ofile& _os;
    };

    struct StatementGenerator : public Ast::Statement::Visitor {
    private:
        virtual void visit(const Ast::ImportStatement& node) {
            if(node.headerType() == Ast::HeaderType::Import) {
                _os() << "import ";
            } else {
                _os() << "include ";
            }

            z::string sep = "";
            for(Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Token& name = it->get().name();
                _os() << sep << name;
                sep = "::";
            }

            if(node.defType() == Ast::DefinitionType::Native) {
                _os() << " native";
            }

            _os() << ";" << std::endl;
        }

        virtual void visit(const Ast::EnterNamespaceStatement& node) {
            z::string fqn;
            z::string sep;
            for(Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Namespace& ns = it->get();
                fqn += sep;
                fqn += ns.name().string();
            }
            if(fqn.size() > 0) {
                _os() << "namespace " << fqn << ";" << std::endl;
                _os() << std::endl;
            }
        }

        virtual void visit(const Ast::LeaveNamespaceStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::UserDefinedTypeSpecStatement& node) {
            ImportGenerator(_config, _os).visitNode(node.typeSpec());
        }

        virtual void visit(const Ast::StructMemberVariableStatement& node) {
            _os() << Indent::get() << ZenlangNameGenerator().qtn(node.defn().qTypeSpec()) << " " << node.defn().name() << " = ";
            ExprGenerator(_os()).visitNode(node.defn().initExpr());
            _os() << ";" << std::endl;
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
        inline StatementGenerator(const Ast::Config& config, z::ofile& os) : _config(config), _os(os) {}
    private:
        const Ast::Config& _config;
        z::ofile& _os;
    };

    void runStatementGenerator(const Ast::Config& config, z::ofile& os, const Ast::Statement& block) {
        StatementGenerator gen(config, os);
        gen.visitNode(block);
    }
}

struct ZenlangGenerator::Impl {
    inline Impl(const Ast::Project& project, const Ast::Config& config, const Ast::Module& module) : _project(project), _config(config), _module(module) {}
    inline void run();
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
    const Ast::Module& _module;
};

inline void ZenlangGenerator::Impl::run() {
    Indent::init();
    z::string basename = getBaseName(_module.filename());
    z::file::mkpath(_config.apidir() + "/");
    z::ofile osImp(_config.apidir() + "/" + basename + ".ipp");

    for(Ast::CompoundStatement::List::const_iterator it = _module.globalStatementList().list().begin(); it != _module.globalStatementList().list().end(); ++it) {
        const Ast::Statement& s = it->get();
        runStatementGenerator(_config, osImp, s);
    }
}

//////////////////////////////////////////////
ZenlangGenerator::ZenlangGenerator(const Ast::Project& project, const Ast::Config& config, const Ast::Module& module) : _impl(0) {_impl = new Impl(project, config, module);}
ZenlangGenerator::~ZenlangGenerator() {delete _impl;}
void ZenlangGenerator::run() {return z::ref(_impl).run();}

z::string ZenlangGenerator::convertExprToString(const Ast::Expr& expr) {
    std::stringstream os;
    ExprGenerator(os).visitNode(expr);
    return z::e2s(os.str());
}
