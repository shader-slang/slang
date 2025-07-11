// slang-ast-support-types.cpp
#include "slang-ast-support-types.h"

#include "slang-ast-base.h"
#include "slang-ast-expr.h"
#include "slang-ast-type.h"
#include "slang-check-impl.h"

namespace Slang
{
QualType::QualType(Type* type)
    : type(type), isLeftValue(false)
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

void printDiagnosticArg(StringBuilder& sb, ParameterDirection direction)
{
    switch (direction)
    {
    case kParameterDirection_In:
        sb << "in";
        break;
    case kParameterDirection_Out:
        sb << "out";
        break;
    case kParameterDirection_Ref:
        sb << "ref";
        break;
    case kParameterDirection_InOut:
        sb << "inout";
        break;
    case kParameterDirection_ConstRef:
        sb << "constref";
        break;
    default:
        sb << "(" << int(direction) << ")";
        break;
    }
}

KnownBuiltinDeclName getKnownBuiltinDeclName(UnownedStringSlice name)
{
    if (name == UnownedStringSlice::fromLiteral("GeometryStreamAppend"))
        return KnownBuiltinDeclName::GeometryStreamAppend;
    else if (name == UnownedStringSlice::fromLiteral("GeometryStreamRestart"))
        return KnownBuiltinDeclName::GeometryStreamRestart;
    else if (name == UnownedStringSlice::fromLiteral("GetAttributeAtVertex"))
        return KnownBuiltinDeclName::GetAttributeAtVertex;
    else if (name == UnownedStringSlice::fromLiteral("DispatchMesh"))
        return KnownBuiltinDeclName::DispatchMesh;
    else if (name == UnownedStringSlice::fromLiteral("saturated_cooperation"))
        return KnownBuiltinDeclName::saturated_cooperation;
    else if (name == UnownedStringSlice::fromLiteral("saturated_cooperation_using"))
        return KnownBuiltinDeclName::saturated_cooperation_using;
    else if (name == UnownedStringSlice::fromLiteral("IDifferentiable"))
        return KnownBuiltinDeclName::IDifferentiable;
    else if (name == UnownedStringSlice::fromLiteral("IDifferentiablePtr"))
        return KnownBuiltinDeclName::IDifferentiablePtr;
    else if (name == UnownedStringSlice::fromLiteral("IDifferentiablePtrType"))
        return KnownBuiltinDeclName::IDifferentiablePtrType;
    else if (name == UnownedStringSlice::fromLiteral("DrawIndex"))
        return KnownBuiltinDeclName::DrawIndex;
    else if (name == UnownedStringSlice::fromLiteral("NullDifferential"))
        return KnownBuiltinDeclName::NullDifferential;
    else
        return KnownBuiltinDeclName::None;
}

} // namespace Slang
