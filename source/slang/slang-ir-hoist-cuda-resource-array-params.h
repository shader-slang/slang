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
/// through `CUDASourceEmitter::emitParameterGroupImpl`, which emits a pointer in `__constant__`
/// memory (`SLANG_globalParams`) to a `.global` backing buffer; the backing data is then read via
/// the read-only data cache (`ld.global.nc`), avoiding the dynamic-address `.param` chain.
///
/// This pass only fires for compute entry points that actually contain such a fixed-size
/// resource array; other entry points are left untouched.
///
/// @note This transformation runs after `ProgramLayout` is finalized, so reflection continues to
/// report the original per-parameter entry-point layout, not the synthesized
/// `ConstantBuffer<GlobalParams>` wrapper. A host driving CUDA binding from reflection must route
/// the entry-point uniform data to the `SLANG_globalParams` symbol when this pass fires (this is
/// why the paired slang-rhi dispatch change exists).
void hoistCUDAResourceArrayParamsToParameterGroup(IRModule* module);

} // namespace Slang
