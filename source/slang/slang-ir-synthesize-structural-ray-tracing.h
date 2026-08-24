#pragma once

namespace Slang
{

struct IRModule;

/// Lower structural stage-input operations through their portable standard-module bodies.
void lowerPortableStructuralRayTracingStageInputOperations(IRModule* module);

} // namespace Slang
