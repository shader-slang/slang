// slang-ir-ray-tracing-legalize.h
#pragma once

namespace Slang
{

class TargetProgram;
struct IRModule;

/// Legalize ray-tracing payload types before general type legalization.
///
/// Structs with no nonempty data normally legalize to `none`, but some ray-tracing ABIs require a
/// physical ray-payload or callable-data object to remain in the generated program. This pass
/// applies the target's pre-type-legalization policy: it identifies D3D ray-payload types from
/// forced-payload markers and entry-point signatures, materializes required D3D and Khronos payload
/// carriers, and normalizes D3D SM 6.7 payload access qualifiers. Targets whose ABI permits an
/// empty payload, such as CUDA/OptiX, need no materialization and are intentionally left to
/// ordinary empty-type legalization. Empty structs are recognized recursively through struct fields
/// and array elements, but padding is added only to the selected payload struct; its original empty
/// fields can still be erased.
void legalizeRayTracingPayloads(IRModule* module, TargetProgram* targetProgram);

} // namespace Slang
