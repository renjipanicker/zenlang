#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "Interpreter.hpp"
#include "ZenlangGenerator.hpp"
#include "typename.hpp"
#include "compiler.hpp"

namespace {
    struct ValuePtr {
        inline ValuePtr() : _value(0) {}
        inline ~ValuePtr() {delete _value;}

        inline void reset(const Ast::Expr* value) {
            assert(_value == 0);
            _value = value;
        }

        template <typename T> inline bool check() const {
            return (dynamic_cast<const T*>(_value) != 0);
        }

        template <typename T>
        inline const T& value() const {
            assert(_value != 0);
            return z::ref(dynamic_cast<const T*>(_value));
        }

        template <typename T> inline operator T() const {
            return value<T>();
        }

        virtual std::string str() const {
            assert(_value != 0);
            std::string estr = ZenlangGenerator::convertExprToString(z::ref(_value));
            return estr;
        }

    private:
        inline ValuePtr(const ValuePtr& src) : _value(0) {}
        const Ast::Expr* _value;
    };

    template <> inline ValuePtr::operator long() const {
        const Ast::ConstantLongExpr& val = value<Ast::ConstantLongExpr>();
        return val.value();
    }

    struct BinaryOperator {
        inline const Ast::Expr* run(ValuePtr& lhs, ValuePtr& rhs, const Ast::Token& op, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            if(lhs.check<Ast::ConstantLongExpr>() && rhs.check<Ast::ConstantLongExpr>()) {
                long nv = runLong((long)lhs, (long)rhs, qTypeSpec);
                return new Ast::ConstantLongExpr(qTypeSpec, op, nv);
            }
            throw z::Exception("Type mismatch\n");
        }
        virtual long runLong(const long& lhs, const long& rhs, const Ast::QualifiedTypeSpec& qTypeSpec) const = 0;
    };

    struct BooleanAndOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            return (long)lhs && (long)rhs;
        }
    };

    struct BooleanOrOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            return (long)lhs || (long)rhs;
        }
    };

    struct BooleanEqualOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            return (long)lhs == (long)rhs;
        }
    };

    struct BooleanNotEqualOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            return (long)lhs != (long)rhs;
        }
    };

    struct BinaryPlusOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            return (long)lhs + (long)rhs;
        }
    };

    struct BinaryMinusOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            return (long)lhs - (long)rhs;
        }
    };

    struct BinaryTimesOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            return (long)lhs * (long)rhs;
        }
    };

    struct BinaryDivideOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            return (long)lhs / (long)rhs;
        }
    };

    class InterpreterContext {
    public:
        inline InterpreterContext(const Ast::Project& project, const Ast::Config& config, Ast::Token& pos)
            : _config(config), _ctx(_unit, "<cmd>", 0), _unit(""), _c(project, config), _global(pos, Ast::ScopeType::Local) {
            _c.initContext(_unit);
            _ctx.enterScope(_global);
        }

        inline ~InterpreterContext() {
            _ctx.leaveScope(_global);
        }

        void setVisitor(Ast::Statement::Visitor& visitor) {
            _ctx.setStatementVisitor(visitor);
        }

        void reset() {
        }

        void process(const std::string& cmd);

    private:
        const Ast::Config& _config;
        Ast::Context _ctx;
        Ast::Unit _unit;
        Compiler _c;
        Ast::Scope _global;
    };

    class ExprGenerator : public Ast::Expr::Visitor {
    private:
        typedef std::list<const Ast::Expr*> Stack;
        Stack _stack;
    private:
        inline void push(const Ast::Expr* value) {
            _stack.push_back(value);
        }

        inline void pop(ValuePtr& ptr) {
            assert(_stack.size() > 0);
            const Ast::Expr* val = _stack.back();
            _stack.pop_back();
            ptr.reset(val);
        }

    private:
        virtual void visit(const Ast::ConditionalExpr& node) {
            visitNode(node.lhs());
            ValuePtr lhs;
            pop(lhs);

            bool cond = false;
//            if(lhs.check<IntValue>()) {
//                cond = ((long)lhs != 0);
//            } else /*if*/ { //add more here
//            }

            if(cond) {
                visitNode(node.rhs1());
            } else {
                visitNode(node.rhs2());
            }
        }

        inline void visitBinary(const Ast::BinaryExpr& node, ValuePtr& lhs, ValuePtr& rhs) {
            visitNode(node.lhs());
            pop(lhs);
            visitNode(node.rhs());
            pop(rhs);
        }

        inline void visitBinary(const Ast::BinaryExpr& node, const BinaryOperator& op) {
            visitNode(node.lhs());
            ValuePtr lhs;
            pop(lhs);

            visitNode(node.rhs());
            ValuePtr rhs;
            pop(rhs);

            push(op.run(lhs, rhs, node.op(), node.qTypeSpec()));
        }

        virtual void visit(const Ast::BooleanAndExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BooleanOrExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BooleanEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BooleanNotEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BooleanLessThanExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BooleanGreaterThanExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BooleanLessThanOrEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BooleanGreaterThanOrEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BooleanHasExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryAssignEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryPlusEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryMinusEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryTimesEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryDivideEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryModEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryBitwiseAndEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryBitwiseOrEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryBitwiseXorEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryShiftLeftEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryShiftRightEqualExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryPlusExpr& node) {
            return visitBinary(node, BinaryPlusOperator());
        }

        virtual void visit(const Ast::BinaryMinusExpr& node) {
            return visitBinary(node, BinaryMinusOperator());
        }

        virtual void visit(const Ast::BinaryTimesExpr& node) {
            return visitBinary(node, BinaryTimesOperator());
        }

        virtual void visit(const Ast::BinaryDivideExpr& node) {
            return visitBinary(node, BinaryDivideOperator());
        }

        virtual void visit(const Ast::BinaryModExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryBitwiseAndExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryBitwiseOrExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryBitwiseXorExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryShiftLeftExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::BinaryShiftRightExpr& node) {
            ValuePtr lhs;
            ValuePtr rhs;
            visitBinary(node, lhs, rhs);
        }

        virtual void visit(const Ast::PostfixIncExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::PostfixDecExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::PrefixNotExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::PrefixPlusExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::PrefixMinusExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::PrefixIncExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::PrefixDecExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::PrefixBitwiseNotExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::SetIndexExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ListExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::DictExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::FormatExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::RoutineCallExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::FunctorCallExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::RunExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::OrderedExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::IndexExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::SpliceExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::TypeofTypeExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::TypeofExprExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::StaticTypecastExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::DynamicTypecastExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::PointerInstanceExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ValueInstanceExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::VariableRefExpr& node) {
            trace("var-ref %s (%lu)\n", node.vref().name().text(), z::pad(node.vref()));
        }

        virtual void visit(const Ast::MemberVariableExpr& node) {
            visitNode(node.expr());
        }

        virtual void visit(const Ast::MemberPropertyExpr& node) {
            visitNode(node.expr());
        }

        virtual void visit(const Ast::EnumMemberExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::StructMemberExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::StructInstanceExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::FunctionInstanceExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::AnonymousFunctionExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantFloatExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantDoubleExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantBooleanExpr& node) {
            push(new Ast::ConstantLongExpr(node.qTypeSpec(), node.token(), node.value()));
        }

        virtual void visit(const Ast::ConstantStringExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantCharExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantLongExpr& node) {
            push(new Ast::ConstantLongExpr(node.qTypeSpec(), node.token(), node.value()));
        }

        virtual void visit(const Ast::ConstantIntExpr& node) {
            push(new Ast::ConstantLongExpr(node.qTypeSpec(), node.token(), node.value()));
        }

        virtual void visit(const Ast::ConstantShortExpr& node) {
            push(new Ast::ConstantLongExpr(node.qTypeSpec(), node.token(), node.value()));
        }

        virtual void sep() {
        }

    private:
        InterpreterContext& _ctx;

    public:
        inline ExprGenerator(InterpreterContext& ctx) : _ctx(ctx) {}
        inline void value(ValuePtr& ptr) {return pop(ptr);}
    };

    class StatementGenerator : public Ast::Statement::Visitor {
        virtual void visit(const Ast::ImportStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::EnterNamespaceStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::LeaveNamespaceStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::UserDefinedTypeSpecStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::StructMemberVariableStatement& node) {
            ExprGenerator(_ctx).visitNode(node.defn().initExpr());
        }

        virtual void visit(const Ast::StructInitStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::EmptyStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::AutoStatement& node) {
            trace("auto statement %s (%lu)\n", node.defn().name().text(), z::pad(node.defn()));
            ExprGenerator(_ctx).visitNode(node.defn().initExpr());
        }

        virtual void visit(const Ast::ExprStatement& node) {
            trace("expr statement\n");
            ExprGenerator(_ctx).visitNode(node.expr());
        }

        virtual void visit(const Ast::PrintStatement& node) {
            ExprGenerator g(_ctx);
            g.visitNode(node.expr());
            ValuePtr p;
            g.value(p);
            printf("%s\n", p.str().c_str());
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
            for(Ast::CompoundStatement::List::const_iterator sit = node.list().begin(); sit != node.list().end(); ++sit) {
                const Ast::Statement& s = z::ref(*sit);
                z::ref(this).visitNode(s);
            }
        }

    private:
        const Ast::Config& _config;
        InterpreterContext& _ctx;

    public:
        inline StatementGenerator(const Ast::Config& config, InterpreterContext& ctx) : _config(config), _ctx(ctx) {}
    };

    void InterpreterContext::process(const std::string& cmd) {
        std::cout << cmd << std::endl;
        Ast::Module module(_unit);
        Parser parser;
        Lexer lexer(parser);
        _c.parseString(_ctx, lexer, module, cmd, 0, true);
    }
}

struct Interpreter::Impl {
    inline Impl(const Ast::Project& project, const Ast::Config& config) : _project(project), _config(config) {}
    inline void run();
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
};

inline void Interpreter::Impl::run() {
    printf("Entering interpretor mode\n");

    Ast::Token pos(0, 0, "");
    InterpreterContext ctx(_project, _config, pos);
    StatementGenerator gen(_config, ctx);
    ctx.setVisitor(gen);

    bool quit = false;
    while (quit == false) {
        std::cout << ">";
        std::string cmd;
        std::getline(std::cin, cmd);
        if(cmd == ".q")
            break;
        try {
            ctx.process(cmd);
        } catch (...) {
            ctx.reset();
        }
    }
}

//////////////////////////////////////////////
Interpreter::Interpreter(const Ast::Project& project, const Ast::Config& config) : _impl(0) {_impl = new Impl(project, config);}
Interpreter::~Interpreter() {delete _impl;}
void Interpreter::run() {return z::ref(_impl).run();}
