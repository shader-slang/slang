#include "slang-ir-bit-field-accessors.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{
static IRInst* maybeUnwrapGeneric(IRInst* inst)
{
    if(const auto g = as<IRGeneric>(inst))
        return findInnerMostGenericReturnVal(g);
    return inst;
}

static IRInst* maybeUnwrapSpecialize(IRInst* inst)
{
    if(const auto g = as<IRSpecialize>(inst))
        return maybeUnwrapGeneric(maybeUnwrapSpecialize(g->getBase()));
    return inst;
}

static void synthesizeBitFieldGetter(IRFunc* func, IRBitFieldAccessorDecoration* dec)
{
    const auto bitFieldType = func->getResultType();
    SLANG_ASSERT(isIntegralType(bitFieldType));
    SLANG_ASSERT(func->getParamCount() == 1);
    const auto structParamType = func->getParamType(0);
    const auto structType = as<IRStructType>(maybeUnwrapSpecialize(structParamType));
    SLANG_ASSERT(structType);

    const auto backingMember = findStructField(structType, dec->getBackingMemberKey());
    const auto backingType = backingMember->getFieldType();
    SLANG_ASSERT(isIntegralType(backingType));

    IRBuilder builder{func};

    const auto isSigned = getIntTypeInfo(func->getResultType()).isSigned;
    builder.setInsertInto(func);
    builder.emitBlock();
    const auto s = builder.emitParam(structParamType);

    // Construct the equivalent of this:
    // Note the cast of the backing value in order to get the correct sign
    // extension behaviour on the right shift
    // return (int(_backing) << (backingWidth-topOfFoo)) >> (backingWidth-fooWidth);

    const auto backingWidth = getIntTypeInfo(backingType).width;
    const auto fieldWidth = dec->getFieldWidth();
    const auto topOfField = dec->getFieldOffset() + fieldWidth;
    const auto leftShiftAmount = builder.getIntValue(builder.getIntType(), backingWidth - topOfField);
    const auto rightShiftAmount = builder.getIntValue(builder.getIntType(), backingWidth - fieldWidth);
    const auto backingValue = builder.emitFieldExtract(backingType, s, dec->getBackingMemberKey());
    const auto castBackingType = builder.getType(getIntTypeOpFromInfo({backingWidth, isSigned}));
    const auto castedBacking = builder.emitCast(castBackingType, backingValue);
    const auto leftShifted = builder.emitShl(castBackingType, castedBacking, leftShiftAmount);
    const auto rightShifted = builder.emitShr(castBackingType, leftShifted, rightShiftAmount);
    const auto castedToBitFieldType = builder.emitCast(bitFieldType, rightShifted);
    builder.emitReturn(castedToBitFieldType);

    builder.addSimpleDecoration<IRForceInlineDecoration>(func);
}

static IRIntegerValue setLowBits(IRIntegerValue bits)
{
    SLANG_ASSERT(bits >= 0 && bits <= 64);
    return ~(bits >= 64 ? 0 : (~0 << bits));
}

static void synthesizeBitFieldSetter(IRFunc* func, IRBitFieldAccessorDecoration* dec)
{
    SLANG_ASSERT(func->getParamCount() == 2);
    const auto ptrType = as<IRPtrTypeBase>(func->getParamType(0));
    SLANG_ASSERT(ptrType);
    const auto structParamType = ptrType->getValueType();
    const auto structType = as<IRStructType>(maybeUnwrapSpecialize(structParamType));
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
    const auto bottomOfFieldValue = builder.getIntValue(builder.getIntType(), bottomOfField);
    const auto maskBits = setLowBits(fieldWidth) << bottomOfField;
    const auto mask = builder.getIntValue(backingType, maskBits);
    const auto notMask = builder.getIntValue(backingType, ~maskBits);
    const auto memberAddr = builder.emitFieldAddress(builder.getPtrType(backingType), s, dec->getBackingMemberKey());
    const auto backingValue = builder.emitLoad(memberAddr);
    const auto maskedOut = builder.emitBitAnd(backingType, backingValue, notMask);
    const auto castValue = builder.emitCast(backingType, v);
    const auto shiftedLeft = builder.emitShl(backingType, castValue, bottomOfFieldValue);
    const auto maskedValue = builder.emitBitAnd(backingType, shiftedLeft, mask);
    const auto combined = builder.emitBitOr(backingType, maskedOut, maskedValue);
    builder.emitStore(memberAddr, combined);
    builder.emitReturn();

    builder.addSimpleDecoration<IRForceInlineDecoration>(func);
}

void synthesizeBitFieldAccessors(IRModule* module)
{
    for(const auto inst : module->getModuleInst()->getGlobalInsts())
    {
        const auto func = as<IRFunc>(maybeUnwrapGeneric(inst));
        if(!func)
            continue;
        const auto bfd = func->findDecoration<IRBitFieldAccessorDecoration>();
        if(!bfd)
            continue;
        if(func->getParamCount() == 1)
            synthesizeBitFieldGetter(func, bfd);
        else
            synthesizeBitFieldSetter(func, bfd);
    }
}
}
