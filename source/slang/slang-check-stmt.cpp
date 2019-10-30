// slang-check-stmt.cpp
#include "slang-check-impl.h"

// This file implements semantic checking logic related to statements.

namespace Slang
{
    void SemanticsVisitor::checkStmt(Stmt* stmt)
    {
        if (!stmt) return;
        dispatchStmt(stmt);
        checkModifiers(stmt);
    }

    void SemanticsStmtVisitor::visitDeclStmt(DeclStmt* stmt)
    {
        // We directly dispatch here instead of using `EnsureDecl()` for two
        // reasons:
        //
        // 1. We expect that a local declaration won't have been referenced
        // before it is declared, so that we can just check things in-order
        //
        // 2. `EnsureDecl()` is specialized for `Decl*` instead of `DeclBase*`
        // and trying to special case `DeclGroup*` here feels silly.
        //
        dispatchDecl(stmt->decl);
        checkModifiers(stmt->decl);
    }

    void SemanticsStmtVisitor::visitBlockStmt(BlockStmt* stmt)
    {
        checkStmt(stmt->body);
    }

    void SemanticsStmtVisitor::visitSeqStmt(SeqStmt* stmt)
    {
        for(auto ss : stmt->stmts)
        {
            checkStmt(ss);
        }
    }

    template<typename T>
    T* SemanticsVisitor::FindOuterStmt()
    {
        auto& outerStmts = getShared()->outerStmts;
        const Index outerStmtCount = outerStmts.getCount();
        for (Index ii = outerStmtCount; ii > 0; --ii)
        {
            auto outerStmt = outerStmts[ii-1];
            auto found = as<T>(outerStmt);
            if (found)
                return found;
        }
        return nullptr;
    }

    void SemanticsStmtVisitor::visitBreakStmt(BreakStmt *stmt)
    {
        auto outer = FindOuterStmt<BreakableStmt>();
        if (!outer)
        {
            getSink()->diagnose(stmt, Diagnostics::breakOutsideLoop);
        }
        stmt->parentStmt = outer;
    }

    void SemanticsStmtVisitor::visitContinueStmt(ContinueStmt *stmt)
    {
        auto outer = FindOuterStmt<LoopStmt>();
        if (!outer)
        {
            getSink()->diagnose(stmt, Diagnostics::continueOutsideLoop);
        }
        stmt->parentStmt = outer;
    }

    void SemanticsVisitor::PushOuterStmt(Stmt* stmt)
    {
        auto& outerStmts = getShared()->outerStmts;
        outerStmts.add(stmt);
    }

    void SemanticsVisitor::PopOuterStmt(Stmt* /*stmt*/)
    {
        auto& outerStmts = getShared()->outerStmts;
        outerStmts.removeAt(outerStmts.getCount() - 1);
    }

    RefPtr<Expr> SemanticsVisitor::checkPredicateExpr(Expr* expr)
    {
        RefPtr<Expr> e = expr;
        e = CheckTerm(e);
        e = coerce(getSession()->getBoolType(), e);
        return e;
    }

    void SemanticsStmtVisitor::visitDoWhileStmt(DoWhileStmt *stmt)
    {
        PushOuterStmt(stmt);
        stmt->Predicate = checkPredicateExpr(stmt->Predicate);
        checkStmt(stmt->Statement);

        PopOuterStmt(stmt);
    }

    void SemanticsStmtVisitor::visitForStmt(ForStmt *stmt)
    {
        PushOuterStmt(stmt);
        checkStmt(stmt->InitialStatement);
        if (stmt->PredicateExpression)
        {
            stmt->PredicateExpression = checkPredicateExpr(stmt->PredicateExpression);
        }
        if (stmt->SideEffectExpression)
        {
            stmt->SideEffectExpression = CheckExpr(stmt->SideEffectExpression);
        }
        checkStmt(stmt->Statement);

        PopOuterStmt(stmt);
    }

    RefPtr<Expr> SemanticsVisitor::checkExpressionAndExpectIntegerConstant(RefPtr<Expr> expr, RefPtr<IntVal>* outIntVal)
    {
        expr = CheckExpr(expr);
        auto intVal = CheckIntegerConstantExpression(expr);
        if (outIntVal)
            *outIntVal = intVal;
        return expr;
    }

    void SemanticsStmtVisitor::visitCompileTimeForStmt(CompileTimeForStmt* stmt)
    {
        PushOuterStmt(stmt);

        stmt->varDecl->type.type = getSession()->getIntType();
        addModifier(stmt->varDecl, new ConstModifier());
        stmt->varDecl->SetCheckState(DeclCheckState::Checked);

        RefPtr<IntVal> rangeBeginVal;
        RefPtr<IntVal> rangeEndVal;

        if (stmt->rangeBeginExpr)
        {
            stmt->rangeBeginExpr = checkExpressionAndExpectIntegerConstant(stmt->rangeBeginExpr, &rangeBeginVal);
        }
        else
        {
            RefPtr<ConstantIntVal> rangeBeginConst = new ConstantIntVal();
            rangeBeginConst->value = 0;
            rangeBeginVal = rangeBeginConst;
        }

        stmt->rangeEndExpr = checkExpressionAndExpectIntegerConstant(stmt->rangeEndExpr, &rangeEndVal);

        stmt->rangeBeginVal = rangeBeginVal;
        stmt->rangeEndVal = rangeEndVal;

        checkStmt(stmt->body);


        PopOuterStmt(stmt);
    }

    void SemanticsStmtVisitor::visitSwitchStmt(SwitchStmt* stmt)
    {
        PushOuterStmt(stmt);
        // TODO(tfoley): need to coerce condition to an integral type...
        stmt->condition = CheckExpr(stmt->condition);
        checkStmt(stmt->body);

        // TODO(tfoley): need to check that all case tags are unique

        // TODO(tfoley): check that there is at most one `default` clause

        PopOuterStmt(stmt);
    }

    void SemanticsStmtVisitor::visitCaseStmt(CaseStmt* stmt)
    {
        // TODO(tfoley): Need to coerce to type being switch on,
        // and ensure that value is a compile-time constant
        auto expr = CheckExpr(stmt->expr);
        auto switchStmt = FindOuterStmt<SwitchStmt>();

        if (!switchStmt)
        {
            getSink()->diagnose(stmt, Diagnostics::caseOutsideSwitch);
        }
        else
        {
            // TODO: need to do some basic matching to ensure the type
            // for the `case` is consistent with the type for the `switch`...
        }

        stmt->expr = expr;
        stmt->parentStmt = switchStmt;
    }

    void SemanticsStmtVisitor::visitDefaultStmt(DefaultStmt* stmt)
    {
        auto switchStmt = FindOuterStmt<SwitchStmt>();
        if (!switchStmt)
        {
            getSink()->diagnose(stmt, Diagnostics::defaultOutsideSwitch);
        }
        stmt->parentStmt = switchStmt;
    }

    void SemanticsStmtVisitor::visitIfStmt(IfStmt *stmt)
    {
        stmt->Predicate = checkPredicateExpr(stmt->Predicate);
        checkStmt(stmt->PositiveStatement);
        checkStmt(stmt->NegativeStatement);
    }

    void SemanticsStmtVisitor::visitUnparsedStmt(UnparsedStmt*)
    {
        // Nothing to do
    }

    void SemanticsStmtVisitor::visitEmptyStmt(EmptyStmt*)
    {
        // Nothing to do
    }

    void SemanticsStmtVisitor::visitDiscardStmt(DiscardStmt*)
    {
        // Nothing to do
    }

    void SemanticsStmtVisitor::visitReturnStmt(ReturnStmt *stmt)
    {
        if (!stmt->Expression)
        {
            if (getFunction() && !getFunction()->ReturnType.Equals(getSession()->getVoidType()))
            {
                getSink()->diagnose(stmt, Diagnostics::returnNeedsExpression);
            }
        }
        else
        {
            stmt->Expression = CheckTerm(stmt->Expression);
            if (!stmt->Expression->type->Equals(getSession()->getErrorType()))
            {
                if (getFunction())
                {
                    stmt->Expression = coerce(getFunction()->ReturnType.Ptr(), stmt->Expression);
                }
                else
                {
                    // TODO(tfoley): this case currently gets triggered for member functions,
                    // which aren't being checked consistently (because of the whole symbol
                    // table idea getting in the way).

//							getSink()->diagnose(stmt, Diagnostics::unimplemented, "case for return stmt");
                }
            }
        }
    }

    void SemanticsStmtVisitor::visitWhileStmt(WhileStmt *stmt)
    {
        PushOuterStmt(stmt);
        stmt->Predicate = checkPredicateExpr(stmt->Predicate);
        checkStmt(stmt->Statement);
        PopOuterStmt(stmt);
    }

    void SemanticsStmtVisitor::visitExpressionStmt(ExpressionStmt *stmt)
    {
        stmt->Expression = CheckExpr(stmt->Expression);
    }

}
