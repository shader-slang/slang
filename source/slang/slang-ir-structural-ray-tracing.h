#pragma once

#include "slang-ir.h"
#include "slang-structural-ray-tracing.h"

namespace Slang
{

struct IRModule;

IROp getStructuralRayTracingStageInterfaceOp(StructuralRayTracingStageKind kind);
IROp getStructuralRayTracingStageInputOperationOp(StructuralRayTracingStageInputOperationKind kind);

bool identifyStructuralRayTracingStageInterfaces(
    Module* module,
    const StructuralRayTracingDeclRegistry& registry,
    StructuralRayTracingStageKind* outMissingStage = nullptr);

} // namespace Slang
