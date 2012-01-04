#include "base/pch.hpp"
#include "base/zenlang.hpp"
#include "ast.hpp"
#include "error.hpp"

void Ast::TypedefDecl::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::TypedefDefn::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::TemplateDecl::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::TemplateDefn::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::EnumDefn::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::StructDecl::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::RootStructDefn::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ChildStructDefn::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::PropertyDeclRW::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::PropertyDeclRO::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::RoutineDecl::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::RoutineDefn::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::FunctionDecl::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::RootFunctionDefn::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ChildFunctionDefn::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::FunctionRetn::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::EventDecl::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::Namespace::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::Root::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}

void Ast::ConditionalExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BooleanAndExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BooleanOrExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BooleanEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BooleanNotEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BooleanLessThanExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BooleanGreaterThanExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BooleanLessThanOrEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BooleanGreaterThanOrEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BooleanHasExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryAssignEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryPlusEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryMinusEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryTimesEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryDivideEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryModEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryBitwiseAndEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryBitwiseOrEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryBitwiseXorEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryShiftLeftEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryShiftRightEqualExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryPlusExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryMinusExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryTimesExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryDivideExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryModExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryBitwiseAndExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryBitwiseOrExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryBitwiseXorExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryShiftLeftExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BinaryShiftRightExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::PostfixIncExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::PostfixDecExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::PrefixNotExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::PrefixPlusExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::PrefixMinusExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::PrefixIncExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::PrefixDecExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::PrefixBitwiseNotExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::SetIndexExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ListExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::DictExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::FormatExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::RunExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::RoutineCallExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::FunctorCallExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::OrderedExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::IndexExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::SpliceExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::TypeofTypeExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::TypeofExprExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::StaticTypecastExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::DynamicTypecastExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::PointerInstanceExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ValueInstanceExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::VariableRefExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::MemberVariableExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::MemberPropertyExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::EnumMemberExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::StructMemberExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::StructInstanceExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::FunctionInstanceExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::AnonymousFunctionExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ConstantFloatExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ConstantDoubleExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ConstantBooleanExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ConstantStringExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ConstantCharExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ConstantLongExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ConstantIntExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ConstantShortExpr::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}

void Ast::ImportStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::EnterNamespaceStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::LeaveNamespaceStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::UserDefinedTypeSpecStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::StructMemberVariableStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::StructInitStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::AutoStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ExprStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::PrintStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::IfStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::IfElseStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::WhileStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::DoWhileStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ForExprStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ForInitStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ForeachStringStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ForeachListStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ForeachDictStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::CaseExprStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::CaseDefaultStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::SwitchValueStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::SwitchExprStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::BreakStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::ContinueStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::AddEventHandlerStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::RoutineReturnStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::FunctionReturnStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::CompoundStatement::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}

void Ast::RoutineBody::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}
void Ast::FunctionBody::visit(Visitor& visitor) const {visitor.visit(z::ref(this));}

Ast::Scope& Ast::Unit::enterScope(Ast::Scope& scope) {
    _scopeStack.push_back(z::ptr(scope));
    return scope;
}

Ast::Scope& Ast::Unit::leaveScope() {
    Ast::Scope* s = _scopeStack.back();
    assert(_scopeStack.size() > 0);
    _scopeStack.pop_back();
    return z::ref(s);
}

Ast::Scope& Ast::Unit::leaveScope(Ast::Scope& scope) {
    Ast::Scope& s = leaveScope();
    assert(z::ptr(s) == z::ptr(scope));
    return s;
}

Ast::Scope& Ast::Unit::currentScope() {
    assert(_scopeStack.size() > 0);
    Ast::Scope* scope = _scopeStack.back();
    return z::ref(scope);
}

const Ast::VariableDefn* Ast::Unit::hasMember(const Ast::Scope& scope, const Ast::Token& name) const {
    for(Ast::Scope::List::const_iterator it = scope.list().begin(); it != scope.list().end(); ++it) {
        const Ast::VariableDefn& vref = z::ref(*it);
        if(vref.name().string() == name.string())
            return z::ptr(vref);
    }
    return 0;
}

const Ast::VariableDefn* Ast::Unit::getVariableDef(const std::string& filename, const Ast::Token& name, Ast::RefType::T& refType) const {
    refType = Ast::RefType::Local;
    typedef std::list<Ast::Scope*> ScopeList;
    ScopeList scopeList;

    for(Unit::ScopeStack::const_reverse_iterator it = _scopeStack.rbegin(); it != _scopeStack.rend(); ++it) {
        Ast::Scope& scope = z::ref(*it);

        if(scope.type() == Ast::ScopeType::XRef) {
            scopeList.push_back(z::ptr(scope));
        }

        switch(refType) {
            case Ast::RefType::Global:
                break;
            case Ast::RefType::XRef:
                break;
            case Ast::RefType::Param:
                switch(scope.type()) {
                    case Ast::ScopeType::Global:
                        throw z::Exception("%s Internal error: Invalid vref %s: Param-Global\n", err(filename, name).c_str(), name.text());
                    case Ast::ScopeType::Member:
                        throw z::Exception("%s Internal error: Invalid vref %s: Param-Member\n", err(filename, name).c_str(), name.text());
                    case Ast::ScopeType::XRef:
                        refType = Ast::RefType::XRef;
                        break;
                    case Ast::ScopeType::Param:
                    case Ast::ScopeType::VarArg:
                        throw z::Exception("%s Internal error: Invalid vref %s: Param-Param\n", err(filename, name).c_str(), name.text());
                    case Ast::ScopeType::Local:
                        refType = Ast::RefType::XRef;
                        break;
                }
                break;
            case Ast::RefType::Local:
                switch(scope.type()) {
                    case Ast::ScopeType::Global:
                        throw z::Exception("%s Internal error: Invalid vref %s: Local-Global\n", err(filename, name).c_str(), name.text());
                    case Ast::ScopeType::Member:
                        throw z::Exception("%s Internal error: Invalid vref %s: Local-Member\n", err(filename, name).c_str(), name.text());
                    case Ast::ScopeType::XRef:
                        throw z::Exception("%s Internal error: Invalid vref %s: Local-XRef\n", err(filename, name).c_str(), name.text());
                    case Ast::ScopeType::Param:
                    case Ast::ScopeType::VarArg:
                        refType = Ast::RefType::Param;
                        break;
                    case Ast::ScopeType::Local:
                        break;
                }
                break;
        }

        const Ast::VariableDefn* vref = hasMember(scope, name);
        if(vref != 0) {
            if(refType == Ast::RefType::XRef) {
                assert(scopeList.size() > 0);
                for(ScopeList::iterator it = scopeList.begin(); it != scopeList.end(); ++it) {
                    Ast::Scope& scope = z::ref(*it);

                    // check if vref already exists in this scope
                    bool found = false;
                    for(Ast::Scope::List::const_iterator xit = scope.list().begin(); xit != scope.list().end(); ++xit) {
                        const Ast::VariableDefn* xref = *xit;
                        if(vref == xref) {
                            found = true;
                            break;
                        }
                    }

                    // if not exists, add it
                    if(!found)
                        scope.addVariableDef(z::ref(vref));
                }
            }

            assert(vref);
            return vref;
        }
    }
    return 0;
}

