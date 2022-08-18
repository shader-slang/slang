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
                : SemanticsStmtVisitor(visitor->withOuterStmts(&m_outerStmt))
            {
                m_outerStmt.next = visitor->getOuterStmts();
                m_outerStmt.stmt = outerStmt;
            }

        private:
            OuterStmtInfo m_outerStmt;
        };
    }

    void SemanticsVisitor::checkStmt(Stmt* stmt, SemanticsContext const& context)
    {
        if (!stmt) return;
        dispatchStmt(stmt, context);
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
        ensureDeclBase(stmt->decl, DeclCheckState::Checked, this);
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
        SemanticsVisitor::checkStmt(stmt, *this);
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

    Expr* SemanticsVisitor::checkPredicateExpr(Expr* expr)
    {
        Expr* e = expr;
        e = CheckTerm(e);
        e = coerce(m_astBuilder->getBoolType(), e);
        return e;
    }

    void SemanticsStmtVisitor::visitDoWhileStmt(DoWhileStmt *stmt)
    {
        WithOuterStmt subContext(this, stmt);

        stmt->predicate = checkPredicateExpr(stmt->predicate);
        subContext.checkStmt(stmt->statement);
    }

    void SemanticsStmtVisitor::visitForStmt(ForStmt *stmt)
    {
        WithOuterStmt subContext(this, stmt);

        checkStmt(stmt->initialStatement);
        if (stmt->predicateExpression)
        {
            stmt->predicateExpression = checkPredicateExpr(stmt->predicateExpression);
        }
        if (stmt->sideEffectExpression)
        {
            stmt->sideEffectExpression = CheckExpr(stmt->sideEffectExpression);
        }
        subContext.checkStmt(stmt->statement);
    }

    Expr* SemanticsVisitor::checkExpressionAndExpectIntegerConstant(Expr* expr, IntVal** outIntVal)
    {
        expr = CheckExpr(expr);
        auto intVal = CheckIntegerConstantExpression(expr, IntegerConstantExpressionCoercionType::AnyInteger, nullptr);
        if (outIntVal)
            *outIntVal = intVal;
        return expr;
    }

    void SemanticsStmtVisitor::visitCompileTimeForStmt(CompileTimeForStmt* stmt)
    {
        WithOuterStmt subContext(this, stmt);

        stmt->varDecl->type.type = m_astBuilder->getIntType();
        addModifier(stmt->varDecl, m_astBuilder->create<ConstModifier>());
        stmt->varDecl->setCheckState(DeclCheckState::Checked);

        IntVal* rangeBeginVal = nullptr;
        IntVal* rangeEndVal = nullptr;

        if (stmt->rangeBeginExpr)
        {
            stmt->rangeBeginExpr = checkExpressionAndExpectIntegerConstant(stmt->rangeBeginExpr, &rangeBeginVal);
        }
        else
        {
            ConstantIntVal* rangeBeginConst = m_astBuilder->create<ConstantIntVal>();
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
        stmt->predicate = checkPredicateExpr(stmt->predicate);
        checkStmt(stmt->positiveStatement);
        checkStmt(stmt->negativeStatement);
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
        if (!stmt->expression)
        {
            if (function && !function->returnType.equals(m_astBuilder->getVoidType()))
            {
                getSink()->diagnose(stmt, Diagnostics::returnNeedsExpression);
            }
        }
        else
        {
            stmt->expression = CheckTerm(stmt->expression);
            if (!stmt->expression->type->equals(m_astBuilder->getErrorType()))
            {
                if (function)
                {
                    stmt->expression = coerce(function->returnType.Ptr(), stmt->expression);
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
        stmt->predicate = checkPredicateExpr(stmt->predicate);
        subContext.checkStmt(stmt->statement);
    }

    void SemanticsStmtVisitor::visitExpressionStmt(ExpressionStmt *stmt)
    {
        stmt->expression = CheckExpr(stmt->expression);
    }

    void SemanticsStmtVisitor::visitGpuForeachStmt(GpuForeachStmt*stmt)
    {
        stmt->device = CheckExpr(stmt->device);
        stmt->gridDims = CheckExpr(stmt->gridDims);
        ensureDeclBase(stmt->dispatchThreadID, DeclCheckState::Checked, this);
        WithOuterStmt subContext(this, stmt);
        stmt->kernelCall = subContext.CheckExpr(stmt->kernelCall);
        return;
    }
}
