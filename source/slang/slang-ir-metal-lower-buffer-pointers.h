#pragma once

namespace Slang
{

struct IRModule;

/// Lower pointer-typed fields in Metal buffer-bound structs to UIntPtr with
/// CastIntToPtr/CastPtrToInt at access sites.
///
/// Metal rejects pointer-to-pointer types (`device T* device*`) in buffer
/// pointee types. This pass handles two cases:
///
///   1. Struct element types used in ConstantBuffer, ParameterBlock, or
///      RWStructuredBuffer: multi-level pointer fields (T**, T***) are
///      lowered to UIntPtr.  Single-level pointers (T*) are valid in Metal
///      and are not lowered.
///
///   2. Direct pointer element types in StorageBuffer
///      (e.g. RWStructuredBuffer<T*>): the element type itself is lowered
///      to UIntPtr since the buffer is already a device pointer, making any
///      pointer element a pointer-to-pointer.
///
/// This pass runs very late in the pipeline (after simplifyNonSSAIR) so
/// that all intermediate passes see real pointer types. It uses primitive
/// CastIntToPtr/CastPtrToInt instructions, not synthesized functions, so
/// it has no dependency on performForceInlining.
void lowerMetalBufferPointerTypes(IRModule* module);

} // namespace Slang
