#pragma once

#include "core/slang-list.h"

namespace Slang
{

struct IRModule;
struct IRFunc;

/// Generate native D3D/Vulkan entry-point adapters for stages selected by structural layouts.
void synthesizePortableStructuralRayTracingEntryPoints(
    IRModule* module,
    List<IRFunc*>& ioEntryPoints);

/// Lower structural stage-input operations through their portable standard-module bodies.
void lowerPortableStructuralRayTracingStageInputOperations(IRModule* module);

/// Lower structural trace operations through their portable standard-module bodies.
void lowerPortableStructuralRayTracingTraceOperations(IRModule* module);

} // namespace Slang
