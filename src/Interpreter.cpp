#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "Interpreter.hpp"
#include "typename.hpp"
#include "compiler.hpp"

namespace {
    class InterpreterContext {
    public:
        inline InterpreterContext(const Ast::Project& project, const Ast::Config& config, Ast::Token& pos)
            : _config(config), _unit(""), _c(project, config), _global(pos, Ast::ScopeType::Local), _lexer(_parser) {
            _c.initContext(_unit);
            _unit.enterScope(_global);
        }
        inline ~InterpreterContext() {
            _unit.leaveScope(_global);
        }

        void process(const std::string& cmd);
    private:
        const Ast::Config& _config;
        Ast::Unit _unit;
        Compiler _c;
        Ast::Scope _global;
        Parser _parser;
        Lexer _lexer;
    };

    struct ExprGenerator : public Ast::Expr::Visitor {
    public:
        inline ExprGenerator(InterpreterContext& ctx) : _ctx(ctx) {}
    private:
        inline void visitTernary(const Ast::TernaryOpExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConditionalExpr& node) {
            return visitTernary(node);
        }

        virtual void visitBinary(const Ast::BinaryExpr& node) {
            unused(node);
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
            printf("var-ref %s (%lu)\n", node.vref().name().text(), z::pad(node.vref()));
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
            unused(node);
        }

        virtual void visit(const Ast::ConstantStringExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantCharExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantLongExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantIntExpr& node) {
            unused(node);
        }

        virtual void visit(const Ast::ConstantShortExpr& node) {
            unused(node);
        }

        virtual void sep() {
        }

    private:
        InterpreterContext& _ctx;
    };

    void runStatementGenerator(const Ast::Config& config, InterpreterContext& ctx, const Ast::Statement& block);

    struct ImportGenerator : public Ast::TypeSpec::Visitor {
        inline void visitChildrenIndent(const Ast::TypeSpec& node) {
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
        }

        void visit(const Ast::FunctionRetn& node) {
            visitChildrenIndent(node);
        }

        void visit(const Ast::FunctionDecl& node) {
            visitChildrenIndent(node);
        }

        void visit(const Ast::RootFunctionDefn& node) {
            visitChildrenIndent(node);
        }

        void visit(const Ast::ChildFunctionDefn& node) {
            visitChildrenIndent(node);
        }

        void visit(const Ast::EventDecl& node) {
            visitChildrenIndent(node);
        }

        void visit(const Ast::Namespace& node) {
            visitChildrenIndent(node);
        }

        void visit(const Ast::Root& node) {
            visitChildrenIndent(node);
        }

    public:
        inline ImportGenerator(const Ast::Config& config, InterpreterContext& ctx) : _config(config), _ctx(ctx) {}

    private:
        const Ast::Config& _config;
        InterpreterContext& _ctx;
    };

    struct StatementGenerator : public Ast::Statement::Visitor {
    private:
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
            ImportGenerator(_config, _ctx).visitNode(node.typeSpec());
        }

        virtual void visit(const Ast::StructMemberVariableStatement& node) {
            ExprGenerator(_ctx).visitNode(node.defn().initExpr());
        }

        virtual void visit(const Ast::StructInitStatement& node) {
            unused(node);
        }

        virtual void visit(const Ast::AutoStatement& node) {
            printf("auto statement %s (%lu)\n", node.defn().name().text(), z::pad(node.defn()));
            ExprGenerator(_ctx).visitNode(node.defn().initExpr());
        }

        virtual void visit(const Ast::ExprStatement& node) {
            printf("expr statement\n");
            ExprGenerator(_ctx).visitNode(node.expr());
        }

        virtual void visit(const Ast::PrintStatement& node) {
            printf("print statement\n");
            ExprGenerator(_ctx).visitNode(node.expr());
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
        inline StatementGenerator(const Ast::Config& config, InterpreterContext& ctx) : _config(config), _ctx(ctx) {}
    private:
        const Ast::Config& _config;
        InterpreterContext& _ctx;
    };

    void runStatementGenerator(const Ast::Config& config, InterpreterContext& ctx, const Ast::Statement& block) {
        StatementGenerator gen(config, ctx);
        gen.visitNode(block);
    }
}

struct Interpreter::Impl {
    inline Impl(const Ast::Project& project, const Ast::Config& config) : _project(project), _config(config) {}
    inline void run();
private:
    const Ast::Project& _project;
    const Ast::Config& _config;
};

void InterpreterContext::process(const std::string& cmd) {
    Ast::Module module(_unit);
    _c.parseString(_lexer, module, cmd, 0);
    for(Ast::Module::StatementList::const_iterator sit = module.globalStatementList().begin(); sit != module.globalStatementList().end(); ++sit) {
        const Ast::Statement& s = z::ref(*sit);
        runStatementGenerator(_config, z::ref(this), s);
    }
}

inline void Interpreter::Impl::run() {
    printf("Entering interpretor mode\n");

    Ast::Token pos(0, 0, "");
    InterpreterContext ctx(_project, _config, pos);

    bool quit = false;
    while (quit == false) {
        std::cout << ">";
        std::string cmd;
        std::getline(std::cin, cmd);
        std::cout << cmd << std::endl;
        if(cmd == ".q")
            break;
        try {
            ctx.process(cmd);
        } catch (...) {
        }
    }
}

//////////////////////////////////////////////
Interpreter::Interpreter(const Ast::Project& project, const Ast::Config& config) : _impl(0) {_impl = new Impl(project, config);}
Interpreter::~Interpreter() {delete _impl;}
void Interpreter::run() {return z::ref(_impl).run();}
