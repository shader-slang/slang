// slang-ir-hoist-cuda-resource-array-params.h
#pragma once

namespace Slang
{
struct IRModule;

/// CUDA-only legalization.
///
/// A compute entry point whose uniform parameters transitively contain a fixed-size array of
/// resources (e.g. `RWStructuredBuffer<T> t[N]`) indexed by a runtime value lowers, by default,
/// to a serial dynamic-address `ld.param` chain in the kernel `.param` bank. Hoisting those
/// uniform parameters into a module-scope `ConstantBuffer<GlobalParams>` instead routes them
/// through `CUDASourceEmitter::emitParameterGroupImpl`, which emits the `GlobalParams` object
/// itself in CUDA `__constant__` memory (`extern "C" __constant__ ... SLANG_globalParams`). The
/// hoisted uniforms — including the resource-array descriptors — are then read from constant
/// memory rather than the per-thread `.param` bank, avoiding the serial dynamic-address `ld.param`
/// chain.
///
/// This pass only fires for compute entry points that actually contain such a fixed-size
/// resource array; other entry points are left untouched. Because CUDA emits a single hardcoded
/// `SLANG_globalParams` symbol per module — shared with `collectGlobalUniformParameters`, which
/// synthesizes the same global for module-scope uniforms / user `cbuffer`s — this pass hoists
/// nothing unless *exactly one* compute entry point qualifies and no module-scope uniform parameter
/// group global already exists. A host therefore sees `SLANG_globalParams` whenever *either* this
/// pass or `collectGlobalUniformParameters` fires, not only from this transform.
///
/// @note This transformation runs after `ProgramLayout` is finalized, so reflection continues to
/// report the original per-parameter entry-point layout, not the synthesized
/// `ConstantBuffer<GlobalParams>` wrapper. A host driving CUDA binding from reflection must route
/// the entry-point uniform data to the `SLANG_globalParams` symbol when this pass fires (this is
/// why the paired slang-rhi dispatch change exists).
///
/// @note The pass empties the entry point's uniform parameter list but deliberately leaves the
/// function's own `IREntryPointLayout` decoration in place (the stale layout is what preserves the
/// reflection behavior described above). The one place in the compiler that positionally zips an
/// entry point's `IRParam`s against `getScopeStructLayout(entryPointLayout)` is
/// `maybeCopyLayoutInformationToParameters` (slang-ir-link.cpp), which runs during linking — i.e.
/// before `finalizeSpecialization`, and therefore before this pass — so the zip is already complete
/// when the parameters are removed and cannot be re-tripped by the now-shorter list. (That zipper's
/// only hard failure, `SLANG_UNEXPECTED("too many parameters")`, fires when parameters *outnumber*
/// layout fields, the opposite of what removing parameters can produce.) Unlike the reference pass
/// `moveEntryPointUniformParamsToGlobalScope`, this one does not rebuild the entry-point layout:
/// its hoist target is reflection-visible and so must stay consistent, whereas here the original
/// layout is intentionally retained for CUDA reflection.
void hoistCUDAResourceArrayParamsToParameterGroup(IRModule* module);

} // namespace Slang
