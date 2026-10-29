#include "slang-ir-spec-const-sized-array.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

/// Return the ordinary opcode for the operation that `constexprOp` performs, or `kIROp_Invalid` if
/// `constexprOp` is not one of the `constexpr*` opcodes.
static IROp getOrdinaryOpForConstexprOp(IROp constexprOp)
{
    switch (constexprOp)
    {
    case kIROp_ConstexprAdd:
        return kIROp_Add;
    case kIROp_ConstexprSub:
        return kIROp_Sub;
    case kIROp_ConstexprMul:
        return kIROp_Mul;
    case kIROp_ConstexprNeg:
        return kIROp_Neg;
    case kIROp_ConstexprDiv:
        return kIROp_Div;
    case kIROp_ConstexprIRem:
        return kIROp_IRem;
    case kIROp_ConstexprShl:
        return kIROp_Lsh;
    case kIROp_ConstexprShr:
        return kIROp_Rsh;
    case kIROp_ConstexprBitAnd:
        return kIROp_BitAnd;
    case kIROp_ConstexprBitOr:
        return kIROp_BitOr;
    case kIROp_ConstexprBitXor:
        return kIROp_BitXor;
    case kIROp_ConstexprBitNot:
        return kIROp_BitNot;
    case kIROp_ConstexprNot:
        return kIROp_Not;
    case kIROp_ConstexprEql:
        return kIROp_Eql;
    case kIROp_ConstexprNeq:
        return kIROp_Neq;
    case kIROp_ConstexprGreater:
        return kIROp_Greater;
    case kIROp_ConstexprLess:
        return kIROp_Less;
    case kIROp_ConstexprGeq:
        return kIROp_Geq;
    case kIROp_ConstexprLeq:
        return kIROp_Leq;
    case kIROp_ConstexprAnd:
        return kIROp_And;
    case kIROp_ConstexprOr:
        return kIROp_Or;
    case kIROp_ConstexprSelect:
        return kIROp_Select;
    case kIROp_ConstexprIntCast:
        return kIROp_IntCast;
    case kIROp_ConstexprCastIntToFloat:
        return kIROp_CastIntToFloat;
    case kIROp_ConstexprCastFloatToInt:
        return kIROp_CastFloatToInt;
    case kIROp_ConstexprFloatCast:
        return kIROp_FloatCast;
    // `lowerEnumType` has already replaced each enum type with its tag type, so the enum casts
    // are integer casts.
    case kIROp_ConstexprCastIntToEnum:
    case kIROp_ConstexprCastEnumToInt:
    case kIROp_ConstexprEnumCast:
        return kIROp_IntCast;
    default:
        return kIROp_Invalid;
    }
}

void lowerConstexprInstsToOrdinaryInsts(IRModule* module)
{
    // `constexpr*` instructions are hoistable, so they all live at module scope.
    List<IRInst*> constexprInsts;
    for (auto inst : module->getGlobalInsts())
    {
        if (getOrdinaryOpForConstexprOp(inst->getOp()) != kIROp_Invalid)
            constexprInsts.add(inst);
    }

    IRBuilder builder(module);
    for (auto inst : constexprInsts)
    {
        builder.setInsertBefore(inst);
        List<IRInst*> operands;
        for (UInt i = 0; i < inst->getOperandCount(); i++)
            operands.add(inst->getOperand(i));
        auto ordinaryInst = builder.emitIntrinsicInst(
            inst->getFullType(),
            getOrdinaryOpForConstexprOp(inst->getOp()),
            operands.getCount(),
            operands.getBuffer());
        inst->replaceUsesWith(ordinaryInst);
        inst->removeAndDeallocate();
    }
}

} // namespace Slang
