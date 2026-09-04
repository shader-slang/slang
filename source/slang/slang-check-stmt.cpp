// slang-check-stmt.cpp
#include "slang-check-impl.h"
#include "slang-ir-util.h"
#include "slang-rich-diagnostics.h"

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

/// Return the operator name of an `OperatorExpr` (`+=`, `,`, `++`, ...), or an empty slice if the
/// callee is a shape that does not carry a spelled name.
///
/// An operator expression names its operator through its `functionExpr`. Depending on how far
/// checking has progressed that callee may be an unresolved `VarExpr` (e.g. a `VarExpr` named
/// `"+="`), a resolved `DeclRefExpr`, or — while overload resolution is still pending — an
/// `OverloadedExpr` that records the looked-up name. Each of those carries the operator's name, so
/// read whichever is present. Any other callee shape carries no name to read; the classifier
/// callers treat that empty result as "not the operator I was testing for".
UnownedStringSlice getOperatorName(OperatorExpr* operatorExpr)
{
    // Use `getUnownedStringSliceText`, which slices the `Name`'s own storage, rather than
    // `getText(...).getUnownedSlice()`, which would slice a temporary `String` returned by value.
    auto functionExpr = operatorExpr->functionExpr;
    if (auto varExpr = as<VarExpr>(functionExpr))
        return getUnownedStringSliceText(varExpr->name);
    if (auto declRefExpr = as<DeclRefExpr>(functionExpr))
        return getUnownedStringSliceText(declRefExpr->declRef.getName());
    if (auto overloadedExpr = as<OverloadedExpr>(functionExpr))
        return overloadedExpr->name ? getUnownedStringSliceText(overloadedExpr->name)
                                    : UnownedStringSlice();
    return UnownedStringSlice();
}

bool isCompoundAssignmentOperatorName(UnownedStringSlice name)
{
    static const char* kCompoundAssignmentOps[] =
        {"+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="};
    for (auto op : kCompoundAssignmentOps)
    {
        if (name == UnownedStringSlice(op))
            return true;
    }
    return false;
}

/// Is `expr` an assignment expression, including a compound assignment (`+=`, `-=`, ...)?
///
/// Only a plain `=` parses to an `AssignExpr`; a compound assignment parses to an operator call
/// (`operator+=`), so testing `AssignExpr` alone would miss the compound forms.
bool isAssignmentOrCompoundAssignmentExpr(Expr* expr)
{
    if (as<AssignExpr>(expr))
        return true;
    if (auto operatorExpr = as<OperatorExpr>(expr))
        return isCompoundAssignmentOperatorName(getOperatorName(operatorExpr));
    return false;
}

/// Is `expr` a prefix or postfix `++`/`--`?
bool isIncrementOrDecrementExpr(Expr* expr)
{
    if (!as<PrefixExpr>(expr) && !as<PostfixExpr>(expr))
        return false;
    // A prefix/postfix expression is an `OperatorExpr`, so its operator name is available.
    auto operatorExpr = as<OperatorExpr>(expr);
    SLANG_ASSERT(operatorExpr);
    auto name = getOperatorName(operatorExpr);
    return name == UnownedStringSlice("++") || name == UnownedStringSlice("--");
}
} // namespace

InvokeExpr* SemanticsVisitor::asCallExpr(Expr* expr)
{
    // A traditional function/method call (`f(x)`, `obj.method()`) is an `InvokeExpr` that is
    // neither an operator expression (`a + b`, `a, b`, which derive from `InvokeExpr` via
    // `OperatorExpr`) nor a cast (`(int)x`, `int(x)`, which are a `TypeCastExpr : InvokeExpr` that
    // is not an `OperatorExpr`). Casts are call-like but are admitted separately by callers, so
    // exclude them here rather than folding them into the notion of a call.
    auto invokeExpr = as<InvokeExpr>(expr);
    if (!invokeExpr || as<OperatorExpr>(invokeExpr) || as<TypeCastExpr>(invokeExpr))
        return nullptr;
    return invokeExpr;
}

void SemanticsVisitor::checkStmt(Stmt* stmt, SemanticsContext const& context)
{
    if (!stmt)
        return;
    dispatchStmt(stmt, context);
    checkModifiers(stmt);
}

CatchStmt* SemanticsVisitor::findMatchingCatchStmt(Type* errorType)
{
    for (auto outerStmtInfo = m_outerStmts; outerStmtInfo; outerStmtInfo = outerStmtInfo->next)
    {
        if (auto catchStmt = as<CatchStmt>(outerStmtInfo->stmt))
        {
            if (!catchStmt->errorVar || catchStmt->errorVar->getType()->equals(errorType))
                return catchStmt;
        }
    }
    return nullptr;
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
    if (auto decl = as<Decl>(stmt->decl))
    {
        decl->hiddenFromLookup = false;
        if (auto varDecl = as<VarDeclBase>(decl))
        {
            if (varDecl->initExpr)
                varDecl->initExpr = maybeRegisterLambdaCapture(varDecl->initExpr);
        }
    }
}

void SemanticsStmtVisitor::visitBlockStmt(BlockStmt* stmt)
{
    // Make sure to fully check all nested agg type decls first.
    if (stmt->scopeDecl)
    {
        for (auto aggDecl : stmt->scopeDecl->getDirectMemberDeclsOfType<AggTypeDeclBase>())
        {
            ensureAllDeclsRec(aggDecl, DeclCheckState::DefinitionChecked);
        }

        // Consider this code:
        // ```
        // {
        //       int a = 5 + b; // should error.
        //       int b = 3;
        // }
        //
        // ```
        // In order to detect the error trying to use `b` before it's declared within
        // a block, our lookup logic contains a condition that ignores a decl if its
        // `hiddenFromLookup` field is set to `true`.
        // See _lookUpDirectAndTransparentMembers().
        // This field will be set to false when we reach the decl through the DeclStmt.
        //
        if (auto seqStmt = as<SeqStmt>(stmt->body))
        {
            for (auto subStmt : seqStmt->stmts)
            {
                if (auto declStmt = as<DeclStmt>(subStmt))
                {
                    if (auto decl = as<Decl>(declStmt->decl))
                        decl->hiddenFromLookup = true;
                }
            }
        }
    }
    checkStmt(stmt->body);
}

void SemanticsStmtVisitor::visitSeqStmt(SeqStmt* stmt)
{
    for (auto& ss : stmt->stmts)
    {
        ss = maybeParseStmt(ss, *this);
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

void SemanticsStmtVisitor::generateUniqueIDForStmt(BreakableStmt* stmt)
{
    stmt->uniqueID = getASTBuilder()->generateUniqueIDForStmt();
}

void SemanticsStmtVisitor::visitBreakStmt(BreakStmt* stmt)
{
    // We need to identify the enclosing statement that
    // this `break` is meant to break out of.
    //
    BreakableStmt* targetOuterStmt = nullptr;
    if (stmt->targetLabel.type == TokenType::Identifier)
    {
        // If this is a `break` statement that specifies
        // an explicit label, then we will search for
        // an outer statement matching that label.
        //
        auto foundOuterStmt = findOuterStmtWithLabel(stmt->targetLabel.getName());
        if (!foundOuterStmt)
        {
            getSink()->diagnose(Diagnostics::BreakLabelNotFound{
                .label = stmt->targetLabel.getName(),
                .stmt = stmt});
        }
        else
        {
            // It is possible that the labelled statement
            // is not a valid one for a `break` to target,
            // so we check for that next.
            //
            targetOuterStmt = as<BreakableStmt>(foundOuterStmt);
            if (!targetOuterStmt)
            {
                getSink()->diagnose(Diagnostics::TargetLabelDoesNotMarkBreakableStmt{
                    .label = stmt->targetLabel.getName(),
                    .stmt = stmt});
            }
        }
    }
    else
    {
        // If there is no explicit label on the `break` statement,
        // then we are simply searching for the inner-most
        // enclosing statement that is a valid `break` target.
        //
        targetOuterStmt = FindOuterStmt<BreakableStmt>();
        if (!targetOuterStmt)
        {
            getSink()->diagnose(Diagnostics::BreakOutsideLoop{.stmt = stmt});
        }
    }

    // We do not (currently) allow a `break` to proceed "through"
    // an enclosing `defer` statement. Thus, we search for
    // a possible enclosing `defer` statement, between the
    // `stmt` being checked and the `targetOuterStmt` that
    // `stmt` is trying to branch to.
    //
    // TODO: This is a reasonable feature to add down the line;
    // it simply involves more implementation complexity than
    // the simpler cases of `defer`.
    //
    if (targetOuterStmt)
    {
        if (FindOuterStmt<DeferStmt>(targetOuterStmt))
        {
            getSink()->diagnose(Diagnostics::BreakInsideDefer{.stmt = stmt});
        }

        // We stash the ID of the target statement in the `break`
        // statement so that they can be correlated later, during
        // code generation.
        //
        stmt->targetOuterStmtID = targetOuterStmt->uniqueID;
    }
}

void SemanticsStmtVisitor::visitContinueStmt(ContinueStmt* stmt)
{
    auto targetOuterStmt = FindOuterStmt<LoopStmt>();
    if (!targetOuterStmt)
    {
        getSink()->diagnose(Diagnostics::ContinueOutsideLoop{.stmt = stmt});
    }
    else
    {
        if (FindOuterStmt<DeferStmt>(targetOuterStmt))
        {
            getSink()->diagnose(Diagnostics::ContinueInsideDefer{.stmt = stmt});
        }

        // We stash the ID of the target statement in the `continue`
        // statement so that they can be correlated later, during
        // code generation.
        //
        stmt->targetOuterStmtID = targetOuterStmt->uniqueID;
    }
}

Expr* SemanticsVisitor::checkPredicateExpr(Expr* expr)
{
    if (as<AssignExpr>(expr))
    {
        getSink()->diagnose(Diagnostics::AssignmentInPredicateExpr{.expr = expr});
    }
    Expr* e = expr;
    e = CheckTerm(e);
    e = maybeRegisterLambdaCapture(e);
    e = coerce(CoercionSite::General, m_astBuilder->getBoolType(), e, getSink());
    return e;
}

void SemanticsStmtVisitor::visitDoWhileStmt(DoWhileStmt* stmt)
{
    generateUniqueIDForStmt(stmt);
    checkModifiers(stmt);
    WithOuterStmt subContext(this, stmt);

    stmt->predicate = checkPredicateExpr(stmt->predicate);
    subContext.checkStmt(stmt->statement);
    checkLoopInDifferentiableFunc(stmt);
}

void SemanticsStmtVisitor::visitForStmt(ForStmt* stmt)
{
    generateUniqueIDForStmt(stmt);
    WithOuterStmt subContext(this, stmt);
    checkModifiers(stmt);
    checkStmt(stmt->initialStatement);

    if (stmt->predicateExpression)
    {
        stmt->predicateExpression = checkPredicateExpr(stmt->predicateExpression);
    }
    if (stmt->sideEffectExpression)
    {
        // A `for` loop's side-effect expression is a statement-like context (its value is ignored),
        // subject to the same rules as an expression statement. Check it through the shared
        // bottleneck, but in a context that permits a comma operator without the usage warning,
        // since `for (;; i++, j++)` is idiomatic.
        SemanticsContext sideEffectContext = withInForLoopSideEffect();
        SemanticsStmtVisitor subVisitor(sideEffectContext);
        stmt->sideEffectExpression =
            subVisitor.checkExprInStmtLikeContext(stmt->sideEffectExpression);
    }
    subContext.checkStmt(stmt->statement);

    tryInferLoopMaxIterations(stmt);

    checkLoopInDifferentiableFunc(stmt);
}

Expr* SemanticsVisitor::checkExpressionAndExpectIntegerConstant(
    Expr* expr,
    IntVal** outIntVal,
    ConstantFoldingKind kind)
{
    expr = CheckExpr(expr);
    auto intVal = CheckIntegerConstantExpression(
        expr,
        IntegerConstantExpressionCoercionType::AnyInteger,
        nullptr,
        kind);
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
        stmt->rangeBeginExpr = checkExpressionAndExpectIntegerConstant(
            stmt->rangeBeginExpr,
            &rangeBeginVal,
            ConstantFoldingKind::LinkTime);
    }
    else
    {
        ConstantIntVal* rangeBeginConst = m_astBuilder->getIntVal(m_astBuilder->getIntType(), 0);
        rangeBeginVal = rangeBeginConst;
    }

    stmt->rangeEndExpr = checkExpressionAndExpectIntegerConstant(
        stmt->rangeEndExpr,
        &rangeEndVal,
        ConstantFoldingKind::LinkTime);

    stmt->rangeBeginVal = rangeBeginVal;
    stmt->rangeEndVal = rangeEndVal;

    subContext.checkStmt(stmt->body);
}

void SemanticsStmtVisitor::validateCaseStmts(SwitchStmt* stmt, DiagnosticSink* sink)
{
    auto blockStmt = as<BlockStmt>(stmt->body);
    if (!blockStmt)
        return;

    auto seqStmt = as<SeqStmt>(blockStmt->body);
    if (!seqStmt)
        return;

    bool hasDefaultStmt = false;
    HashSet<Val*> caseStmtVals;
    for (auto& sStmt : seqStmt->stmts)
    {
        if (auto caseStmt = as<CaseStmt>(sStmt))
        {
            // check that all case tags are unique
            if (caseStmt->exprVal)
            {
                // exprVal contains the constant folded expr, that is checked for
                // uniqueness within the scope of the switch statement.
                if (!caseStmtVals.add(caseStmt->exprVal))
                {
                    sink->diagnose(Diagnostics::SwitchDuplicateCases{.stmt = sStmt});
                    return;
                }
            }
        }
        else if (as<DefaultStmt>(sStmt))
        {
            // check that there is at most one `default` clause
            if (hasDefaultStmt)
            {
                sink->diagnose(Diagnostics::SwitchMultipleDefault{.stmt = sStmt});
                return;
            }
            hasDefaultStmt = true;
        }
    }
}

void SemanticsStmtVisitor::visitSwitchStmt(SwitchStmt* stmt)
{
    generateUniqueIDForStmt(stmt);
    WithOuterStmt subContext(this, stmt);

    stmt->condition = CheckExpr(stmt->condition);

    // Reject a non-integer/enum selector here so no inconsistent `switch` reaches IR
    // lowering; skip when the condition already failed to check to avoid a cascade.
    auto conditionType = stmt->condition->type.type;
    if (conditionType && !as<ErrorType>(conditionType) &&
        !isValidCompileTimeConstantType(conditionType))
    {
        getSink()->diagnose(
            Diagnostics::SwitchConditionNotInteger{.type = conditionType, .expr = stmt->condition});
        return;
    }

    subContext.checkStmt(stmt->body);

    // check the case value exits within the switch
    validateCaseStmts(stmt, getSink());
}

void SemanticsStmtVisitor::visitCaseStmt(CaseStmt* stmt)
{
    // A 'case' statement must be directly enclosed by a 'switch' statement. If
    // this is not the case, the parser has already diagnosed an error.
    SwitchStmt* switchStmt = m_outerStmts ? as<SwitchStmt>(m_outerStmts->stmt) : nullptr;
    if (!switchStmt)
        return;

    // Check that the type for the `case` is consistent with the type for the `switch`.
    auto expr = CheckExpr(stmt->expr);
    expr = coerce(CoercionSite::Argument, switchStmt->condition->type, expr, getSink());

    // coerce to type being switch on, and ensure that value is a compile-time constant
    // The Vals in the AST are pointer-unique, making them easy to check for duplicates
    // by addeing them to a HashSet.
    auto exprVal = checkConstantIntVal(expr);

    stmt->expr = expr;
    stmt->exprVal = exprVal;

    // We stash the ID of the target statement in the `case`
    // statement so that they can be correlated later, during
    // code generation.
    //
    stmt->targetOuterStmtID = switchStmt->uniqueID;
}

void SemanticsStmtVisitor::visitTargetSwitchStmt(TargetSwitchStmt* stmt)
{
    generateUniqueIDForStmt(stmt);
    WithOuterStmt subContext(this, stmt);
    HashSet<Stmt*> checkedStmt;
    for (auto caseStmt : stmt->targetCases)
    {
        CapabilitySet set((CapabilityName)caseStmt->capability);

        CapabilityName canonicalStage = CapabilityName::Invalid;
        bool isStage = isStageAtom((CapabilityName)caseStmt->capability, canonicalStage);
        if (as<StageSwitchStmt>(stmt))
        {
            if (!isStage && caseStmt->capability != (int32_t)CapabilityName::Invalid)
            {
                getSink()->diagnose(Diagnostics::UnknownStageName{
                    .stageName = String(caseStmt->capabilityToken.getContent()),
                    .location = caseStmt->capabilityToken.loc});
            }
            caseStmt->capability = (int32_t)canonicalStage;
        }
        else
        {
            if (isStage)
            {
                getSink()->diagnose(Diagnostics::TargetSwitchCaseCannotBeAStage{
                    .location = caseStmt->capabilityToken.loc});
            }
            else if (
                caseStmt->capabilityToken.getContentLength() != 0 &&
                (set.getCapabilityTargetSets().getCount() != 1 || set.isInvalid() || set.isEmpty()))
            {
                getSink()->diagnose(Diagnostics::InvalidTargetSwitchCase{
                    .capability = capabilityNameToString((CapabilityName)caseStmt->capability),
                    .location = caseStmt->capabilityToken.loc});
            }
        }

        if (checkedStmt.contains(caseStmt->body))
            continue;
        subContext.checkStmt(caseStmt);
        checkedStmt.add(caseStmt->body);
    }
}

void SemanticsStmtVisitor::visitTargetCaseStmt(TargetCaseStmt* stmt)
{
    auto switchStmt = FindOuterStmt<TargetSwitchStmt>();
    if (getShared()->isInLanguageServer() &&
        getShared()->getSession()->getCompletionRequestTokenName() ==
            stmt->capabilityToken.getName())
    {
        getShared()->getLinkage()->contentAssistInfo.completionSuggestions.scopeKind =
            CompletionSuggestions::ScopeKind::Capabilities;
    }
    if (!switchStmt)
    {
        getSink()->diagnose(Diagnostics::CaseOutsideSwitch{.stmt = stmt});
    }
    else
    {
        stmt->targetOuterStmtID = switchStmt->uniqueID;
    }
    WithOuterStmt subContext(this, stmt);
    subContext.checkStmt(stmt->body);
}

void SemanticsStmtVisitor::visitIntrinsicAsmStmt(IntrinsicAsmStmt* stmt)
{
    WithOuterStmt subContext(this, stmt);
    for (auto& arg : stmt->args)
        arg = subContext.CheckExpr(arg);
}

void SemanticsStmtVisitor::visitDefaultStmt(DefaultStmt* stmt)
{
    // A 'default' statement must be directly enclosed by a 'switch'
    // statement. If this is not the case, the parser has already diagnosed an
    // error.
    SwitchStmt* switchStmt = m_outerStmts ? as<SwitchStmt>(m_outerStmts->stmt) : nullptr;
    if (!switchStmt)
        return;

    // We stash the ID of the target statement in the `default`
    // statement so that they can be correlated later, during
    // code generation.
    //
    stmt->targetOuterStmtID = switchStmt->uniqueID;
}

void SemanticsStmtVisitor::visitIfStmt(IfStmt* stmt)
{
    WithOuterStmt subContext(this, stmt);
    stmt->predicate = checkPredicateExpr(stmt->predicate);
    subContext.checkStmt(stmt->positiveStatement);
    subContext.checkStmt(stmt->negativeStatement);
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

void SemanticsStmtVisitor::visitReturnStmt(ReturnStmt* stmt)
{
    auto function = getParentFunc();
    Type* returnType = nullptr;
    Type* expectedReturnType = nullptr;
    if (m_parentLambdaDecl)
    {
        expectedReturnType = m_parentLambdaDecl->funcDecl->returnType.type;
    }
    else if (function)
    {
        expectedReturnType = function->returnType.type;
    }
    if (!stmt->expression)
    {
        if (expectedReturnType && !expectedReturnType->equals(m_astBuilder->getVoidType()) &&
            !as<ConstructorDecl>(function))
        {
            getSink()->diagnose(Diagnostics::ReturnNeedsExpression{.stmt = stmt});
        }
    }
    else
    {
        stmt->expression = CheckExpr(stmt->expression);
        returnType = stmt->expression->type.type;
        if (!stmt->expression->type->equals(m_astBuilder->getErrorType()))
        {
            if (!m_parentLambdaExpr && expectedReturnType)
            {
                stmt->expression =
                    coerce(CoercionSite::Return, expectedReturnType, stmt->expression, getSink());
            }
        }
    }
    if (m_parentLambdaDecl)
    {
        if (!returnType)
            returnType = m_astBuilder->getVoidType();
        if (!m_parentLambdaDecl->funcDecl->returnType.type)
            m_parentLambdaDecl->funcDecl->returnType.type = returnType;
        if (!m_parentLambdaDecl->funcDecl->returnType.type->equals(returnType))
        {
            getSink()->diagnose(Diagnostics::ReturnTypeMismatchInsideLambda{
                .returnedType = returnType,
                .previousType = m_parentLambdaDecl->funcDecl->returnType.type,
                .stmt = stmt});
        }
    }

    if (FindOuterStmt<DeferStmt>())
    {
        getSink()->diagnose(Diagnostics::ReturnInsideDefer{.stmt = stmt});
    }
}

void SemanticsStmtVisitor::visitWhileStmt(WhileStmt* stmt)
{
    generateUniqueIDForStmt(stmt);
    checkModifiers(stmt);
    WithOuterStmt subContext(this, stmt);
    stmt->predicate = checkPredicateExpr(stmt->predicate);
    subContext.checkStmt(stmt->statement);
    checkLoopInDifferentiableFunc(stmt);
}

void SemanticsStmtVisitor::visitDeferStmt(DeferStmt* stmt)
{
    WithOuterStmt subContext(this, stmt);
    subContext.checkStmt(stmt->statement);
}

void SemanticsStmtVisitor::visitThrowStmt(ThrowStmt* stmt)
{
    stmt->expression = CheckExpr(stmt->expression);
    Stmt* catchStmt = findMatchingCatchStmt(stmt->expression->type);

    auto parentFunc = getParentFunc();
    if (!catchStmt && (!parentFunc || parentFunc->errorType->equals(m_astBuilder->getBottomType())))
    {
        getSink()->diagnose(Diagnostics::UncaughtThrowInNonThrowFunc{.stmt = stmt});
        return;
    }

    if (!catchStmt && !stmt->expression->type->equals(m_astBuilder->getErrorType()))
    {
        if (!parentFunc->errorType->equals(stmt->expression->type))
        {
            getSink()->diagnose(Diagnostics::ThrowTypeIncompatibleWithErrorType{
                .throwType = stmt->expression->type.type,
                .errorType = parentFunc->errorType,
                .expr = stmt->expression});
        }
    }

    if (FindOuterStmt<DeferStmt>(catchStmt))
    {
        // Allowing 'throw' to escape a defer statement gets quite complex, for
        // similar reasons as 'return' - if you have two (or more) defers,
        // both of which exit the outer scope, it's unclear which one gets
        // called and when. Both can't fully run. That kind of goes against the
        // point of 'defer', which is to _always_ run some code when exiting
        // scopes.
        getSink()->diagnose(Diagnostics::UncaughtThrowInsideDefer{.stmt = stmt});
    }
}

void SemanticsStmtVisitor::visitCatchStmt(CatchStmt* stmt)
{
    if (stmt->errorVar)
    {
        ensureDeclBase(stmt->errorVar, DeclCheckState::DefinitionChecked, this);
        stmt->errorVar->hiddenFromLookup = false;
    }

    WithOuterStmt subContext(this, stmt);
    subContext.checkStmt(stmt->tryBody);
    subContext.checkStmt(stmt->handleBody);
}

void SemanticsStmtVisitor::visitExpressionStmt(ExpressionStmt* stmt)
{
    stmt->expression = checkExprInStmtLikeContext(stmt->expression);
}

/// Peel wrappers that are transparent to a discarded-position analysis, so a caller classifies the
/// underlying expression rather than the wrapper. `ParenExpr` (`(f());`) and `TryExpr` (`try f();`)
/// are present both before and after checking; a `LetExpr` (a temporary binding checking inserts,
/// e.g. `moveTemp` in `slang-check-expr.cpp`) appears only after checking and carries no user
/// source location of its own, so it too must be peeled. Passing `peelLetExpr = false` peels only
/// the wrappers present pre-check.
static Expr* peelDiscardedPositionWrappers(Expr* expr, bool peelLetExpr)
{
    for (;;)
    {
        if (auto paren = as<ParenExpr>(expr))
        {
            expr = paren->base;
            continue;
        }
        if (auto tryExpr = as<TryExpr>(expr))
        {
            expr = tryExpr->base;
            continue;
        }
        if (peelLetExpr)
        {
            if (auto letExpr = as<LetExpr>(expr))
            {
                expr = letExpr->body;
                continue;
            }
        }
        break;
    }
    return expr;
}

/// If `expr` is a comma operator (`a, b`), return it as an `OperatorExpr`; otherwise null. A comma's
/// last operand carries its result, so a comma in a discarded position discards that last operand.
static OperatorExpr* asCommaExpr(Expr* expr)
{
    auto operatorExpr = as<OperatorExpr>(expr);
    if (operatorExpr && getOperatorName(operatorExpr) == UnownedStringSlice(","))
        return operatorExpr;
    return nullptr;
}

/// If `expr` is an unchecked short-circuit `&&`/`||` operator, return it as an `OperatorExpr`;
/// otherwise null. Before checking, `&&`/`||` are ordinary named operator calls; checking later
/// rewrites them to a `LogicOperatorShortCircuitExpr`. The right operand carries the value that
/// flows out (the left operand is consumed to decide short-circuiting), so a discarded short-circuit
/// discards that right operand.
static OperatorExpr* asUncheckedShortCircuitExpr(Expr* expr)
{
    auto operatorExpr = as<OperatorExpr>(expr);
    if (operatorExpr)
    {
        auto name = getOperatorName(operatorExpr);
        if (name == UnownedStringSlice("&&") || name == UnownedStringSlice("||"))
            return operatorExpr;
    }
    return nullptr;
}

bool SemanticsVisitor::doesExprFormAllowIgnoringResult(Expr* expr)
{
    // These forms compute a value but are written for their effect, so ignoring the result is
    // idiomatic and not worth a diagnostic: an assignment or compound assignment (`x = y`, `x += y`)
    // yields the assigned value, and a prefix/postfix `++`/`--` yields the incremented value.
    return isAssignmentOrCompoundAssignmentExpr(expr) || isIncrementOrDecrementExpr(expr);
}

bool SemanticsVisitor::isExprSyntacticFormAppropriateForStmtLikeContext(Expr* expr)
{
    // Judge the form on the parsed, unchecked expression: checking can rewrite an arithmetic or
    // comparison operator into a `BuiltinOperatorExpr` and erase the original form, so this must run
    // before that. Peel the wrappers that do not change the essential form (parentheses, a `try`
    // clause); a `LetExpr` cannot appear here because this runs pre-check.
    expr = peelDiscardedPositionWrappers(expr, /*peelLetExpr*/ false);

    // An `expand e` evaluates the pack expansion `e` for effect, so it is appropriate exactly when
    // its base expression `e` is.
    if (auto expandExpr = as<ExpandExpr>(expr))
        return isExprSyntacticFormAppropriateForStmtLikeContext(expandExpr->baseExpr);

    // A comma `a, b` yields its last operand's value, so the statement's essential form is that
    // operand's form (the earlier operands are checked as their own statement-like contexts where
    // the comma is checked). Judge the form of the last operand.
    if (auto commaExpr = asCommaExpr(expr))
    {
        auto count = commaExpr->arguments.getCount();
        if (count > 0)
            return isExprSyntacticFormAppropriateForStmtLikeContext(commaExpr->arguments[count - 1]);
    }

    // A ternary `c ? a : b` yields the value of the chosen arm, so its essential statement form is
    // that of its arms: appropriate exactly when both arms are. (The condition `c` is consumed to
    // choose an arm, so its form is not judged here.)
    if (auto selectExpr = as<SelectExpr>(expr))
    {
        if (selectExpr->arguments.getCount() == 3)
            return isExprSyntacticFormAppropriateForStmtLikeContext(selectExpr->arguments[1]) &&
                   isExprSyntacticFormAppropriateForStmtLikeContext(selectExpr->arguments[2]);
    }

    // A short-circuit `a && b` / `a || b` yields its right operand's value (the left operand is
    // consumed to decide short-circuiting), so its essential form is that operand's form.
    if (auto shortCircuitExpr = asUncheckedShortCircuitExpr(expr))
    {
        auto count = shortCircuitExpr->arguments.getCount();
        if (count > 0)
            return isExprSyntacticFormAppropriateForStmtLikeContext(
                shortCircuitExpr->arguments[count - 1]);
    }

    // The forms whose result is normally discarded on purpose are the ones appropriate as a
    // statement: a genuine call (`f(x)`), a cast (`(int)x` / `int(x)`, admitted here as a separate
    // call-like form since `asCallExpr` excludes it), an assignment or compound assignment, a
    // prefix/postfix `++`/`--`, and an inline `spirv_asm { ... }` block executed for effect.
    // Anything else (a bare variable, a bare member reference, a bare unapplied function reference,
    // a lambda, ...) does nothing useful as a statement.
    return asCallExpr(expr) || as<TypeCastExpr>(expr) || doesExprFormAllowIgnoringResult(expr) ||
           as<SPIRVAsmExpr>(expr);
}

void SemanticsVisitor::collectResultIgnoringDiscardedLeafLocs(
    Expr* uncheckedExpr,
    HashSet<SourceLoc::RawValue>& ioLocs)
{
    // Walk the unchecked discarded-position structure and record the source location of every
    // discarded leaf whose form (`x = y`, `x += y`, `++x`, ...) means its result may be ignored.
    // The type-pass below reads this decision by source location, so the syntactic judgment is made
    // on the unchecked form (which is what the user wrote) while the type is read off the checked
    // tree. A source location survives checking: coercion wrapping a leaf in an `ImplicitCastExpr`
    // copies the operand's location, and identity-cast removal keeps the operand's location.
    uncheckedExpr = peelDiscardedPositionWrappers(uncheckedExpr, /*peelLetExpr*/ false);

    if (auto expandExpr = as<ExpandExpr>(uncheckedExpr))
    {
        collectResultIgnoringDiscardedLeafLocs(expandExpr->baseExpr, ioLocs);
        return;
    }

    // A comma discards its last operand's result (its other operands are handled where the comma
    // itself is checked), so recurse only into that last operand.
    if (auto commaExpr = asCommaExpr(uncheckedExpr))
    {
        auto count = commaExpr->arguments.getCount();
        if (count > 0)
            collectResultIgnoringDiscardedLeafLocs(commaExpr->arguments[count - 1], ioLocs);
        return;
    }

    // A ternary discards the value of whichever arm is chosen; recurse into both arms (the condition
    // is consumed to choose an arm, not discarded).
    if (auto selectExpr = as<SelectExpr>(uncheckedExpr))
    {
        if (selectExpr->arguments.getCount() == 3)
        {
            collectResultIgnoringDiscardedLeafLocs(selectExpr->arguments[1], ioLocs);
            collectResultIgnoringDiscardedLeafLocs(selectExpr->arguments[2], ioLocs);
        }
        return;
    }

    // A short-circuit discards its right operand's result (the left operand is consumed to decide
    // short-circuiting), so recurse only into that right operand.
    if (auto shortCircuitExpr = asUncheckedShortCircuitExpr(uncheckedExpr))
    {
        auto count = shortCircuitExpr->arguments.getCount();
        if (count > 0)
            collectResultIgnoringDiscardedLeafLocs(shortCircuitExpr->arguments[count - 1], ioLocs);
        return;
    }

    if (doesExprFormAllowIgnoringResult(uncheckedExpr))
        ioLocs.add(uncheckedExpr->loc.getRaw());
}

void SemanticsVisitor::forEachDiscardedLeaf(
    Expr* checkedExpr,
    const std::function<void(Expr*)>& callback)
{
    // Walk the checked discarded-position structure, peeling the checker-inserted `LetExpr` in
    // addition to parentheses and `try`, and invoke `callback` at each leaf. The transparent
    // containers recursed into here match those in `collectResultIgnoringDiscardedLeafLocs`, so the
    // two walks visit the same leaves: a comma to its last operand, an `expand` to its base, a
    // ternary to both arms, and a short-circuit `&&`/`||` to its right operand. Each of these yields
    // the value of a sub-expression, so the discarded result is really that sub-expression's;
    // recursing lets the per-leaf pass see the actual discarded call and prefer its `[NoDiscard]`
    // diagnostic, rather than diagnosing the container as a single opaque whole.
    checkedExpr = peelDiscardedPositionWrappers(checkedExpr, /*peelLetExpr*/ true);

    if (auto expandExpr = as<ExpandExpr>(checkedExpr))
    {
        forEachDiscardedLeaf(expandExpr->baseExpr, callback);
        return;
    }

    if (auto commaExpr = asCommaExpr(checkedExpr))
    {
        auto count = commaExpr->arguments.getCount();
        if (count > 0)
            forEachDiscardedLeaf(commaExpr->arguments[count - 1], callback);
        return;
    }

    // A ternary yields the value of whichever arm is chosen, so both arms are discarded-position
    // leaves (the condition is consumed to choose an arm, not discarded).
    if (auto selectExpr = as<SelectExpr>(checkedExpr))
    {
        if (selectExpr->arguments.getCount() == 3)
        {
            forEachDiscardedLeaf(selectExpr->arguments[1], callback);
            forEachDiscardedLeaf(selectExpr->arguments[2], callback);
        }
        return;
    }

    // A short-circuit `a && b` / `a || b` yields its right operand's value (the left operand is
    // consumed to decide short-circuiting), so the right operand is the discarded-position leaf.
    if (auto shortCircuitExpr = as<LogicOperatorShortCircuitExpr>(checkedExpr))
    {
        auto count = shortCircuitExpr->arguments.getCount();
        if (count > 0)
            forEachDiscardedLeaf(shortCircuitExpr->arguments[count - 1], callback);
        return;
    }

    callback(checkedExpr);
}

Expr* SemanticsVisitor::checkExprInStmtLikeContext(Expr* expr)
{
    // The statement-like diagnostics are warnings by default, but are raised to errors under Slang
    // 202c, chosen per emission rather than by a global severity override.
    Severity severity = isSlang202cOrLater(this) ? Severity::Error : Severity::Warning;

    // The top-level form of the whole statement is judged on the unchecked expression, because
    // checking can rewrite an arithmetic/comparison operator into a `BuiltinOperatorExpr` and erase
    // the form the user wrote. This is a single top-level fact, not a per-leaf one.
    if (!isExprSyntacticFormAppropriateForStmtLikeContext(expr))
        getSink()->diagnoseWithSeverity(
            severity,
            Diagnostics::ExpressionStatementDisallowedForm{.expr = expr});

    // The per-leaf carve-out for a form that ignores its result on purpose (an assignment, `++`,
    // ...) is likewise judged on the unchecked tree, and recorded by source location so the type
    // pass below can consult it against the checked leaves.
    HashSet<SourceLoc::RawValue> resultIgnoringLeafLocs;
    collectResultIgnoringDiscardedLeafLocs(expr, resultIgnoringLeafLocs);

    // Check the expression and enforce that it has a proper type. Naming a type, generic, or
    // namespace where a value is required is diagnosed here and the expression is rewritten to an
    // `ErrorType`, which the per-leaf pass below then skips via cascading-error avoidance.
    expr = checkExprOfProperType(expr);

    // Diagnose the discarded result at each leaf that can become the whole expression's value, using
    // the checked leaf's type and attributes and the pre-check carve-out decision.
    forEachDiscardedLeaf(
        expr,
        [&](Expr* leaf) { maybeDiagnoseDiscardedResultAtLeaf(leaf, resultIgnoringLeafLocs); });

    return expr;
}

bool SemanticsVisitor::maybeDiagnoseDanglingEquality(Expr* leaf)
{
    // A discarded `==` is a likely mistyped `=`, and gets a dedicated diagnostic instead of the
    // generic discarded-result one. The comparison may be a resolved `operator==` call or, for the
    // common scalar case, a builtin fast-path `BuiltinOperatorExpr`.
    bool isDanglingEquality = false;
    if (auto operatorExpr = as<OperatorExpr>(leaf))
    {
        if (auto func = as<VarExpr>(operatorExpr->functionExpr))
            isDanglingEquality = func->name && func->name->text == "==";
    }
    else if (auto builtinOp = as<BuiltinOperatorExpr>(leaf))
    {
        isDanglingEquality = (builtinOp->op == BuiltinOperationKind::Eql);
    }
    if (isDanglingEquality)
        getSink()->diagnose(Diagnostics::DanglingEqualityExpr{.expr = leaf});
    return isDanglingEquality;
}

bool SemanticsVisitor::maybeDiagnoseDiscardedNoDiscardResult(Expr* leaf)
{
    // If the discarded expression is a call to a function marked `[NoDiscard]`, report that its
    // result is being ignored.
    auto invokeExpr = as<InvokeExpr>(leaf);
    if (!invokeExpr)
        return false;

    // `[NoDiscard]` on a `void`-returning function is already rejected at the declaration (see
    // `NoDiscardOnVoidFunction` in `checkCallableDeclCommon`). That diagnostic is an error but not
    // fatal, so calls to such a function still reach here; this guard suppresses a second,
    // nonsensical "result is discarded" error at every call site on top of the declaration error.
    if (invokeExpr->type.type && invokeExpr->type.type->equals(m_astBuilder->getVoidType()))
        return false;

    auto funcDeclRefExpr = as<DeclRefExpr>(invokeExpr->functionExpr);
    if (!funcDeclRefExpr)
        return false;
    auto calleeDecl = funcDeclRefExpr->declRef.getDecl();
    if (!calleeDecl)
        return false;

    // A bare discarded construction is intentionally outside the `[NoDiscard]` diagnostic.
    if (as<ConstructorDecl>(calleeDecl) || !calleeDecl->findModifier<NoDiscardAttribute>())
        return false;

    getSink()->diagnose(
        Diagnostics::DiscardedNoDiscardResult{.name = calleeDecl->getName(), .expr = invokeExpr});
    return true;
}

bool SemanticsVisitor::calleeDeclAllowsDiscardingResult(Expr* leaf)
{
    // A call to a function marked `[DiscardableResult]` may have its result discarded without a
    // diagnostic (e.g. an atomic that both updates memory and returns the previous value).
    auto invokeExpr = as<InvokeExpr>(leaf);
    if (!invokeExpr)
        return false;
    auto funcDeclRefExpr = as<DeclRefExpr>(invokeExpr->functionExpr);
    if (!funcDeclRefExpr)
        return false;
    auto calleeDecl = funcDeclRefExpr->declRef.getDecl();
    return calleeDecl && calleeDecl->findModifier<DiscardableResultAttribute>();
}

void SemanticsVisitor::maybeDiagnoseDiscardedResultAtLeaf(
    Expr* leaf,
    const HashSet<SourceLoc::RawValue>& resultIgnoringLeafLocs)
{
    // A leaf whose unchecked form was recorded as result-ignoring (an assignment, `++`, ...) is
    // idiomatic as a statement and draws no discarded-result diagnostic.
    if (resultIgnoringLeafLocs.contains(leaf->loc.getRaw()))
        return;

    auto type = leaf->type.type;

    // Skip an error-typed result (cascading-error avoidance; e.g. a bare type name was already
    // diagnosed and rewritten to an `ErrorType`) and a `void` result (nothing is discarded).
    if (!type || as<ErrorType>(type) || type->equals(m_astBuilder->getVoidType()))
        return;

    // Specific-then-general, first match wins: a dangling `==`, then a `[NoDiscard]` call (its own
    // error), then a `[DiscardableResult]` call (suppressed), then the general discarded-result
    // warning.
    if (maybeDiagnoseDanglingEquality(leaf))
        return;
    if (maybeDiagnoseDiscardedNoDiscardResult(leaf))
        return;
    if (calleeDeclAllowsDiscardingResult(leaf))
        return;

    // The discarded-result diagnostic is a warning by default, raised to an error under Slang 202c.
    Severity severity = isSlang202cOrLater(this) ? Severity::Error : Severity::Warning;
    getSink()->diagnoseWithSeverity(
        severity,
        Diagnostics::DiscardedExpressionResult{.expr = leaf});

    // A follow-up hint when the discarded value has a function type, since a bare function name is a
    // common mistake for a call. This keys on the type only, with no form gate.
    if (as<FuncType>(type))
        getSink()->diagnose(Diagnostics::DiscardedResultOfFunctionType{.expr = leaf});
}

void SemanticsStmtVisitor::visitRequireCapabilityStmt(RequireCapabilityStmt*)
{
    // Nothing to do
}

void SemanticsStmtVisitor::tryInferLoopMaxIterations(ForStmt* stmt)
{
    // If a for loop is in the form of `for (var = initialVal; var $compareOp otherVal; var
    // sideEffectOp operand)` we will try to constant fold the operands and see if we can statically
    // determine the maximum number of iterations this loop will run, and insert the inferred result
    // as a `[MaxIters]` attribute on the stmt.
    //
    // ++, --, +=, -= are supported in side effect expressions.
    // >, <, >=, <= are supported in predicate expressions.
    // induction variable can appear in either side of the expressions.
    //
    // Other forms like for (var1 = .., var2 = ..; ) will not be recognized here.
    // If we see suspicious code like `for (int i = 0; i < 5; j++)`, we will produce a warning along
    // the way.
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

    auto initialLitVal = as<ConstantIntVal>(
        tryFoldIntegerConstantExpression(initialVal, ConstantFoldingKind::CompileTime, nullptr));

    ConstantIntVal* finalVal = nullptr;
    IROp compareOp = kIROp_Nop;
    // A comparison loop predicate `i < N` on builtin scalar operands is always rewritten by the
    // fast path to a `BuiltinOperatorExpr`, so that is the only form we need to recognize here.
    auto cmpExpr = as<BuiltinOperatorExpr>(stmt->predicateExpression);
    if (!cmpExpr)
        return;
    switch (cmpExpr->op)
    {
    case BuiltinOperationKind::Less:
        compareOp = kIROp_Less;
        break;
    case BuiltinOperationKind::Leq:
        compareOp = kIROp_Leq;
        break;
    case BuiltinOperationKind::Greater:
        compareOp = kIROp_Greater;
        break;
    case BuiltinOperationKind::Geq:
        compareOp = kIROp_Geq;
        break;
    default:
        // Only ordering comparisons drive trip-count inference.
        return;
    }
    if (cmpExpr->arguments.getCount() != 2)
        return;
    auto leftCompareOperand = cmpExpr->arguments[0];
    auto rightCompareOperand = cmpExpr->arguments[1];
    if (!leftCompareOperand)
        return;
    if (!rightCompareOperand)
        return;
    if (auto rightVal = tryFoldIntegerConstantExpression(
            cmpExpr->arguments[1],
            ConstantFoldingKind::CompileTime,
            nullptr))
    {
        auto leftVar = as<VarExpr>(leftCompareOperand);
        if (!leftVar)
            return;
        predicateVar = leftVar->declRef;
        finalVal = as<ConstantIntVal>(rightVal);
    }
    else if (
        auto leftVal = tryFoldIntegerConstantExpression(
            cmpExpr->arguments[0],
            ConstantFoldingKind::CompileTime,
            nullptr))
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
                getSink()->diagnose(Diagnostics::ForLoopSideEffectChangingDifferentVar{
                    .initVar = initialVar.getDecl(),
                    .modifiedVar = varExpr->declRef.getDecl(),
                    .sideEffect = varExpr});
            return;
        }
    }
    else
        return;
    if (opSideEffectExpr->arguments.getCount() == 2)
    {
        auto stepVal = tryFoldIntegerConstantExpression(
            opSideEffectExpr->arguments[1],
            ConstantFoldingKind::CompileTime,
            nullptr);
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
            getSink()->diagnose(Diagnostics::ForLoopPredicateCheckingDifferentVar{
                .initVar = initialVar.getDecl(),
                .predicateVar = predicateVar.getDecl(),
                .predicate = stmt->predicateExpression});
        return;
    }
    if (!stepSize)
        return;
    if (stepSize->getValue() > 0)
    {
        if (sideEffectFuncOp == kIROp_Add && compareOp == kIROp_Greater ||
            sideEffectFuncOp == kIROp_Sub && compareOp == kIROp_Less)
        {
            getSink()->diagnose(Diagnostics::ForLoopChangingIterationVariableInOppsoiteDirection{
                .var = initialVar.getDecl(),
                .sideEffect = stmt->sideEffectExpression});
            return;
        }
    }
    else if (stepSize->getValue() < 0)
    {
        if (sideEffectFuncOp == kIROp_Add && compareOp == kIROp_Less ||
            sideEffectFuncOp == kIROp_Sub && compareOp == kIROp_Greater)
        {
            getSink()->diagnose(Diagnostics::ForLoopChangingIterationVariableInOppsoiteDirection{
                .var = initialVar.getDecl(),
                .sideEffect = stmt->sideEffectExpression});
            return;
        }
    }
    else
    {
        getSink()->diagnose(Diagnostics::ForLoopNotModifyingIterationVariable{
            .var = initialVar.getDecl(),
            .sideEffect = stmt->sideEffectExpression});
        return;
    }

    if (!initialLitVal || !finalVal)
        return;

    auto absStepSize = abs(stepSize->getValue());
    int adjustment = 0;
    if (compareOp == kIROp_Geq || compareOp == kIROp_Leq)
        adjustment = 1;

    auto iterations = (Math::Max(finalVal->getValue(), initialLitVal->getValue()) -
                       Math::Min(finalVal->getValue(), initialLitVal->getValue()) + absStepSize -
                       1 + adjustment) /
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
        getSink()->diagnose(Diagnostics::LoopRunsForZeroIterations{.stmt = stmt});
    }

    // Note: the inferred max iterations may not be valid if the loop body
    // also modifies the induction variable.
    // We detect this case during lower-to-ir and will remove the `InferredMaxItersAttribute`
    // if the loop body modifies the induction variable.
    //
    auto maxItersAttr = m_astBuilder->create<InferredMaxItersAttribute>();
    auto litExpr = m_astBuilder->create<IntegerLiteralExpr>();
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

void SemanticsStmtVisitor::visitGpuForeachStmt(GpuForeachStmt* stmt)
{
    stmt->device = CheckExpr(stmt->device);
    stmt->gridDims = CheckExpr(stmt->gridDims);
    ensureDeclBase(stmt->dispatchThreadID, DeclCheckState::DefinitionChecked, this);
    WithOuterStmt subContext(this, stmt);
    stmt->kernelCall = subContext.CheckExpr(stmt->kernelCall);
    return;
}
} // namespace Slang
