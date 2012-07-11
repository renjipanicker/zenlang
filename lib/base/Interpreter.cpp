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
        inline ValuePtr(const z::Ast::Expr* value) : _value(z::ref(value)) {}
        inline ~ValuePtr() {}

        inline void reset(const ValuePtr& val) {
            _value.reset(val.get());
        }

        template <typename T> inline bool isOfT() const {
            return _value.isOfT<T>();
        }

        template <typename T> inline const T& value() const {
            return _value.getT<T>();
        }

        inline const z::Ast::Expr& get() const {
            return _value.get();
        }

        template <typename T> inline operator T() const;

        inline bool isLong() const;
        inline bool isTrue() const;

        virtual z::string str() const {
            z::string estr = z::ZenlangGenerator::convertExprToString(_value.get());
            return estr;
        }

        inline ValuePtr(const ValuePtr& src) : _value(src._value) {}
    private:
        z::Ast::Ptr<const z::Ast::Expr> _value;
    };

    template <> inline ValuePtr::operator int64_t() const {
        assert(isLong());
        const z::Ast::ConstantLongExpr& val = value<z::Ast::ConstantLongExpr>();
        return val.value();
    }

    inline bool ValuePtr::isLong() const {
        if(isOfT<z::Ast::ConstantLongExpr>())
            return true;
        return false;
    }

    inline bool ValuePtr::isTrue() const {
        const ValuePtr& This = z::ref(this);
        if(isOfT<z::Ast::ConstantLongExpr>()) {
            if((int64_t)This)
                return true;
        }
        return false;
    }

    class InterpreterContext : public z::Ast::Unit::ScopeCallback {
    private:
        typedef std::map<const z::Ast::VariableDefn*, ValuePtr > ValueMap;
        ValueMap _valueMap;

    private:
        virtual void enteringScope(z::Ast::Scope& scope) {
            for(z::Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                const z::Ast::VariableDefn& vdef = it->get();
                /*ValuePtr& vptr = */_valueMap[z::ptr(vdef)];
            }
        }

        virtual void leavingScope(z::Ast::Scope& scope) {
            for(z::Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
                const z::Ast::VariableDefn& vdef = it->get();
                ValueMap::iterator vit = _valueMap.find(z::ptr(vdef));
                if(vit == _valueMap.end()) {
                    throw z::Exception("Interpreter", z::zfmt(scope.pos(), "Internal error: Variable %{s} not found in scope").arg("s", vdef.name()));
                }
                _valueMap.erase(vit);
            }
        }

    public:
        inline InterpreterContext(const z::Ast::Project& project, const z::Ast::Config& config, z::Ast::Token& pos)
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

        inline void addValue(const z::Ast::VariableDefn& key, const ValuePtr& val) {
            _valueMap[z::ptr(key)].reset(val);
        }

        inline bool hasValue(const z::Ast::VariableDefn& key) {
            ValueMap::iterator it = _valueMap.find(z::ptr(key));
            return (it != _valueMap.end());
        }

        inline const ValuePtr& getValue(const z::Ast::Token& pos, const z::Ast::VariableDefn& key) {
            ValueMap::iterator it = _valueMap.find(z::ptr(key));
            if(it == _valueMap.end()) {
                throw z::Exception("Interpreter", z::zfmt(pos, "Variable not found %{s}").arg("s", key.name()));
            }
            const ValuePtr& val = it->second;
            return val;
        }

    private:
        inline void process(const z::Ast::Module& module);

    private:
        const z::Ast::Config& _config;
        z::Ast::Unit _unit;
        z::Compiler _c;
    };

    struct BooleanOperator {
        inline ValuePtr run(ValuePtr& lhs, ValuePtr& rhs, const z::Ast::Token& op, const z::Ast::QualifiedTypeSpec& qTypeSpec) const {
            if(lhs.isLong() && rhs.isLong()) {
                int64_t nv = runLong((int64_t)lhs, (int64_t)rhs);
                return ValuePtr(new z::Ast::ConstantLongExpr(op, qTypeSpec, nv, 'd'));
            }
            throw z::Exception("Interpreter", z::zfmt(op, "Type mismatch"));
        }
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const = 0;
    };

    struct BooleanAndOperator : public BooleanOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs && (int64_t)rhs;
        }
    };

    struct BooleanOrOperator : public BooleanOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs || (int64_t)rhs;
        }
    };

    struct BooleanEqualOperator : public BooleanOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs == (int64_t)rhs;
        }
    };

    struct BooleanNotEqualOperator : public BooleanOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs != (int64_t)rhs;
        }
    };

    struct BooleanLessThanOperator : public BooleanOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs < (int64_t)rhs;
        }
    };

    struct BooleanGreaterThanOperator : public BooleanOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs > (int64_t)rhs;
        }
    };

    struct BooleanLessThanOrEqualOperator : public BooleanOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs <= (int64_t)rhs;
        }
    };

    struct BooleanGreaterThanOrEqualOperator : public BooleanOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs >= (int64_t)rhs;
        }
    };

    struct BooleanHasOperator : public BooleanOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            z::unused_t(lhs);
            z::unused_t(rhs);
            assert(false);
            return 0;
        }
    };

    struct BinaryOperator {
        inline ValuePtr run(ValuePtr& lhs, ValuePtr& rhs, const z::Ast::Token& op, const z::Ast::QualifiedTypeSpec& qTypeSpec) const {
            if(lhs.isLong() && rhs.isLong()) {
                int64_t nv = runLong((int64_t)lhs, (int64_t)rhs);
                return ValuePtr(new z::Ast::ConstantLongExpr(op, qTypeSpec, nv, 'd'));
            }
            throw z::Exception("Interpreter", z::zfmt(op, "Type mismatch"));
        }

        inline ValuePtr assign(ValuePtr& lhs, ValuePtr& rhs, const z::Ast::Token& op, const z::Ast::QualifiedTypeSpec& qTypeSpec) const {
            ValuePtr rv = run(lhs, rhs, op, qTypeSpec);
            return rv;
        }

        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const = 0;
    };

    struct BinaryNoopOperator : public BinaryOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            z::unused_t(lhs);
            return (int64_t)rhs;
        }
    };

    struct BinaryPlusOperator : public BinaryOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs + (int64_t)rhs;
        }
    };

    struct BinaryMinusOperator : public BinaryOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs - (int64_t)rhs;
        }
    };

    struct BinaryTimesOperator : public BinaryOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs * (int64_t)rhs;
        }
    };

    struct BinaryDivideOperator : public BinaryOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs / (int64_t)rhs;
        }
    };

    struct BinaryModOperator : public BinaryOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs % (int64_t)rhs;
        }
    };

    struct BinaryBitwiseAndOperator : public BinaryOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs & (int64_t)rhs;
        }
    };

    struct BinaryBitwiseOrOperator : public BinaryOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs | (int64_t)rhs;
        }
    };

    struct BinaryBitwiseXorOperator : public BinaryOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs ^ (int64_t)rhs;
        }
    };

    struct BinaryShiftLeftOperator : public BinaryOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs << (int64_t)rhs;
        }
    };

    struct BinaryShiftRightOperator : public BinaryOperator {
        virtual int64_t runLong(const int64_t& lhs, const int64_t& rhs) const {
            return (int64_t)lhs >> (int64_t)rhs;
        }
    };

    struct UnaryOperator {
        inline ValuePtr run(ValuePtr& lhs, const z::Ast::Token& op, const z::Ast::QualifiedTypeSpec& qTypeSpec) const {
            if(lhs.isLong()) {
                int64_t nv = runLong((int64_t)lhs);
                return ValuePtr(new z::Ast::ConstantLongExpr(op, qTypeSpec, nv, 'd'));
            }
            throw z::Exception("Interpreter", z::zfmt(op, "Type mismatch"));
        }
        virtual int64_t runLong(const int64_t& lhs) const = 0;
    };

    class ExprGenerator : public z::Ast::Expr::Visitor {
    private:
        typedef std::list<ValuePtr> Stack;
        Stack _stack;
    private:
        inline void push(const ValuePtr& val) {
            _stack.push_back(val);
        }

        inline ValuePtr pop() {
            ValuePtr val = _stack.back();
            _stack.pop_back();
            return val;
        }

    public:
        inline ValuePtr evaluate(const z::Ast::Expr& expr) {
            visitNode(expr);
            return pop();
        }

    private:
        virtual void visit(const z::Ast::ConditionalExpr& node) {
            ValuePtr lhs = evaluate(node.lhs());

            if(lhs.isTrue()) {
                visitNode(node.rhs1());
            } else {
                visitNode(node.rhs2());
            }
        }

        inline void visitBoolean(const z::Ast::BinaryExpr& node, const BooleanOperator& op) {
            ValuePtr lhs = evaluate(node.lhs());
            ValuePtr rhs = evaluate(node.rhs());
            push(op.run(lhs, rhs, node.op(), node.qTypeSpec()));
        }

        inline void visitBinary(const z::Ast::BinaryExpr& node, const BinaryOperator& op, const bool& assign = false) {
            ValuePtr lhs = evaluate(node.lhs());
            ValuePtr rhs = evaluate(node.rhs());
            if(assign) {
                push(op.assign(lhs, rhs, node.op(), node.qTypeSpec()));
                const z::Ast::Expr* pref = z::ptr(node.lhs());
                const z::Ast::VariableRefExpr* vRefExpr = dynamic_cast<const z::Ast::VariableRefExpr*>(pref);
                if(!vRefExpr) {
                    throw z::Exception("Interpreter", z::zfmt(node.op(), "LHS of assignment is not a variable reference"));
                }
                _ctx.addValue(z::ref(vRefExpr).vref(), rhs);
            } else {
                push(op.run(lhs, rhs, node.op(), node.qTypeSpec()));
            }
        }

        inline void visitBinaryAssign(const z::Ast::BinaryExpr& node, const BinaryOperator& op) {
            visitBinary(node, op, true);
        }

        virtual void visit(const z::Ast::BooleanAndExpr& node) {
            return visitBoolean(node, BooleanAndOperator());
        }

        virtual void visit(const z::Ast::BooleanOrExpr& node) {
            return visitBoolean(node, BooleanOrOperator());
        }

        virtual void visit(const z::Ast::BooleanEqualExpr& node) {
            return visitBoolean(node, BooleanEqualOperator());
        }

        virtual void visit(const z::Ast::BooleanNotEqualExpr& node) {
            return visitBoolean(node, BooleanNotEqualOperator());
        }

        virtual void visit(const z::Ast::BooleanLessThanExpr& node) {
            return visitBoolean(node, BooleanLessThanOperator());
        }

        virtual void visit(const z::Ast::BooleanGreaterThanExpr& node) {
            return visitBoolean(node, BooleanGreaterThanOperator());
        }

        virtual void visit(const z::Ast::BooleanLessThanOrEqualExpr& node) {
            return visitBoolean(node, BooleanLessThanOrEqualOperator());
        }

        virtual void visit(const z::Ast::BooleanGreaterThanOrEqualExpr& node) {
            return visitBoolean(node, BooleanGreaterThanOrEqualOperator());
        }

        virtual void visit(const z::Ast::BooleanHasExpr& node) {
            return visitBoolean(node, BooleanHasOperator());
        }

        virtual void visit(const z::Ast::BinaryAssignEqualExpr& node) {
            return visitBinaryAssign(node, BinaryNoopOperator());
        }

        virtual void visit(const z::Ast::BinaryPlusEqualExpr& node) {
            return visitBinaryAssign(node, BinaryPlusOperator());
        }

        virtual void visit(const z::Ast::BinaryMinusEqualExpr& node) {
            return visitBinaryAssign(node, BinaryMinusOperator());
        }

        virtual void visit(const z::Ast::BinaryTimesEqualExpr& node) {
            return visitBinaryAssign(node, BinaryTimesOperator());
        }

        virtual void visit(const z::Ast::BinaryDivideEqualExpr& node) {
            return visitBinaryAssign(node, BinaryDivideOperator());
        }

        virtual void visit(const z::Ast::BinaryModEqualExpr& node) {
            return visitBinaryAssign(node, BinaryModOperator());
        }

        virtual void visit(const z::Ast::BinaryBitwiseAndEqualExpr& node) {
            return visitBinaryAssign(node, BinaryBitwiseAndOperator());
        }

        virtual void visit(const z::Ast::BinaryBitwiseOrEqualExpr& node) {
            return visitBinaryAssign(node, BinaryBitwiseOrOperator());
        }

        virtual void visit(const z::Ast::BinaryBitwiseXorEqualExpr& node) {
            return visitBinaryAssign(node, BinaryBitwiseXorOperator());
        }

        virtual void visit(const z::Ast::BinaryShiftLeftEqualExpr& node) {
            return visitBinaryAssign(node, BinaryShiftLeftOperator());
        }

        virtual void visit(const z::Ast::BinaryShiftRightEqualExpr& node) {
            return visitBinaryAssign(node, BinaryShiftRightOperator());
        }

        virtual void visit(const z::Ast::BinaryPlusExpr& node) {
            return visitBinary(node, BinaryPlusOperator());
        }

        virtual void visit(const z::Ast::BinaryMinusExpr& node) {
            return visitBinary(node, BinaryMinusOperator());
        }

        virtual void visit(const z::Ast::BinaryTimesExpr& node) {
            return visitBinary(node, BinaryTimesOperator());
        }

        virtual void visit(const z::Ast::BinaryDivideExpr& node) {
            return visitBinary(node, BinaryDivideOperator());
        }

        virtual void visit(const z::Ast::BinaryModExpr& node) {
            return visitBinary(node, BinaryModOperator());
        }

        virtual void visit(const z::Ast::BinaryBitwiseAndExpr& node) {
            return visitBinary(node, BinaryBitwiseAndOperator());
        }

        virtual void visit(const z::Ast::BinaryBitwiseOrExpr& node) {
            return visitBinary(node, BinaryBitwiseOrOperator());
        }

        virtual void visit(const z::Ast::BinaryBitwiseXorExpr& node) {
            return visitBinary(node, BinaryBitwiseXorOperator());
        }

        virtual void visit(const z::Ast::BinaryShiftLeftExpr& node) {
            return visitBinary(node, BinaryShiftLeftOperator());
        }

        virtual void visit(const z::Ast::BinaryShiftRightExpr& node) {
            return visitBinary(node, BinaryShiftRightOperator());
        }

        virtual void visit(const z::Ast::PostfixIncExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::PostfixDecExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::PrefixNotExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::PrefixPlusExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::PrefixMinusExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::PrefixIncExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::PrefixDecExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::PrefixBitwiseNotExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::SetIndexExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ListExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::DictExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::FormatExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::RoutineCallExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::FunctorCallExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::RunExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::OrderedExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::IndexExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::SpliceExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::SizeofTypeExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::SizeofExprExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::TypeofTypeExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::TypeofExprExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::StaticTypecastExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::DynamicTypecastExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::PointerInstanceExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ValueInstanceExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::MapDataInstanceExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::DeRefInstanceExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::VariableRefExpr& node) {
            ValuePtr val = _ctx.getValue(node.pos(), node.vref());
            push(val);
        }

        virtual void visit(const z::Ast::MemberVariableExpr& node) {
            visitNode(node.expr());
        }

        virtual void visit(const z::Ast::MemberPropertyExpr& node) {
            visitNode(node.expr());
        }

        virtual void visit(const z::Ast::EnumMemberExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::StructMemberExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::StructInstanceExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::FunctionInstanceExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::AnonymousFunctionExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ConstantNullExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ConstantFloatExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ConstantDoubleExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ConstantBooleanExpr& node) {
            push(new z::Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value(), 'd'));
        }

        virtual void visit(const z::Ast::ConstantStringExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ConstantCharExpr& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ConstantLongExpr& node) {
            push(new z::Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value(), 'd'));
        }

        virtual void visit(const z::Ast::ConstantIntExpr& node) {
            push(new z::Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value(), 'd'));
        }

        virtual void visit(const z::Ast::ConstantShortExpr& node) {
            push(new z::Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value(), 'd'));
        }

        virtual void visit(const z::Ast::ConstantByteExpr& node) {
            push(new z::Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value(), 'd'));
        }

        virtual void visit(const z::Ast::ConstantUnLongExpr& node) {
            push(new z::Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value(), 'd'));
        }

        virtual void visit(const z::Ast::ConstantUnIntExpr& node) {
            push(new z::Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value(), 'd'));
        }

        virtual void visit(const z::Ast::ConstantUnShortExpr& node) {
            push(new z::Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value(), 'd'));
        }

        virtual void visit(const z::Ast::ConstantUnByteExpr& node) {
            push(new z::Ast::ConstantLongExpr(node.pos(), node.qTypeSpec(), node.value(), 'd'));
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

    class StatementGenerator : public z::Ast::Statement::Visitor {
        virtual void visit(const z::Ast::ImportStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::EnterNamespaceStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::LeaveNamespaceStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::UserDefinedTypeSpecStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::StructMemberVariableStatement& node) {
            ExprGenerator(_ctx).evaluate(node.defn().initExpr());
        }

        virtual void visit(const z::Ast::StructInitStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::EmptyStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::AutoStatement& node) {
            ValuePtr p = ExprGenerator(_ctx).evaluate(node.defn().initExpr());
            _ctx.addValue(node.defn(), p);
        }

        virtual void visit(const z::Ast::ExprStatement& node) {
            ExprGenerator g(_ctx);
            /*ValuePtr p = */ExprGenerator(_ctx).evaluate(node.expr());
        }

        virtual void visit(const z::Ast::PrintStatement& node) {
            ValuePtr p = ExprGenerator(_ctx).evaluate(node.expr());
            std::cout << p.str() << std::endl;
        }

        virtual void visit(const z::Ast::IfStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::IfElseStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::WhileStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::DoWhileStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ForExprStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ForInitStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ForeachStringStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ForeachListStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ForeachDictStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::CaseExprStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::CaseDefaultStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::SwitchValueStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::SwitchExprStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::BreakStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ContinueStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::AddEventHandlerStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::RoutineReturnStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::FunctionReturnStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::RaiseStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::ExitStatement& node) {
            z::unused_t(node);
        }

        virtual void visit(const z::Ast::CompoundStatement& node) {
            for(z::Ast::CompoundStatement::List::const_iterator it = node.list().begin(); it != node.list().end(); ++it) {
                const z::Ast::Statement& s = it->get();
                z::ref(this).visitNode(s);
            }
        }

    private:
        const z::Ast::Config& _config;
        InterpreterContext& _ctx;

    public:
        inline StatementGenerator(const z::Ast::Config& config, InterpreterContext& ctx) : _config(config), _ctx(ctx) {}
    };

    inline void InterpreterContext::process(const z::Ast::Module& module) {
        StatementGenerator gen(_config, z::ref(this));
        for(z::Ast::CompoundStatement::List::const_iterator it = module.globalStatementList().list().begin(); it != module.globalStatementList().list().end(); ++it) {
            const z::Ast::Statement& s = it->get();
            gen.visitNode(s);
        }
    }

    inline void InterpreterContext::processCmd(const z::string& cmd) {
        std::cout << cmd << std::endl;
        z::Parser parser;
        z::Lexer lexer(parser);
        z::Ast::Module module(_unit, "<cmd>", 0);
        _c.compileString(module, lexer, cmd, true);
        process(module);
    }

    inline void InterpreterContext::processFile(const z::string& filename) {
        z::Ast::Module module(_unit, filename, 0);
        _c.compileFile(module, filename, "Loading");
        process(module);
    }
}

struct z::Interpreter::Impl {
    inline Impl(const z::Ast::Project& project, const z::Ast::Config& config) : _project(project), _config(config) {}
    inline void run();
private:
    const z::Ast::Project& _project;
    const z::Ast::Config& _config;
};

inline void z::Interpreter::Impl::run() {
    printf("Entering interpretor mode\n");

    z::Ast::Token pos("", 0, 0, "");
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
        for(z::Ast::Config::PathList::const_iterator it = _config.sourceFileList().begin(); it != _config.sourceFileList().end(); ++it) {
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
z::Interpreter::Interpreter(const z::Ast::Project& project, const z::Ast::Config& config) : _impl(0) {_impl = new Impl(project, config);}
z::Interpreter::~Interpreter() {delete _impl;}
void z::Interpreter::run() {return z::ref(_impl).run();}
