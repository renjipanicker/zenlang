#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/typename.hpp"
#include "base/ZenlangGenerator.hpp"

namespace zg {
    inline z::string getDefinitionType(const z::Ast::Token& pos, const z::Ast::DefinitionType::T& defType) {
        switch(defType) {
            case z::Ast::DefinitionType::Final:
                return "";
            case z::Ast::DefinitionType::Native:
                return " native";
            case z::Ast::DefinitionType::Abstract:
                return " abstract";
        }
        throw z::Exception("ZenlangGenerator", z::zfmt(pos, "Internal error: Unknown Definition Type '%{s}'").arg("s", defType ));
    }

    inline z::string getAccessType(const z::Ast::Token& pos, const z::Ast::AccessType::T& accessType) {
        switch(accessType) {
            case z::Ast::AccessType::Private:
                return "";
            case z::Ast::AccessType::Protected:
                return "protected ";
            case z::Ast::AccessType::Public:
                return "public ";
            case z::Ast::AccessType::Internal:
                return "internal ";
            case z::Ast::AccessType::External:
                return "external ";
            case z::Ast::AccessType::Parent:
                return "";
        }
        throw z::Exception("ZenlangGenerator", z::zfmt(pos, "Internal error: Unknown Access Type '%{s}'").arg("s", accessType ));
    }

    struct ExprGenerator : public z::Ast::Expr::Visitor {
    public:
        inline ExprGenerator(std::ostream& os, const z::string& sep2 = "", const z::string& sep1 = "") : _os(os), _sep2(sep2), _sep1(sep1), _sep0(sep1) {}
    private:
        inline void visitTernary(const z::Ast::TernaryOpExpr& node) {
            _os << "(";
            visitNode(node.lhs());
            _os << node.op1().string();
            visitNode(node.rhs1());
            _os << node.op2().string();
            visitNode(node.rhs2());
            _os << ")";
        }

        virtual void visit(const z::Ast::ConditionalExpr& node) {
            return visitTernary(node);
        }

        virtual void visitBinary(const z::Ast::BinaryExpr& node) {
            visitNode(node.lhs());
            _os << node.op().string();
            visitNode(node.rhs());
        }

        virtual void visit(const z::Ast::BooleanAndExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BooleanOrExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BooleanEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BooleanNotEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BooleanLessThanExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BooleanGreaterThanExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BooleanLessThanOrEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BooleanGreaterThanOrEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BooleanHasExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryAssignEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryPlusEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryMinusEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryTimesEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryDivideEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryModEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryBitwiseAndEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryBitwiseOrEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryBitwiseXorEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryShiftLeftEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryShiftRightEqualExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryPlusExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryMinusExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryTimesExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryDivideExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryModExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryBitwiseAndExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryBitwiseOrExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryBitwiseXorExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryShiftLeftExpr& node) {
            return visitBinary(node);
        }

        virtual void visit(const z::Ast::BinaryShiftRightExpr& node) {
            return visitBinary(node);
        }

        inline void visitPostfix(const z::Ast::PostfixExpr& node) {
            visitNode(node.lhs());
            _os << node.op().string();
        }

        virtual void visit(const z::Ast::PostfixIncExpr& node) {
            return visitPostfix(node);
        }

        virtual void visit(const z::Ast::PostfixDecExpr& node) {
            return visitPostfix(node);
        }

        inline void visitPrefix(const z::Ast::PrefixExpr& node) {
            _os << node.op().string();
            visitNode(node.rhs());
        }

        virtual void visit(const z::Ast::PrefixNotExpr& node) {
            return visitPrefix(node);
        }

        virtual void visit(const z::Ast::PrefixPlusExpr& node) {
            return visitPrefix(node);
        }

        virtual void visit(const z::Ast::PrefixMinusExpr& node) {
            return visitPrefix(node);
        }

        virtual void visit(const z::Ast::PrefixIncExpr& node) {
            return visitPrefix(node);
        }

        virtual void visit(const z::Ast::PrefixDecExpr& node) {
            return visitPrefix(node);
        }

        virtual void visit(const z::Ast::PrefixBitwiseNotExpr& node) {
            return visitPrefix(node);
        }

        virtual void visit(const z::Ast::SetIndexExpr& node) {
            visitNode(node.lhs().expr());
            _os << "[";
            visitNode(node.lhs().index());
            _os << "] = ";
            visitNode(node.rhs());
        }

        virtual void visit(const z::Ast::ListExpr& node) {
            _os << "[";
            if(node.list().list().size() == 0) {
                _os << z::ZenlangNameGenerator().qtn(node.list().valueType());
            } else {
                z::string sep;
                for(z::Ast::ListList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                    const z::Ast::ListItem& item = it->get();
                    _os << sep;
                    visitNode(item.valueExpr());
                    sep = ", ";
                }
            }
            _os << "]";
        }

        virtual void visit(const z::Ast::DictExpr& node) {
            _os << "[";
            if(node.list().list().size() == 0) {
                _os << z::ZenlangNameGenerator().qtn(node.list().keyType()) << ":" << z::ZenlangNameGenerator().qtn(node.list().valueType());
            } else {
                z::string sep;
                for(z::Ast::DictList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                    const z::Ast::DictItem& item = it->get();
                    _os << sep;
                    visitNode(item.valueExpr());
                    _os << ":";
                    visitNode(item.valueExpr());
                    sep = ", ";
                }
            }
            _os << "]";
        }

        virtual void visit(const z::Ast::FormatExpr& node) {
            visitNode(node.stringExpr());
            if(node.dictExpr().list().list().size() > 0) {
                z::string sep;
                _os << " @ {";
                for(z::Ast::DictList::List::const_iterator it = node.dictExpr().list().list().begin(); it != node.dictExpr().list().list().end(); ++it) {
                    const z::Ast::DictItem& item = it->get();
                    _os << sep;
                    visitNode(item.keyExpr());
                    _os << ":";
                    visitNode(item.valueExpr());
                    sep = ", ";
                }
                _os << "}";
            }
        }

        virtual void visit(const z::Ast::RoutineCallExpr& node) {
            _os << z::ZenlangNameGenerator().tn(node.routine()) << "(";
            ExprGenerator(_os, ", ").visitList(node.exprList());
            _os << ")";
        }

        virtual void visit(const z::Ast::FunctorCallExpr& node) {
            ExprGenerator(_os).visitNode(node.expr());
            _os << "(";
            ExprGenerator(_os, ", ").visitList(node.exprList());
            _os << ")";
        }

        virtual void visit(const z::Ast::RunExpr& node) {
            _os << "run ";
            ExprGenerator(_os).visitNode(node.callExpr().expr());
            _os << "(";
            ExprGenerator(_os, ", ").visitList(node.callExpr().exprList());
            _os << ")";
        }

        virtual void visit(const z::Ast::OrderedExpr& node) {
            _os << "(";
            visitNode(node.expr());
            _os << ")";
        }

        virtual void visit(const z::Ast::IndexExpr& node) {
            visitNode(node.expr());
            _os << "[";
            visitNode(node.index());
            _os << "]";
        }

        virtual void visit(const z::Ast::SpliceExpr& node) {
            visitNode(node.expr());
            _os << "[";
            visitNode(node.from());
            _os << ":";
            visitNode(node.to());
            _os << "]";
        }

        virtual void visit(const z::Ast::SizeofTypeExpr& node) {
            _os << "sizeof(" << z::ZenlangNameGenerator().qtn(node.typeSpec()) << ")";
        }

        virtual void visit(const z::Ast::SizeofExprExpr& node) {
            _os << "sizeof(";
            ExprGenerator(_os).visitNode(node.expr());
            _os << ")";
        }

        virtual void visit(const z::Ast::TypeofTypeExpr& node) {
            _os << "typeof(" << z::ZenlangNameGenerator().qtn(node.typeSpec()) << ")";
        }

        virtual void visit(const z::Ast::TypeofExprExpr& node) {
            _os << "typeof(";
            ExprGenerator(_os).visitNode(node.expr());
            _os << ")";
        }

        virtual void visit(const z::Ast::StaticTypecastExpr& node) {
            _os << "(" << z::ZenlangNameGenerator().qtn(node.qTypeSpec()) << ")(";
            ExprGenerator(_os).visitNode(node.expr());
            _os << ")";
        }

        virtual void visit(const z::Ast::DynamicTypecastExpr& node) {
            _os << "(" << z::ZenlangNameGenerator().tn(node.qTypeSpec().typeSpec()) << ")(";
            ExprGenerator(_os).visitNode(node.expr());
            _os << ")";
        }

        virtual void visit(const z::Ast::PointerInstanceExpr& node) {
            const z::Ast::Expr& expr = node.exprList().at(0);
            const z::string dname = z::ZenlangNameGenerator().tn(expr.qTypeSpec().typeSpec());
            _os << "&(" <<dname << "())";
        }

        virtual void visit(const z::Ast::ValueInstanceExpr& node) {
            _os << "<" << z::ZenlangNameGenerator().qtn(node.qTypeSpec()) << ">(";
            ExprGenerator(_os).visitList(node.exprList());
            _os << ")";
        }

        virtual void visit(const z::Ast::MapDataInstanceExpr& node) {
            _os << "map<" << z::ZenlangNameGenerator().qtn(node.qTypeSpec()) << ">(";
            ExprGenerator(_os).visitList(node.exprList());
            _os << ")";
        }

        virtual void visit(const z::Ast::DeRefInstanceExpr& node) {
            ExprGenerator(_os).visitList(node.exprList());
        }

        virtual void visit(const z::Ast::VariableRefExpr& node) {
            _os << node.vref().name().string();
        }

        virtual void visit(const z::Ast::MemberVariableExpr& node) {
            visitNode(node.expr());
            _os << "." << node.vref().name().string();
        }

        virtual void visit(const z::Ast::MemberPropertyExpr& node) {
            visitNode(node.expr());
            _os << "." << node.pref().name().string();
        }

        virtual void visit(const z::Ast::EnumMemberExpr& node) {
            _os << z::ZenlangNameGenerator().tn(node.typeSpec());
            _os << "." << node.vref().name().string();
        }

        virtual void visit(const z::Ast::StructMemberExpr& node) {
            _os << z::ZenlangNameGenerator().tn(node.typeSpec());
            _os << "::" << node.vref().name().string();
        }

        virtual void visit(const z::Ast::StructInstanceExpr& node) {
            _os << z::ZenlangNameGenerator().tn(node.structDefn()) << "(";
            for(z::Ast::StructInitPartList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const z::Ast::StructInitPart& part = it->get();
                _os << part.vdef().name().string() << ":";
                visitNode(part.expr());
                _os << ";";
            }
            _os << ")";
        }

        inline void visitFunctionTypeInstance(const z::Ast::Function& function) {
            z::string fname = z::ZenlangNameGenerator().tn(function);
            _os << fname << "(";
            z::string sep;
            for(z::Ast::Scope::List::const_iterator it = function.xref().begin(); it != function.xref().end(); ++it) {
                const z::Ast::VariableDefn& vref = it->get();
                _os << sep << vref.name().string();
                sep = ", ";
            }
            _os << ")";
        }

        virtual void visit(const z::Ast::FunctionInstanceExpr& node) {
            visitFunctionTypeInstance(node.function());
        }

        virtual void visit(const z::Ast::AnonymousFunctionExpr& node) {
            visitFunctionTypeInstance(node.function());
        }

        virtual void visit(const z::Ast::ConstantNullExpr& node) {
            unused(node);
            _os << "null";
        }

        virtual void visit(const z::Ast::ConstantFloatExpr& node) {
            _os << node.value();
        }

        virtual void visit(const z::Ast::ConstantDoubleExpr& node) {
            _os << node.value();
        }

        virtual void visit(const z::Ast::ConstantBooleanExpr& node) {
            _os << ((node.value() == true)?"true":"false");
        }

        virtual void visit(const z::Ast::ConstantStringExpr& node) {
            _os << "\"" << node.value() << "\"";
        }

        virtual void visit(const z::Ast::ConstantCharExpr& node) {
            _os << "\'" << node.value() << "\'";
        }

        virtual void visit(const z::Ast::ConstantLongExpr& node) {
            _os << node.value() << "l";
        }

        virtual void visit(const z::Ast::ConstantIntExpr& node) {
            _os << node.value();
        }

        virtual void visit(const z::Ast::ConstantShortExpr& node) {
            _os << node.value() << "s";
        }

        virtual void visit(const z::Ast::ConstantByteExpr& node) {
            _os << node.value() << "b";
        }

        virtual void visit(const z::Ast::ConstantUnLongExpr& node) {
            _os << node.value() << "ul";
        }

        virtual void visit(const z::Ast::ConstantUnIntExpr& node) {
            _os << node.value() << "u";
        }

        virtual void visit(const z::Ast::ConstantUnShortExpr& node) {
            _os << node.value() << "us";
        }

        virtual void visit(const z::Ast::ConstantUnByteExpr& node) {
            _os << node.value() << "ub";
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

    void runStatementGenerator(const z::Ast::Config& config, z::ofile& fp, const z::Ast::Statement& block);

    struct ImportGenerator : public z::Ast::TypeSpec::Visitor {
        inline bool canWrite(const z::Ast::AccessType::T& accessType) const {
            return ((accessType == z::Ast::AccessType::Public) || (accessType == z::Ast::AccessType::Parent));
        }

        inline void visitChildrenIndent(const z::Ast::TypeSpec& node) {
            visitChildren(node);
        }

        void visit(const z::Ast::TypedefDecl& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "typedef " << node.name() << getDefinitionType(node.pos(), node.defType()) << std::endl;
            }
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::TypedefDefn& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType())
                      << "typedef " << node.name()
                      << getDefinitionType(node.pos(), node.defType())
                      << " " << z::ZenlangNameGenerator().qtn(node.qTypeSpec())
                      << ";" << std::endl;
            }
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::TemplateDecl& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "template <";
                z::string sep;
                for(z::Ast::TemplatePartList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const z::Ast::Token& token = *it;
                    _os() << sep << token << std::endl;
                    sep = ", ";
                }
                _os() << "> " << node.name() << getDefinitionType(node.pos(), node.defType()) << ";" << std::endl;
            }
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::TemplateDefn& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::EnumDecl& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "enum " << node.name() << getDefinitionType(node.pos(), node.defType()) << ";" << std::endl;
            }
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::EnumDefn& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "enum " << node.name() << getDefinitionType(node.pos(), node.defType()) << " {" << std::endl;
                for(z::Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const z::Ast::VariableDefn& def = it->get();
                    _os() << "    " << def.name();
                    const z::Ast::ConstantIntExpr* cexpr = dynamic_cast<const z::Ast::ConstantIntExpr*>(z::ptr(def.initExpr()));
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

        inline void visitStructDefn(const z::Ast::StructDefn& node, const z::Ast::StructDefn* base) {
            if(node.accessType() == z::Ast::AccessType::Protected) {
                _os() << getAccessType(node.pos(), node.accessType()) << "struct " << node.name() << ";" << std::endl;
                return;
            }
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "struct " << node.name();
                if(base) {
                    _os() << " : " << z::ZenlangNameGenerator().tn(z::ref(base));
                }
                _os() << getDefinitionType(node.pos(), node.defType()) << " {" << std::endl;
                runStatementGenerator(_config, _os, node.block());
                _os() << "};" << std::endl;
            }
        }

        void visit(const z::Ast::StructDecl& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "struct " << node.name() << getDefinitionType(node.pos(), node.defType()) << ";" << std::endl;
            }
        }

        void visit(const z::Ast::RootStructDefn& node) {
            visitStructDefn(node, 0);
        }

        void visit(const z::Ast::ChildStructDefn& node) {
            visitStructDefn(node, z::ptr(node.base()));
        }

        void visit(const z::Ast::PropertyDeclRW& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType())
                      << "property " << z::ZenlangNameGenerator().qtn(node.qTypeSpec())
                      << " " << node.name()
                      << " " << getDefinitionType(node.pos(), node.defType())
                      << " get set;" << std::endl;
            }
        }

        void visit(const z::Ast::PropertyDeclRO& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType())
                      << "property " << z::ZenlangNameGenerator().qtn(node.qTypeSpec())
                      << " " << node.name()
                      << " " << getDefinitionType(node.pos(), node.defType())
                      << " get;" << std::endl;
            }
        }

        inline void visitRoutineImp(const z::Ast::Routine& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType()) << "routine " << z::ZenlangNameGenerator().qtn(node.outType()) << " ";
                _os() << node.name();
                _os() << "(";
                z::string sep;
                for(z::Ast::Scope::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
                    const z::Ast::VariableDefn& vdef = it->get();
                    _os() << sep << z::ZenlangNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name();
                    sep = ", ";
                }
                _os() << ")" << getDefinitionType(node.pos(), node.defType()) << ";" << std::endl;
            }
        }

        void visit(const z::Ast::RoutineDecl& node) {
            visitRoutineImp(node);
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::RoutineDefn& node) {
            visitRoutineImp(node);
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::FunctionRetn& node) {
            visitChildrenIndent(node);
        }

        inline void visitFunctionImp(const z::Ast::Function& node, const z::string& name, const bool& isEvent) {
            if((name.size() > 0) || (canWrite(node.accessType()))) {
                if(!isEvent) {
                    _os() << getAccessType(node.pos(), node.accessType());
                }

                _os() << "function ";
                if(node.sig().outScope().isTuple()) {
                    _os() << "(";
                    z::string sep;
                    for(z::Ast::Scope::List::const_iterator it = node.sig().out().begin(); it != node.sig().out().end(); ++it) {
                        const z::Ast::VariableDefn& vdef = it->get();
                        _os() << sep << z::ZenlangNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name();
                        sep = ", ";
                    }
                    _os() << ")";
                } else {
                    const z::Ast::VariableDefn& vdef = node.sig().out().front();
                    _os() << z::ZenlangNameGenerator().qtn(vdef.qTypeSpec()) << " ";
                }

                if(name.size() > 0) {
                    _os() << name << "(";
                } else {
                    _os() << node.name() << "(";
                }
                z::string sep = "";
                for(z::Ast::Scope::List::const_iterator it = node.sig().in().begin(); it != node.sig().in().end(); ++it) {
                    const z::Ast::VariableDefn& vdef = it->get();
                    _os() << sep << z::ZenlangNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name();
                    sep = ", ";
                }
                _os() << ")" << getDefinitionType(node.pos(), node.defType()) << ";" << std::endl;
            }
        }

        void visit(const z::Ast::RootFunctionDecl& node) {
            visitFunctionImp(node, "", false);
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::ChildFunctionDecl& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType());
                _os() << "function " << node.name();
                _os() << " : " << z::ZenlangNameGenerator().tn(node.base());
                _os() << getDefinitionType(node.pos(), node.defType()) << ";" << std::endl;
            }

            visitChildrenIndent(node);
        }

        void visit(const z::Ast::RootFunctionDefn& node) {
            visitFunctionImp(node, "", false);
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::ChildFunctionDefn& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::EventDecl& node) {
            if(canWrite(node.accessType())) {
                _os() << getAccessType(node.pos(), node.accessType())
                      << "event(" << z::ZenlangNameGenerator().qtn(node.in().qTypeSpec())
                      << " " << node.in().name() << ")" << getDefinitionType(node.pos(), node.defType()) << " => ";
                visitFunctionImp(node.handler(), node.name().string(), true);
            }
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::Namespace& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::Root& node) {
            visitChildrenIndent(node);
        }

    public:
        inline ImportGenerator(const z::Ast::Config& config, z::ofile& os) : _config(config), _os(os) {}

    private:
        const z::Ast::Config& _config;
        z::ofile& _os;
    };

    struct StatementGenerator : public z::Ast::Statement::Visitor {
    private:
        virtual void visit(const z::Ast::ImportStatement& node) {
            if(node.headerType() == z::Ast::HeaderType::Import) {
                _os() << "import ";
            } else {
                _os() << "include ";
            }

            z::string sep = "";
            for(z::Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const z::Ast::Token& name = it->get().name();
                _os() << sep << name;
                sep = "::";
            }

            if(node.defType() == z::Ast::DefinitionType::Native) {
                _os() << " native";
            }

            _os() << ";" << std::endl;
        }

        virtual void visit(const z::Ast::EnterNamespaceStatement& node) {
            z::string fqn;
            z::string sep;
            for(z::Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const z::Ast::Namespace& ns = it->get();
                fqn += sep;
                fqn += ns.name().string();
                sep = "::";
            }
            if(fqn.size() > 0) {
                _os() << "namespace " << fqn << ";" << std::endl;
                _os() << std::endl;
            }
        }

        virtual void visit(const z::Ast::LeaveNamespaceStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::UserDefinedTypeSpecStatement& node) {
            ImportGenerator(_config, _os).visitNode(node.typeSpec());
        }

        virtual void visit(const z::Ast::StructMemberVariableStatement& node) {
            _os() << z::Indent::get() << z::ZenlangNameGenerator().qtn(node.defn().qTypeSpec()) << " " << node.defn().name() << " = ";
            ExprGenerator(_os()).visitNode(node.defn().initExpr());
            _os() << ";" << std::endl;
        }

        virtual void visit(const z::Ast::StructInitStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::EmptyStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::AutoStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::ExprStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::PrintStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::IfStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::IfElseStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::WhileStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::DoWhileStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::ForExprStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::ForInitStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::ForeachStringStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::ForeachListStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::ForeachDictStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::CaseExprStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::CaseDefaultStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::SwitchValueStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::SwitchExprStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::BreakStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::ContinueStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::AddEventHandlerStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::RoutineReturnStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::FunctionReturnStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::RaiseStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::ExitStatement& node) {
            unused(node);
        }

        virtual void visit(const z::Ast::CompoundStatement& node) {
            INDENT;
            for(z::Ast::CompoundStatement::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const z::Ast::Statement& s = it->get();
                z::ref(this).visitNode(s);
            }
        }

    public:
        inline StatementGenerator(const z::Ast::Config& config, z::ofile& os) : _config(config), _os(os) {}
    private:
        const z::Ast::Config& _config;
        z::ofile& _os;
    };

    void runStatementGenerator(const z::Ast::Config& config, z::ofile& os, const z::Ast::Statement& block) {
        StatementGenerator gen(config, os);
        gen.visitNode(block);
    }
}

struct z::ZenlangGenerator::Impl {
    inline Impl(const z::Ast::Project& project, const z::Ast::Config& config, const z::Ast::Module& module) : _project(project), _config(config), _module(module) {}
    inline void run();
private:
    const z::Ast::Project& _project;
    const z::Ast::Config& _config;
    const z::Ast::Module& _module;
};

inline void z::ZenlangGenerator::Impl::run() {
    Indent::init();
    z::string basename = z::dir::getBaseName(_module.filename());
    z::dir::mkpath(_config.apidir() + "/");
    z::ofile osImp(_config.apidir() + "/" + basename + ".ipp");

    for(z::Ast::CompoundStatement::List::const_iterator it = _module.globalStatementList().list().begin(); it != _module.globalStatementList().list().end(); ++it) {
        const z::Ast::Statement& s = it->get();
        zg::runStatementGenerator(_config, osImp, s);
    }
}

//////////////////////////////////////////////
z::ZenlangGenerator::ZenlangGenerator(const z::Ast::Project& project, const z::Ast::Config& config, const z::Ast::Module& module) : _impl(0) {_impl = new Impl(project, config, module);}
z::ZenlangGenerator::~ZenlangGenerator() {delete _impl;}
void z::ZenlangGenerator::run() {return z::ref(_impl).run();}

z::string z::ZenlangGenerator::convertExprToString(const z::Ast::Expr& expr) {
    std::stringstream os;
    zg::ExprGenerator(os).visitNode(expr);
    return z::e2s(os.str());
}
