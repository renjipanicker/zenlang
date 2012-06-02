#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/typename.hpp"
#include "base/StlcppGenerator.hpp"

namespace sg {
    struct StlcppNameGenerator : public z::TypespecNameGenerator {
        virtual void getTypeName(const z::Ast::TypeSpec& typeSpec, z::string& name);
    public:
        inline StlcppNameGenerator(const z::string& sep = "::") : TypespecNameGenerator(sep) {}
    };

    void StlcppNameGenerator::getTypeName(const z::Ast::TypeSpec& typeSpec, z::string& name) {
        const z::Ast::TemplateDefn* templateDefn = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(typeSpec));
        if(templateDefn) {
            if(z::ref(templateDefn).name().string() == "ptr") {
                const z::Ast::QualifiedTypeSpec& qTypeSpec = z::ref(templateDefn).list().front();
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
            for(z::Ast::TemplateTypePartList::List::const_iterator it = z::ref(templateDefn).list().begin(); it != z::ref(templateDefn).list().end(); ++it) {
                const z::Ast::QualifiedTypeSpec& qTypeSpec = it->get();
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

        if(typeSpec.name().string() == "stringlist") {
            name += "z::stringlist";
            return;
        }

        name += typeSpec.name().string();

        const z::Ast::EnumDefn* enumDefn = dynamic_cast<const z::Ast::EnumDefn*>(z::ptr(typeSpec));
        if(enumDefn != 0) {
            name += _sep;
            name += "T";
        }
    }

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
        inline GeneratorContext(const TargetMode::T& targetMode, const IndentMode::T& indentMode)
            : _targetMode(targetMode), _indentMode(indentMode) {}
        void run(const z::Ast::Config& config, FileSet& fs, const z::Ast::Statement& block);
    };

    struct ExprGenerator : public z::Ast::Expr::Visitor {
    public:
        inline ExprGenerator(z::ofile& os, const z::string& sep2 = "", const z::string& sep1 = "") : _os(os), _sep2(sep2), _sep1(sep1), _sep0(sep1) {}
    private:
        inline void visitTernary(const z::Ast::TernaryOpExpr& node) {
            _os() << "(";
            visitNode(node.lhs());
            _os() << node.op1();
            visitNode(node.rhs1());
            _os() << node.op2();
            visitNode(node.rhs2());
            _os() << ")";
        }

        virtual void visit(const z::Ast::ConditionalExpr& node) {
            return visitTernary(node);
        }

        virtual void visitBinary(const z::Ast::BinaryExpr& node) {
            visitNode(node.lhs());
            _os() << node.op();
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
            _os() << node.op();
        }

        virtual void visit(const z::Ast::PostfixIncExpr& node) {
            return visitPostfix(node);
        }

        virtual void visit(const z::Ast::PostfixDecExpr& node) {
            return visitPostfix(node);
        }

        inline void visitPrefix(const z::Ast::PrefixExpr& node) {
            _os() << node.op();
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
            _os() << ".set(";
            visitNode(node.lhs().index());
            _os() << ", ";
            visitNode(node.rhs());
            _os() << ")";
        }

        virtual void visit(const z::Ast::ListExpr& node) {
            _os() << "z::list<" << StlcppNameGenerator().qtn(node.list().valueType()) << ">::creator()";
            for(z::Ast::ListList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const z::Ast::ListItem& item = it->get();
                _os() << ".add(";
                visitNode(item.valueExpr());
                _os() << ")";
            }
            _os() << ".get()";
        }

        virtual void visit(const z::Ast::DictExpr& node) {
            _os() << "z::dict<" << StlcppNameGenerator().qtn(node.list().keyType()) << ", " << StlcppNameGenerator().qtn(node.list().valueType()) << ">::creator()";
            for(z::Ast::DictList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const z::Ast::DictItem& item = it->get();
                _os() << ".add(";
                visitNode(item.keyExpr());
                _os() << ", ";
                visitNode(item.valueExpr());
                _os() << ")";
            }
            _os() << ".get()";
        }

        virtual void visit(const z::Ast::FormatExpr& node) {
            _os() << "z::string(";
            visitNode(node.stringExpr());
            _os() << ")";
            for(z::Ast::DictList::List::const_iterator it = node.dictExpr().list().list().begin(); it != node.dictExpr().list().list().end(); ++it) {
                const z::Ast::DictItem& item = it->get();
                _os() << ".arg(";
                visitNode(item.keyExpr());
                _os() << ", ";
                visitNode(item.valueExpr());
                _os() << ")";
            }
        }

        virtual void visit(const z::Ast::RoutineCallExpr& node) {
            const z::string name = StlcppNameGenerator().tn(node.routine());
            if((name == "assert") || (name == "unused") || (name == "verify")) {
                z::string sep;
                for(z::Ast::ExprList::List::const_iterator it = node.exprList().list().begin(); it != node.exprList().list().end(); ++it) {
                    const z::Ast::Expr& expr = it->get();
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

        virtual void visit(const z::Ast::FunctorCallExpr& node) {
            ExprGenerator(_os).visitNode(node.expr());
            _os() << ".run(";
            ExprGenerator(_os, ", ").visitList(node.exprList());
            _os() << ")";
        }

        virtual void visit(const z::Ast::RunExpr& node) {
            _os() << "z::ctx().add(";
            ExprGenerator(_os).visitNode(node.callExpr().expr());
            _os() << ", ";
            _os() << StlcppNameGenerator().tn(node.callExpr().expr().qTypeSpec().typeSpec()) << "::_In(";
            ExprGenerator(_os, ", ").visitList(node.callExpr().exprList());
            _os() << "))";
        }

        virtual void visit(const z::Ast::OrderedExpr& node) {
            _os() << "(";
            visitNode(node.expr());
            _os() << ")";
        }

        virtual void visit(const z::Ast::IndexExpr& node) {
            z::string at = "at";

            const z::Ast::PropertyDecl* ts = z::resolveTypedefT<z::Ast::PropertyDecl>(node.expr().qTypeSpec().typeSpec());
            if(ts != 0) {
                at = z::ref(ts).name().string();
            }

            visitNode(node.expr());
            _os() << "." << at << "(";
            visitNode(node.index());
            _os() << ")";
        }

        virtual void visit(const z::Ast::SpliceExpr& node) {
            _os() << "splice(";
            visitNode(node.expr());
            _os() << ", ";
            visitNode(node.from());
            _os() << ", ";
            visitNode(node.to());
            _os() << ")";
        }

        virtual void visit(const z::Ast::TypeofTypeExpr& node) {
            _os() << "z::type(\"" << StlcppNameGenerator().qtn(node.typeSpec()) << "\")";
        }

        virtual void visit(const z::Ast::TypeofExprExpr& node) {
            const z::Ast::TypeSpec* typeSpec = z::ptr(node.expr().qTypeSpec().typeSpec());
            const z::Ast::TemplateDefn* td = dynamic_cast<const z::Ast::TemplateDefn*>(typeSpec);
            if(td) {
                visitNode(node.expr());
                _os() << ".tname()";
            } else {
                _os() << "z::type(\"" << StlcppNameGenerator().qtn(node.expr().qTypeSpec()) << "\")";
            }
        }

        virtual void visit(const z::Ast::StaticTypecastExpr& node) {
            _os() << "static_cast<" << StlcppNameGenerator().qtn(node.qTypeSpec()) << ">(";
            ExprGenerator(_os).visitNode(node.expr());
            _os() << ")";
        }

        virtual void visit(const z::Ast::DynamicTypecastExpr& node) {
            ExprGenerator(_os).visitNode(node.expr());
            _os() << ".getT<" << StlcppNameGenerator().tn(node.qTypeSpec().typeSpec()) << ">()";
        }

        virtual void visit(const z::Ast::PointerInstanceExpr& node) {
            const z::Ast::Expr& expr = node.exprList().at(0);
            const z::string dname = StlcppNameGenerator().tn(expr.qTypeSpec().typeSpec());
            const z::string bname = StlcppNameGenerator().tn(node.templateDefn().at(0).typeSpec());

            _os() << "z::pointer<" << bname << ">(\"" << dname << "\", ";
            ExprGenerator(_os).visitNode(expr);
            _os() << ")";
        }

        virtual void visit(const z::Ast::ValueInstanceExpr& node) {
            const z::Ast::Expr& expr = node.exprList().at(0);
            ExprGenerator(_os).visitNode(expr);
            _os() << ".getT<" << StlcppNameGenerator().tn(node.qTypeSpec().typeSpec()) << ">()";
        }

        virtual void visit(const z::Ast::VariableRefExpr& node) {
            switch(node.refType()) {
                case z::Ast::RefType::Global:
                    break;
                case z::Ast::RefType::XRef:
                    _os() << "z::ref(this)." << node.vref().name();
                    break;
                case z::Ast::RefType::Param:
                    _os() << node.vref().name();
                    break;
                case z::Ast::RefType::Local:
                    _os() << node.vref().name();
                    break;
            }
        }

        virtual void visit(const z::Ast::MemberVariableExpr& node) {
            visitNode(node.expr());
            _os() << "." << node.vref().name();
        }

        virtual void visit(const z::Ast::MemberPropertyExpr& node) {
            visitNode(node.expr());
            _os() << "._" << node.pref().name() << "()";
        }

        virtual void visit(const z::Ast::EnumMemberExpr& node) {
            _os() << z::ZenlangNameGenerator().tn(node.typeSpec());
            _os() << "::" << node.vref().name();
        }

        virtual void visit(const z::Ast::StructMemberExpr& node) {
            _os() << z::ZenlangNameGenerator().tn(node.typeSpec());
            _os() << "::" << node.vref().name();
        }

        virtual void visit(const z::Ast::StructInstanceExpr& node) {
            _os() << StlcppNameGenerator().tn(node.structDefn()) << "()";
            for(z::Ast::StructInitPartList::List::const_iterator it = node.list().list().begin(); it != node.list().list().end(); ++it) {
                const z::Ast::StructInitPart& part = it->get();
                _os() << "._" << part.vdef().name() << "<" << StlcppNameGenerator().tn(node.structDefn()) << ">(";
                visitNode(part.expr());
                _os() << ")";
            }
        }

        inline void visitFunctionTypeInstance(const z::Ast::Function& function, const z::Ast::ExprList& exprList) {
            unused(exprList); // will be implementing function-type-instantiation with ctor parameters in future.
            z::string fname = StlcppNameGenerator().tn(function);
            _os() << fname << "(";
            z::string sep;
            for(z::Ast::Scope::List::const_iterator it = function.xref().begin(); it != function.xref().end(); ++it) {
                const z::Ast::VariableDefn& vref = it->get();
                _os() << sep << vref.name();
                sep = ", ";
            }
            _os() << ")";
        }

        virtual void visit(const z::Ast::FunctionInstanceExpr& node) {
            visitFunctionTypeInstance(node.function(), node.exprList());
        }

        virtual void visit(const z::Ast::AnonymousFunctionExpr& node) {
            visitFunctionTypeInstance(node.function(), node.exprList());
        }

        virtual void visit(const z::Ast::ConstantNullExpr& node) {
            unused(node);
            _os() << "0";
        }

        virtual void visit(const z::Ast::ConstantFloatExpr& node) {
            _os() << node.value();
        }

        virtual void visit(const z::Ast::ConstantDoubleExpr& node) {
            _os() << node.value();
        }

        virtual void visit(const z::Ast::ConstantBooleanExpr& node) {
            _os() << (node.value()?"true":"false");
        }

        virtual void visit(const z::Ast::ConstantStringExpr& node) {
            _os() << "\"" << node.value() << "\"";
        }

        virtual void visit(const z::Ast::ConstantCharExpr& node) {
            _os() << "\'" << node.value() << "\'";
        }

        virtual void visit(const z::Ast::ConstantLongExpr& node) {
            _os() << node.value();
        }

        virtual void visit(const z::Ast::ConstantIntExpr& node) {
            _os() << node.value();
        }

        virtual void visit(const z::Ast::ConstantShortExpr& node) {
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

    struct TypeDeclarationGenerator : public z::Ast::TypeSpec::Visitor {
        inline void visitChildrenIndent(const z::Ast::TypeSpec& node) {
            INDENT;
            visitChildren(node);
        }

        void visit(const z::Ast::TypedefDecl& node) {
            if(node.defType() == z::Ast::DefinitionType::Native) {
                _os() << z::Indent::get() << "// typedef " << node.name() << " native;" << std::endl;
            } else {
                throw z::Exception("StlcppGenerator", z::zfmt(node.pos(), "Internal error: '%{s}'").arg("s", node.name()) );
            }
        }

        void visit(const z::Ast::TypedefDefn& node) {
            if(node.defType() != z::Ast::DefinitionType::Native) {
                _os() << z::Indent::get() << "typedef " << StlcppNameGenerator().qtn(node.qTypeSpec()) << " " << node.name() << ";" << std::endl;
            }
        }

        void visit(const z::Ast::TemplateDecl& node) {
            if(node.defType() != z::Ast::DefinitionType::Native) {
                throw z::Exception("StlcppGenerator", z::zfmt(node.pos(), "Internal error: template declaration cannot be generated '%{s}'").arg("s", node.name()) );
            }
        }

        void visit(const z::Ast::TemplateDefn& node) {
            throw z::Exception("StlcppGenerator", z::zfmt(node.pos(), "Internal error: template definition cannot be generated '%{s}'").arg("s", node.name()) );
        }

        void visit(const z::Ast::EnumDecl& node) {
            if(node.defType() != z::Ast::DefinitionType::Native) {
                throw z::Exception("StlcppGenerator", z::zfmt(node.pos(), "Internal error: enum definition cannot be generated '%{s}'").arg("s", node.name()) );
            }
        }

        void visit(const z::Ast::EnumDefn& node) {
            if(node.defType() != z::Ast::DefinitionType::Native) {
                _os() << z::Indent::get() << "struct " << node.name() << " {" << std::endl;
                _os() << z::Indent::get() << "  enum T {" << std::endl;
                z::string sep = " ";
                for(z::Ast::Scope::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    INDENT;
                    const z::Ast::VariableDefn& def = it->get();
                    _os() << z::Indent::get() << sep << " " << def.name();
                    const z::Ast::ConstantIntExpr* cexpr = dynamic_cast<const z::Ast::ConstantIntExpr*>(z::ptr(def.initExpr()));
                    if((cexpr == 0) || (z::ref(cexpr).pos().string() != "#")) { /// \todo hack
                        _os() << " = ";
                        ExprGenerator(_os).visitNode(def.initExpr());
                    }
                    _os() << std::endl;
                    sep = ",";
                }

                _os() << z::Indent::get() << "  };" << std::endl;
                _os() << z::Indent::get() << "};" << std::endl;
                _os() << std::endl;
            }
        }

        inline void visitStructDefn(const z::Ast::StructDefn& node, const z::Ast::StructDefn* base) {
            if(node.accessType() == z::Ast::AccessType::Protected) {
                _fs._osHdr() << z::Indent::get() << "struct " << node.name() << ";" << std::endl;
                // do not return here. We still need to fall-thru and generate the body in the source file.
            }

            if(node.defType() == z::Ast::DefinitionType::Native) {
                _os() << z::Indent::get() << "struct " << node.name() << ";" << std::endl;
                return;
            }

            _os() << z::Indent::get() << "struct " << node.name();
            if(base) {
                _os() << " : public " << StlcppNameGenerator().tn(z::ref(base));
            }

            _os() << " {" << std::endl;

            // if abstract type, generate virtual dtor
            if(node.defType() == z::Ast::DefinitionType::Abstract) {
                _os() << z::Indent::get() << "    virtual ~" << node.name() << "() {}" << std::endl;
            }

            GeneratorContext(GeneratorContext::TargetMode::TypeDecl, GeneratorContext::IndentMode::NoBrace).run(_config, _fs, node.block());

            _os() << z::Indent::get() << "};" << std::endl;
            _os() << std::endl;
        }

        void visit(const z::Ast::StructDecl& node) {
            _os() << z::Indent::get() << "struct " << node.name() << ";" << std::endl;
        }

        void visit(const z::Ast::RootStructDefn& node) {
            visitStructDefn(node, 0);
        }

        void visit(const z::Ast::ChildStructDefn& node) {
            visitStructDefn(node, z::ptr(node.base()));
        }

        inline void visitProperty(const z::Ast::PropertyDecl& node) {
            z::string cnst;
            if(node.qTypeSpec().isConst())
                cnst = "const ";
            _os() << z::Indent::get() << cnst << StlcppNameGenerator().qtn(node.qTypeSpec()) << "& _" << node.name() << "() const;" << std::endl;
        }

        void visit(const z::Ast::PropertyDeclRW& node) {
            visitProperty(node);
            _os() << z::Indent::get() << "void _" << node.name() << "(const " << StlcppNameGenerator().tn(node.qTypeSpec().typeSpec()) << "& val);" << std::endl;
        }

        void visit(const z::Ast::PropertyDeclRO& node) {
            visitProperty(node);
        }

        inline bool isVoid(const z::Ast::Scope& out) const {
            if(out.isTuple())
                return false;

            const z::Ast::VariableDefn& vdef = out.list().front();
            if(vdef.qTypeSpec().typeSpec().name().string() == "void")
                return true;

            return false;
        }

        static inline void visitRoutine(z::ofile& os, const z::Ast::Routine& node, const bool& inNS) {
            os() << z::Indent::get() << StlcppNameGenerator().qtn(node.outType()) << " ";
            if(inNS) {
                os() << node.name();
            } else {
                os() << StlcppNameGenerator().tn(node);
            }
            os() << "(";
            z::string sep;
            for(z::Ast::Scope::List::const_iterator it = node.in().begin(); it != node.in().end(); ++it) {
                const z::Ast::VariableDefn& vdef = it->get();
                os() << sep << StlcppNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name();
                sep = ", ";
            }
            os() << ")";
        }

        inline void writeScopeMemberList(const z::Ast::Scope& scope) {
            for(z::Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                INDENT;
                const z::Ast::VariableDefn& vdef = it->get();
                _os() << z::Indent::get() << StlcppNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name() << ";" << std::endl;
            }
            if(scope.hasPosParam()) {
                const z::Ast::Scope& posParam = scope.posParam();
                for(z::Ast::Scope::List::const_iterator it = posParam.list().begin(); it != posParam.list().end(); ++it) {
                    INDENT;
                    const z::Ast::VariableDefn& vdef = it->get();
                    _os() << z::Indent::get() << StlcppNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name() << ";" << std::endl;
                }
            }
        }

        static inline void writeScopeParamList(z::ofile& os, const z::Ast::Scope& scope, const z::string& prefix) {
            z::string sep = "";
            for(z::Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                const z::Ast::VariableDefn& vdef = it->get();
                os() << sep << " " << StlcppNameGenerator().qtn(vdef.qTypeSpec()) << " " << prefix << vdef.name();
                sep = ", ";
            }
        }

        inline void writeScopeInCallList(const z::Ast::Scope& scope) {
            z::string sep = "";
            for(z::Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                const z::Ast::VariableDefn& vdef = it->get();
                _os() << sep << "_in." << vdef.name();
                sep = ", ";
            }
        }

        inline void writeCtor(const z::string& cname, const z::Ast::Scope& scope) {
            _os() << z::Indent::get() << "    inline " << cname << "(";
            writeScopeParamList(_os, scope, "p");
            _os() << ")";

            z::string sep = " : ";
            for(z::Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                const z::Ast::VariableDefn& vdef = it->get();
                _os() << sep << vdef.name() << "(p" << vdef.name() << ")";
                sep = ", ";
            }

            if(scope.hasPosParam()) {
                const z::Ast::Scope& posParam = scope.posParam();
                unused(posParam);
                /// \todo implement positional-param
            }

            _os() << " {}" << std::endl;
        }

        inline z::string getOutType(const z::Ast::Function& node) const {
            z::string out1;
            if(isVoid(node.sig().outScope())) {
                out1 = "void";
            } else if(node.sig().outScope().isTuple()) {
//                out1 = "const _Out&";
                out1 = "_Out";
            } else {
                const z::Ast::VariableDefn& vdef = node.sig().out().front();
                out1 = StlcppNameGenerator().qtn(vdef.qTypeSpec());
            }
            return out1;
        }

        inline void visitFunctionXRef(const z::Ast::Function& node) {
            if((node.xref().size() == 0) && (node.iref().size() == 0)) {
                return;
            }

            if(node.xref().size() > 0) {
                _os() << z::Indent::get() << "public: // xref-list" << std::endl;
                writeScopeMemberList(node.xrefScope());
            }
            if(node.iref().size() > 0) {
                _os() << z::Indent::get() << "public: // iref-list" << std::endl;
                writeScopeMemberList(node.irefScope());
            }
            writeCtor(node.name().string(), node.xrefScope());
        }

        inline void visitFunctionSig(const z::Ast::Function& node, const bool& isRoot, const bool& isDecl, const bool& isTest) {
            if(node.childCount() > 0) {
                _os() << z::Indent::get() << "public:// child-typespec" << std::endl;
                visitChildrenIndent(node);
            }

            visitFunctionXRef(node);

            if(isRoot) {
                _os() << z::Indent::get() << "public: // in-param-list" << std::endl;
                INDENT;
                _os() << z::Indent::get() << "struct _In {" << std::endl;
                writeScopeMemberList(node.sig().inScope());
                writeCtor("_In", node.sig().inScope());
                _os() << z::Indent::get() << "};" << std::endl;
            }

            z::string out1 = getOutType(node);
            _os() << z::Indent::get() << "public: // run-function" << std::endl;
            if(isTest) {
                _os() << z::Indent::get() << "    " << out1 << " run();" << std::endl;
            } else {
                if((isDecl) && ((node.defType() == z::Ast::DefinitionType::Final) || (node.defType() == z::Ast::DefinitionType::Abstract))) {
                    _os() << z::Indent::get() << "    virtual ~" << node.sig().name() << "(){}" << std::endl;
                    _os() << z::Indent::get() << "    virtual " << out1 << " run(";
                    writeScopeParamList(_os, node.sig().inScope(), "p");
                    _os() << ") = 0;" << std::endl;
                } else {
                    _os() << z::Indent::get() << "    " << out1 << " run(";
                    writeScopeParamList(_os, node.sig().inScope(), "p");
                    _os() << ");" << std::endl;
                }

                _os() << z::Indent::get() << "    inline _Out _run(const _In& _in) {";
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

        inline void visitFunction(const z::Ast::Function& node, const bool isDecl) {
            _os() << z::Indent::get() << "class " << node.name() << " {" << std::endl;
            visitFunctionSig(node, true, isDecl, false);
            _os() << z::Indent::get() << "};" << std::endl;
        }

        void visit(const z::Ast::RoutineDecl& node) {
            visitRoutine(_os, node, true);
            _os() << ";" << std::endl;
            _os() << std::endl;
        }

        void visit(const z::Ast::RoutineDefn& node) {
            visitRoutine(_os, node, true);
            _os() << ";" << std::endl;
            _os() << std::endl;
        }

        void visit(const z::Ast::FunctionRetn& node) {
            _os() << z::Indent::get() << "struct _Out {" << std::endl;

            if(!isVoid(node.outScope())) {
                // generate out parameters
                for(z::Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
                    INDENT;
                    const z::Ast::VariableDefn& vdef = it->get();
                    _os() << z::Indent::get() << StlcppNameGenerator().qtn(vdef.qTypeSpec()) << " " << vdef.name() << ";" << std::endl;
                }

                // generate out setter
                _os() << z::Indent::get() << "    inline " << node.name() << "(";
                z::string sep = "";
                for(z::Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
                    const z::Ast::VariableDefn& vdef = it->get();
                    _os() << sep << StlcppNameGenerator().qtn(vdef.qTypeSpec()) << " p" << vdef.name();
                    sep = ", ";
                }
                _os() << ")";
                sep = " : ";
                for(z::Ast::Scope::List::const_iterator it = node.out().begin(); it != node.out().end(); ++it) {
                    const z::Ast::VariableDefn& vdef = it->get();
                    _os() << sep << vdef.name() << "(p" << vdef.name() << ")";
                    sep = ", ";
                }
                _os() << "{}" << std::endl;

            }

            // end return struct
            _os() << z::Indent::get() << "};" << std::endl;
            _os() << std::endl;
        }

        void visit(const z::Ast::RootFunctionDecl& node) {
            visitFunction(node, true);
            _os() << std::endl;
        }

        template<typename T>
        inline void enterFunction(const T& node) {
            if(StlcppNameGenerator().tn(node.base()) == "test") {
                _os() << z::Indent::get() << "class " << node.name() << " : public z::test_< " << node.name() << " > {" << std::endl;
                _os() << z::Indent::get() << "public:" << std::endl;
                _os() << z::Indent::get() << "    inline const char* name() const {return \"" << StlcppNameGenerator().tn(node) << "\";}" << std::endl;
            } else if(StlcppNameGenerator().tn(node.base()) == "main") {
                _os() << z::Indent::get() << "class " << node.name() << " : public z::main_< " << node.name() << " > {" << std::endl;
            } else {
                _os() << z::Indent::get() << "class " << node.name() << " : public " << StlcppNameGenerator().tn(node.base()) << " {" << std::endl;
            }
        }


        void visit(const z::Ast::ChildFunctionDecl& node) {
            if(node.defType() == z::Ast::DefinitionType::Native) {
                z::string out1 = getOutType(node);

                enterFunction(node);
                visitFunctionXRef(node);
                _os() << z::Indent::get() << "public:" << std::endl;
                _os() << z::Indent::get() << "    virtual ~" << node.name() << "(){}";
                _os() << z::Indent::get() << "    virtual " << out1 << " run(";
                writeScopeParamList(_os, node.sig().inScope(), "p");
                _os() << ");" << std::endl;
                if(StlcppNameGenerator().tn(node.base()) == "test") {
                    _os() << z::Indent::get() << "    static z::TestInstanceT<" << node.name() << "> s_test;" << std::endl;
                } else if(StlcppNameGenerator().tn(node.base()) == "main") {
                    _os() << z::Indent::get() << "    static z::MainInstanceT<" << node.name() << "> s_main;" << std::endl;
                }
                _os() << z::Indent::get() << "};" << std::endl;
            } else {
                _os() << z::Indent::get() << "class " << node.name() << ";" << std::endl;
            }
            _os() << std::endl;
        }

        void visit(const z::Ast::RootFunctionDefn& node) {
            visitFunction(node, false);
            _os() << std::endl;
        }

        void visit(const z::Ast::ChildFunctionDefn& node) {
            bool isTest = (StlcppNameGenerator().tn(node.base()) == "test");
            if((isTest) && (!_config.test())) {
                return;
            }

            enterFunction(node);
            visitFunctionSig(node, false, false, isTest);

            if(StlcppNameGenerator().tn(node.base()) == "test") {
                _os() << z::Indent::get() << "    static z::TestInstanceT<" << node.name() << "> s_test;" << std::endl;
            } else if(StlcppNameGenerator().tn(node.base()) == "main") {
                _os() << z::Indent::get() << "    static z::MainInstanceT<" << node.name() << "> s_main;" << std::endl;
            }

            _os() << z::Indent::get() << "};" << std::endl;
            _os() << std::endl;
        }

        void visit(const z::Ast::EventDecl& node) {
            _os() << z::Indent::get() << "struct " << node.name() << " {" << std::endl;
            // child-typespecs
            if(node.childCount() > 0) {
                _os() << z::Indent::get() << "public:" << std::endl;
                visitChildrenIndent(node);
            }
            _os() << z::Indent::get() << "    z::FunctorList<Handler> _list;" << std::endl;
            _os() << z::Indent::get() << "    static " << node.name() << " instance;" << std::endl;
            _os() << z::Indent::get() << "    static inline Handler& add(Handler* h) {return instance._list.add(h);}" << std::endl;
            _os() << z::Indent::get() << "    static void addHandler(" << StlcppNameGenerator().qtn(node.in().qTypeSpec()) << " " << node.in().name() << ", Handler* h);" << std::endl;
            _os() << z::Indent::get() << "    template<typename T>static inline void addHandlerT("
                  << StlcppNameGenerator().qtn(node.in().qTypeSpec()) << " " << node.in().name() << ", T h) {return addHandler("
                  << node.in().name() << ", new T(h));}" << std::endl;
            _os() << z::Indent::get() << "};" << std::endl;
            _os() << std::endl;
            return;
        }

        void visit(const z::Ast::Namespace& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::Root& node) {
            visitChildrenIndent(node);
        }

    public:
        inline TypeDeclarationGenerator(const z::Ast::Config& config, FileSet& fs, z::ofile& os) : _config(config), _fs(fs), _os(os) {}

    private:
        const z::Ast::Config& _config;
        FileSet& _fs;
        z::ofile& _os;
    };

    struct TypeDefinitionGenerator : public z::Ast::TypeSpec::Visitor {
        inline void visitChildrenIndent(const z::Ast::TypeSpec& node) {
            INDENT;
            visitChildren(node);
        }

        void visit(const z::Ast::TypedefDecl& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::TypedefDefn& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::TemplateDecl& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::TemplateDefn& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::EnumDecl& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::EnumDefn& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::StructDecl& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::RootStructDefn& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::ChildStructDefn& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::PropertyDeclRW& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::PropertyDeclRO& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::RoutineDecl& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::RoutineDefn& node) {
            visitChildrenIndent(node);
            TypeDeclarationGenerator::visitRoutine(_os, node, false);
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
            _os() << std::endl;
        }

        void visit(const z::Ast::FunctionRetn& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::RootFunctionDecl& node) {
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

        void visit(const z::Ast::ChildFunctionDecl& node) {
            visitChildrenIndent(node);
            if(node.defType() == z::Ast::DefinitionType::Native) {
                writeSpecialStatic(node);
            }
        }

        inline void visitFunction(const z::Ast::FunctionDefn& node) {
            const z::string tname = StlcppNameGenerator().tn(node);
            z::string out;
            if(node.sig().outScope().isTuple()) {
                out = "" + tname + "::_Out";
            } else {
                const z::Ast::VariableDefn& vdef = node.sig().out().front();
                out = StlcppNameGenerator().qtn(vdef.qTypeSpec());
            }

            _os() << out << " " << tname << "::run(";
            TypeDeclarationGenerator::writeScopeParamList(_os, node.sig().inScope(), "");
            _os() << ") ";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
            _os() << std::endl;
        }

        void visit(const z::Ast::RootFunctionDefn& node) {
            visitChildrenIndent(node);
            visitFunction(node);
        }

        void visit(const z::Ast::ChildFunctionDefn& node) {
            bool isTest = (StlcppNameGenerator().tn(node.base()) == "test");
            if((isTest) && (!_config.test())) {
                return;
            }

            visitChildrenIndent(node);
            visitFunction(node);
            writeSpecialStatic(node);
        }

        void visit(const z::Ast::EventDecl& node) {
            visitChildrenIndent(node);
            _os() << StlcppNameGenerator().tn(node) << " " << StlcppNameGenerator().tn(node) << "::instance;" << std::endl;
            if(node.defType() == z::Ast::DefinitionType::Final) {
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

        void visit(const z::Ast::Namespace& node) {
            visitChildrenIndent(node);
        }

        void visit(const z::Ast::Root& node) {
            visitChildrenIndent(node);
        }

    public:
        inline TypeDefinitionGenerator(const z::Ast::Config& config, FileSet& fs, z::ofile& os) : _config(config), _fs(fs), _os(os) {}
    private:
        const z::Ast::Config& _config;
        FileSet& _fs;
        z::ofile& _os;
    };

    struct StatementGenerator : public z::Ast::Statement::Visitor {
    private:
        inline z::ofile& fpDecl1(const z::Ast::AccessType::T& accessType) const {
            switch(accessType) {
                case z::Ast::AccessType::Private:
                case z::Ast::AccessType::Protected:
                    return _fs._osSrc;
                default:
                    break;
            }
            return _fs._osHdr;
        }

        inline z::ofile& fpDecl(const z::Ast::TypeSpec& node) const {
            if(node.accessType() == z::Ast::AccessType::Parent) {
                const z::Ast::ChildTypeSpec* child = dynamic_cast<const z::Ast::ChildTypeSpec*>(z::ptr(node));
                if(!child) {
                    throw z::Exception("StlcppGenerator", z::zfmt(node.pos(), "Internal error: Invalid access type in typespec"));
                }
                return fpDecl(z::ref(child).parent());
            }
            return fpDecl1(node.accessType());
        }

        inline z::ofile& fpDefn() const {
            return _fs._osSrc;
        }
    private:
        virtual void visit(const z::Ast::ImportStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                if (node.headerType() == z::Ast::HeaderType::Import) {
                    return;
                }
                z::ofile& os = fpDecl1(node.accessType());
                z::string qt = (node.headerType() == z::Ast::HeaderType::Import)?"<>":"\"\"";
                os() << "#include " << (char)qt.at(0);
                z::string sep = "";
                for(z::Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const z::Ast::Token& name = it->get().name();
                    os() << sep << name;
                    sep = "/";
                }
                os() << ".hpp" << (char)qt.at(1) << std::endl;
            }
        }

        virtual void visit(const z::Ast::EnterNamespaceStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                for(z::Ast::NamespaceList::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const z::Ast::Namespace& ns = it->get();
                    _fs._osHdr() << "namespace " << ns.name() << "{ ";
                    _fs._osSrc() << "namespace " << ns.name() << "{ ";
                }

                if(node.list().size() > 0) {
                    _fs._osHdr() << std::endl;
                    _fs._osSrc() << std::endl;
                }
            }
        }

        virtual void visit(const z::Ast::LeaveNamespaceStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                for(z::Ast::NamespaceList::List::const_reverse_iterator it = node.statement().list().rbegin(); it != node.statement().list().rend(); ++it) {
                    const z::Ast::Namespace& ns = it->get();
                    _fs._osHdr() << "}/*" << ns.name() << "*/ ";
                    _fs._osSrc() << "}/*" << ns.name() << "*/ ";
                }
                if(node.statement().list().size() > 0) {
                    _fs._osHdr() << std::endl;
                    _fs._osSrc() << std::endl;
                }
            }
        }

        virtual void visit(const z::Ast::UserDefinedTypeSpecStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                TypeDeclarationGenerator(_config, _fs, fpDecl(node.typeSpec())).visitNode(node.typeSpec());
            }

            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDefn) {
                TypeDefinitionGenerator(_config, _fs, _fs._osSrc).visitNode(node.typeSpec());
            }
        }

        inline bool isPtr(const z::Ast::TypeSpec& typeSpec) const {
            const z::Ast::TemplateDefn* templateDefn = dynamic_cast<const z::Ast::TemplateDefn*>(z::ptr(typeSpec));
            if(templateDefn) {
                if(typeSpec.name().string() == "ptr") {
                    return true;
                }
            }
            return false;
        }

        virtual void visit(const z::Ast::StructMemberVariableStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                z::string tname = StlcppNameGenerator().qtn(node.defn().qTypeSpec());
                z::string pname = "const " + tname + "&";
                if(isPtr(node.defn().qTypeSpec().typeSpec())) {
                    pname = tname;
                }
                fpDecl(node.structDefn())() << z::Indent::get() << tname << " " << node.defn().name() << "; ";
                fpDecl(node.structDefn())() << "template <typename T> inline T& _" << node.defn().name()
                                            << "(" << pname <<" val) {"
                                            << node.defn().name() << " = val; return z::ref(static_cast<T*>(this));}"
                                            << std::endl;
            }
        }

        virtual void visit(const z::Ast::StructInitStatement& node) {
            if(_ctx._targetMode == GeneratorContext::TargetMode::TypeDecl) {
                // default-ctor
                fpDecl(node.structDefn())() << z::Indent::get() << "explicit inline " << node.structDefn().name() << "()";
                z::string sep = " : ";
                for(z::Ast::Scope::List::const_iterator it = node.structDefn().list().begin(); it != node.structDefn().list().end(); ++it) {
                    const z::Ast::VariableDefn& vdef = it->get();
                    fpDecl(node.structDefn())() << sep << vdef.name() << "(";
                    ExprGenerator(fpDecl(node.structDefn())).visitNode(vdef.initExpr());
                    fpDecl(node.structDefn())() << ")";
                    sep = ", ";
                }
                fpDecl(node.structDefn())() << " {}" << std::endl;
            }
        }

        virtual void visit(const z::Ast::EmptyStatement& node) {
            unused(node);
            fpDefn()() << ";" << std::endl;
        }

        virtual void visit(const z::Ast::AutoStatement& node) {
            fpDefn()() << z::Indent::get() << StlcppNameGenerator().qtn(node.defn().qTypeSpec()) << " " << node.defn().name() << " = ";
            ExprGenerator(fpDefn()).visitNode(node.defn().initExpr());
            fpDefn()() << ";" << std::endl;
        }

        virtual void visit(const z::Ast::ExprStatement& node) {
            fpDefn()() << z::Indent::get();
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ";" << std::endl;
        }

        virtual void visit(const z::Ast::PrintStatement& node) {
            fpDefn()() << z::Indent::get() << "z::mlog(\"Message\", z::string(\"%{s}\").arg(\"s\", ";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << "));" << std::endl;
        }

        virtual void visit(const z::Ast::IfStatement& node) {
            fpDefn()() << z::Indent::get() << "if(";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ") ";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.tblock());
        }

        virtual void visit(const z::Ast::IfElseStatement& node) {
            fpDefn()() << z::Indent::get() << "if(";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ") ";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.tblock());
            fpDefn()() << z::Indent::get() << "else";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.fblock());
        }

        virtual void visit(const z::Ast::WhileStatement& node) {
            fpDefn()() << z::Indent::get() << "while(";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ") ";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
        }

        virtual void visit(const z::Ast::DoWhileStatement& node) {
            fpDefn()() << z::Indent::get() << "do";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
            fpDefn()() << z::Indent::get() << "while(";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ");" << std::endl;
        }

        virtual void visit(const z::Ast::ForExprStatement& node) {
            fpDefn()() << z::Indent::get() << "for(";
            ExprGenerator(fpDefn()).visitNode(node.init());
            fpDefn()() << "; ";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << "; ";
            ExprGenerator(fpDefn()).visitNode(node.incr());
            fpDefn()() << ") ";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
        }

        virtual void visit(const z::Ast::ForInitStatement& node) {
            fpDefn()() << z::Indent::get() << "for(" << StlcppNameGenerator().qtn(node.init().qTypeSpec())<< " " << node.init().name() << " = ";
            ExprGenerator(fpDefn()).visitNode(node.init().initExpr());
            fpDefn()() << "; ";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << "; ";
            ExprGenerator(fpDefn()).visitNode(node.incr());
            fpDefn()() << ") ";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
        }

        virtual void visit(const z::Ast::ForeachStringStatement& node) {
            const z::string etype = StlcppNameGenerator().qtn(node.expr().qTypeSpec());

            z::string constit = "";
            if(node.expr().qTypeSpec().isConst()) {
                constit = "const_";
            }

            fpDefn()() << z::Indent::get() << "{" << std::endl;
            fpDefn()() << z::Indent::get() << "  " << etype << " _str = ";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ";" << std::endl;

            fpDefn()() << z::Indent::get() << "  for(" << StlcppNameGenerator().tn(node.expr().qTypeSpec().typeSpec()) << "::" << constit << "iterator _it = _str.begin(); _it != _str.end(); ++_it) {" << std::endl;
            fpDefn()() << z::Indent::get() << "  " << StlcppNameGenerator().qtn(node.valDef().qTypeSpec()) << " " << node.valDef().name() << " = *_it;" << std::endl;

            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::IndentedBrace).run(_config, _fs, node.block());

            fpDefn()() << z::Indent::get() << "}" << std::endl;
            fpDefn()() << z::Indent::get() << "}" << std::endl;
        }

        virtual void visit(const z::Ast::ForeachListStatement& node) {
            const z::string etype = StlcppNameGenerator().qtn(node.expr().qTypeSpec());

            z::string constit = "";
            if(node.expr().qTypeSpec().isConst()) {
                constit = "const_";
            }

            fpDefn()() << z::Indent::get() << "{" << std::endl;
            fpDefn()() << z::Indent::get() << "  " << etype << " _list = ";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ";" << std::endl;
            fpDefn()() << z::Indent::get() << "  for(" << StlcppNameGenerator().tn(node.expr().qTypeSpec().typeSpec()) << "::" << constit << "iterator _it = _list.begin(); _it != _list.end(); ++_it) {" << std::endl;
            fpDefn()() << z::Indent::get() << "  " << StlcppNameGenerator().qtn(node.valDef().qTypeSpec()) << " " << node.valDef().name() << " = *_it;" << std::endl;

            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::IndentedBrace).run(_config, _fs, node.block());

            fpDefn()() << z::Indent::get() << "}" << std::endl;
            fpDefn()() << z::Indent::get() << "}" << std::endl;
        }

        virtual void visit(const z::Ast::ForeachDictStatement& node) {
            z::string constit = "";
            fpDefn()() << z::Indent::get() << "{" << std::endl;
            if(node.expr().qTypeSpec().isConst()) {
                constit = "const_";
            }
            fpDefn()() << z::Indent::get() << "  " << StlcppNameGenerator().qtn(node.expr().qTypeSpec()) << " _list = ";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ";" << std::endl;
            fpDefn()() << z::Indent::get() << "  for(" << StlcppNameGenerator().tn(node.expr().qTypeSpec().typeSpec()) << "::" << constit << "iterator _it = _list.begin(); _it != _list.end(); ++_it) {" << std::endl;
            fpDefn()() << z::Indent::get() << "  " << StlcppNameGenerator().qtn(node.keyDef().qTypeSpec()) << " " << node.keyDef().name() << " = _it->first;" << std::endl;
            fpDefn()() << z::Indent::get() << "  " << StlcppNameGenerator().qtn(node.valDef().qTypeSpec()) << " " << node.valDef().name() << " = _it->second;" << std::endl;

            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::IndentedBrace).run(_config, _fs, node.block());

            fpDefn()() << z::Indent::get() << "}" << std::endl;
            fpDefn()() << z::Indent::get() << "}" << std::endl;
        }

        virtual void visit(const z::Ast::CaseExprStatement& node) {
            fpDefn()() << z::Indent::get() << "case (";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ") :";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
        }

        virtual void visit(const z::Ast::CaseDefaultStatement& node) {
            fpDefn()() << z::Indent::get() << "default :";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
        }

        virtual void visit(const z::Ast::SwitchValueStatement& node) {
            fpDefn()() << z::Indent::get() << "switch(";
            ExprGenerator(fpDefn()).visitNode(node.expr());
            fpDefn()() << ")";
            GeneratorContext(GeneratorContext::TargetMode::Local, GeneratorContext::IndentMode::WithBrace).run(_config, _fs, node.block());
        }

        virtual void visit(const z::Ast::SwitchExprStatement& node) {
            z::string ifstr = "if";
            for(z::Ast::CompoundStatement::List::const_iterator it = node.block().list().begin(); it != node.block().list().end(); ++it) {
                const z::Ast::Statement& s = it->get();
                const z::Ast::CaseExprStatement* ce = dynamic_cast<const z::Ast::CaseExprStatement*>(z::ptr(s));
                const z::Ast::CaseDefaultStatement* cd = dynamic_cast<const z::Ast::CaseDefaultStatement*>(z::ptr(s));
                if(ce) {
                    fpDefn()() << z::Indent::get() << ifstr << "(" << std::endl;
                    ExprGenerator(fpDefn()).visitNode(z::ref(ce).expr());
                    fpDefn()() << ")" << std::endl;
                    visitNode(z::ref(ce).block());
                    ifstr = "else if";
                } else if(cd) {
                    fpDefn()() << z::Indent::get() << "else" << std::endl;
                    visitNode(z::ref(cd).block());
                    break;
                } else {
                    throw z::Exception("StlcppGenerator", z::zfmt(node.pos(), "Internal error: not a case statement inside switch"));

                }
            }
        }

        virtual void visit(const z::Ast::BreakStatement& node) {
            unused(node);
            fpDefn()() << z::Indent::get() << "break;" << std::endl;
        }

        virtual void visit(const z::Ast::ContinueStatement& node) {
            unused(node);
            fpDefn()() << z::Indent::get() << "continue;" << std::endl;
        }

        virtual void visit(const z::Ast::AddEventHandlerStatement& node) {
            z::string ename = StlcppNameGenerator().tn(node.event());
            fpDefn()() << z::Indent::get() << ename << "::addHandlerT(";
            ExprGenerator(fpDefn()).visitNode(node.source());
            fpDefn()() << ", ";
            ExprGenerator(fpDefn()).visitNode(node.functor());
            fpDefn()() << ");" << std::endl;
        }

        virtual void visit(const z::Ast::RoutineReturnStatement& node) {
            fpDefn()() << z::Indent::get() << "return";
            if(node.exprList().list().size() > 0) {
                fpDefn()() << " (";
                ExprGenerator(fpDefn(), ", ").visitList(node.exprList());
                fpDefn()() << ")";
            }
            fpDefn()() << ";" << std::endl;
        }

        virtual void visit(const z::Ast::FunctionReturnStatement& node) {
            const z::string out = node.sig().outScope().isTuple()?"_Out":"";
            fpDefn()() << z::Indent::get() << "return " << out << "(";
            ExprGenerator(fpDefn(), ", ").visitList(node.exprList());
            fpDefn()() << ");" << std::endl;
        }

        virtual void visit(const z::Ast::ExitStatement& node) {
            fpDefn()() << z::Indent::get() << "z::app().exit(";
            ExprGenerator(fpDefn(), ", ").visitNode(node.expr());
            fpDefn()() << ");" << std::endl;
        }

        virtual void visit(const z::Ast::CompoundStatement& node) {
            if(_ctx._indentMode == GeneratorContext::IndentMode::IndentedBrace) {
                fpDefn()() << z::Indent::get();
            }

            if((_ctx._indentMode == GeneratorContext::IndentMode::IndentedBrace) || (_ctx._indentMode == GeneratorContext::IndentMode::WithBrace)) {
                fpDefn()() << "{" << std::endl;
            }

            {
                INDENT;
                for(z::Ast::CompoundStatement::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                    const z::Ast::Statement& s = it->get();
                    z::ref(this).visitNode(s);
                }
            }

            if((_ctx._indentMode == GeneratorContext::IndentMode::IndentedBrace) || (_ctx._indentMode == GeneratorContext::IndentMode::WithBrace)) {
                fpDefn()() << z::Indent::get() << "}" << std::endl;
            }
        }

    public:
        inline StatementGenerator(const z::Ast::Config& config, FileSet& fs, GeneratorContext& ctx) : _config(config), _fs(fs), _ctx(ctx) {}
    private:
        const z::Ast::Config& _config;
        FileSet& _fs;
        GeneratorContext& _ctx;
    };

    void GeneratorContext::run(const z::Ast::Config& config, FileSet& fs, const z::Ast::Statement& block) {
        StatementGenerator gen(config, fs, z::ref(this));
        gen.visitNode(block);
    }
}

struct z::StlcppGenerator::Impl {
    inline Impl(const z::Ast::Project& project, const z::Ast::Config& config, const z::Ast::Module& module) : _project(project), _config(config), _module(module) {}
    inline void run();
private:
    const z::Ast::Project& _project;
    const z::Ast::Config& _config;
    const z::Ast::Module& _module;
};

inline void z::StlcppGenerator::Impl::run() {
    Indent::init();
    z::string basename = z::file::getBaseName(_module.filename());
    z::file::mkpath(_config.apidir() + "/");
    z::file::mkpath(_config.srcdir() + "/");
    z::ofile ofHdr(_config.apidir() + "/" + basename + ".hpp");
    z::ofile ofSrc(_config.srcdir() + "/" + basename + ".cpp");
    sg::FileSet fs(ofHdr, ofSrc);

    ofHdr() << "#pragma once" << std::endl << std::endl;
    for(z::Ast::Config::PathList::const_iterator it = _config.includeFileList().begin(); it != _config.includeFileList().end(); ++it) {
        const z::string& filename = *it;
        ofSrc() << "#include \"" << filename << "\"" << std::endl;
    }
    ofSrc() << "#include \"" << ofHdr.name() << "\"" << std::endl;

    for(z::Ast::CompoundStatement::List::const_iterator it = _module.globalStatementList().list().begin(); it != _module.globalStatementList().list().end(); ++it) {
        const z::Ast::Statement& s = it->get();
        sg::GeneratorContext(sg::GeneratorContext::TargetMode::TypeDecl, sg::GeneratorContext::IndentMode::WithBrace).run(_config, fs, s);
    }

    for(z::Ast::CompoundStatement::List::const_iterator it = _module.globalStatementList().list().begin(); it != _module.globalStatementList().list().end(); ++it) {
        const z::Ast::Statement& s = it->get();
        sg::GeneratorContext(sg::GeneratorContext::TargetMode::TypeDefn, sg::GeneratorContext::IndentMode::WithBrace).run(_config, fs, s);
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
z::StlcppGenerator::StlcppGenerator(const z::Ast::Project& project, const z::Ast::Config& config, const z::Ast::Module& module) : _impl(0) {_impl = new Impl(project, config, module);}
z::StlcppGenerator::~StlcppGenerator() {delete _impl;}
void z::StlcppGenerator::run() {return z::ref(_impl).run();}
