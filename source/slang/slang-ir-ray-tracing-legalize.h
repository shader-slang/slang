// slang-ir-ray-tracing-legalize.h
#pragma once

#include "core/slang-basic.h"

namespace Slang
{

class TargetProgram;
struct IRModule;

/// Give empty ray/callable payloads physical storage only at native target interfaces.
/// Run after specialization and before type legalization. D3D/GLSL/SPIR-V use nonempty dummy
/// structs; CUDA/OptiX needs none. Original empty types, copies, and ordinary helper parameters
/// remain unchanged for normal type legalization to erase. Also resolve D3D struct-only argument
/// markers and normalize ray-payload access qualifiers.
void legalizeRayTracingPayloads(IRModule* module, TargetProgram* targetProgram);

} // namespace Slang
