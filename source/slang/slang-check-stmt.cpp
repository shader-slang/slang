// slang-check-stmt.cpp
#include "slang-check-impl.h"
#include "slang-ir-util.h"

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
        ensureDeclBase(stmt->decl, DeclCheckState::DefinitionChecked, this);
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

    void SemanticsStmtVisitor::visitLabelStmt(LabelStmt* stmt)
    {
        WithOuterStmt subContext(this, stmt);
        subContext.checkStmt(stmt->innerStmt);
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

    Stmt* SemanticsStmtVisitor::findOuterStmtWithLabel(Name* label)
    {
        for (auto outerStmtInfo = m_outerStmts; outerStmtInfo; outerStmtInfo = outerStmtInfo->next)
        {
            auto outerStmt = outerStmtInfo->stmt;
            auto found = as<LabelStmt>(outerStmt);
            if (found)
            {
                if (found->label.getName() == label)
                {
                    return found->innerStmt;
                }
            }
        }
        return nullptr;
    }

    void SemanticsStmtVisitor::visitBreakStmt(BreakStmt *stmt)
    {
        Stmt* targetStmt = nullptr;
        if (stmt->targetLabel.type == TokenType::Identifier)
        {
            // This is a break statement with an explicit target label.
            // Try to find the outer stmt with the label.
            targetStmt = findOuterStmtWithLabel(stmt->targetLabel.getName());
            if (!targetStmt)
            {
                getSink()->diagnose(stmt, Diagnostics::breakLabelNotFound, stmt->targetLabel.getName());
            }
            if (!as<BreakableStmt>(targetStmt))
            {
                getSink()->diagnose(stmt, Diagnostics::targetLabelDoesNotMarkBreakableStmt, stmt->targetLabel.getName());
            }
        }
        else
        {
            // For `break` statements without an explicit target,
            // find the inner most breakable stmt.
            targetStmt = FindOuterStmt<BreakableStmt>();
            if (!targetStmt)
            {
                getSink()->diagnose(stmt, Diagnostics::breakOutsideLoop);
            }
        }
        stmt->parentStmt = targetStmt;
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
        if (as<AssignExpr>(expr))
        {
            getSink()->diagnose(expr, Diagnostics::assignmentInPredicateExpr);
        }
        Expr* e = expr;
        e = CheckTerm(e);
        e = coerce(CoercionSite::General, m_astBuilder->getBoolType(), e);
        return e;
    }

    void SemanticsStmtVisitor::visitDoWhileStmt(DoWhileStmt *stmt)
    {
        checkModifiers(stmt);
        WithOuterStmt subContext(this, stmt);

        stmt->predicate = checkPredicateExpr(stmt->predicate);
        subContext.checkStmt(stmt->statement);
        checkLoopInDifferentiableFunc(stmt);
    }

    void SemanticsStmtVisitor::visitForStmt(ForStmt *stmt)
    {
        WithOuterStmt subContext(this, stmt);
        checkModifiers(stmt);
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
        
        tryInferLoopMaxIterations(stmt);

        checkLoopInDifferentiableFunc(stmt);
    }

    Expr* SemanticsVisitor::checkExpressionAndExpectIntegerConstant(Expr* expr, IntVal** outIntVal, ConstantFoldingKind kind)
    {
        expr = CheckExpr(expr);
        auto intVal = CheckIntegerConstantExpression(expr, IntegerConstantExpressionCoercionType::AnyInteger, nullptr, kind);
        if (outIntVal)
            *outIntVal = intVal;
        return expr;
    }

    void SemanticsStmtVisitor::visitCompileTimeForStmt(CompileTimeForStmt* stmt)
    {
        WithOuterStmt subContext(this, stmt);

        stmt->varDecl->type.type = m_astBuilder->getIntType();
        addModifier(stmt->varDecl, m_astBuilder->create<ConstModifier>());
        stmt->varDecl->setCheckState(DeclCheckState::DefinitionChecked);

        IntVal* rangeBeginVal = nullptr;
        IntVal* rangeEndVal = nullptr;

        if (stmt->rangeBeginExpr)
        {
            stmt->rangeBeginExpr = checkExpressionAndExpectIntegerConstant(stmt->rangeBeginExpr, &rangeBeginVal, ConstantFoldingKind::LinkTime);
        }
        else
        {
            ConstantIntVal* rangeBeginConst = m_astBuilder->getIntVal(m_astBuilder->getIntType(), 0);
            rangeBeginVal = rangeBeginConst;
        }

        stmt->rangeEndExpr = checkExpressionAndExpectIntegerConstant(stmt->rangeEndExpr, &rangeEndVal, ConstantFoldingKind::LinkTime);

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

    void SemanticsStmtVisitor::visitTargetSwitchStmt(TargetSwitchStmt* stmt)
    {
        WithOuterStmt subContext(this, stmt);
        HashSet<Stmt*> checkedStmt;
        for (auto caseStmt : stmt->targetCases)
        {
            if (checkedStmt.contains(caseStmt->body))
                continue;
            subContext.checkStmt(caseStmt);
            checkedStmt.add(caseStmt->body);
        }
    }

    void SemanticsStmtVisitor::visitTargetCaseStmt(TargetCaseStmt* stmt)
    {
        auto switchStmt = FindOuterStmt<TargetSwitchStmt>();
        CapabilitySet set((CapabilityName)stmt->capability);
        if (getShared()->isInLanguageServer() && getShared()->getSession()->getCompletionRequestTokenName() == stmt->capabilityToken.getName())
        {
            getShared()->getLinkage()->contentAssistInfo.completionSuggestions.scopeKind = CompletionSuggestions::ScopeKind::Capabilities;
        }

        if (stmt->capabilityToken.getContentLength() != 0 &&
            (set.getExpandedAtoms().getCount() != 1 || set.isInvalid() || set.isEmpty()))
        {
            getSink()->diagnose(
                stmt->capabilityToken.loc,
                Diagnostics::invalidTargetSwitchCase,
                capabilityNameToString((CapabilityName)stmt->capability));
        }
        if (!switchStmt)
        {
            getSink()->diagnose(stmt, Diagnostics::caseOutsideSwitch);
        }
        WithOuterStmt subContext(this, stmt);
        subContext.checkStmt(stmt->body);
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
                    stmt->expression = coerce(CoercionSite::Return, function->returnType.Ptr(), stmt->expression);
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
        checkModifiers(stmt);
        WithOuterStmt subContext(this, stmt);
        stmt->predicate = checkPredicateExpr(stmt->predicate);
        subContext.checkStmt(stmt->statement);
        checkLoopInDifferentiableFunc(stmt);
    }

    void SemanticsStmtVisitor::visitExpressionStmt(ExpressionStmt *stmt)
    {
        stmt->expression = CheckExpr(stmt->expression);
        if (auto operatorExpr = as<OperatorExpr>(stmt->expression))
        {
            if (auto func = as<VarExpr>(operatorExpr->functionExpr))
            {
                if (func->name && func->name->text == "==")
                {
                    getSink()->diagnose(operatorExpr, Diagnostics::danglingEqualityExpr);
                }
            }
        }
    }

    void SemanticsStmtVisitor::tryInferLoopMaxIterations(ForStmt* stmt)
    {
        // If a for loop is in the form of `for (var = initialVal; var $compareOp otherVal; var sideEffectOp operand)`
        // we will try to constant fold the operands and see if we can statically determine the maximum number of
        // iterations this loop will run, and insert the inferred result as a `[MaxIters]` attribute on the stmt.
        // 
        // ++, --, +=, -= are supported in side effect expressions.
        // >, <, >=, <= are supported in predicate expressions.
        // induction variable can appear in either side of the expressions.
        // 
        // Other forms like for (var1 = .., var2 = ..; ) will not be recognized here.
        // If we see suspicious code like `for (int i = 0; i < 5; j++)`, we will produce a warning along the way.
        // 
        DeclRef<Decl> predicateVar = {};
        Expr* initialVal = nullptr;
        DeclRef<Decl> initialVar = {};
        if (auto varStmt = as<DeclStmt>(stmt->initialStatement))
        {
            auto varDecl = as<VarDecl>(varStmt->decl);
            if (!varDecl)
                return;
            initialVar = makeDeclRef<Decl>(varDecl);
            initialVal = varDecl->initExpr;
        }
        else if (auto exprStmt = as<ExpressionStmt>(stmt->initialStatement))
        {
            auto assignExpr = as<AssignExpr>(exprStmt->expression);
            if (!assignExpr)
                return;
            auto varExpr = as<VarExpr>(assignExpr->left);
            if (!varExpr)
                return;
            initialVar = varExpr->declRef;
            initialVal = assignExpr->right;
        }
        else
            return;

        auto initialLitVal =
            as<ConstantIntVal>(tryFoldIntegerConstantExpression(initialVal, ConstantFoldingKind::CompileTime, nullptr));

        ConstantIntVal* finalVal = nullptr;
        auto binaryExpr = as<InfixExpr>(stmt->predicateExpression);
        if (!binaryExpr)
            return;
        auto compareFuncExpr = as<DeclRefExpr>(binaryExpr->functionExpr);
        if (!compareFuncExpr)
            return;
        if (!compareFuncExpr->declRef.getDecl())
            return;
        IROp compareOp = kIROp_Nop;
        if (auto intrinsicOpModifier = compareFuncExpr->declRef.getDecl()->findModifier<IntrinsicOpModifier>())
        {
            compareOp = (IROp)intrinsicOpModifier->op;
        }
        else
        {
            return;
        }
        if (binaryExpr->arguments.getCount() != 2)
            return;
        auto leftCompareOperand = binaryExpr->arguments[0];
        auto rightCompareOperand = binaryExpr->arguments[1];
        if (!leftCompareOperand)
            return;
        if (!rightCompareOperand)
            return;
        if (auto rightVal = tryFoldIntegerConstantExpression(binaryExpr->arguments[1], ConstantFoldingKind::CompileTime, nullptr))
        {
            auto leftVar = as<VarExpr>(leftCompareOperand);
            if (!leftVar)
                return;
            predicateVar = leftVar->declRef;
            finalVal = as<ConstantIntVal>(rightVal);
        }
        else if (auto leftVal = tryFoldIntegerConstantExpression(binaryExpr->arguments[0], ConstantFoldingKind::CompileTime, nullptr))
        {
            auto rightVar = as<VarExpr>(rightCompareOperand);
            if (!rightVar)
                return;
            predicateVar = rightVar->declRef;
            finalVal = as<ConstantIntVal>(leftVal);
            compareOp = getSwapSideComparisonOp(compareOp);
        }
        else
        {
            // If neither left or right is constant, we assume left is variable and continue checking.
            if (auto leftVar = as<VarExpr>(leftCompareOperand))
            {
                predicateVar = leftVar->declRef;
            }
            if (auto rightVar = as<VarExpr>(rightCompareOperand))
            {
                if (rightVar->declRef == initialVar)
                {
                    predicateVar = rightVar->declRef;
                    compareOp = getSwapSideComparisonOp(compareOp);
                }
            }
        }

        switch (compareOp)
        {
        case kIROp_Less:
        case kIROp_Leq:
        case kIROp_Greater:
        case kIROp_Geq:
            break;
        default:
            return;
        }

        ConstantIntVal* stepSize = nullptr;
        IROp sideEffectFuncOp = kIROp_Nop;
        auto opSideEffectExpr = as<InvokeExpr>(stmt->sideEffectExpression);
        if (!opSideEffectExpr)
            return;
        auto sideEffectFuncExpr = as<DeclRefExpr>(opSideEffectExpr->functionExpr);
        if (!sideEffectFuncExpr)
            return;
        auto sideEffectFuncDecl = sideEffectFuncExpr->declRef.getDecl();
        if (!sideEffectFuncDecl)
            return;
        if (auto opName = sideEffectFuncDecl->getName())
        {
            if (opName->text == "++")
                sideEffectFuncOp = kIROp_Add;
            else if (opName->text == "--")
                sideEffectFuncOp = kIROp_Sub;
            else if (opName->text == "+=")
                sideEffectFuncOp = kIROp_Add;
            else if (opName->text == "-=")
                sideEffectFuncOp = kIROp_Sub;
            else
                return;
        }
        if (opSideEffectExpr->arguments.getCount())
        {
            auto varExpr = as<VarExpr>(opSideEffectExpr->arguments[0]);
            if (!varExpr)
                return;
            if (varExpr->declRef.getDecl() != initialVar.getDecl())
            {
                // If the user writes something like `for (int i = 0; i < 5; j++)`,
                // it is most likely a bug, so we issue a warning.
                if (predicateVar == initialVar)
                    getSink()->diagnose(varExpr, Diagnostics::forLoopSideEffectChangingDifferentVar, initialVar, varExpr->declRef);
                return;
            }
        }
        else
            return;
        if (opSideEffectExpr->arguments.getCount() == 2)
        {
            auto stepVal = tryFoldIntegerConstantExpression(opSideEffectExpr->arguments[1], ConstantFoldingKind::CompileTime, nullptr);
            if (!stepVal)
                return;
            if (auto constantIntVal = as<ConstantIntVal>(stepVal))
            {
                stepSize = constantIntVal;
            }
        }
        else
        {
            stepSize = m_astBuilder->getIntVal(m_astBuilder->getIntType(), 1);
        }

        if (predicateVar.getDecl() != initialVar.getDecl())
        {
            if (predicateVar)
                getSink()->diagnose(stmt->predicateExpression, Diagnostics::forLoopPredicateCheckingDifferentVar, initialVar, predicateVar);
            return;
        }
        if (!stepSize)
            return;
        if (stepSize->getValue() > 0)
        {
            if (sideEffectFuncOp == kIROp_Add && compareOp == kIROp_Greater ||
                sideEffectFuncOp == kIROp_Sub && compareOp == kIROp_Less)
            {
                getSink()->diagnose(stmt->sideEffectExpression, Diagnostics::forLoopChangingIterationVariableInOppsoiteDirection, initialVar);
                return;
            }
        }
        else if (stepSize->getValue() < 0)
        {
            if (sideEffectFuncOp == kIROp_Add && compareOp == kIROp_Less ||
                sideEffectFuncOp == kIROp_Sub && compareOp == kIROp_Greater)
            {
                getSink()->diagnose(stmt->sideEffectExpression, Diagnostics::forLoopChangingIterationVariableInOppsoiteDirection, initialVar);
                return;
            }
        }
        else
        {
            getSink()->diagnose(stmt->sideEffectExpression, Diagnostics::forLoopNotModifyingIterationVariable, initialVar);
            return;
        }

        if (!initialLitVal || !finalVal)
            return;

        auto absStepSize = abs(stepSize->getValue());
        int adjustment = 0;
        if (compareOp == kIROp_Geq || compareOp == kIROp_Leq)
            adjustment = 1;

        auto iterations = (Math::Max(finalVal->getValue(), initialLitVal->getValue()) -
            Math::Min(finalVal->getValue(), initialLitVal->getValue()) + absStepSize - 1 + adjustment) /
            absStepSize;
        switch (compareOp)
        {
        case kIROp_Geq:
        case kIROp_Greater:
            // Expect final value to be less than initial value.
            if (finalVal->getValue() > initialLitVal->getValue())
                iterations = 0;
            break;
        case kIROp_Leq:
        case kIROp_Less:
            if (finalVal->getValue() < initialLitVal->getValue())
                iterations = 0;
            break;
        }
        if (iterations == 0)
        {
            getSink()->diagnose(stmt, Diagnostics::loopRunsForZeroIterations);
        }
        
        // Note: the inferred max iterations may not be valid if the loop body
        // also modifies the induction variable.
        // We detect this case during lower-to-ir and will remove the `InferredMaxItersAttribute`
        // if the loop body modifies the induction variable.
        //
        auto maxItersAttr = m_astBuilder->create<InferredMaxItersAttribute>();
        auto litExpr = m_astBuilder->create<LiteralExpr>();
        litExpr->type.type = m_astBuilder->getIntType();
        litExpr->token.setName(getNamePool()->getName(String(iterations)));
        maxItersAttr->args.add(litExpr);
        maxItersAttr->intArgVals.add(m_astBuilder->getIntVal(m_astBuilder->getIntType(), iterations));
        maxItersAttr->value = (int32_t)iterations;
        maxItersAttr->inductionVar = initialVar;
        addModifier(stmt, maxItersAttr);
        return;
    }

    void SemanticsStmtVisitor::checkLoopInDifferentiableFunc(Stmt* stmt)
    {
        SLANG_UNUSED(stmt);
        if (getParentDifferentiableAttribute())
        {
            if (!getParentFunc())
                return;
            
            // If the function is itself a derivative, or has a user defined derivative,
            // then we don't require anything.

            if (getParentFunc()->findModifier<ForwardDerivativeOfAttribute>())
                return;
            if (getParentFunc()->findModifier<ForwardDerivativeAttribute>())
                return;
            if (getParentFunc()->findModifier<BackwardDerivativeOfAttribute>())
                return;
            if (getParentFunc()->findModifier<BackwardDerivativeAttribute>())
                return;
        }
    }

    void SemanticsStmtVisitor::visitGpuForeachStmt(GpuForeachStmt*stmt)
    {
        stmt->device = CheckExpr(stmt->device);
        stmt->gridDims = CheckExpr(stmt->gridDims);
        ensureDeclBase(stmt->dispatchThreadID, DeclCheckState::DefinitionChecked, this);
        WithOuterStmt subContext(this, stmt);
        stmt->kernelCall = subContext.CheckExpr(stmt->kernelCall);
        return;
    }
}
