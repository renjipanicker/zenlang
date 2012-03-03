#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "base/typename.hpp"
#include "StlcppGenerator.hpp"

struct StlcppNameGenerator : public TypespecNameGenerator {
    virtual void getTypeName(const Ast::TypeSpec& typeSpec, z::string& name);
public:
    inline StlcppNameGenerator(const z::string& sep = "::") : TypespecNameGenerator(sep) {}
};

void StlcppNameGenerator::getTypeName(const Ast::TypeSpec& typeSpec, z::string& name) {
    const Ast::TemplateDefn* templateDefn = dynamic_cast<const Ast::TemplateDefn*>(z::ptr(typeSpec));
    if(templateDefn) {
        if(z::ref(templateDefn).name().string() == "ptr") {
            const Ast::QualifiedTypeSpec& qTypeSpec = z::ref(templateDefn).list().front();
            if(qTypeSpec.isConst()) {
                name += "const ";
            }
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
//@        } else if(z::ref(templateDefn).name().string() == "functor") {
//            name += "z::FunctorT";
        } else {
            name += z::ref(templateDefn).name().string();
        }

        name += "<";
        z::string sep;
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

    if(typeSpec.name().string() == "size") {
        name += "z::size";
        return;
    }

    if(typeSpec.name().string() == "char") {
        name += "z::char_t";
        return;
    }

    if(typeSpec.name().string() == "string") {
        name += "z::string";
        return;
    }

    if(typeSpec.name().string() == "datetime") {
        name += "z::datetime";
        return;
    }

    if(typeSpec.name().string() == "type") {
        name += "z::type";
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
        inline FileSet(z::ofile& osHdr, z::ofile& osSrc) : _osHdr(osHdr), _osSrc(osSrc) {}
        z::ofile& _osHdr;
        z::ofile& _osSrc;
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
        void run(const Ast::Config& config, FileSet& fs, const Ast::Statement& block);
    };

    struct ExprGenerator : public Ast::Expr::Visitor {
    public:
        inline ExprGenerator(z::ofile& os, const z::string& sep2 = "", const z::string& sep1 = "") : _os(os), _sep2(sep2), _sep1(sep1), _sep0(sep1) {}
    private:
        inline void visitTernary(const Ast::TernaryOpExpr& node) {
            _os() << "(";
            visitNode(node.lhs());
            _os() << node.op1();
            visitNode(node.rhs1());
            _os() << node.op2();
            visitNode(node.rhs2());
            _os() << ")";
        }

        virtual void visit(const Ast::ConditionalExpr& node) {
            return visitTernary(node);
        }

        virtual void visitBinary(const Ast::BinaryExpr& node) {
            visitNode(node.lhs());
            _os() << node.op();
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
            _os() << node.op();
        }

        virtual void visit(const Ast::PostfixIncExpr& node) {
            return visitPostfix(node);
        }

        virtual void visit(const Ast::PostfixDecExpr& node) {
            return visitPostfix(node);
        }

        inline void visitPrefix(const Ast::PrefixExpr& node) {
            _os() << node.op();
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
            _os() << ".set(";
            visitNode(node.lhs().index());
            _os() << ", ";
            visitNode(node.rhs());
            _os() << ")";
        }

        virtual void visit(const Ast::ListExpr& node) {
            _os() << "z::list<" << StlcppNameGenerator().qtn(node.list().valueType()) << ">::creator()";
            for(Ast::ListList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const Ast::ListItem& item = it->get();
                _os() << ".add(";
                visitNode(item.valueExpr());
                _os() << ")";
            }
            _os() << ".get()";
        }

        virtual void visit(const Ast::DictExpr& node) {
            _os() << "z::dict<" << StlcppNameGenerator().qtn(node.list().keyType()) << ", " << StlcppNameGenerator().qtn(node.list().valueType()) << ">::creator()";
            for(Ast::DictList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const Ast::DictItem& item = it->get();
                _os() << ".add(";
                visitNode(item.keyExpr());
                _os() << ", ";
                visitNode(item.valueExpr());
                _os() << ")";
            }
            _os() << ".get()";
        }

        virtual void visit(const Ast::FormatExpr& node) {
            _os() << "z::string(";
            visitNode(node.stringExpr());
            _os() << ")";
            for(Ast::DictList::List::const_iterator it = node.dictExpr().list().list().begin(); it != node.dictExpr().list().list().end(); ++it) {
                const Ast::DictItem& item = it->get();
                _os() << ".arg(";
                visitNode(item.keyExpr());
                _os() << ", ";
                visitNode(item.valueExpr());
                _os() << ")";
            }
        }

        virtual void visit(const Ast::RoutineCallExpr& node) {
            const z::string name = StlcppNameGenerator().tn(node.routine());
            if((name == "assert") || (name == "unused") || (name == "verify")) {
                z::string sep;
                for(Ast::ExprList::List::const_iterator it = node.exprList().list().begin(); it != node.exprList().list().end(); ++it) {
                    const Ast::Expr& expr = it->get();
                    _os() << sep;
                    if(name == "verify")
                        _os() << "if(!";
                    _os() << name << "(";
                    ExprGenerator(_os).visitNode(expr);
                    _os() << ")";
                    if(name == "verify")
                        _os() << ")return _Out(false)";
                    sep = ";";
                }
                return;
            }

            _os() << name << "(";
            ExprGenerator(_os, ", ").visitList(node.exprList());
            _os() << ")";
        }

        virtual void visit(const Ast::FunctorCallExpr& node) {
            ExprGenerator(_os).visitNode(node.expr());
            _os() << ".run(";
            ExprGenerator(_os, ", ").visitList(node.exprList());
            _os() << ")";
        }

        virtual void visit(const Ast::RunExpr& node) {
            _os() << "z::ctx().add(";
            ExprGenerator(_os).visitNode(node.callExpr().expr());
            _os() << ", ";
            _os() << StlcppNameGenerator().tn(node.callExpr().expr().qTypeSpec().typeSpec()) << "::_In(";
            ExprGenerator(_os, ", ").visitList(node.callExpr().exprList());
            _os() << "))";
        }

        virtual void visit(const Ast::OrderedExpr& node) {
            _os() << "(";
            visitNode(node.expr());
            _os() << ")";
        }

        virtual void visit(const Ast::IndexExpr& node) {
            z::string at = "at";

            const Ast::PropertyDecl* ts = resolveTypedefT<Ast::PropertyDecl>(node.expr().qTypeSpec().typeSpec());
            if(ts != 0) {
                at = z::ref(ts).name().string();
            }

            visitNode(node.expr());
            _os() << "." << at << "(";
            visitNode(node.index());
            _os() << ")";
        }

        virtual void visit(const Ast::SpliceExpr& node) {
            _os() << "splice(";
            visitNode(node.expr());
            _os() << ", ";
            visitNode(node.from());
            _os() << ", ";
            visitNode(node.to());
            _os() << ")";
        }

        virtual void visit(const Ast::TypeofTypeExpr& node) {
            _os() << "z::type(\"" << StlcppNameGenerator().qtn(node.typeSpec()) << "\")";
        }

        virtual void visit(const Ast::TypeofExprExpr& node) {
            const Ast::TypeSpec* typeSpec = z::ptr(node.expr().qTypeSpec().typeSpec());
            const Ast::TemplateDefn* td = dynamic_cast<const Ast::TemplateDefn*>(typeSpec);
            if(td) {
                visitNode(node.expr());
                _os() << ".tname()";
            } else {
                _os() << "z::type(\"" << StlcppNameGenerator().qtn(node.expr().qTypeSpec()) << "\")";
            }
        }

        virtual void visit(const Ast::StaticTypecastExpr& node) {
            _os() << "static_cast<" << StlcppNameGenerator().qtn(node.qTypeSpec()) << ">(";
            ExprGenerator(_os).visitNode(node.expr());
            _os() << ")";
        }

        virtual void visit(const Ast::DynamicTypecastExpr& node) {
            ExprGenerator(_os).visitNode(node.expr());
            _os() << ".getT<" << StlcppNameGenerator().tn(node.qTypeSpec().typeSpec()) << ">()";
        }

        virtual void visit(const Ast::PointerInstanceExpr& node) {
            const Ast::Expr& expr = node.exprList().at(0);
            const z::string dname = StlcppNameGenerator().tn(expr.qTypeSpec().typeSpec());
            const z::string bname = StlcppNameGenerator().tn(node.templateDefn().at(0).typeSpec());

            _os() << "z::pointer<" << bname << ">(\"" << dname << "\", ";
            ExprGenerator(_os).visitNode(expr);
            _os() << ")";
        }

        virtual void visit(const Ast::ValueInstanceExpr& node) {
            const Ast::Expr& expr = node.exprList().at(0);
            ExprGenerator(_os).visitNode(expr);
            _os() << ".getT<" << StlcppNameGenerator().tn(node.qTypeSpec().typeSpec()) << ">()";
        }

        virtual void visit(const Ast::VariableRefExpr& node) {
            switch(node.refType()) {
                case Ast::RefType::Global:
                    break;
                case Ast::RefType::XRef:
                    _os() << "z::ref(this)." << node.vref().name();
                    break;
                case Ast::RefType::Param:
                    _os() << node.vref().name();
                    break;
                case Ast::RefType::Local:
                    _os() << node.vref().name();
                    break;
            }
        }

        virtual void visit(const Ast::MemberVariableExpr& node) {
            visitNode(node.expr());
            _os() << "." << node.vref().name();
        }

        virtual void visit(const Ast::MemberPropertyExpr& node) {
            visitNode(node.expr());
            _os() << "._" << node.pref().name() << "()";
        }

        virtual void visit(const Ast::EnumMemberExpr& node) {
            _os() << ZenlangNameGenerator().tn(node.typeSpec());
            _os() << "::" << node.vref().name();
        }

        virtual void visit(const Ast::StructMemberExpr& node) {
            _os() << ZenlangNameGenerator().tn(node.typeSpec());
            _os() << "::" << node.vref().name();
        }

        virtual void visit(const Ast::StructInstanceExpr& node) {
            _os() << StlcppNameGenerator().tn(node.structDefn()) << "()";
            for(Ast::StructInitPartList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const Ast::StructInitPart& part = it->get();
                _os() << "._" << part.vdef().name() << "<" << StlcppNameGenerator().tn(node.structDefn()) << ">(";
                visitNode(part.expr());
                _os() << ")";
            }
        }

        inline void visitFunctionTypeInstance(const Ast::Function& function, const Ast::ExprList& exprList) {
            unused(exprList); // will be implementing function-type-instantiation with ctor parameters in future.
            z::string fname = StlcppNameGenerator().tn(function);
            _os() << fname << "(";
            z::string sep;
            for(Ast::Scope::List::const_iterator it = function.xref().begin(); it != function.xref().end(); ++it) {
                const Ast::VariableDefn& vref = it->get();
                _os() << sep << vref.name();
                sep = ", ";
            }
            _os() << ")";
        }

        virtual void visit(const Ast::FunctionInstanceExpr& node) {
            visitFunctionTypeInstance(node.function(), node.exprList());
        }

        virtual void visit(const Ast::AnonymousFunctionExpr& node) {
            visitFunctionTypeInstance(node.function(), node.exprList());
        }

        virtual void visit(const Ast::ConstantNullExpr& node) {
            unused(node);
            _os() << "0";
        }

        virtual void visit(const Ast::ConstantFloatExpr& node) {
            _os() << node.value();
        }

        virtual void visit(const Ast::ConstantDoubleExpr& node) {
            _os() << node.value();
        }

        virtual void visit(const Ast::ConstantBooleanExpr& node) {
            _os() << (node.value()?"true":"false");
        }

        virtual void visit(const Ast::ConstantStringExpr& node) {
            _os() << "\"" << node.value() << "\"";
        }

        virtual void visit(const Ast::ConstantCharExpr& node) {
            _os() << "\'" << node.value() << "\'";
        }

        virtual void visit(const Ast::ConstantLongExpr& node) {
            _os() << node.value();
        }

        virtual void visit(const Ast::ConstantIntExpr& node) {
            _os() << node.value();
        }

        virtual void visit(const Ast::ConstantShortExpr& node) {
            _os() << node.value();
        }

        virtual void sep() {
            _os() << _sep0;
            _sep0 = _sep2;
        }

    private:
        z::ofile& _os;
        const z::string _sep2;
        const z::string _sep1;
        z::string _sep0;
    };

    struct TypeDeclarationGenerator : public Ast::TypeSpec::Visitor {
        inline void visitChildrenIndent(const Ast::TypeSpec& node) {
            INDENT;
            visitChildren(node);
        }

        void visit(const Ast::TypedefDecl& node) {
            if(node.defType() == Ast::DefinitionType::Native) {
                _os() << Indent::get() << "// typedef " << node.name() << " native;" << std::endl;
            } else {
                throw z::Exception("StlcppGenerator", zfmt(node.pos(), "Internal error: '%{s}'").arg("s", node.name()) );
            }
        }

        void visit(const Ast::TypedefDefn& node) {
            if(node.defType() != Ast::DefinitionType::Native) {
                _os() << Indent::get() << "typedef " << StlcppNameGenerator().qtn(node.qTypeSpec()) << " " << node.name() << ";" << std::endl;
            }
        }

        void visit(const Ast::TemplateDecl& node) {
            if(node.defType() != Ast::DefinitionType::Native) {
                throw z::Exception("StlcppGenerator", zfmt(node.pos(), "Internal error: template declaration cannot be generated '%{s}'").arg("s", node.name()) );
            }
        }

        void visit(const Ast::TemplateDefn& node) {
            throw z::Exception("StlcppGenerator", zfmt(node.pos(), "Internal error: template definition cannot be generated '%{s}'").arg("s", node.name()) );
        }

        void visit(const Ast::EnumDecl& node) {
            if(node.defType() != Ast::DefinitionType::Native) {
                throw z::Exception("StlcppGenerator", zfmt(node.pos(), "Internal error: enum definition cannot be generated '%{s}'").arg("s", node.name()) );
            }
        }

        void visit(const Ast::EnumDefn& node) {
            if(node.defType() != Ast::DefinitionType::Native) {
                _os() << Indent::get() << "struct " << node.name() << " {" << std::endl;
                _os() << Indent::get() << "  enum T {" << std::endl;
                z::string sep = " ";
                for(Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    INDENT;
                    const Ast::VariableDefn& def = it->get();
                    _os() << Indent::get() << sep << " " << def.name();
                    const Ast::ConstantIntExpr* cexpr = dynamic_cast<const Ast::ConstantIntExpr*>(z::ptr(def.initExpr()));
                    if((cexpr == 0) || (z::ref(cexpr).pos().string() != "#")) { /// \todo hack
                        _os() << " = ";
                        ExprGenerator(_os).visitNode(def.initExpr());
                    }
                    _os() << std::endl;
                    sep = ",";
                }

                _os() << Indent::get() << "  };" << std::endl;
                _os() << Indent::get() << "};" << std::endl;
                _os() << std::endl;
            }
        }

        inline void visitStructDefn(const Ast::StructDefn& node, const Ast::StructDefn* base) {
            if(node.defType() == Ast::DefinitionType::Native) {
                _os() << Indent::get() << "struct " << node.name() << ";" << std::endl;
                return;
            }

            _os() << Indent::get() << "struct " << node.name();
            if(base) {
                _os() << " : public " << StlcppNameGenerator().tn(z::ref(base));
            }

            _os() << " {" << std::endl;

            // if abstract type, generate virtual dtor
            if(node.defType() == Ast::DefinitionType::Abstract) {
                _os() << Indent::get() << "    virtual ~" << node.name() << "() {}" << std::endl;
            }

            GeneratorContext(GeneratorContext::TargetMode::TypeDecl, GeneratorContext::IndentMode::NoBrace).run(_config, _fs, node.block());

            _os() << Indent::get() << "};" << std::endl;
            _os() << std::endl;
        }

        void visit(const Ast::StructDecl& node) {
            _os() << Indent::get() << "struct " << node.name() << ";" << std::endl;
        }

        void visit(const Ast::RootStructDefn& node) {
            visitStructDefn(node, 0);
        }

        void visit(const Ast::ChildStructDefn& node) {
            visitStructDefn(node, z::ptr(node.base()));
        }

        inline void visitProperty(const Ast::PropertyDecl& node) {
            z::string cnst;
            if(node.qTypeSpec().isConst())
                cnst = "const ";
            _os() << Indent::get() << cnst << StlcppNameGenerator().qtn(node.qTypeSpec()) << "& _" << node.name() << "() const;" << std::endl;
        }

        void visit(const Ast::PropertyDeclRW& node) {
            visitProperty(node);
            _os() << Indent::get() << "void _" << node.name() << "(const " << StlcppNameGenerator().tn(node.qTypeSpec().typeSpec()) << "& val);" << std::endl;
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

        static inline void visitRoutine(z::ofile& os, const Ast::Routine& node, const bool& inNS) {
            os() << Indent::get() << StlcppNameGenerator().qtn(node.outType()) << " ";
            if(inNS) {
                os() << node.name();
            } else {
                os() << StlcppNameGenerator().tn(node);
            }
            os() << "(";
            z::string sep;
            for(Ast::Scope::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
                const Ast::VariableDefn& vdef = it->get();
                os() << sep << StlcppNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name();
                sep = ", ";
            }
            os() << ")";
        }

        inline void writeScopeMemberList(const Ast::Scope& scope) {
            for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                INDENT;
                const Ast::VariableDefn& vdef = it->get();
                _os() << Indent::get() << StlcppNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name() << ";" << std::endl;
            }
            if(scope.hasPosParam()) {
                const Ast::Scope& posParam = scope.posParam();
                for(Ast::Scope::List::const_iterator it = posParam.list().begin(); it != posParam.list().end(); ++it) {
                    INDENT;
                    const Ast::VariableDefn& vdef = it->get();
                    _os() << Indent::get() << StlcppNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name() << ";" << std::endl;
                }
            }
        }

        static inline void writeScopeParamList(z::ofile& os, const Ast::Scope& scope, const z::string& prefix) {
            z::string sep = "";
            for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                const Ast::VariableDefn& vdef = it->get();
                os() << sep << " " << StlcppNameGenerator().qtn(vdef.qTypeSpec()) << " " << prefix << vdef.name();
                sep = ", ";
            }
        }

        inline void writeScopeInCallList(const Ast::Scope& scope) {
            z::string sep = "";
            for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                const Ast::VariableDefn& vdef = it->get();
                _os() << sep << "_in." << vdef.name();
                sep = ", ";
            }
        }

        inline void writeCtor(const z::string& cname, const Ast::Scope& scope) {
            _os() << Indent::get() << "    inline " << cname << "(";
            writeScopeParamList(_os, scope, "p");
            _os() << ")";

            z::string sep = " : ";
            for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                const Ast::VariableDefn& vdef = it->get();
                _os() << sep << vdef.name() << "(p" << vdef.name() << ")";
                sep = ", ";
            }

            if(scope.hasPosParam()) {
                const Ast::Scope& posParam = scope.posParam();
                unused(posParam);
                /// \todo implement positional-param
            }

            _os() << " {}" << std::endl;
        }

        inline z::string getOutType(const Ast::Function& node) const {
            z::string out1;
            if(isVoid(node.sig().outScope())) {
                out1 = "void";
            } else if(node.sig().outScope().isTuple()) {
//                out1 = "const _Out&";
                out1 = "_Out";
            } else {
                const Ast::VariableDefn& vdef = node.sig().out().front();
                out1 = StlcppNameGenerator().qtn(vdef.qTypeSpec());
            }
            return out1;
        }

        inline void visitFunctionXRef(const Ast::Function& node) {
            if((node.xref().size() == 0) && (node.iref().size() == 0)) {
                return;
            }

            if(node.xref().size() > 0) {
                _os() << Indent::get() << "public: // xref-list" << std::endl;
                writeScopeMemberList(node.xrefScope());
            }
            if(node.iref().size() > 0) {
                _os() << Indent::get() << "public: // iref-list" << std::endl;
                writeScopeMemberList(node.irefScope());
            }
            writeCtor(node.name().string(), node.xrefScope());
        }

        inline void visitFunctionSig(const Ast::Function& node, const bool& isRoot, const bool& isDecl, const bool& isTest) {
            if(node.childCount() > 0) {
                _os() << Indent::get() << "public:// child-typespec" << std::endl;
                visitChildrenIndent(node);
            }

            visitFunctionXRef(node);

            if(isRoot) {
                _os() << Indent::get() << "public: // in-param-list" << std::endl;
                INDENT;
                _os() << Indent::get() << "struct _In {" << std::endl;
                writeScopeMemberList(node.sig().inScope());
                writeCtor("_In", node.sig().inScope());
                _os() << Indent::get() << "};" << std::endl;
            }

            z::string out1 = getOutType(node);
            _os() << Indent::get() << "public: // run-function" << std::endl;
            if(isTest) {
                _os() << Indent::get() << "    " << out1 << " run();" << std::endl;
            } else {
                if((isDecl) && ((node.defType() == Ast::DefinitionType::Final) || (node.defType() == Ast::DefinitionType::Abstract))) {
                    _os() << Indent::get() << "    virtual " << out1 << " run(";
                    writeScopeParamList(_os, node.sig().inScope(), "p");
                    _os() << ") = 0;" << std::endl;
                } else {
                    _os() << Indent::get() << "    " << out1 << " run(";
                    writeScopeParamList(_os, node.sig().inScope(), "p");
                    _os() << ");" << std::endl;
                }

                _os() << Indent::get() << "    inline _Out _run(const _In& _in) {";
                if(node.sig().in().size() == 0) {
                    _os() << "unused(_in);";
                }

                // if void function, call the function and return default instance of _Out()
                if(isVoid(node.sig().outScope())) {
                    _os() << "run(";
                    writeScopeInCallList(node.sig().inScope());
                    _os() << "); return _Out();" << std::endl;
                } else {
                    // if non-void function, return the return-value of run() as-is.
                    _os() << "return run(";
                    writeScopeInCallList(node.sig().inScope());
                    _os() << ");" << std::endl;
                }
                _os() << "}" << std::endl;
            }
        }

        inline void visitFunction(const Ast::Function& node, const bool isDecl) {
            _os() << Indent::get() << "class " << node.name() << " {" << std::endl;
            visitFunctionSig(node, true, isDecl, false);
            _os() << Indent::get() << "};" << std::endl;
        }

        void visit(const Ast::RoutineDecl& node) {
            visitRoutine(_os, node, true);
            _os() << ";" << std::endl;
            _os() << std::endl;
        }

        void visit(const Ast::RoutineDefn& node) {
            visitRoutine(_os, node, true);
            _os() << ";" << std::endl;
            _os() << std::endl;
        }

        void visit(const Ast::FunctionRetn& node) {
            _os() << Indent::get() << "struct _Out {" << std::endl;

            if(!isVoid(node.outScope())) {
                // generate out parameters
                for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
                    INDENT;
                    const Ast::VariableDefn& vdef = it->get();
                    _os() << Indent::get() << StlcppNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name() << ";" << std::endl;
                }

                // generate out setter
                _os() << Indent::get() << "    inline " << node.name() << "(";
                z::string sep = "";
                for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
                    const Ast::VariableDefn& vdef = it->get();
                    _os() << sep << StlcppNameGenerator().qtn(vdef.qTypeSpec()) << " p" << vdef.name();
                    sep = ", ";
                }
                _os() << ")";
                sep = " : ";
                for(Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
                    const Ast::VariableDefn& vdef = it->get();
                    _os() << sep << vdef.name() << "(p" << vdef.name() << ")";
                    sep = ", ";
                }
                _os() << "{}" << std::endl;

            }

            // end return struct
            _os() << Indent::get() << "};" << std::endl;
            _os() << std::endl;
        }

        void visit(const Ast::RootFunctionDecl& node) {
            visitFunction(node, true);
            _os() << std::endl;
        }

        template<typename T>
        inline void enterFunction(const T& node) {
            if(StlcppNameGenerator().tn(node.base()) == "test") {
                _os() << Indent::get() << "class " << node.name() << " : public z::test_< " << node.name() << " > {" << std::endl;
                _os() << Indent::get() << "public:" << std::endl;
                _os() << Indent::get() << "    inline const char* name() const {return \"" << StlcppNameGenerator().tn(node) << "\";}" << std::endl;
            } else if(StlcppNameGenerator().tn(node.base()) == "main") {
                _os() << Indent::get() << "class " << node.name() << " : public z::main_< " << node.name() << " > {" << std::endl;
            } else {
                _os() << Indent::get() << "class " << node.name() << " : public " << StlcppNameGenerator().tn(node.base()) << " {" << std::endl;
            }
        }


        void visit(const Ast::ChildFunctionDecl& node) {
            if(node.defType() == Ast::DefinitionType::Native) {
                z::string out1 = getOutType(node);

                enterFunction(node);
                visitFunctionXRef(node);
                _os() << Indent::get() << "public:" << std::endl;
                _os() << Indent::get() << "    virtual " << out1 << " run(";
                writeScopeParamList(_os, node.sig().inScope(), "p");
                _os() << ");" << std::endl;
                if(StlcppNameGenerator().tn(node.base()) == "test") {
                    _os() << Indent::get() << "    static z::TestInstanceT<" << node.name() << "> s_test;" << std::endl;
                } else if(StlcppNameGenerator().tn(node.base()) == "main") {
                    _os() << Indent::get() << "    static z::MainInstanceT<" << node.name() << "> s_main;" << std::endl;
                }
                _os() << Indent::get() << "};" << std::endl;
            } else {
                _os() << Indent::get() << "class " << node.name() << ";" << std::endl;
            }
            _os() << std::endl;
        }

        void visit(const Ast::RootFunctionDefn& node) {
            visitFunction(node, false);
            _os() << std::endl;
        }

        void visit(const Ast::ChildFunctionDefn& node) {
            bool isTest = (StlcppNameGenerator().tn(node.base()) == "test");
            if((isTest) && (!_config.test())) {
                return;
            }

            enterFunction(node);
            visitFunctionSig(node, false, false, isTest);

            if(StlcppNameGenerator().tn(node.base()) == "test") {
                _os() << Indent::get() << "    static z::TestInstanceT<" << node.name() << "> s_test;" << std::endl;
            } else if(StlcppNameGenerator().tn(node.base()) == "main") {
                _os() << Indent::get() << "    static z::MainInstanceT<" << node.name() << "> s_main;" << std::endl;
            }

            _os() << Indent::get() << "};" << std::endl;
            _os() << std::endl;
        }

        void visit(const Ast::EventDecl& node) {
            _os() << Indent::get() << "struct " << node.name() << " {" << std::endl;
            // child-typespecs
            if(node.childCount() > 0) {
                _os() << Indent::get() << "public:" << std::endl;
                visitChildrenIndent(node);
            }
            _os() << Indent::get() << "    z::FunctorList<Handler> _list;" << std::endl;
            _os() << Indent::get() << "    static " << node.name() << " instance;" << std::endl;
            _os() << Indent::get() << "    static inline Handler& add(Handler* h) {return instance._list.add(h);}" << std::endl;
            _os() << Indent::get() << "    static void addHandler(" << StlcppNameGenerator().qtn(node.in().qTypeSpec()) << " " << node.in().name() << ", Handler* h);" << std::endl;
            _os() << Indent::get() << "    template<typename T>static inline void addHandlerT("
                  << StlcppNameGenerator().qtn(node.in().qTypeSpec()) << " " << node.in().name() << ", T h) {return addHandler("
                  << node.in().name() << ", new T(h));}" << std::endl;
            _os() << Indent::get() << "};" << std::endl;
            _os() << std::endl;
            return;
        }

        void visit(const Ast::Namespace& node) {
            visitChildrenIndent(node);
        }

        void visit(const Ast::Root& node) {
            visitChildrenIndent(node);
        }

    public:
        inline TypeDeclarationGenerator(const Ast::Config& config, FileSet& fs, z::ofile& os) : _config(config), _fs(fs), _os(os) {}

    private:
        const Ast::Config& _config;
        FileSet& _fs;
        z::ofile& _os;
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

        void visit(const Ast::EnumDecl& node) {
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
            TypeDeclarationGenerator::visitRoutine(_os, node, false);
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
            _os() << std::endl;
        }

        void visit(const Ast::FunctionRetn& node) {
            visitChildrenIndent(node);
        }

        void visit(const Ast::RootFunctionDecl& node) {
            visitChildrenIndent(node);
        }

        template <typename T>
        inline void writeSpecialStatic(const T& node) {
            z::string fname = StlcppNameGenerator().tn(node);
            if((StlcppNameGenerator().tn(node.base()) == "test")) {
                _os() << "z::TestInstanceT<" << fname << "> " << fname << "::s_test = z::TestInstanceT<" << fname << ">();" << std::endl << std::endl;
            } else if(StlcppNameGenerator().tn(node.base()) == "main") {
                _os() << "z::MainInstanceT<" << fname << "> " << fname << "::s_main = z::MainInstanceT<" << fname << ">();" << std::endl << std::endl;
            }
        }

        void visit(const Ast::ChildFunctionDecl& node) {
            visitChildrenIndent(node);
            if(node.defType() == Ast::DefinitionType::Native) {
                writeSpecialStatic(node);
            }
        }

        inline void visitFunction(const Ast::FunctionDefn& node) {
            const z::string tname = StlcppNameGenerator().tn(node);
            z::string out;
            if(node.sig().outScope().isTuple()) {
                out = "" + tname + "::_Out";
            } else {
                const Ast::VariableDefn& vdef = node.sig().out().front();
                out = StlcppNameGenerator().qtn(vdef.qTypeSpec());
            }

            _os() << out << " " << tname << "::run(";
            TypeDeclarationGenerator::writeScopeParamList(_os, node.sig().inScope(), "");
            _os() << ") ";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
            _os() << std::endl;
        }

        void visit(const Ast::RootFunctionDefn& node) {
            visitChildrenIndent(node);
            visitFunction(node);
        }

        void visit(const Ast::ChildFunctionDefn& node) {
            bool isTest = (StlcppNameGenerator().tn(node.base()) == "test");
            if((isTest) && (!_config.test())) {
                return;
            }

            visitChildrenIndent(node);
            visitFunction(node);
            writeSpecialStatic(node);
        }

        void visit(const Ast::EventDecl& node) {
            visitChildrenIndent(node);
            _os() << StlcppNameGenerator().tn(node) << " " << StlcppNameGenerator().tn(node) << "::instance;" << std::endl;
            if(node.defType() == Ast::DefinitionType::Final) {
                _os() << "void " << StlcppNameGenerator().tn(node)
                      << "::addHandler(" << StlcppNameGenerator().qtn(node.in().qTypeSpec())
                      << " " << node.in().name() << ", Handler* h) {"
                      << std::endl;
                _os() << "}" << std::endl;
                _os() << "const " << StlcppNameGenerator().tn(node) << "::Add::_Out& " << StlcppNameGenerator().tn(node) << "::Add::run(";
                TypeDeclarationGenerator::writeScopeParamList(_os, node.addFunction().sig().inScope(), "");
                _os() << ") {" << std::endl;
                _os() << "    assert(false); //addHandler(in." << node.in().name() << ", in.handler);" << std::endl;
                _os() << "    return _Out();" << std::endl;
                _os() << "}" << std::endl;
            }
        }

        void visit(const Ast::Namespace& node) {
            visitChildrenIndent(node);
        }

        void visit(const Ast::Root& node) {
            visitChildrenIndent(node);
        }

    public:
        inline TypeDefinitionGenerator(const Ast::Config& config, FileSet& fs, z::ofile& os) : _config(config), _fs(fs), _os(os) {}
    private:
        const Ast::Config& _config;
        FileSet& _fs;
        z::ofile& _os;
    };

    struct StatementGenerator : public Ast::Statement::Visitor {
    private:
        inline z::ofile& fpDecl(const Ast::AccessType::T& accessType) const {
            return (accessType == Ast::AccessType::Private)?_fs._osSrc:_fs._osHdr;
        }

        inline z::ofile& fpDecl(const Ast::TypeSpec& node) const {
            if(node.accessType() == Ast::AccessType::Parent) {
                const Ast::ChildTypeSpec* child = dynamic_cast<const Ast::ChildTypeSpec*>(z::ptr(node));
                if(!child) {
                    throw z::Exception("StlcppGenerator", zfmt(node.pos(), "Internal error: Invalid access type in typespec"));
                }
                return fpDecl(z::ref(child).parent());
            }
            return (node.accessType() == Ast::AccessType::Private)?_fs._osSrc:_fs._osHdr;
        }

        inline z::ofile& fpDefn() const {
            return _fs._osSrc;
        }
    private:
        virtual void visit(const Ast::ImportStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                z::ofile& os = fpDecl(node.accessType());
                z::string qt = (node.headerType() == Ast::HeaderType::Import)?"<>":"\"\"";
                os() << "#include " << (char)qt.at(0);
                z::string sep = "";
                for(Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const Ast::Token& name = it->get().name();
                    os() << sep << name;
                    sep = "/";
                }
                os() << ".hpp" << (char)qt.at(1) << std::endl;
            }
        }

        virtual void visit(const Ast::EnterNamespaceStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                for(Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const Ast::Namespace& ns = it->get();
                    _fs._osHdr() << "namespace " << ns.name() << " {";
                    _fs._osSrc() << "namespace " << ns.name() << " {";
                }

                if(node.list().size() > 0) {
                    _fs._osHdr() << std::endl;
                    _fs._osSrc() << std::endl;
                }
            }
        }

        virtual void visit(const Ast::LeaveNamespaceStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                for(Ast::NamespaceList::List::const_reverse_iterator it = node.statement().list().rbegin(); it != node.statement().list().rend(); ++it) {
                    const Ast::Namespace& ns = it->get();
                    _fs._osHdr() << "} // " << ns.name();
                    _fs._osSrc() << "} // " << ns.name();
                }
                if(node.statement().list().size() > 0) {
                    _fs._osHdr() << std::endl;
                    _fs._osSrc() << std::endl;
                }
            }
        }

        virtual void visit(const Ast::UserDefinedTypeSpecStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                TypeDeclarationGenerator(_config, _fs, fpDecl(node.typeSpec())).visitNode(node.typeSpec());
            }

            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDefn) {
                TypeDefinitionGenerator(_config, _fs, _fs._osSrc).visitNode(node.typeSpec());
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
                z::string tname = StlcppNameGenerator().qtn(node.defn().qTypeSpec());
                z::string pname = "const " + tname + "&";
                if(isPtr(node.defn().qTypeSpec().typeSpec())) {
                    pname = tname;
                }
                fpDecl(node.structDefn())() << Indent::get() << tname << " " << node.defn().name() << "; ";
                fpDecl(node.structDefn())() << "template <typename T> inline T& _" << node.defn().name()
                                            << "(" << pname <<" val) {"
                                            << node.defn().name() << " = val; return z::ref(static_cast<T*>(this));}"
                                            << std::endl;
            }
        }

        virtual void visit(const Ast::StructInitStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                // default-ctor
                fpDecl(node.structDefn())() << Indent::get() << "explicit inline " << node.structDefn().name() << "()";
                z::string sep = " : ";
                for(Ast::Scope::List::const_iterator it = node.structDefn().list().begin(); it != node.structDefn().list().end(); ++it) {
                    const Ast::VariableDefn& vdef = it->get();
                    fpDecl(node.structDefn())() << sep << vdef.name() << "(";
                    ExprGenerator(fpDecl(node.structDefn())).visitNode(vdef.initExpr());
                    fpDecl(node.structDefn())() << ")";
                    sep = ", ";
                }
                fpDecl(node.structDefn())() << " {}" << std::endl;
            }
        }

        virtual void visit(const Ast::EmptyStatement& node) {
            unused(node);
            fpDefn()() << ";" << std::endl;
        }

        virtual void visit(const Ast::AutoStatement& node) {
            fpDefn()() << Indent::get() << StlcppNameGenerator().qtn(node.defn().qTypeSpec()) << " " << node.defn().name() << " = ";
            ExprGenerator(fpDefn()).visitNode(node.defn().initExpr());
            fpDefn()() << ";" << std::endl;
        }

        virtual void visit(const Ast::ExprStatement& node) {
            fpDefn()() << Indent::get();
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ";" << std::endl;
        }

        virtual void visit(const Ast::PrintStatement& node) {
            fpDefn()() << Indent::get() << "z::mlog(z::string(\"%{s}\\n\").arg(\"s\", ";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << "));" << std::endl;
        }

        virtual void visit(const Ast::IfStatement& node) {
            fpDefn()() << Indent::get() << "if(";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ") ";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.tblock());
        }

        virtual void visit(const Ast::IfElseStatement& node) {
            fpDefn()() << Indent::get() << "if(";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ") ";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.tblock());
            fpDefn()() << Indent::get() << "else";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.fblock());
        }

        virtual void visit(const Ast::WhileStatement& node) {
            fpDefn()() << Indent::get() << "while(";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ") ";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
        }

        virtual void visit(const Ast::DoWhileStatement& node) {
            fpDefn()() << Indent::get() << "do";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
            fpDefn()() << Indent::get() << "while(";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ");" << std::endl;
        }

        virtual void visit(const Ast::ForExprStatement& node) {
            fpDefn()() << Indent::get() << "for(";
            ExprGenerator(fpDefn()).visitNode(node.init());
            fpDefn()() << "; ";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << "; ";
            ExprGenerator(fpDefn()).visitNode(node.incr());
            fpDefn()() << ") ";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
        }

        virtual void visit(const Ast::ForInitStatement& node) {
            fpDefn()() << Indent::get() << "for(" << StlcppNameGenerator().qtn(node.init().qTypeSpec())<< " " << node.init().name() << " = ";
            ExprGenerator(fpDefn()).visitNode(node.init().initExpr());
            fpDefn()() << "; ";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << "; ";
            ExprGenerator(fpDefn()).visitNode(node.incr());
            fpDefn()() << ") ";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
        }

        virtual void visit(const Ast::ForeachStringStatement& node) {
            const z::string etype = StlcppNameGenerator().qtn(node.expr().qTypeSpec());

            z::string constit = "";
            if(node.expr().qTypeSpec().isConst()) {
                constit = "const_";
            }

            fpDefn()() << Indent::get() << "{" << std::endl;
            fpDefn()() << Indent::get() << "  " << etype << " _str = ";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ";" << std::endl;

            fpDefn()() << Indent::get() << "  for(" << StlcppNameGenerator().tn(node.expr().qTypeSpec().typeSpec()) << "::" << constit << "iterator _it = _str.begin(); _it != _str.end(); ++_it) {" << std::endl;
            fpDefn()() << Indent::get() << "  " << StlcppNameGenerator().qtn(node.valDef().qTypeSpec()) << " " << node.valDef().name() << " = *_it;" << std::endl;

            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::IndentedBrace).run(_config, _fs, node.block());

            fpDefn()() << Indent::get() << "}" << std::endl;
            fpDefn()() << Indent::get() << "}" << std::endl;
        }

        virtual void visit(const Ast::ForeachListStatement& node) {
            const z::string etype = StlcppNameGenerator().qtn(node.expr().qTypeSpec());

            z::string constit = "";
            if(node.expr().qTypeSpec().isConst()) {
                constit = "const_";
            }

            fpDefn()() << Indent::get() << "{" << std::endl;
            fpDefn()() << Indent::get() << "  " << etype << " _list = ";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ";" << std::endl;
            fpDefn()() << Indent::get() << "  for(" << StlcppNameGenerator().tn(node.expr().qTypeSpec().typeSpec()) << "::" << constit << "iterator _it = _list.begin(); _it != _list.end(); ++_it) {" << std::endl;
            fpDefn()() << Indent::get() << "  " << StlcppNameGenerator().qtn(node.valDef().qTypeSpec()) << " " << node.valDef().name() << " = *_it;" << std::endl;

            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::IndentedBrace).run(_config, _fs, node.block());

            fpDefn()() << Indent::get() << "}" << std::endl;
            fpDefn()() << Indent::get() << "}" << std::endl;
        }

        virtual void visit(const Ast::ForeachDictStatement& node) {
            z::string constit = "";
            fpDefn()() << Indent::get() << "{" << std::endl;
            if(node.expr().qTypeSpec().isConst()) {
                constit = "const_";
            }
            fpDefn()() << Indent::get() << "  " << StlcppNameGenerator().qtn(node.expr().qTypeSpec()) << " _list = ";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ";" << std::endl;
            fpDefn()() << Indent::get() << "  for(" << StlcppNameGenerator().tn(node.expr().qTypeSpec().typeSpec()) << "::" << constit << "iterator _it = _list.begin(); _it != _list.end(); ++_it) {" << std::endl;
            fpDefn()() << Indent::get() << "  " << StlcppNameGenerator().qtn(node.keyDef().qTypeSpec()) << " " << node.keyDef().name() << " = _it->first;" << std::endl;
            fpDefn()() << Indent::get() << "  " << StlcppNameGenerator().qtn(node.valDef().qTypeSpec()) << " " << node.valDef().name() << " = _it->second;" << std::endl;

            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::IndentedBrace).run(_config, _fs, node.block());

            fpDefn()() << Indent::get() << "}" << std::endl;
            fpDefn()() << Indent::get() << "}" << std::endl;
        }

        virtual void visit(const Ast::CaseExprStatement& node) {
            fpDefn()() << Indent::get() << "case (";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ") :";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
        }

        virtual void visit(const Ast::CaseDefaultStatement& node) {
            fpDefn()() << Indent::get() << "default :";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
        }

        virtual void visit(const Ast::SwitchValueStatement& node) {
            fpDefn()() << Indent::get() << "switch(";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ")";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
        }

        virtual void visit(const Ast::SwitchExprStatement& node) {
            z::string ifstr = "if";
            for(Ast::CompoundStatement::List::const_iterator it = node.block().list().begin(); it != node.block().list().end(); ++it) {
                const Ast::Statement& s = it->get();
                const Ast::CaseExprStatement* ce = dynamic_cast<const Ast::CaseExprStatement*>(z::ptr(s));
                const Ast::CaseDefaultStatement* cd = dynamic_cast<const Ast::CaseDefaultStatement*>(z::ptr(s));
                if(ce) {
                    fpDefn()() << Indent::get() << ifstr << "(" << std::endl;
                    ExprGenerator(fpDefn()).visitNode(z::ref(ce).expr());
                    fpDefn()() << ")" << std::endl;
                    visitNode(z::ref(ce).block());
                    ifstr = "else if";
                } else if(cd) {
                    fpDefn()() << Indent::get() << "else" << std::endl;
                    visitNode(z::ref(cd).block());
                    break;
                } else {
                    throw z::Exception("StlcppGenerator", zfmt(node.pos(), "Internal error: not a case statement inside switch"));

                }
            }
        }

        virtual void visit(const Ast::BreakStatement& node) {
            unused(node);
            fpDefn()() << Indent::get() << "break;" << std::endl;
        }

        virtual void visit(const Ast::ContinueStatement& node) {
            unused(node);
            fpDefn()() << Indent::get() << "continue;" << std::endl;
        }

        virtual void visit(const Ast::AddEventHandlerStatement& node) {
            z::string ename = StlcppNameGenerator().tn(node.event());
            fpDefn()() << Indent::get() << ename << "::addHandlerT(";
            ExprGenerator(fpDefn()).visitNode(node.source());
            fpDefn()() << ", ";
            ExprGenerator(fpDefn()).visitNode(node.functor());
            fpDefn()() << ");";
        }

        virtual void visit(const Ast::RoutineReturnStatement& node) {
            fpDefn()() << Indent::get() << "return";
            if(node.exprList().list().size() > 0) {
                fpDefn()() << " (";
                ExprGenerator(fpDefn(), ", ").visitList(node.exprList());
                fpDefn()() << ")";
            }
            fpDefn()() << ";" << std::endl;
        }

        virtual void visit(const Ast::FunctionReturnStatement& node) {
            const z::string out = node.sig().outScope().isTuple()?"_Out":"";
            fpDefn()() << Indent::get() << "return " << out << "(";
            ExprGenerator(fpDefn(), ", ").visitList(node.exprList());
            fpDefn()() << ");" << std::endl;
        }

        virtual void visit(const Ast::CompoundStatement& node) {
            if(_ctx._indentMode == GeneratorContext::IndentMode::IndentedBrace) {
                fpDefn()() << Indent::get();
            }

            if((_ctx._indentMode == GeneratorContext::IndentMode::IndentedBrace) || (_ctx._indentMode == GeneratorContext::IndentMode::WithBrace)) {
                fpDefn()() << "{" << std::endl;
            }

            {
                INDENT;
                for(Ast::CompoundStatement::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const Ast::Statement& s = it->get();
                    z::ref(this).visitNode(s);
                }
            }

            if((_ctx._indentMode == GeneratorContext::IndentMode::IndentedBrace) || (_ctx._indentMode == GeneratorContext::IndentMode::WithBrace)) {
                fpDefn()() << Indent::get() << "}" << std::endl;
            }
        }

    public:
        inline StatementGenerator(const Ast::Config& config, FileSet& fs, GeneratorContext& ctx) : _config(config), _fs(fs), _ctx(ctx) {}
    private:
        const Ast::Config& _config;
        FileSet& _fs;
        GeneratorContext& _ctx;
    };

    void GeneratorContext::run(const Ast::Config& config, FileSet& fs, const Ast::Statement& block) {
        StatementGenerator gen(config, fs, z::ref(this));
        gen.visitNode(block);
    }
}

struct StlcppGenerator::Impl {
    inline Impl(const Ast::Project& project, const Ast::Config& config, const Ast::Module& module) : _project(project), _config(config), _module(module) {}
    inline void run();
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
    const Ast::Module& _module;
};

inline void StlcppGenerator::Impl::run() {
    Indent::init();
    z::string basename = getBaseName(_module.filename());
    z::file::mkpath(_config.apidir() + "/");
    z::file::mkpath(_config.srcdir() + "/");
    z::ofile ofHdr(_config.apidir() + "/" + basename + ".hpp");
    z::ofile ofSrc(_config.srcdir() + "/" + basename + ".cpp");
    FileSet fs(ofHdr, ofSrc);

    ofHdr() << "#pragma once" << std::endl << std::endl;
    for(Ast::Config::PathList::const_iterator it = _config.includeFileList().begin(); it != _config.includeFileList().end(); ++it) {
        const z::string& filename = *it;
        ofSrc() << "#include \"" << filename << "\"" << std::endl;
    }
    ofSrc() << "#include \"" << ofHdr.name() << "\"" << std::endl;

    for(Ast::CompoundStatement::List::const_iterator it = _module.globalStatementList().list().begin(); it != _module.globalStatementList().list().end(); ++it) {
        const Ast::Statement& s = it->get();
        GeneratorContext(GeneratorContext::TargetMode::TypeDecl, GeneratorContext::IndentMode::WithBrace).run(_config, fs, s);
    }

    for(Ast::CompoundStatement::List::const_iterator it = _module.globalStatementList().list().begin(); it != _module.globalStatementList().list().end(); ++it) {
        const Ast::Statement& s = it->get();
        GeneratorContext(GeneratorContext::TargetMode::TypeDefn, GeneratorContext::IndentMode::WithBrace).run(_config, fs, s);
    }
    z::string fn = ofHdr.name();
    fn.replace("\\", "_");
    fn.replace(":", "_");
    fn.replace(".", "_");
    fn.replace("/", "_");
    fn.replace("-", "_");

    ofSrc() << std::endl;
    ofSrc() << "// Suppress LNK4221" << std::endl;
    ofSrc() << "#ifdef WIN32" << std::endl;
    ofSrc() << "int _" << fn << "_dummy = 0;" << std::endl;
    ofSrc() << "#endif" << std::endl;
}

//////////////////////////////////////////////
StlcppGenerator::StlcppGenerator(const Ast::Project& project, const Ast::Config& config, const Ast::Module& module) : _impl(0) {_impl = new Impl(project, config, module);}
StlcppGenerator::~StlcppGenerator() {delete _impl;}
void StlcppGenerator::run() {return z::ref(_impl).run();}
