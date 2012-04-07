#include "zenlang.hpp"
#include "base/base.hpp"
#include "base/typename.hpp"
#include "base/Interpreter.hpp"
#include "base/compiler.hpp"
#include "base/ZenlangGenerator.hpp"

//#define DBGMODE 1

namespace in {
    struct ValuePtr {
        inline ValuePtr() {}
        inline ValuePtr(const Ast::Expr* value) : _value(z::ref(value)) {}
        inline ~ValuePtr() {}

        inline void reset(const ValuePtr& val) {
            _value.reset(val.get());
        }

        template <typename T> inline bool check() const {
            return _value.check<T>();
        }

        template <typename T> inline const T& value() const {
            return _value.getT<T>();
        }

        inline const Ast::Expr& get() const {
            return _value.get();
        }

        template <typename T> inline operator T() const;

        inline bool isLong() const;
        inline bool isTrue() const;

        virtual z::string str() const {
            z::string estr = ZenlangGenerator::convertExprToString(_value.get());
            return estr;
        }

        inline ValuePtr(const ValuePtr& src) : _value(src._value) {}
    private:
        Ast::Ptr<const Ast::Expr> _value;
    };

    template <> inline ValuePtr::operator long() const {
        assert(isLong());
        const Ast::ConstantLongExpr& val = value<Ast::ConstantLongExpr>();
        return val.value();
    }

    inline bool ValuePtr::isLong() const {
        if(check<Ast::ConstantLongExpr>())
            return true;
        return false;
    }

    inline bool ValuePtr::isTrue() const {
        const ValuePtr& This = z::ref(this);
        if(check<Ast::ConstantLongExpr>()) {
            if((long)This)
                return true;
        }
        return false;
    }

    class InterpreterContext : public Ast::Unit::ScopeCallback {
    private:
        typedef std::map<const Ast::VariableDefn*, ValuePtr > ValueMap;
        ValueMap _valueMap;

    private:
        virtual void enteringScope(Ast::Scope& scope) {
//            trace("InterpreterContext::enteringScope %lu\n", z::pad(scope));
            for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                const Ast::VariableDefn& vdef = it->get();
//                trace("Adding variable %s to scope\n", vdef.name().text());
                /*ValuePtr& vptr = */_valueMap[z::ptr(vdef)];
            }
        }

        virtual void leavingScope(Ast::Scope& scope) {
//            trace("InterpreterContext::leavingScope %lu\n", z::pad(scope));
            for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                const Ast::VariableDefn& vdef = it->get();
                ValueMap::iterator vit = _valueMap.find(z::ptr(vdef));
//                trace("Deleting variable %s from scope\n", vdef.name().text());
                if(vit == _valueMap.end()) {
                    throw z::Exception("Interpreter", zfmt(scope.pos(), "Internal error: Variable %{s} not found in scope").arg("s", vdef.name()));
                }
                _valueMap.erase(vit);
            }
        }

    public:
        inline InterpreterContext(const Ast::Project& project, const Ast::Config& config, Ast::Token& pos)
            : _config(config), _c(project, config) {
            _unit.setScopeCallback(this);
#if !defined(DBGMODE)
            _c.initContext(_unit);
#endif
            _unit.enterScope(pos);
        }

        inline ~InterpreterContext() {
            _unit.leaveScope();
        }

        inline void reset() {
        }

        inline void processCmd(const z::string& cmd);
        inline void processFile(const z::string& filename);

        inline void addValue(const Ast::VariableDefn& key, const ValuePtr& val) {
//            trace("InterpreterContext::addValue %lu, %s\n", z::pad(val.get()), val.str().c_str());
            _valueMap[z::ptr(key)].reset(val);
        }

        inline bool hasValue(const Ast::VariableDefn& key) {
            ValueMap::iterator it = _valueMap.find(z::ptr(key));
            return (it != _valueMap.end());
        }

        inline const ValuePtr& getValue(const Ast::Token& pos, const Ast::VariableDefn& key) {
            ValueMap::iterator it = _valueMap.find(z::ptr(key));
            if(it == _valueMap.end()) {
                throw z::Exception("Interpreter", zfmt(pos, "Variable not found %{s}").arg("s", key.name()));
            }
            const ValuePtr& val = it->second;
//            trace("InterpreterContext::getValue %lu %s\n", z::pad(val.get()), val.str().c_str());
            return val;
        }

    private:
        inline void process(const Ast::Module& module);

    private:
        const Ast::Config& _config;
        Ast::Unit _unit;
        Compiler _c;
    };

    struct BooleanOperator {
        inline ValuePtr run(ValuePtr& lhs, ValuePtr& rhs, const Ast::Token& op, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            if(lhs.isLong() && rhs.isLong()) {
                long nv = runLong((long)lhs, (long)rhs);
                return ValuePtr(new Ast::ConstantLongExpr(op, qTypeSpec, nv));
            }
            throw z::Exception("Interpreter", zfmt(op, "Type mismatch"));
        }
        virtual long runLong(const long& lhs, const long& rhs) const = 0;
    };

    struct BooleanAndOperator : public BooleanOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs && (long)rhs;
        }
    };

    struct BooleanOrOperator : public BooleanOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs || (long)rhs;
        }
    };

    struct BooleanEqualOperator : public BooleanOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs == (long)rhs;
        }
    };

    struct BooleanNotEqualOperator : public BooleanOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs != (long)rhs;
        }
    };

    struct BooleanLessThanOperator : public BooleanOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs < (long)rhs;
        }
    };

    struct BooleanGreaterThanOperator : public BooleanOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs > (long)rhs;
        }
    };

    struct BooleanLessThanOrEqualOperator : public BooleanOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs <= (long)rhs;
        }
    };

    struct BooleanGreaterThanOrEqualOperator : public BooleanOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs >= (long)rhs;
        }
    };

    struct BooleanHasOperator : public BooleanOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            unused(lhs);
            unused(rhs);
            assert(false);
            return 0;
        }
    };

    struct BinaryOperator {
        inline ValuePtr run(ValuePtr& lhs, ValuePtr& rhs, const Ast::Token& op, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            if(lhs.isLong() && rhs.isLong()) {
                long nv = runLong((long)lhs, (long)rhs);
                return ValuePtr(new Ast::ConstantLongExpr(op, qTypeSpec, nv));
            }
            throw z::Exception("Interpreter", zfmt(op, "Type mismatch"));
        }

        inline ValuePtr assign(ValuePtr& lhs, ValuePtr& rhs, const Ast::Token& op, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            ValuePtr rv = run(lhs, rhs, op, qTypeSpec);
            return rv;
        }

        virtual long runLong(const long& lhs, const long& rhs) const = 0;
    };

    struct BinaryNoopOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            unused(lhs);
            return (long)rhs;
        }
    };

    struct BinaryPlusOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs + (long)rhs;
        }
    };

    struct BinaryMinusOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs - (long)rhs;
        }
    };

    struct BinaryTimesOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs * (long)rhs;
        }
    };

    struct BinaryDivideOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs / (long)rhs;
        }
    };

    struct BinaryModOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs % (long)rhs;
        }
    };

    struct BinaryBitwiseAndOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs & (long)rhs;
        }
    };

    struct BinaryBitwiseOrOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs | (long)rhs;
        }
    };

    struct BinaryBitwiseXorOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs ^ (long)rhs;
        }
    };

    struct BinaryShiftLeftOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs << (long)rhs;
        }
    };

    struct BinaryShiftRightOperator : public BinaryOperator {
        virtual long runLong(const long& lhs, const long& rhs) const {
            return (long)lhs >> (long)rhs;
        }
    };

    struct UnaryOperator {
        inline ValuePtr run(ValuePtr& lhs, const Ast::Token& op, const Ast::QualifiedTypeSpec& qTypeSpec) const {
            if(lhs.isLong()) {
                long nv = runLong((long)lhs);
                return ValuePtr(new Ast::ConstantLongExpr(op, qTypeSpec, nv));
            }
            throw z::Exception("Interpreter", zfmt(op, "Type mismatch"));
        }
        virtual long runLong(const long& lhs) const = 0;
    };

    class ExprGenerator : public Ast::Expr::Visitor {
    private:
        typedef std::list<ValuePtr> Stack;
        Stack _stack;
    private:
        inline void push(const ValuePtr& val) {
//            trace("ExprGenerator::push %lu, %s\n", z::pad(val.get()), val.str().c_str());
            _stack.push_back(val);
        }

        inline ValuePtr pop() {
            ValuePtr val = _stack.back();
            _stack.pop_back();
//            trace("ExprGenerator::pop %lu, %s\n", z::pad(val.get()), val.str().c_str());
            return val;
        }

    public:
        inline ValuePtr evaluate(const Ast::Expr& expr) {
            visitNode(expr);
            return pop();
        }

    private:
        virtual void visit(const Ast::ConditionalExpr& node) {
            ValuePtr lhs = evaluate(node.lhs());

            if(lhs.isTrue()) {
                visitNode(node.rhs1());
            } else {
                visitNode(node.rhs2());
            }
        }

        inline void visitBoolean(const Ast::BinaryExpr& node, const BooleanOperator& op) {
            ValuePtr lhs = evaluate(node.lhs());
            ValuePtr rhs = evaluate(node.rhs());
            push(op.run(lhs, rhs, node.op(), node.qTypeSpec()));
        }

        inline void visitBinary(const Ast::BinaryExpr& node, const BinaryOperator& op, const bool& assign = false) {
            ValuePtr lhs = evaluate(node.lhs());
            ValuePtr rhs = evaluate(node.rhs());
            if(assign) {
                push(op.assign(lhs, rhs, node.op(), node.qTypeSpec()));
                const Ast::Expr* pref = z::ptr(node.lhs());
                const Ast::VariableRefExpr* vRefExpr = dynamic_cast<const Ast::VariableRefExpr*>(pref);
                if(!vRefExpr) {
                    throw z::Exception("Interpreter", zfmt(node.op(), "LHS of assignment is not a variable reference"));
                }
                _ctx.addValue(z::ref(vRefExpr).vref(), rhs);
            } else {
                push(op.run(lhs, rhs, node.op(), node.qTypeSpec()));
            }
        }

        inline void visitBinaryAssign(const Ast::BinaryExpr& node, const BinaryOperator& op) {
            visitBinary(node, op, true);
        }

        virtual void visit(const Ast::BooleanAndExpr& node) {
            return visitBoolean(node, BooleanAndOperator());
        }

        virtual void visit(const Ast::BooleanOrExpr& node) {
            return visitBoolean(node, BooleanOrOperator());
        }

        virtual void visit(const Ast::BooleanEqualExpr& node) {
            return visitBoolean(node, BooleanEqualOperator());
        }

        virtual void visit(const Ast::BooleanNotEqualExpr& node) {
            return visitBoolean(node, BooleanNotEqualOperator());
        }

        virtual void visit(const Ast::BooleanLessThanExpr& node) {
            return visitBoolean(node, BooleanLessThanOperator());
        }

        virtual void visit(const Ast::BooleanGreaterThanExpr& node) {
            return visitBoolean(node, BooleanGreaterThanOperator());
        }

        virtual void visit(const Ast::BooleanLessThanOrEqualExpr& node) {
            return visitBoolean(node, BooleanLessThanOrEqualOperator());
        }

        virtual void visit(const Ast::BooleanGreaterThanOrEqualExpr& node) {
            return visitBoolean(node, BooleanGreaterThanOrEqualOperator());
        }

        virtual void visit(const Ast::BooleanHasExpr& node) {
            return visitBoolean(node, BooleanHasOperator());
        }

        virtual void visit(const Ast::BinaryAssignEqualExpr& node) {
            return visitBinaryAssign(node, BinaryNoopOperator());
        }

        virtual void visit(const Ast::BinaryPlusEqualExpr& node) {
            return visitBinaryAssign(node, BinaryPlusOperator());
        }

        virtual void visit(const Ast::BinaryMinusEqualExpr& node) {
            return visitBinaryAssign(node, BinaryMinusOperator());
        }

        virtual void visit(const Ast::BinaryTimesEqualExpr& node) {
            return visitBinaryAssign(node, BinaryTimesOperator());
        }

        virtual void visit(const Ast::BinaryDivideEqualExpr& node) {
            return visitBinaryAssign(node, BinaryDivideOperator());
        }

        virtual void visit(const Ast::BinaryModEqualExpr& node) {
            return visitBinaryAssign(node, BinaryModOperator());
        }

        virtual void visit(const Ast::BinaryBitwiseAndEqualExpr& node) {
            return visitBinaryAssign(node, BinaryBitwiseAndOperator());
        }

        virtual void visit(const Ast::BinaryBitwiseOrEqualExpr& node) {
            return visitBinaryAssign(node, BinaryBitwiseOrOperator());
        }

        virtual void visit(const Ast::BinaryBitwiseXorEqualExpr& node) {
            return visitBinaryAssign(node, BinaryBitwiseXorOperator());
        }

        virtual void visit(const Ast::BinaryShiftLeftEqualExpr& node) {
            return visitBinaryAssign(node, BinaryShiftLeftOperator());
        }

        virtual void visit(const Ast::BinaryShiftRightEqualExpr& node) {
            return visitBinaryAssign(node, BinaryShiftRightOperator());
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
            return visitBinary(node, BinaryModOperator());
        }

        virtual void visit(const Ast::BinaryBitwiseAndExpr& node) {
            return visitBinary(node, BinaryBitwiseAndOperator());
        }

        virtual void visit(const Ast::BinaryBitwiseOrExpr& node) {
            return visitBinary(node, BinaryBitwiseOrOperator());
        }

        virtual void visit(const Ast::BinaryBitwiseXorExpr& node) {
            return visitBinary(node, BinaryBitwiseXorOperator());
        }

        virtual void visit(const Ast::BinaryShiftLeftExpr& node) {
            return visitBinary(node, BinaryShiftLeftOperator());
        }

        virtual void visit(const Ast::BinaryShiftRightExpr& node) {
            return visitBinary(node, BinaryShiftRightOperator());
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
//            trace("var-ref %s (%lu)\n", node.vref().name().text(), z::pad(node.vref()));
            ValuePtr val = _ctx.getValue(node.pos(), node.vref());
            push(val);
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

        virtual void visit(const Ast::ConstantNullExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantFloatExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantDoubleExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantBooleanExpr& node) {
            push(new Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value()));
        }

        virtual void visit(const Ast::ConstantStringExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantCharExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantLongExpr& node) {
            push(new Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value()));
        }

        virtual void visit(const Ast::ConstantIntExpr& node) {
            push(new Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value()));
        }

        virtual void visit(const Ast::ConstantShortExpr& node) {
            push(new Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value()));
        }

        virtual void sep() {
        }

    private:
        InterpreterContext& _ctx;

    public:
        inline ExprGenerator(InterpreterContext& ctx) : _ctx(ctx) {}
        inline ~ExprGenerator() {
            assert(_stack.size() == 0);
        }
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
            ExprGenerator(_ctx).evaluate(node.defn().initExpr());
        }

        virtual void visit(const Ast::StructInitStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::EmptyStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::AutoStatement& node) {
//            trace("auto statement %s (%lu)\n", node.defn().name().text(), z::pad(node.defn()));
            ValuePtr p = ExprGenerator(_ctx).evaluate(node.defn().initExpr());
            _ctx.addValue(node.defn(), p);
        }

        virtual void visit(const Ast::ExprStatement& node) {
//            trace("expr statement\n");
            ExprGenerator g(_ctx);
            /*ValuePtr p = */ExprGenerator(_ctx).evaluate(node.expr());
        }

        virtual void visit(const Ast::PrintStatement& node) {
            ValuePtr p = ExprGenerator(_ctx).evaluate(node.expr());
            std::cout << p.str() << std::endl;
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
            for(Ast::CompoundStatement::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const Ast::Statement& s = it->get();
                z::ref(this).visitNode(s);
            }
        }

    private:
        const Ast::Config& _config;
        InterpreterContext& _ctx;

    public:
        inline StatementGenerator(const Ast::Config& config, InterpreterContext& ctx) : _config(config), _ctx(ctx) {}
    };

    inline void InterpreterContext::process(const Ast::Module& module) {
        StatementGenerator gen(_config, z::ref(this));
        for(Ast::CompoundStatement::List::const_iterator it = module.globalStatementList().list().begin(); it != module.globalStatementList().list().end(); ++it) {
            const Ast::Statement& s = it->get();
            gen.visitNode(s);
        }
    }

    inline void InterpreterContext::processCmd(const z::string& cmd) {
        std::cout << cmd << std::endl;
        Parser parser;
        Lexer lexer(parser);
        Ast::Module module(_unit, "<cmd>", 0);
        _c.compileString(module, lexer, cmd, true);
        process(module);
    }

    inline void InterpreterContext::processFile(const z::string& filename) {
        Ast::Module module(_unit, filename, 0);
        _c.compileFile(module, filename, "Loading");
        process(module);
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

    Ast::Token pos("", 0, 0, "");
    in::InterpreterContext ctx(_project, _config, pos);

#if defined(DBGMODE)
//    const char* str =
//            "typedef int native;\n"
//            "auto i = 0;"
//        ;
//    ctx.processCmd(str);

    ctx.processCmd("typedef int native;");
    ctx.processCmd("auto i = 0;");
    ctx.processCmd("i = 23;");
    ctx.processCmd("print i;");
    return;
#endif

    if(_config.sourceFileList().size() > 0) {
        for(Ast::Config::PathList::const_iterator it = _config.sourceFileList().begin(); it != _config.sourceFileList().end(); ++it) {
            const z::string& filename = *it;
            ctx.processFile(filename);
        }
    } else {
        bool quit = false;
        while (quit == false) {
            std::cout << ">";
            std::string cmd;
            std::getline(std::cin, cmd);
            if(cmd == ".q")
                break;
            try {
                z::string icmd = z::e2s(cmd);
                ctx.processCmd(icmd);
            } catch (...) {
                ctx.reset();
            }
        }
    }
}

//////////////////////////////////////////////
Interpreter::Interpreter(const Ast::Project& project, const Ast::Config& config) : _impl(0) {_impl = new Impl(project, config);}
Interpreter::~Interpreter() {delete _impl;}
void Interpreter::run() {return z::ref(_impl).run();}
