#include "slang-ast-support-types.h"
#include "slang-ast-base.h"
#include "slang-ast-type.h"
#include "slang-ast-expr.h"

namespace Slang
{
QualType::QualType(Type* type)
    : type(type)
    , isLeftValue(false)
{
    if (as<RefType>(type))
    {
        isLeftValue = true;
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

Expr* getInnerMostExprFromHigherOrderExpr(Expr* expr)
{
    HashSet<Expr*> workListSet;
    while (auto higherOrder = as<HigherOrderInvokeExpr>(expr))
    {
        if (workListSet.Add(higherOrder))
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
}
