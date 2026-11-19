// slang-ir-lower-cpu-resource-types.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
struct IRModule;
struct CodeGenContext;

/// Lowers resource types (buffers, textures, acceleration structures) to
/// concrete types on CPU targets. Unlike GPU targets, CPU targets don't have
/// any built-in "Textures" or "StructuredBuffers", so they must be turned into
/// something concrete, such as pointers.
///
/// With `onlyUnsizedBuffers`, lower only unsized buffer types and their operations,
/// retaining the sized resource wrappers used by C++ and CUDA.
void lowerCPUResourceTypes(
    IRModule* module,
    CodeGenContext* codeGenContext,
    bool onlyUnsizedBuffers = false);

/// Map unsized buffers to the existing resource types on targets where both have
/// the same representation. For example, HLSL uses `StructuredBuffer<T>` for both
/// forms, while Metal uses a device pointer for both. Sized-to-unsized conversion
/// is an identity operation on these targets.
void lowerUnsizedBufferTypesToResources(IRModule* module);

} // namespace Slang
