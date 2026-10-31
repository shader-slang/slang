// slang-ir-undefined-opaque-handle.h
#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;

/// Remove every operation that reads or writes through an undefined opaque handle, so that no
/// `LoadFromUninitializedMemory` or `Poison` of an image, sampler or acceleration-structure type
/// needs to be emitted as SPIR-V.
///
/// The pass expects SSA form and runs after intrinsic functions have been inlined into `SPIRVAsm`
/// blocks, because it inlines calls that pass an undefined handle.
void eliminateUsesOfUndefinedOpaqueHandles(IRModule* module);

/// Report an undefined opaque handle that remains in `module` as E56003, because SPIR-V for Vulkan
/// has no valid `OpUndef` of such a type. E56003 is fatal, so compilation stops at the first one.
void diagnoseUndefinedOpaqueHandles(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
