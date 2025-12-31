#include "slang-ir-bit-field-accessors.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
static IRInst* shl(IRBuilder& builder, IRInst* inst, const IRIntegerValue value)
{
    if (value == 0)
        return inst;
    auto width = maybeGetIntTypeWidth(inst->getDataType());
    if (width && value >= width.value())
        return builder.getIntValue(inst->getDataType(), 0);
    return builder.emitShl(
        inst->getDataType(),
        inst,
        builder.getIntValue(builder.getIntType(), value));
}

static IRInst* shr(IRBuilder& builder, IRInst* inst, const IRIntegerValue value)
{
    if (value == 0)
        return inst;
    auto width = maybeGetIntTypeWidth(inst->getDataType());
    auto isSigned = getIntTypeSigned(inst->getDataType());
    // If it's not signed, then we just shift all the set bits away
    if (width && value >= width.value() && !isSigned)
        return builder.getIntValue(inst->getDataType(), 0);
    // Since on many platforms bit shifting by the number of bits in the number
    // is undefined, correct this here assuming that the Slang IR has the same
    // restriction
    if (width && value >= width.value() && isSigned)
        return builder.emitShr(
            inst->getDataType(),
            inst,
            builder.getIntValue(builder.getIntType(), width.value() - 1));
    return builder.emitShr(
        inst->getDataType(),
        inst,
        builder.getIntValue(builder.getIntType(), value));
}

static void synthesizeBitFieldGetter(IRFunc* func, IRBitFieldAccessorDecoration* dec)
{
    const auto bitFieldType = func->getResultType();
    SLANG_ASSERT(isIntegralType(bitFieldType));
    SLANG_ASSERT(func->getParamCount() == 1);
    const auto structParamType = func->getParamType(0);
    const auto structType = as<IRStructType>(getResolvedInstForDecorations(structParamType));
    SLANG_ASSERT(structType);

    const auto backingMember = findStructField(structType, dec->getBackingMemberKey());
    const auto backingType = backingMember->getFieldType();
    SLANG_ASSERT(isIntegralType(backingType));

    IRBuilder builder{func};

    builder.setInsertInto(func);
    builder.emitBlock();
    const auto s = builder.emitParam(structParamType);

    // Construct the equivalent of this:
    // Note the cast of the backing value in order to get the correct sign
    // extension behaviour on the right shift
    // return (int(_backing) << (backingWidth-topOfFoo)) >> (backingWidth-fooWidth);

    const auto backingWidth = maybeGetIntTypeWidth(backingType);
    const auto fieldWidth = dec->getFieldWidth();
    const auto topOfField = dec->getFieldOffset() + fieldWidth;

    auto castBackingType = backingType;
    if (getIntTypeSigned(backingType) != getIntTypeSigned(func->getResultType()))
        castBackingType = builder.getType(getOppositeSignIntTypeOp(backingType->getOp()));

    const auto backingValue = builder.emitFieldExtract(backingType, s, dec->getBackingMemberKey());
    const auto castedBacking = builder.emitCast(castBackingType, backingValue);

    IRInst* rightShifted;
    if (backingWidth)
    {
        // Backing width is target-independent and known already.
        const auto leftShiftAmount = backingWidth.value() - topOfField;
        const auto rightShiftAmount = backingWidth.value() - fieldWidth;
        const auto leftShifted = shl(builder, castedBacking, leftShiftAmount);
        rightShifted = shr(builder, leftShifted, rightShiftAmount);
    }
    else
    {
        // Backing width is not yet know, so we need to express width with
        // sizeof(backingType)*8 and let peephole optimization deal with this
        // later on.
        const auto backingWidthSizeOf = builder.emitSizeOf(backingType);
        const auto intType = builder.getIntType();
        const auto backingBitWidth =
            builder.emitMul(intType, builder.getIntValue(intType, 8), backingWidthSizeOf);
        const auto leftShiftAmount =
            builder.emitSub(intType, backingBitWidth, builder.getIntValue(intType, topOfField));
        const auto rightShiftAmount =
            builder.emitSub(intType, backingBitWidth, builder.getIntValue(intType, fieldWidth));
        const auto leftShifted = builder.emitShl(backingType, castedBacking, leftShiftAmount);
        rightShifted = builder.emitShr(backingType, leftShifted, rightShiftAmount);
    }
    const auto castedToBitFieldType = builder.emitCast(bitFieldType, rightShifted);
    builder.emitReturn(castedToBitFieldType);

    builder.addSimpleDecoration<IRForceInlineDecoration>(func);
}

static IRIntegerValue setLowBits(IRIntegerValue bits)
{
    SLANG_ASSERT(bits >= 0 && bits <= 64);
    return ~(bits >= 64 ? 0 : (~0ULL << bits));
}

static void synthesizeBitFieldSetter(IRFunc* func, IRBitFieldAccessorDecoration* dec)
{
    SLANG_ASSERT(func->getParamCount() == 2);
    const auto ptrType = as<IRPtrTypeBase>(func->getParamType(0));
    SLANG_ASSERT(ptrType);
    const auto structParamType = ptrType->getValueType();
    const auto structType = as<IRStructType>(getResolvedInstForDecorations(structParamType));
    SLANG_ASSERT(structType);
    const auto bitFieldType = func->getParamType(1);
    SLANG_ASSERT(isIntegralType(bitFieldType));

    const auto backingMember = findStructField(structType, dec->getBackingMemberKey());
    const auto backingType = backingMember->getFieldType();
    SLANG_ASSERT(isIntegralType(backingType));

    IRBuilder builder{func};

    builder.setInsertInto(func);
    builder.emitBlock();
    const auto s = builder.emitParam(ptrType);
    const auto v = builder.emitParam(bitFieldType);

    // Construct the equivalent of this:
    // let fooMask = 0x00000FF0;
    // let bottomOfFoo = 4;
    // _backing = int((_backing & ~fooMask) | ((int(x) << bottomOfFoo) & fooMask));

    const auto fieldWidth = dec->getFieldWidth();
    const auto bottomOfField = dec->getFieldOffset();
    const auto maskBits = setLowBits(fieldWidth) << bottomOfField;
    const auto mask = builder.getIntValue(backingType, maskBits);
    const auto notMask = builder.getIntValue(backingType, ~maskBits);
    const auto memberAddr =
        builder.emitFieldAddress(builder.getPtrType(backingType), s, dec->getBackingMemberKey());
    const auto backingValue = builder.emitLoad(memberAddr);
    const auto maskedOut = builder.emitBitAnd(backingType, backingValue, notMask);
    const auto castValue = builder.emitCast(backingType, v);
    const auto shiftedLeft = shl(builder, castValue, bottomOfField);
    const auto maskedValue = builder.emitBitAnd(backingType, shiftedLeft, mask);
    const auto combined = builder.emitBitOr(backingType, maskedOut, maskedValue);
    builder.emitStore(memberAddr, combined);
    builder.emitReturn();

    builder.addSimpleDecoration<IRForceInlineDecoration>(func);
}

void synthesizeBitFieldAccessors(IRModule* module)
{
    for (const auto inst : module->getModuleInst()->getGlobalInsts())
    {
        const auto func = as<IRFunc>(getResolvedInstForDecorations(inst));
        if (!func)
            continue;
        const auto bfd = func->findDecoration<IRBitFieldAccessorDecoration>();
        if (!bfd)
            continue;
        if (func->getParamCount() == 1)
            synthesizeBitFieldGetter(func, bfd);
        else
            synthesizeBitFieldSetter(func, bfd);
    }
}
} // namespace Slang
