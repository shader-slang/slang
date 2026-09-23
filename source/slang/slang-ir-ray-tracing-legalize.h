// slang-ir-ray-tracing-legalize.h
#pragma once

namespace Slang
{

class TargetProgram;
struct IRModule;

/// Legalize ray-tracing payload types before general type legalization.
///
/// Zero-field structs normally legalize to `none`, but some ray-tracing ABIs require a physical
/// ray-payload or callable-data object to remain in the generated program. This pass applies the
/// target's complete pre-type-legalization policy: it resolves D3D forced-payload markers,
/// materializes required D3D and Khronos payload carriers, and normalizes D3D SM 6.7 payload access
/// qualifiers. Targets whose ABI permits an empty payload, such as CUDA/OptiX, need no
/// materialization and are intentionally left to ordinary empty-type legalization. Structs that
/// only become empty through legalization of their fields require a separate legalization design.
void legalizeRayTracingPayloads(IRModule* module, TargetProgram* targetProgram);

} // namespace Slang
