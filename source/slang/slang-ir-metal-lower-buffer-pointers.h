#pragma once

namespace Slang
{

struct IRModule;

/// Lower pointer-typed fields in Metal buffer-bound structs to UIntPtr with
/// CastIntToPtr/CastPtrToInt at access sites.
///
/// Metal rejects pointer-to-pointer types (`device T* device*`) in buffer
/// pointee types. This pass rewrites struct fields that would produce such
/// illegal patterns:
///   - In StorageBuffer element types: all pointer fields (the buffer is
///     already a device pointer, so any pointer element creates ptr-to-ptr).
///   - In ConstantBuffer element types: only multi-level pointer fields
///     (T**, T***). Single-level pointers (T*) are valid in Metal buffer
///     structs.
///
/// This pass runs very late in the pipeline (after simplifyNonSSAIR) so
/// that all intermediate passes see real pointer types. It uses primitive
/// CastIntToPtr/CastPtrToInt instructions, not synthesized functions, so
/// it has no dependency on performForceInlining.
void lowerMetalBufferPointerTypes(IRModule* module);

} // namespace Slang
