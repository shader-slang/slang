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
/// target's pre-type-legalization policy: it resolves D3D forced-payload markers,
/// materializes required D3D and Khronos payload carriers, and normalizes D3D SM 6.7 payload access
/// qualifiers. Targets whose ABI permits an empty payload, such as CUDA/OptiX, need no
/// materialization and are intentionally left to ordinary empty-type legalization. Known gap:
/// structs whose fields all legalize to `none` are not materialized by the current zero-field
/// check, so required D3D/Khronos payload carriers can still be erased.
void legalizeRayTracingPayloads(IRModule* module, TargetProgram* targetProgram);

} // namespace Slang
