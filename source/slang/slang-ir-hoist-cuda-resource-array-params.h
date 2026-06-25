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
/// through `CUDASourceEmitter::emitParameterGroupImpl` (a `__constant__` pointer plus global
/// backing read via the read-only cache), which avoids that chain.
///
/// This pass only fires for compute entry points that actually contain such a fixed-size
/// resource array; other entry points are left untouched.
void hoistCUDAResourceArrayParamsToParameterGroup(IRModule* module);

} // namespace Slang
