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

            // Checking rejects a write-only `Ref`, but a write-only memory location
            // may still be assigned and must not be read.
            case AccessQualifier::WriteOnly:
                isLeftValue = true;
                isWriteOnlyRef = true;
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

Expr* getInnerMostExprFromHigherOrderExpr(Expr* expr, FunctionDifferentiableLevel& outLevel)
{
    HashSet<Expr*> workListSet;
    outLevel = FunctionDifferentiableLevel::None;
    while (auto higherOrder = as<HigherOrderInvokeExpr>(expr))
    {
        if (as<BackwardDifferentiateExpr>(expr))
            outLevel = FunctionDifferentiableLevel::Backward;
        else if (
            as<ForwardDifferentiateExpr>(expr) && outLevel == FunctionDifferentiableLevel::None)
            outLevel = FunctionDifferentiableLevel::Forward;
        if (workListSet.add(higherOrder))
        {
            expr = higherOrder->baseFunction;
        }
        else
        {
            // Circularity, return null.
            return nullptr;
        }
    }
    return expr;
}

UnownedStringSlice getHigherOrderOperatorName(HigherOrderInvokeExpr* expr)
{
    if (as<ForwardDifferentiateExpr>(expr))
        return UnownedStringSlice("fwd_diff");
    else if (as<BackwardDifferentiateExpr>(expr))
        return UnownedStringSlice("bwd_diff");
    return UnownedStringSlice();
}

AccessQualifier getRefParamPassingModeAccess(ParamPassingMode mode)
{
    switch (mode)
    {
    case ParamPassingMode::RefReadWrite:
        return AccessQualifier::ReadWrite;
    case ParamPassingMode::RefReadOnly:
        return AccessQualifier::Read;
    case ParamPassingMode::RefWriteOnly:
        return AccessQualifier::WriteOnly;
    default:
        SLANG_UNEXPECTED("parameter-passing mode has no reference access qualifier");
        UNREACHABLE_RETURN(AccessQualifier::ReadWrite);
    }
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
    case ParamPassingMode::RefReadWrite:
        sb << "ref";
        break;
    case ParamPassingMode::RefReadOnly:
        sb << "readonly ref";
        break;
    case ParamPassingMode::RefWriteOnly:
        sb << "writeonly ref";
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
