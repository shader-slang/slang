#include "slang-ir-structural-ray-tracing.h"

#include "slang-ir-insts.h"

// Source annotations produce special AST types and operation metadata during ordinary checking.
// AST-to-IR lowering preserves those identities with distinct interface opcodes and decorations;
// module serialization, linking, and specialization then carry them without import-time fixups.

namespace Slang
{

IROp getStructuralRayTracingStageInterfaceOp(Stage kind)
{
    switch (kind)
    {
    case Stage::ClosestHit:
        return kIROp_ClosestHitStageInterface;
    case Stage::AnyHit:
        return kIROp_AnyHitStageInterface;
    case Stage::Intersection:
        return kIROp_IntersectionStageInterface;
    case Stage::Miss:
        return kIROp_MissStageInterface;
    case Stage::Callable:
        return kIROp_CallableStageInterface;
    default:
        return kIROp_Invalid;
    }
}

void addStructuralRayTracingEntryPointInfo(
    IRBuilder& builder,
    IRInst* entryPointValue,
    IRInst* stageWitness)
{
    SLANG_RELEASE_ASSERT(entryPointValue && stageWitness);
    SLANG_RELEASE_ASSERT(as<IRWitnessTableType>(stageWitness->getDataType()));
    builder.addDecoration(
        entryPointValue,
        kIROp_StructuralRayTracingEntryPointInfoDecoration,
        stageWitness);
}

void addStructuralRayTracingSourceOperation(
    IRBuilder& builder,
    IRFunc* func,
    StructuralRayTracingSourceOperationKind kind)
{
    SLANG_RELEASE_ASSERT(func && kind == StructuralRayTracingSourceOperationKind::CallShader);

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
