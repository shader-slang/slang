// slang-ast-support-types.cpp
#include "slang-ast-support-types.h"

#include "slang-ast-base.h"
#include "slang-ast-expr.h"
#include "slang-ast-print.h"
#include "slang-ast-type.h"
#include "slang-check-impl.h"

namespace Slang
{
QualType::QualType(Type* type)
    : type(type), isLeftValue(false)
{
    if (auto refType = as<ExplicitRefType>(type))
    {
        if (auto optAccessQualifier = refType->tryGetAccessQualifierValue())
        {
            auto accessQualifier = *optAccessQualifier;
            switch (accessQualifier)
            {
            case AccessQualifier::ReadWrite:
                isLeftValue = true;
                break;

            case AccessQualifier::Read:
            case AccessQualifier::Immutable:
                isLeftValue = false;
                break;

            default:
                SLANG_UNEXPECTED("unhandled access qualifier");
                break;
            }
        }
    }
}

void removeModifier(ModifiableSyntaxNode* syntax, Modifier* toRemove)
{
    Modifier* prev = nullptr;
    for (auto modifier = syntax->modifiers.first; modifier; modifier = modifier->next)
    {
        if (modifier == toRemove)
        {
            if (prev)
            {
                prev->next = modifier->next;
            }
            else
            {
                syntax->modifiers.first = syntax->modifiers.first->next;
            }
            break;
        }
        prev = modifier;
    }
}

Expr* getCallableExprThroughTransparentWrappers(Expr* expr, List<Expr*>* outWrappers)
{
    for (;;)
    {
        if (auto parenExpr = as<ParenExpr>(expr))
        {
            if (outWrappers)
                outWrappers->add(parenExpr);
            expr = parenExpr->base;
            continue;
        }
        if (auto genericAppExpr = as<GenericAppExpr>(expr))
        {
            if (outWrappers)
                outWrappers->add(genericAppExpr);
            expr = genericAppExpr->functionExpr;
            continue;
        }
        if (auto letExpr = as<LetExpr>(expr))
        {
            if (outWrappers)
                outWrappers->add(letExpr);
            expr = letExpr->body;
            continue;
        }
        return expr;
    }
}

bool isDirectAddressTupleElementSwizzle(SwizzleExpr* expr)
{
    return expr->elementIndices.getCount() == 1 &&
           as<TupleType>(expr->base->type.type) != nullptr;
}

Expr* getInnerMostExprFromHigherOrderExpr(Expr* expr, FunctionDifferentiableLevel& outLevel)
{
    HashSet<Expr*> workListSet;
    outLevel = FunctionDifferentiableLevel::None;
    for (;;)
    {
        expr = getCallableExprThroughTransparentWrappers(expr);
        auto higherOrder = as<HigherOrderInvokeExpr>(expr);
        if (!higherOrder)
            return expr;

        if (as<BackwardDifferentiateExpr>(higherOrder))
            outLevel = FunctionDifferentiableLevel::Backward;
        else if (as<ForwardDifferentiateExpr>(higherOrder))
        {
            if (outLevel == FunctionDifferentiableLevel::None)
                outLevel = FunctionDifferentiableLevel::Forward;
        }
        if (!workListSet.add(higherOrder))
            return nullptr;
        expr = higherOrder->baseFunction;
    }
}

UnownedStringSlice getHigherOrderOperatorName(HigherOrderInvokeExpr* expr)
{
    if (as<ForwardDifferentiateExpr>(expr))
        return UnownedStringSlice("fwd_diff");
    else if (as<BackwardDifferentiateExpr>(expr))
        return UnownedStringSlice("bwd_diff");
    return UnownedStringSlice();
}

void printDiagnosticArg(StringBuilder& sb, ParamPassingMode direction)
{
    switch (direction)
    {
    case ParamPassingMode::In:
        sb << "in";
        break;
    case ParamPassingMode::Out:
        sb << "out";
        break;
    case ParamPassingMode::Ref:
        sb << "ref";
        break;
    case ParamPassingMode::BorrowInOut:
        sb << "inout";
        break;
    case ParamPassingMode::BorrowIn:
        sb << "constref";
        break;
    default:
        sb << "(" << int(direction) << ")";
        break;
    }
}

String SpecializationArg::toString()
{
    if (val)
        return val->toString();
    else if (expr)
    {
        ASTPrinter printer(getCurrentASTBuilder());
        printer.addExpr(expr);
        return printer.getString();
    }
    return String();
}


} // namespace Slang
