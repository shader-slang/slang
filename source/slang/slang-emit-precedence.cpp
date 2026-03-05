// slang-emit-precedence.cpp
#include "slang-emit-precedence.h"

namespace Slang
{

#define SLANG_OP_INFO_EXPAND(op, name, precedence) \
    {                                              \
        name,                                      \
        kEPrecedence_##precedence##_Left,          \
        kEPrecedence_##precedence##_Right,         \
    },

/* static */ const EmitOpInfo EmitOpInfo::s_infos[int(EmitOp::CountOf)] = {
    SLANG_OP_INFO(SLANG_OP_INFO_EXPAND)};


EmitOp getEmitOpForOp(IROp op)
{
    switch (op)
    {
    case kIROp_Add:
    case kIROp_ConstexprAdd:
        return EmitOp::Add;
    case kIROp_Sub:
    case kIROp_ConstexprSub:
        return EmitOp::Sub;
    case kIROp_Mul:
    case kIROp_ConstexprMul:
        return EmitOp::Mul;
    case kIROp_Div:
    case kIROp_ConstexprDiv:
        return EmitOp::Div;
    case kIROp_IRem:
    case kIROp_ConstexprIRem:
        return EmitOp::Rem;
    case kIROp_FRem:
        return EmitOp::Rem;

    case kIROp_Lsh:
    case kIROp_ConstexprShl:
        return EmitOp::Lsh;
    case kIROp_Rsh:
    case kIROp_ConstexprShr:
        return EmitOp::Rsh;

    case kIROp_Eql:
    case kIROp_ConstexprEql:
        return EmitOp::Eql;
    case kIROp_Neq:
    case kIROp_ConstexprNeq:
        return EmitOp::Neq;
    case kIROp_Greater:
    case kIROp_ConstexprGreater:
        return EmitOp::Greater;
    case kIROp_Less:
    case kIROp_ConstexprLess:
        return EmitOp::Less;
    case kIROp_Geq:
    case kIROp_ConstexprGeq:
        return EmitOp::Geq;
    case kIROp_Leq:
    case kIROp_ConstexprLeq:
        return EmitOp::Leq;

    case kIROp_BitXor:
    case kIROp_ConstexprBitXor:
        return EmitOp::BitXor;
    case kIROp_BitOr:
    case kIROp_ConstexprBitOr:
        return EmitOp::BitOr;
    case kIROp_BitAnd:
    case kIROp_ConstexprBitAnd:
        return EmitOp::BitAnd;

    case kIROp_And:
    case kIROp_ConstexprAnd:
        return EmitOp::And;
    case kIROp_Or:
    case kIROp_ConstexprOr:
        return EmitOp::Or;

    case kIROp_Not:
    case kIROp_ConstexprNot:
        return EmitOp::Not;
    case kIROp_Neg:
    case kIROp_ConstexprNeg:
        return EmitOp::Neg;
    case kIROp_BitNot:
    case kIROp_ConstexprBitNot:
        return EmitOp::BitNot;
    }

    return EmitOp::None;
}

} // namespace Slang
