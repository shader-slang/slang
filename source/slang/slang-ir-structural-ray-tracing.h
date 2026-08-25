#pragma once

#include "slang-ir.h"
#include "slang-structural-ray-tracing.h"

namespace Slang
{

struct IRModule;

struct StructuralRayTracingEntryPointIRInfo
{
    StructuralRayTracingStageKind stageKind = StructuralRayTracingStageKind::Count;
    IRInst* invoke = nullptr;
    IRType* stageType = nullptr;
    IRType* contextType = nullptr;
    IRType* payloadType = nullptr;
    IRType* recordType = nullptr;
    IRType* hitAttributesType = nullptr;
    IRType* callableDataType = nullptr;
    StructuralRayTracingHitAttributesKind hitAttributesKind =
        StructuralRayTracingHitAttributesKind::None;
};

IROp getStructuralRayTracingStageInterfaceOp(StructuralRayTracingStageKind kind);
IROp getStructuralRayTracingStageInputOperationOp(StructuralRayTracingStageInputOperationKind kind);
String getStructuralRayTracingSourceTypeName(IRType* type);
void addStructuralRayTracingEntryPointInfo(
    IRBuilder& builder,
    IRFunc* func,
    const StructuralRayTracingEntryPointIRInfo& info);

bool identifyStructuralRayTracingStageInterfaces(
    Module* module,
    const StructuralRayTracingDeclRegistry& registry,
    StructuralRayTracingStageKind* outMissingStage = nullptr);

} // namespace Slang
