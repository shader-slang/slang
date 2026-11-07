// slang-ir-eliminate-undefined-opaque-handles.h
#pragma once

namespace Slang
{
struct CodeGenContext;
struct IRModule;
class DiagnosticSink;

/// Eliminate every undefined opaque handle in `module`: each `LoadFromUninitializedMemory` or
/// `Poison` of an image, sampler or acceleration-structure type, so that none of them needs to be
/// emitted as an `OpUndef` that spirv-val rejects for Vulkan.
///
/// Each call that passes such a handle, or a combined texture-sampler that includes one, is
/// redirected to a clone of its callee that is specialized to an undefined handle. Each operation
/// that accesses a resource through an undefined handle is then removed, and its result is replaced
/// by a fresh undefined value of the result type. A handle left without uses is removed by
/// dead-code elimination. A use that the pass does not recognize as an access is left in place, so
/// a handle that such a use keeps, for example in an array that also holds a defined handle, is
/// left for `diagnoseUndefinedOpaqueHandles`.
///
/// The pass expects SSA form, and runs after intrinsic functions have been inlined into
/// `SPIRVAsm` blocks, so that the accesses it must find are visible in the caller.
void eliminateUndefinedOpaqueHandles(IRModule* module, CodeGenContext* codeGenContext);

/// Report each undefined opaque handle that remains in `module` as a use of an uninitialized
/// opaque handle, because SPIR-V for Vulkan has no valid `OpUndef` of such a type.
void diagnoseUndefinedOpaqueHandles(IRModule* module, DiagnosticSink* sink);
} // namespace Slang
