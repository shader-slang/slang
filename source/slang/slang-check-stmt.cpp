// slang-check-stmt.cpp
#include "slang-check-impl.h"

// This file implements semantic checking logic related to statements.

namespace Slang
{
    namespace
    {
            /// RAII-like type for establishing an "outer" statement during nested checks.
            ///
            /// The `SemanticsStmtVisitor` maintains a linked list of outer statements
            /// using `OuterStmtInfo` records stored on the recursive call stack during
            /// checking. This type creates a sub-`SemanticsStmtVisitor` that has one
            /// additional outer statement added to the stack of outer statements.
            ///
            /// The outer statements are used to validate and resolve things like
            /// the target of `break` or `continue` statements.
            ///
        struct WithOuterStmt : public SemanticsStmtVisitor
        {
        public:
            WithOuterStmt(SemanticsStmtVisitor* visitor, Stmt* outerStmt)
                : SemanticsStmtVisitor(*visitor)
            {
                m_parentFunc = visitor->m_parentFunc;

                m_outerStmt.next = visitor->m_outerStmts;
                m_outerStmt.stmt = outerStmt;
                m_outerStmts = &m_outerStmt;
            }

        private:
            OuterStmtInfo m_outerStmt;
        };
    }

    void SemanticsVisitor::checkStmt(Stmt* stmt, FunctionDeclBase* parentDecl, OuterStmtInfo* outerStmts)
    {
        if (!stmt) return;
        dispatchStmt(stmt, parentDecl, outerStmts);
        checkModifiers(stmt);
    }

    void SemanticsStmtVisitor::visitDeclStmt(DeclStmt* stmt)
    {
        // When we encounter a declaration during statement checking,
        // we expect that it hasn't been checked yet (because otherwise
        // it would be referenced before its declaration point), but
        // we will bottleneck through the `ensureDecl()` path anyway,
        // to unify with the rest of semantic checking.
        //
        // TODO: This logic might not suffice for something like a
        // local `struct` declaration, where it would have members
        // that need to be recursively checked.
        //
        ensureDeclBase(stmt->decl, DeclCheckState::Checked);
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

    void SemanticsStmtVisitor::checkStmt(Stmt* stmt)
    {
        SemanticsVisitor::checkStmt(stmt, m_parentFunc, m_outerStmts);
    }

    template<typename T>
    T* SemanticsStmtVisitor::FindOuterStmt()
    {
        for(auto outerStmtInfo = m_outerStmts; outerStmtInfo; outerStmtInfo = outerStmtInfo->next)
        {
            auto outerStmt = outerStmtInfo->stmt;
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

    RefPtr<Expr> SemanticsVisitor::checkPredicateExpr(Expr* expr)
    {
        RefPtr<Expr> e = expr;
        e = CheckTerm(e);
        e = coerce(getSession()->getBoolType(), e);
        return e;
    }

    void SemanticsStmtVisitor::visitDoWhileStmt(DoWhileStmt *stmt)
    {
        WithOuterStmt subContext(this, stmt);

        stmt->Predicate = checkPredicateExpr(stmt->Predicate);
        subContext.checkStmt(stmt->Statement);
    }

    void SemanticsStmtVisitor::visitForStmt(ForStmt *stmt)
    {
        WithOuterStmt subContext(this, stmt);

        checkStmt(stmt->InitialStatement);
        if (stmt->PredicateExpression)
        {
            stmt->PredicateExpression = checkPredicateExpr(stmt->PredicateExpression);
        }
        if (stmt->SideEffectExpression)
        {
            stmt->SideEffectExpression = CheckExpr(stmt->SideEffectExpression);
        }
        subContext.checkStmt(stmt->Statement);
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
        WithOuterStmt subContext(this, stmt);

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

        subContext.checkStmt(stmt->body);
    }

    void SemanticsStmtVisitor::visitSwitchStmt(SwitchStmt* stmt)
    {
        WithOuterStmt subContext(this, stmt);

        // TODO(tfoley): need to coerce condition to an integral type...
        stmt->condition = CheckExpr(stmt->condition);
        subContext.checkStmt(stmt->body);

        // TODO(tfoley): need to check that all case tags are unique

        // TODO(tfoley): check that there is at most one `default` clause
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
        auto function = getParentFunc();
        if (!stmt->Expression)
        {
            if (function && !function->ReturnType.Equals(getSession()->getVoidType()))
            {
                getSink()->diagnose(stmt, Diagnostics::returnNeedsExpression);
            }
        }
        else
        {
            stmt->Expression = CheckTerm(stmt->Expression);
            if (!stmt->Expression->type->Equals(getSession()->getErrorType()))
            {
                if (function)
                {
                    stmt->Expression = coerce(function->ReturnType.Ptr(), stmt->Expression);
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
        WithOuterStmt subContext(this, stmt);
        stmt->Predicate = checkPredicateExpr(stmt->Predicate);
        subContext.checkStmt(stmt->Statement);
    }

    void SemanticsStmtVisitor::visitExpressionStmt(ExpressionStmt *stmt)
    {
        stmt->Expression = CheckExpr(stmt->Expression);
    }

}
