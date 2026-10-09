#include "slang-ir-structural-ray-tracing.h"

#include "slang-ir-insts.h"

// Source annotations produce special AST types and operation metadata during ordinary checking.
// AST-to-IR lowering preserves those identities with distinct interface opcodes and decorations;
// module serialization, linking, and specialization then carry them without import-time fixups.

namespace Slang
{

IROp getStructuralRayTracingStageInterfaceOp(StructuralRayTracingStageKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
        return kIROp_ClosestHitStageInterface;
    case StructuralRayTracingStageKind::AnyHit:
        return kIROp_AnyHitStageInterface;
    case StructuralRayTracingStageKind::Intersection:
        return kIROp_IntersectionStageInterface;
    case StructuralRayTracingStageKind::Miss:
        return kIROp_MissStageInterface;
    case StructuralRayTracingStageKind::Callable:
        return kIROp_CallableStageInterface;
    default:
        return kIROp_Invalid;
    }
}

bool isCompilerOwnedStructuralRayTracingIROp(IROp op)
{
    if (op >= kIROp_FirstRaytracingStageInterface && op <= kIROp_LastRaytracingStageInterface)
        return true;
    return op == kIROp_StructuralRayTracingEntryPointInfoDecoration ||
           op == kIROp_StructuralRayTracingSourceOperationDecoration;
}

void addStructuralRayTracingEntryPointInfo(
    IRBuilder& builder,
    IRInst* entryPointValue,
    const StructuralRayTracingEntryPointIRInfo& info)
{
    SLANG_RELEASE_ASSERT(
        entryPointValue && info.stageType && info.stageSourceTypeName && info.stageTypeIdentity);

    // `void` is the canonical absent value for stage-specific types. Keeping a fixed operand list
    // makes this decoration stable across stage kinds and module serialization.
    auto voidType = builder.getVoidType();
    IRInst* operands[] = {
        builder.getIntValue(builder.getIntType(), IRIntegerValue(info.stageKind)),
        info.stageType,
        info.stageSourceTypeName,
        info.stageTypeIdentity,
        info.contextType ? info.contextType : voidType,
        info.payloadType ? info.payloadType : voidType,
        info.recordType ? info.recordType : voidType,
        info.hitAttributesType ? info.hitAttributesType : voidType,
        info.callableDataType ? info.callableDataType : voidType,
        builder.getIntValue(builder.getIntType(), IRIntegerValue(info.hitAttributesKind)),
    };
    builder.addDecoration(
        entryPointValue,
        kIROp_StructuralRayTracingEntryPointInfoDecoration,
        operands,
        SLANG_COUNT_OF(operands));
}

void addStructuralRayTracingSourceOperation(
    IRBuilder& builder,
    IRFunc* func,
    StructuralRayTracingSourceOperationKind kind)
{
    SLANG_RELEASE_ASSERT(
        func && kind >= StructuralRayTracingSourceOperationKind::TraceExplicitPayload &&
        kind < StructuralRayTracingSourceOperationKind::Count);

    if (auto existing = func->findDecoration<IRStructuralRayTracingSourceOperationDecoration>())
    {
        auto existingKind = as<IRIntLit>(existing->getOperationKind());
        SLANG_RELEASE_ASSERT(existingKind && existingKind->getValue() == IRIntegerValue(kind));
        return;
    }

    builder.addDecoration(
        func,
        kIROp_StructuralRayTracingSourceOperationDecoration,
        builder.getIntValue(builder.getIntType(), IRIntegerValue(kind)));
}

} // namespace Slang
