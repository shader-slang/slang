// slang-ir-ray-tracing-legalize.h
#pragma once

#include "core/slang-basic.h"

namespace Slang
{

class TargetProgram;
struct IRModule;

/// Adapt ray/callable payloads at native target interfaces. Empty payloads get physical storage:
/// D3D/GLSL/SPIR-V use nonempty dummy structs; CUDA/OptiX needs none. Original empty types,
/// copies, and ordinary helper parameters remain unchanged for normal type legalization to erase.
/// On D3D, which accepts only struct payloads, a nonempty non-struct payload gets a one-field
/// wrapper struct: a receiving shader's parameter is retyped, and a dispatch or ordinary call
/// passes a copy-in/copy-out wrapper temporary. A non-struct D3D hit attribute instead gets a
/// one-field struct by value at ReportHit, which is also the closesthit/anyhit parameter's type.
/// Also normalize ray-payload access qualifiers.
/// Run after specialization and before type legalization.
void legalizeRayTracingPayloads(IRModule* module, TargetProgram* targetProgram);

} // namespace Slang
