// slang-ir-legalize-cuda-param-dynamic-index.h
#pragma once

namespace Slang
{

struct IRModule;

/// Legalize dynamic indexing into by-value kernel parameters on CUDA targets.
///
/// A CUDA `__global__` kernel receives its parameters in `.param` (kernel-argument)
/// space, which ptxas cannot address dynamically: a `GetElement` with a runtime index
/// whose base is (a projection of) a by-value kernel parameter is lowered to a serial
/// per-element load chain, costing O(N) dependent loads per access (see issue #11774).
///
/// This pass rewrites each such access to go through a function-local copy of the
/// parameter instead: it materializes one `AddressSpace::Function` variable per
/// affected parameter in the kernel prologue, stores the parameter into it once, and
/// rewrites the offending `FieldExtract`/`GetElement` projection chains into address
/// chains off that variable followed by a single load. Local memory is dynamically
/// addressable, so each access becomes one O(1) load after a pipelined one-time copy.
///
/// For example, for `uniform float weights[256]` indexed as `weights[indices[tid]]`,
/// the kernel body becomes (in emitted CUDA terms):
///
///     FixedArray<float, 256> _w = weights; // one-time prologue copy
///     ... _w[indices[tid]] ...             // O(1) dynamically addressed load
///
/// The transform changes neither the kernel signature nor any reflected layout, so it
/// is invisible to hosts; it is the CUDA analogue of the SPIR-V legalization in
/// `SPIRVLegalizationContext::processGetElement` (slang-ir-spirv-legalize.cpp), which
/// performs the same value-to-addressable-memory materialization because SPIR-V cannot
/// dynamically index values either. Statically-indexed parameters are left untouched.
void legalizeCUDAKernelParamDynamicIndex(IRModule* module);

} // namespace Slang
