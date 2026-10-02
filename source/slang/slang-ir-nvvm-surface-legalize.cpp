#include "slang-ir-nvvm-surface-legalize.h"

#include "slang-emit-nvvm-type-lowering.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
namespace
{

struct SurfaceAccess
{
    IRInst* surface = nullptr;
    IRInst* coordinate = nullptr;
    IRType* logicalType = nullptr;
    IRType* physicalType = nullptr;
    UInt texelBytes = 0;
    NVVMSurfaceNormalization normalization = NVVMSurfaceNormalization::None;
};

// Resolves only typed aggregate selectors rooted in a launch binding. Consider an entry parameter
// `Wrapper image` with a field `RWTexture2D<float4> value`: inlining produces
// `get_field(image, value)`, and the field key still owns the format declaration. Follow that
// canonical selector path, checking each field/array type, until its entry or collected-global
// root. Calls, pointer arithmetic and arbitrary helper parameters establish no binding contract.
bool getSurfaceFormat(IRInst* resource, NVVMSurfaceType& type, SlangNVVMValueTypeDesc& physical)
{
    IRFormatDecoration* format = nullptr;
    IRInst* root = resource;
    bool followsAddress = false;
    while (root)
    {
        if (auto param = as<IRParam>(root))
        {
            auto block = as<IRBlock>(param->getParent());
            auto function = block ? as<IRFunc>(block->getParent()) : nullptr;
            if (followsAddress || !function || function->getFirstBlock() != block ||
                !function->findDecoration<IREntryPointDecoration>())
                return false;
            if (root == resource)
                format = param->findDecoration<IRFormatDecoration>();
            return getNVVMSupportedSurfaceFormat(resource->getDataType(), format, type, physical);
        }
        if (auto global = as<IRGlobalParam>(root))
        {
            auto buffer = as<IRConstantBufferType>(global->getDataType());
            auto record = buffer ? as<IRStructType>(buffer->getElementType()) : nullptr;
            if (!record || !record->findDecoration<IRSynthesizedParameterGroupDecoration>())
                return false;
            return getNVVMSupportedSurfaceFormat(resource->getDataType(), format, type, physical);
        }
        if (auto load = as<IRLoad>(root))
        {
            auto pointer = as<IRPtrTypeBase>(load->getPtr()->getDataType());
            if (!pointer || !isTypeEqual(pointer->getValueType(), root->getDataType()))
                return false;
            followsAddress = true;
            root = load->getPtr();
            continue;
        }
        bool address = root->getOp() == kIROp_FieldAddress || root->getOp() == kIROp_GetElementPtr;
        if (address || root->getOp() == kIROp_FieldExtract || root->getOp() == kIROp_GetElement)
        {
            IRInst* base = root->getOperand(0);
            IRType* baseType = base->getDataType();
            IRType* selectedType = root->getDataType();
            if (address)
            {
                followsAddress = true;
                auto pointer = as<IRPtrTypeBase>(baseType);
                auto buffer = as<IRConstantBufferType>(baseType);
                auto result = as<IRPtrTypeBase>(selectedType);
                if ((!pointer && !buffer) || !result)
                    return false;
                baseType = pointer ? pointer->getValueType() : buffer->getElementType();
                selectedType = result->getValueType();
            }
            if (root->getOp() == kIROp_FieldExtract || root->getOp() == kIROp_FieldAddress)
            {
                auto record = as<IRStructType>(baseType);
                auto key = as<IRStructKey>(root->getOperand(1));
                auto field = record && key ? findStructField(record, key) : nullptr;
                if (!field || !isTypeEqual(field->getFieldType(), selectedType))
                    return false;
                if (!format)
                    format = key->findDecoration<IRFormatDecoration>();
            }
            else
            {
                auto array = as<IRArrayType>(baseType);
                if (!array || !isTypeEqual(array->getElementType(), selectedType))
                    return false;
            }
            root = base;
            continue;
        }
        if (root == resource && root->getOp() == kIROp_CastDescriptorHandleToResource &&
            root->getOperandCount() == 1)
        {
            IRType* resourceType = nullptr;
            if (asNVVMSupportedDescriptorHandleType(
                    root->getOperand(0)->getDataType(),
                    &resourceType) &&
                isTypeEqual(resourceType, resource->getDataType()))
                return getNVVMSupportedSurfaceFormat(resourceType, nullptr, type, physical);
        }
        return false;
    }
    return false;
}

// Builds the one physical contract used for reads, writes, and masked updates. Unsupported static
// formats and resource provenance remain for NVVM preflight to reject; no runtime inference occurs.
bool getSurfaceAccess(
    IRBuilder& builder,
    IRInst* resource,
    IRInst* coordinate,
    SurfaceAccess& outAccess)
{
    NVVMSurfaceType surfaceType;
    SlangNVVMValueTypeDesc physicalType = {};
    if (!resource || !coordinate || !getSurfaceFormat(resource, surfaceType, physicalType) ||
        (physicalType.laneCount != 1 && physicalType.laneCount != 2 && physicalType.laneCount != 4))
        return false;

    IRType* coordinateScalar = getIRVectorBaseType(coordinate->getDataType());
    if (!isNVVMInteger32Type(coordinateScalar) ||
        UInt(getIRVectorElementSize(coordinate->getDataType())) != surfaceType.coordinateLaneCount)
        return false;

    IRType* logicalType = surfaceType.textureType->getElementType();
    IRType* physicalIRType = logicalType;
    if (physicalType.bitWidth != surfaceType.elementType.bitWidth ||
        physicalType.kind != surfaceType.elementType.kind)
    {
        if (physicalType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT)
        {
            SLANG_RELEASE_ASSERT(physicalType.bitWidth == 16);
            physicalIRType = builder.getHalfType();
        }
        else
        {
            SLANG_RELEASE_ASSERT(physicalType.bitWidth == 8 || physicalType.bitWidth == 16);
            const bool isSigned = physicalType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER;
            physicalIRType = builder.getBasicType(
                physicalType.bitWidth == 8 ? (isSigned ? BaseType::Int8 : BaseType::UInt8)
                                           : (isSigned ? BaseType::Int16 : BaseType::UInt16));
        }
        if (physicalType.laneCount != 1)
            physicalIRType = builder.getVectorType(physicalIRType, physicalType.laneCount);
    }
    outAccess = {
        resource,
        coordinate,
        logicalType,
        physicalIRType,
        physicalType.laneCount * physicalType.bitWidth / 8,
        surfaceType.normalization,
    };
    return true;
}

// Converts only the X coordinate to bytes. Remaining dimensions, including an array layer, keep
// their existing units. i32 arithmetic deliberately preserves the previous provider's wrapping.
IRInst* emitPhysicalCoordinate(IRBuilder& builder, const SurfaceAccess& access)
{
    const auto laneCount = getIRVectorElementSize(access.coordinate->getDataType());
    IRType* coordinateType = builder.getIntType();
    if (laneCount != 1)
        coordinateType = builder.getVectorType(builder.getIntType(), laneCount);
    IRInst* coordinate = builder.emitCast(coordinateType, access.coordinate);
    IRInst* x =
        laneCount == 1 ? coordinate : builder.emitGetElement(builder.getIntType(), coordinate, 0);
    IRInst* scale = builder.getIntValue(builder.getIntType(), access.texelBytes);
    IRInst* operands[] = {x, scale};
    IRInst* byteX = builder.emitIntrinsicInst(builder.getIntType(), kIROp_Mul, 2, operands);
    if (laneCount == 1)
        return byteX;
    IRInst* index = builder.getIntValue(builder.getIntType(), 0);
    return builder.emitSwizzleSet(coordinateType, coordinate, byteX, 1, &index);
}

// Converts only the selected value. Integer stores clamp in the logical 32-bit type before
// narrowing; for example, storing 256 into r8ui writes 255, rather than wrapping to zero.
// Loads extend according to the physical integer signedness. Half conversion remains FloatCast.
IRInst* emitSurfaceConversion(IRBuilder& builder, IRType* type, IRInst* value)
{
    if (isTypeEqual(type, value->getDataType()))
        return value;
    uint32_t width = 0;
    bool isSigned = false;
    if (isNVVMSupportedIntegerScalarType(getIRVectorBaseType(type), &width, &isSigned))
    {
        if (width < 32)
        {
            IRType* sourceType = value->getDataType();
            IRType* sourceScalar = getIRVectorBaseType(sourceType);
            IRType* conditionType = builder.getBoolType();
            const IRIntegerValue maximum = (IRIntegerValue(1) << (width - (isSigned ? 1 : 0))) - 1;
            IRInst* upper = builder.getIntValue(sourceScalar, maximum);
            if (as<IRVectorType>(sourceType))
            {
                upper = builder.emitMakeVectorFromScalar(sourceType, upper);
                conditionType =
                    builder.getVectorType(conditionType, getIRVectorElementSize(sourceType));
            }
            IRInst* upperComparison[] = {upper, value};
            IRInst* upperOperands[] = {
                builder.emitIntrinsicInst(conditionType, kIROp_Less, 2, upperComparison),
                upper,
                value};
            value = builder.emitIntrinsicInst(sourceType, kIROp_Select, 3, upperOperands);
            if (isSigned)
            {
                IRInst* lower = builder.getIntValue(sourceScalar, -maximum - 1);
                if (as<IRVectorType>(sourceType))
                    lower = builder.emitMakeVectorFromScalar(sourceType, lower);
                IRInst* lowerComparison[] = {value, lower};
                IRInst* lowerOperands[] = {
                    builder.emitIntrinsicInst(conditionType, kIROp_Less, 2, lowerComparison),
                    lower,
                    value};
                value = builder.emitIntrinsicInst(sourceType, kIROp_Select, 3, lowerOperands);
            }
        }
        return builder.emitIntrinsicInst(type, kIROp_IntCast, 1, &value);
    }
    return builder.emitIntrinsicInst(type, kIROp_FloatCast, 1, &value);
}

// Converts one normalized channel with a bounded Float32 calculation. Half values widen before
// quantization. Rounding uses the fractional remainder instead of adding 0.5: for example, adding
// 0.5 to the Float32 immediately below 0.5 can itself round to 1 and choose the wrong integer.
IRInst* emitNormalizedChannel(
    IRBuilder& builder,
    NVVMSurfaceNormalization normalization,
    IRType* physicalType,
    IRType* resultType,
    IRInst* value,
    bool isStore)
{
    uint32_t width = 0;
    bool isSigned = false;
    SLANG_RELEASE_ASSERT(isNVVMSupportedIntegerScalarType(physicalType, &width, &isSigned));
    SLANG_RELEASE_ASSERT(isSigned == (normalization == NVVMSurfaceNormalization::SNorm));
    IRType* floatType = builder.getFloatType();
    IRType* intType = builder.getIntType();
    IRInst* zero = builder.getFloatValue(floatType, 0);
    IRInst* one = builder.getFloatValue(floatType, 1);
    IRInst* lower = builder.getFloatValue(floatType, isSigned ? -1 : 0);
    IRInst* scale = builder.getFloatValue(floatType, (UInt(1) << (width - isSigned)) - 1);
    value = builder.emitCast(floatType, value);
    if (!isStore)
    {
        value = builder.emitDiv(floatType, value, scale);
        // The most negative signed code has no distinct normalized value: both it and -max
        // decode to -1. Unselected channels never take this conversion during partial stores.
        IRInst* clamp[] = {builder.emitLess(value, lower), lower, value};
        value = builder.emitIntrinsicInst(floatType, kIROp_Select, 3, clamp);
        return builder.emitCast(resultType, value);
    }
    IRInst* finite[] = {builder.emitNeq(value, value), zero, value};
    value = builder.emitIntrinsicInst(floatType, kIROp_Select, 3, finite);
    IRInst* clampLower[] = {builder.emitLess(value, lower), lower, value};
    value = builder.emitIntrinsicInst(floatType, kIROp_Select, 3, clampLower);
    IRInst* clampUpper[] = {builder.emitLess(one, value), one, value};
    value = builder.emitIntrinsicInst(floatType, kIROp_Select, 3, clampUpper);
    value = builder.emitMul(floatType, value, scale);
    IRInst* truncated = builder.emitCast(intType, value);
    IRInst* residual = builder.emitSub(floatType, value, builder.emitCast(floatType, truncated));
    IRInst* intZero = builder.getIntValue(intType, 0);
    IRInst* increment[] = {
        builder.emitGeq(residual, builder.getFloatValue(floatType, 0.5)),
        builder.getIntValue(intType, 1),
        intZero};
    IRInst* adjustment = builder.emitIntrinsicInst(intType, kIROp_Select, 3, increment);
    if (isSigned)
    {
        IRInst* decrement[] = {
            builder.emitGeq(builder.getFloatValue(floatType, -0.5), residual),
            builder.getIntValue(intType, -1),
            adjustment};
        adjustment = builder.emitIntrinsicInst(intType, kIROp_Select, 3, decrement);
    }
    return builder.emitCast(resultType, builder.emitAdd(intType, truncated, adjustment));
}

// Executes the selected format conversion for the whole texel or just the written channels.
IRInst* emitSelectedSurfaceConversion(
    IRBuilder& builder,
    const SurfaceAccess& access,
    IRType* type,
    IRInst* value,
    bool isStore)
{
    if (access.normalization == NVVMSurfaceNormalization::None)
        return emitSurfaceConversion(builder, type, value);
    IRType* physicalScalar = getIRVectorBaseType(access.physicalType);
    if (!as<IRVectorType>(type))
        return emitNormalizedChannel(
            builder,
            access.normalization,
            physicalScalar,
            type,
            value,
            isStore);
    List<IRInst*> lanes;
    for (IRIntegerValue lane = 0; lane < getIRVectorElementSize(type); ++lane)
        lanes.add(emitNormalizedChannel(
            builder,
            access.normalization,
            physicalScalar,
            getIRVectorBaseType(type),
            builder.emitElementExtract(value, lane),
            isStore));
    return builder.emitMakeVector(type, lanes.getCount(), lanes.getBuffer());
}

IRInst* emitPhysicalLoad(IRBuilder& builder, const SurfaceAccess& access, IRInst* coordinate)
{
    IRInst* operands[] = {access.surface, coordinate};
    return builder.emitIntrinsicInst(access.physicalType, kIROp_NVVMSurfaceLoad, 2, operands);
}

void emitPhysicalStore(
    IRBuilder& builder,
    const SurfaceAccess& access,
    IRInst* coordinate,
    IRInst* value)
{
    IRInst* operands[] = {access.surface, coordinate, value};
    builder.emitIntrinsicInst(builder.getVoidType(), kIROp_NVVMSurfaceStore, 3, operands);
}

// Rewrites a complete logical texel read or write at its use site, where the static format is
// known. Two calls of the same intrinsic may therefore access differently formatted surfaces.
bool legalizeWholeAccess(
    IRBuilder& builder,
    IRInst* inst,
    IRInst* resource,
    IRInst* coordinate,
    IRInst* value)
{
    SurfaceAccess access;
    if (!getSurfaceAccess(builder, resource, coordinate, access) ||
        (value ? !isTypeEqual(value->getDataType(), access.logicalType)
               : !isTypeEqual(inst->getDataType(), access.logicalType)))
        return false;
    builder.setInsertBefore(inst);
    IRInst* physicalCoordinate = emitPhysicalCoordinate(builder, access);
    if (value)
    {
        emitPhysicalStore(
            builder,
            access,
            physicalCoordinate,
            emitSelectedSurfaceConversion(builder, access, access.physicalType, value, true));
    }
    else
    {
        IRInst* physical = emitPhysicalLoad(builder, access, physicalCoordinate);
        inst->replaceUsesWith(
            emitSelectedSurfaceConversion(builder, access, access.logicalType, physical, false));
    }
    inst->removeAndDeallocate();
    return true;
}

// Preserves a component write in physical storage. For `image[p].xz = value`, untouched Half
// channels are copied directly from the physical load; widening and narrowing them would lose
// representations such as NaN payloads. A statically selected scalar component uses the same merge
// operation. For `image[p][lane] = value`, a dynamic in-range lane selects the converted
// replacement against each old physical channel without converting the untouched channels.
bool legalizeComponentStore(IRBuilder& builder, IRInst* inst)
{
    IRInst* address = inst->getOperand(0);
    auto element = as<IRGetElementPtr>(address);
    auto subscript = as<IRImageSubscript>(element ? element->getBase() : address);
    if (!subscript || subscript->hasSampleCoord())
        return false;
    SurfaceAccess access;
    if (!getSurfaceAccess(builder, subscript->getImage(), subscript->getCoord(), access))
        return false;
    IRInst* value = inst->getOperand(1);
    List<IRInst*> indices;
    IRInst* dynamicIndex = nullptr;
    if (auto swizzle = as<IRSwizzledStore>(inst))
    {
        if (element)
            return false;
        for (UInt i = 0; i < swizzle->getElementCount(); ++i)
        {
            auto index = as<IRIntLit>(swizzle->getElementIndex(i));
            if (!index || index->getValue() < 0 ||
                index->getValue() >= getIRVectorElementSize(access.logicalType))
                return false;
            indices.add(index);
        }
    }
    else if (element)
    {
        IRInst* index = element->getIndex();
        if (!isNVVMInteger32Type(index->getDataType()))
            return false;
        if (auto literal = as<IRIntLit>(index))
        {
            if (literal->getValue() < 0 ||
                literal->getValue() >= getIRVectorElementSize(access.logicalType))
                return false;
        }
        else
            dynamicIndex = index;
        indices.add(index);
    }
    else
        return legalizeWholeAccess(builder, inst, access.surface, access.coordinate, value);

    if (!as<IRVectorType>(access.logicalType) || !indices.getCount() ||
        indices.getCount() != getIRVectorElementSize(value->getDataType()) ||
        !isTypeEqual(
            getIRVectorBaseType(value->getDataType()),
            getIRVectorBaseType(access.logicalType)))
        return false;
    IRType* physicalScalar = getIRVectorBaseType(access.physicalType);
    IRType* replacementType = indices.getCount() == 1
                                  ? physicalScalar
                                  : builder.getVectorType(physicalScalar, indices.getCount());
    builder.setInsertBefore(inst);
    IRBuilderSourceLocRAII sourceLoc(&builder, inst->sourceLoc);
    IRInst* coordinate = emitPhysicalCoordinate(builder, access);
    IRInst* oldValue = emitPhysicalLoad(builder, access, coordinate);
    IRInst* replacement =
        emitSelectedSurfaceConversion(builder, access, replacementType, value, true);
    IRInst* merged = nullptr;
    if (dynamicIndex)
    {
        List<IRInst*> lanes;
        const auto laneCount = getIRVectorElementSize(access.physicalType);
        for (IRIntegerValue lane = 0; lane < laneCount; ++lane)
        {
            IRInst* oldLane = builder.emitElementExtract(oldValue, lane);
            IRInst* selected = builder.emitEql(
                dynamicIndex,
                builder.getIntValue(dynamicIndex->getDataType(), lane));
            IRInst* operands[] = {selected, replacement, oldLane};
            lanes.add(builder.emitIntrinsicInst(physicalScalar, kIROp_Select, 3, operands));
        }
        merged = builder.emitMakeVector(access.physicalType, lanes.getCount(), lanes.getBuffer());
    }
    else
        merged = builder.emitSwizzleSet(
            access.physicalType,
            oldValue,
            replacement,
            indices.getCount(),
            indices.getBuffer());
    emitPhysicalStore(builder, access, coordinate, merged);
    inst->removeAndDeallocate();
    return true;
}

} // namespace

void legalizeNVVMSurfaceOperations(IRModule* module)
{
    IRBuilder builder(module);
    for (auto global : module->getGlobalInsts())
    {
        auto function = as<IRFunc>(global);
        if (!function)
            continue;
        for (auto block : function->getBlocks())
        {
            for (IRInst* inst = block->getFirstOrdinaryInst(); inst;)
            {
                IRInst* next = inst->getNextInst();
                switch (inst->getOp())
                {
                case kIROp_ImageLoad:
                case kIROp_ImageStore:
                    {
                        bool isLoad = inst->getOp() == kIROp_ImageLoad;
                        if (inst->getOperandCount() == (isLoad ? 2u : 3u))
                            legalizeWholeAccess(
                                builder,
                                inst,
                                inst->getOperand(0),
                                inst->getOperand(1),
                                isLoad ? nullptr : inst->getOperand(2));
                    }
                    break;
                case kIROp_Store:
                case kIROp_SwizzledStore:
                    legalizeComponentStore(builder, inst);
                    break;
                default:
                    break;
                }
                inst = next;
            }
        }
    }
}
} // namespace Slang
