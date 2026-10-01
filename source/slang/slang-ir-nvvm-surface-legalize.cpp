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
};

// Resolves the producer-owned field key of a collected global parameter. Consider this example:
//
//     [format("rgba16f")] RWTexture2D<float4> image;
//     float4 value = image[p];
//
// Global collection preserves the format on the field key. The load at this access still names
// that key, so legalization can choose half4 storage without inspecting CUDA source strings or
// guessing a format from callers of an arbitrary user helper.
IRStructField* findSurfaceField(IRInst* resource)
{
    auto load = as<IRLoad>(resource);
    auto address = load ? as<IRFieldAddress>(load->getPtr()) : nullptr;
    auto global = address ? as<IRGlobalParam>(address->getBase()) : nullptr;
    auto buffer = global ? as<IRConstantBufferType>(global->getDataType()) : nullptr;
    auto record = buffer ? as<IRStructType>(buffer->getElementType()) : nullptr;
    if (!record || !record->findDecoration<IRSynthesizedParameterGroupDecoration>())
        return nullptr;
    auto field = findStructField(record, cast<IRStructKey>(address->getField()));
    return field && isTypeEqual(field->getFieldType(), resource->getDataType()) ? field : nullptr;
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
    if (!resource || !coordinate ||
        !getNVVMSupportedSurfaceField(findSurfaceField(resource), surfaceType, physicalType) ||
        (physicalType.laneCount != 1 && physicalType.laneCount != 2 && physicalType.laneCount != 4))
        return false;

    IRType* coordinateScalar = getIRVectorBaseType(coordinate->getDataType());
    if (!isNVVMInteger32Type(coordinateScalar) ||
        UInt(getIRVectorElementSize(coordinate->getDataType())) != surfaceType.coordinateLaneCount)
        return false;

    IRType* logicalType = surfaceType.textureType->getElementType();
    IRType* physicalIRType = logicalType;
    if (physicalType.bitWidth != surfaceType.elementType.bitWidth)
    {
        SLANG_RELEASE_ASSERT(
            physicalType.bitWidth == 16 &&
            physicalType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT);
        physicalIRType = builder.getHalfType();
        if (physicalType.laneCount != 1)
            physicalIRType = builder.getVectorType(builder.getHalfType(), physicalType.laneCount);
    }
    outAccess = {
        resource,
        coordinate,
        logicalType,
        physicalIRType,
        physicalType.laneCount * physicalType.bitWidth / 8,
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

// Leaves matching representations untouched and expresses the admitted Half/Float32 conversion
// as ordinary value IR. The provider only sees the physical type selected by the surface access.
IRInst* emitSurfaceConversion(IRBuilder& builder, IRType* type, IRInst* value)
{
    if (isTypeEqual(type, value->getDataType()))
        return value;
    return builder.emitIntrinsicInst(type, kIROp_FloatCast, 1, &value);
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
            emitSurfaceConversion(builder, access.physicalType, value));
    }
    else
    {
        IRInst* physical = emitPhysicalLoad(builder, access, physicalCoordinate);
        inst->replaceUsesWith(emitSurfaceConversion(builder, access.logicalType, physical));
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
    IRInst* replacement = emitSurfaceConversion(builder, replacementType, value);
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
